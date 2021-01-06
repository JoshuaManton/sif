#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>

#include "parser.h"
#include "os_windows.h"

Array<Expr_Identifier *>              g_identifiers_to_resolve;
Array<Declaration *>                  g_all_declarations;
Array<Ast_Directive_Assert *>         g_all_assert_directives;
Array<Ast_Directive_Print *>          g_all_print_directives;
Array<Ast_Directive_C_Code *>         g_all_c_code_directives;
Array<Ast_Directive_Foreign_Import *> g_all_foreign_import_directives;

Array<char *> g_all_included_files; // note(josh): for deduplicating #includes

Ast_Proc_Header *g_currently_parsing_proc;

void init_parser() {
    g_identifiers_to_resolve.allocator        = g_global_linear_allocator;
    g_all_declarations.allocator              = g_global_linear_allocator;
    g_all_assert_directives.allocator         = g_global_linear_allocator;
    g_all_print_directives.allocator          = g_global_linear_allocator;
    g_all_c_code_directives.allocator         = g_global_linear_allocator;
    g_all_foreign_import_directives.allocator = g_global_linear_allocator;
    g_all_included_files.allocator            = g_global_linear_allocator;
}

Ast_Block *push_ast_block(Ast_Block *block) {
    Ast_Block *old_block = current_block;
    current_block = block;
    return old_block;
}

void pop_ast_block(Ast_Block *old_block) {
    current_block = old_block;
}

bool register_declaration(Ast_Block *block, Declaration *new_declaration) {
    assert(new_declaration->parent_block != nullptr);
    Declaration **existing_declaration = block->declarations_lookup.get(new_declaration->name);
    if (existing_declaration) {
        Declaration *declaration = *existing_declaration;
        report_error(new_declaration->location, "Name collision with '%s'.", new_declaration->name);
        report_info(declaration->location, "Here is the other declaration.");
        return false;
    }
    block->declarations.append(new_declaration);
    block->declarations_lookup.insert(new_declaration->name, new_declaration);
    g_all_declarations.append(new_declaration);
    return true;
}



#define EXPECT(_lexer, _token_kind, _token_ptr) if (!expect_token(_lexer, _token_kind, _token_ptr)) { return nullptr; }



Array<char *> parse_notes(Lexer *lexer) {
    Array<char *> notes = {};
    notes.allocator = g_global_linear_allocator;
    Token note_token;
    while (peek_next_token(lexer, &note_token) && note_token.kind == TK_NOTE) {
        eat_next_token(lexer);
        assert(note_token.text[0] != '@');
        notes.append(note_token.text);
    }
    return notes;
}



bool did_parse_polymorphic_thing = false;

Ast_Var *parse_var(Lexer *lexer, bool require_var = true) {
    Token root_token;
    if (!peek_next_token(lexer, &root_token)) {
        assert(false && "unexpected EOF");
        return nullptr;
    }
    bool had_var_or_const = true;
    if (root_token.kind != TK_VAR) {
        if (root_token.kind != TK_CONST) {
            if (require_var) {
                report_error(root_token.location, "Unexpected token %s, expected `var` or `const`.", token_string(root_token.kind));
                return nullptr;
            }
            else {
                had_var_or_const = false;
            }
        }
    }
    if (had_var_or_const) {
        eat_next_token(lexer);
    }

    bool is_constant = root_token.kind == TK_CONST;

    did_parse_polymorphic_thing = false;

    Ast_Expr *name_expr = parse_expr(lexer);
    bool is_polymorphic_value = false;
    char *var_name = nullptr;
    switch (name_expr->expr_kind) {
        case EXPR_IDENTIFIER: {
            Expr_Identifier *ident = (Expr_Identifier *)name_expr;
            var_name = ident->name;
            break;
        }
        case EXPR_POLYMORPHIC_VARIABLE: {
            Expr_Polymorphic_Variable *poly = (Expr_Polymorphic_Variable *)name_expr;
            var_name = poly->ident->name;
            is_polymorphic_value = true;
            break;
        }
        default: {
            report_error(name_expr->location, "Bad variable name."); // todo(josh): @ErrorMessage
            return nullptr;
        }
    }
    assert(var_name != nullptr);

    Token colon;
    if (!peek_next_token(lexer, &colon)) {
        assert(false && "unexpected EOF");
        return nullptr;
    }
    Ast_Expr *type_expr = nullptr;
    if (colon.kind == TK_COLON) {
        eat_next_token(lexer);
        type_expr = parse_expr(lexer);
        if (!type_expr) {
            return nullptr;
        }
    }

    Ast_Expr *expr = nullptr;
    Token assign;
    if (peek_next_token(lexer, &assign) && assign.kind == TK_ASSIGN) {
        eat_next_token(lexer);
        expr = parse_expr(lexer);
        if (!expr) {
            return nullptr;
        }
    }

    if (type_expr == nullptr && expr == nullptr) {
        report_error(root_token.location, "Variable declaration missing both type and expression. Please provide at least one.");
        return nullptr;
    }

    Ast_Var *var = SIF_NEW_CLONE(Ast_Var(var_name, name_expr, type_expr, expr, is_constant, did_parse_polymorphic_thing, root_token.location));
    var->declaration = SIF_NEW_CLONE(Var_Declaration(var, current_block));
    var->is_polymorphic_value = is_polymorphic_value;
    if (!var->is_polymorphic_value) {
        if (!register_declaration(current_block, var->declaration)) {
            return nullptr;
        }
    }
    return var;
}

Ast_Proc_Header *parse_proc_header(Lexer *lexer, char *name_override) {
    Token token;
    if (!peek_next_token(lexer, &token)) {
        assert(false && "unexpected EOF");
        return nullptr;
    }

    Ast_Proc_Header *header = nullptr;
    {
        Location proc_location = token.location;
        Ast_Block *procedure_block = SIF_NEW_CLONE(Ast_Block(token.location));
        Ast_Block *old_block = push_ast_block(procedure_block);
        defer(pop_ast_block(old_block));

        bool is_operator_overload = false;
        Token_Kind operator_to_overload = TK_INVALID;
        char *proc_name = nullptr;
        if (token.kind == TK_PROC) {
            eat_next_token(lexer, &token);
            Token name_token = {};
            if (!peek_next_token(lexer, &name_token)) {
                assert(false && "unexpected EOF");
                return nullptr;
            }
            if (name_token.kind != TK_LEFT_PAREN) {
                EXPECT(lexer, TK_IDENTIFIER, &name_token);
                proc_name = name_token.text;
            }
        }
        else if (token.kind == TK_OPERATOR) {
            is_operator_overload = true;
            eat_next_token(lexer, &token);
            Token operator_token;
            if (!peek_next_token(lexer, &operator_token)) {
                assert(false && "unexpected EOF");
                return nullptr;
            }
            switch (operator_token.kind) {
                case TK_PLUS: {
                    eat_next_token(lexer);
                    break;
                }
                case TK_MINUS: {
                    eat_next_token(lexer);
                    break;
                }
                case TK_MULTIPLY: {
                    eat_next_token(lexer);
                    break;
                }
                case TK_DIVIDE: {
                    eat_next_token(lexer);
                    break;
                }
                case TK_LEFT_SQUARE: {
                    eat_next_token(lexer);
                    EXPECT(lexer, TK_RIGHT_SQUARE, nullptr);
                    break;
                }
                default: {
                    report_error(operator_token.location, "Cannot overload operator '%s'.", token_string(operator_token.kind));
                    return nullptr;
                }
            }
            operator_to_overload = operator_token.kind;
        }
        else {
            assert(false);
        }

        // parameter list
        EXPECT(lexer, TK_LEFT_PAREN, nullptr);
        Array<Ast_Var *> parameters = {};
        parameters.allocator = g_global_linear_allocator;
        bool first = true;
        Array<int> polymorphic_parameter_indices;
        polymorphic_parameter_indices.allocator = g_global_linear_allocator;
        while (peek_next_token(lexer, &token) && token.kind != TK_RIGHT_PAREN) {
            if (!first) {
                EXPECT(lexer, TK_COMMA, nullptr);
            }

            Ast_Var *var = parse_var(lexer, false);
            if (!var) {
                return nullptr;
            }
            if (var->is_polymorphic) {
                polymorphic_parameter_indices.append(parameters.count);
            }

            parameters.append(var);
            first = false;
        }
        EXPECT(lexer, TK_RIGHT_PAREN, nullptr);

        // return type
        Ast_Expr *return_type_expr = {};
        Token colon = {};
        if (!peek_next_token(lexer, &colon)) {
            assert(false && "unexpected EOF");
            return nullptr;
        }
        if (colon.kind == TK_COLON) {
            eat_next_token(lexer);
            assert(lexer->allow_compound_literals);
            lexer->allow_compound_literals = false;
            return_type_expr = parse_expr(lexer);
            lexer->allow_compound_literals = true;
            if (!return_type_expr) {
                return nullptr;
            }
        }

        Token foreign;
        bool is_foreign = false;
        if (!peek_next_token(lexer, &foreign)) {
            assert(false && "unexpected EOF");
            return nullptr;
        }
        if (foreign.kind == TK_DIRECTIVE_FOREIGN) {
            eat_next_token(lexer);
            is_foreign = true;
            EXPECT(lexer, TK_SEMICOLON, nullptr); // note(josh): this isn't really necessary but it looks nicer
        }

        if (name_override != nullptr) {
            proc_name = intern_string(name_override);
        }

        header = SIF_NEW_CLONE(Ast_Proc_Header(proc_name, procedure_block, parameters, return_type_expr, is_foreign, operator_to_overload, polymorphic_parameter_indices, proc_location));
    }

    return header;
}

Ast_Proc *parse_proc(Lexer *lexer, char *name_override) {
    Ast_Proc_Header *header = parse_proc_header(lexer, name_override);
    if (!header) {
        return nullptr;
    }

    Ast_Block *body = nullptr;
    if (!header->is_foreign) {
        EXPECT(lexer, TK_LEFT_CURLY, nullptr);

        Ast_Proc_Header *old_current_proc = g_currently_parsing_proc;
        g_currently_parsing_proc = header;
        Ast_Block *old_block = push_ast_block(header->procedure_block);
        body = parse_block(lexer);
        pop_ast_block(old_block);
        g_currently_parsing_proc = old_current_proc;
        if (body == nullptr) {
            return nullptr;
        }

        EXPECT(lexer, TK_RIGHT_CURLY, nullptr);
    }

    if (header->operator_to_overload == TK_INVALID) {
        assert(header->name != nullptr);
        header->declaration = SIF_NEW_CLONE(Proc_Declaration(header, current_block));
        header->declaration->notes = parse_notes(lexer);
        header->declaration->is_polymorphic = header->is_polymorphic;
        if (!register_declaration(current_block, header->declaration)) {
            return nullptr;
        }
    }
    else {
        assert(header->name == nullptr);
    }

    Ast_Proc *proc = SIF_NEW_CLONE(Ast_Proc(header, body, header->location));
    header->procedure = proc;
    return proc;
}

Ast_Struct *parse_struct_or_union(Lexer *lexer, char *name_override) {
    Token token;
    if (!peek_next_token(lexer, &token)) {
        assert(false && "unexpected EOF");
        return nullptr;
    }
    bool is_union = false;
    if (token.kind == TK_UNION) {
        EXPECT(lexer, TK_UNION, &token);
        is_union = true;
    }
    else {
        EXPECT(lexer, TK_STRUCT, &token);
    }

    Ast_Struct *structure = SIF_NEW_CLONE(Ast_Struct(is_union, token.location));

    bool is_polymorphic = false;
    {
        structure->struct_block = SIF_NEW_CLONE(Ast_Block(token.location));
        Ast_Block *old_block1 = push_ast_block(structure->struct_block);
        defer(pop_ast_block(old_block1));

        // name
        Token name_token;
        if (!peek_next_token(lexer, &name_token)) {
            assert(false && "unexpected EOF");
            return nullptr;
        }
        if (name_token.kind != TK_LEFT_CURLY) {
            EXPECT(lexer, TK_IDENTIFIER, &name_token);
            if (name_override == nullptr) {
                structure->name = name_token.text;
            }
            else {
                structure->name = intern_string(name_override);
            }
        }
        else {
            static int num_anonymous_structs = 0;
            num_anonymous_structs += 1;
            String_Builder name_sb = make_string_builder(g_global_linear_allocator, 128);
            name_sb.printf("Anonymous_Struct_%d", num_anonymous_structs);
            structure->name = name_sb.string();
        }

        // polymorphic parameters
        Token maybe_poly;
        if (!peek_next_token(lexer, &maybe_poly)) {
            assert(false && "unexpected EOF");
            return nullptr;
        }
        if (maybe_poly.kind == TK_NOT) {
            is_polymorphic = true;
            eat_next_token(lexer);
            EXPECT(lexer, TK_LEFT_PAREN, nullptr);
            bool first = true;
            while (peek_next_token(lexer, &token) && token.kind != TK_RIGHT_PAREN) {
                if (!first) {
                    EXPECT(lexer, TK_COMMA, nullptr);
                }

                Ast_Var *var = parse_var(lexer, false);
                if (!var) {
                    return nullptr;
                }
                structure->polymorphic_parameters.append(var);
                first = false;
            }
            EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
        }

        // fields
        EXPECT(lexer, TK_LEFT_CURLY, nullptr);

        structure->body = parse_block(lexer);
        if (!structure->body) {
            return nullptr;
        }
        EXPECT(lexer, TK_RIGHT_CURLY, nullptr);
        For (idx, structure->body->nodes) {
            Ast_Node *node = structure->body->nodes[idx];
            switch (node->ast_kind) {
                case AST_VAR: {
                    Ast_Var *var = (Ast_Var *)node;
                    structure->fields.append(var);
                    var->belongs_to_struct = structure;
                    break;
                }
                case AST_PROC: {
                    Ast_Proc *proc = (Ast_Proc *)node;
                    if (proc->header->operator_to_overload != TK_INVALID) {
                        assert(proc->header->name == nullptr);
                        proc->header->struct_to_operator_overload = structure;
                        structure->operator_overloads.append(proc);
                        break;
                    }
                    else {
                        assert(proc->header->name != nullptr);
                        break;
                    }
                }
                default: {
                    report_error(node->location, "Only variables, constants, and procedures are allowed in structs.");
                    return nullptr;
                }
            }
        }
    }

    structure->declaration = SIF_NEW_CLONE(Struct_Declaration(structure, current_block));
    structure->declaration->notes = parse_notes(lexer);
    structure->declaration->is_polymorphic = is_polymorphic;
    if (!register_declaration(current_block, structure->declaration)) {
        return nullptr;
    }
    return structure;
}

Ast_Enum *parse_enum(Lexer *lexer) {
    bool old_allow_compound_literals = lexer->allow_compound_literals;
    lexer->allow_compound_literals = false;
    defer(lexer->allow_compound_literals = old_allow_compound_literals);

    Token root_token;
    EXPECT(lexer, TK_ENUM, &root_token);

    Token name_token;
    EXPECT(lexer, TK_IDENTIFIER, &name_token);

    Ast_Enum *ast_enum = SIF_NEW_CLONE(Ast_Enum(name_token.text, root_token.location));

    Array<Enum_Field> enum_fields = {};
    enum_fields.allocator = g_global_linear_allocator;

    Token maybe_type;
    if (!peek_next_token(lexer, &maybe_type)) {
        assert(false && "unexpected EOF");
        return nullptr;
    }
    if (maybe_type.kind != TK_LEFT_CURLY) {
        ast_enum->base_type_expr = parse_expr(lexer);
    }

    EXPECT(lexer, TK_LEFT_CURLY, nullptr);
    {
        ast_enum->enum_block = SIF_NEW_CLONE(Ast_Block(lexer->location));
        Ast_Block *old_block = push_ast_block(ast_enum->enum_block);
        defer(pop_ast_block(old_block));

        Token right_curly;
        while (peek_next_token(lexer, &right_curly) && right_curly.kind != TK_RIGHT_CURLY) {
            Token ident_token;
            EXPECT(lexer, TK_IDENTIFIER, &ident_token);

            Token maybe_equals;
            if (!peek_next_token(lexer, &maybe_equals)) {
                assert(false && "unexpected EOF");
                return nullptr;
            }

            Ast_Expr *expr = nullptr;
            if (maybe_equals.kind == TK_ASSIGN) {
                eat_next_token(lexer);
                expr = parse_expr(lexer);
            }
            EXPECT(lexer, TK_SEMICOLON, nullptr);

            Enum_Field field = {};
            field.name = ident_token.text;
            field.expr = expr;
            field.location = ident_token.location;
            enum_fields.append(field);
        }
    }
    EXPECT(lexer, TK_RIGHT_CURLY, nullptr);

    ast_enum->fields = enum_fields;
    ast_enum->declaration = SIF_NEW_CLONE(Enum_Declaration(ast_enum, current_block));
    ast_enum->declaration->notes = parse_notes(lexer);
    if (!register_declaration(current_block, ast_enum->declaration)) {
        return nullptr;
    }
    return ast_enum;
}

Ast_Block *parse_block_including_curly_brackets(Lexer *lexer) {
    Token open_curly;
    if (!peek_next_token(lexer, &open_curly)) {
        assert(false && "unexpected EOF");
        return nullptr;
    }
    bool only_parse_one_statement = true;
    if (open_curly.kind == TK_LEFT_CURLY) {
        eat_next_token(lexer);
        only_parse_one_statement = false;
    }
    Ast_Block *body = parse_block(lexer, only_parse_one_statement);
    if (!only_parse_one_statement) {
        EXPECT(lexer, TK_RIGHT_CURLY, nullptr);
    }
    return body;
}

extern char *sif_core_lib_path;

Ast_Node *parse_single_statement(Lexer *lexer, bool eat_semicolon, char *name_override) {
    Token root_token;
    if (!peek_next_token(lexer, &root_token)) {
        assert(false && "unexpected EOF");
        return nullptr;
    }

    switch (root_token.kind) {
        case TK_VAR: {
            Ast_Var *var = parse_var(lexer, true);
            if (!var) {
                return nullptr;
            }
            if (eat_semicolon) {
                EXPECT(lexer, TK_SEMICOLON, nullptr);
            }
            var->declaration->notes = parse_notes(lexer);
            return var;
        }

        case TK_CONST: {
            Ast_Var *var = parse_var(lexer, true);
            if (!var) {
                return nullptr;
            }
            if (eat_semicolon) {
                EXPECT(lexer, TK_SEMICOLON, nullptr);
            }
            return var;
        }

        case TK_USING: {
            Token using_token;
            eat_next_token(lexer, &using_token);
            Token maybe_var;
            if (!peek_next_token(lexer, &maybe_var)) {
                assert(false && "unexpected EOF");
                return nullptr;
            }

            Ast_Var *var = nullptr;
            Ast_Expr *expr = nullptr;
            if (maybe_var.kind == TK_VAR) {
                var = parse_var(lexer, false);
                var->is_using = true;
            }
            else {
                expr = parse_expr(lexer);
            }
            if (eat_semicolon) {
                EXPECT(lexer, TK_SEMICOLON, nullptr);
            }

            if (var) {
                return var;
            }
            return SIF_NEW_CLONE(Ast_Using(expr, using_token.location));
        }

        case TK_PROC: {
            Ast_Proc *proc = parse_proc(lexer, name_override);
            if (!proc) {
                return nullptr;
            }
            return proc;
        }

        case TK_OPERATOR: {
            Ast_Proc *proc = parse_proc(lexer);
            if (!proc) {
                return nullptr;
            }
            return proc;
        }

        case TK_STRUCT: {
            Ast_Struct *structure = parse_struct_or_union(lexer, name_override);
            if (!structure) {
                return nullptr;
            }
            return structure;
        }

        case TK_UNION: {
            Ast_Struct *structure = parse_struct_or_union(lexer, name_override);
            if (!structure) {
                return nullptr;
            }
            return structure;
        }

        case TK_ENUM: {
            Ast_Enum *ast_enum = parse_enum(lexer);
            if (!ast_enum) {
                return nullptr;
            }
            return ast_enum;
        }

        case TK_LEFT_CURLY: {
            Ast_Block *block = parse_block_including_curly_brackets(lexer);
            if (!block) {
                return nullptr;
            }
            Ast_Block_Statement *stmt = SIF_NEW_CLONE(Ast_Block_Statement(block, block->location));
            return stmt;
        }

        case TK_DIRECTIVE_INCLUDE: {
            eat_next_token(lexer);
            Token filename_token;
            EXPECT(lexer, TK_STRING, &filename_token);
            bool ok = parse_file(filename_token.text, filename_token.location);
            if (!ok) {
                return nullptr;
            }
            Ast_Directive_Include *include = SIF_NEW_CLONE(Ast_Directive_Include(filename_token.text, root_token.location));
            return include;
        }

        case TK_DIRECTIVE_FOREIGN_IMPORT: {
            eat_next_token(lexer);
            // Token import_name_token;
            // EXPECT(lexer, TK_IDENTIFIER, &import_name_token);
            Token path_token;
            EXPECT(lexer, TK_STRING, &path_token);
            Ast_Directive_Foreign_Import *foreign_import = SIF_NEW_CLONE(Ast_Directive_Foreign_Import(nullptr, path_token.text, root_token.location));
            return foreign_import;
        }

        case TK_DIRECTIVE_ASSERT: {
            eat_next_token(lexer);
            EXPECT(lexer, TK_LEFT_PAREN, nullptr);
            Ast_Expr *expr = parse_expr(lexer);
            if (!expr) {
                return nullptr;
            }
            EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
            Ast_Directive_Assert *assert = SIF_NEW_CLONE(Ast_Directive_Assert(expr, root_token.location));
            return assert;
        }

        case TK_DIRECTIVE_PRINT: {
            eat_next_token(lexer);
            EXPECT(lexer, TK_LEFT_PAREN, nullptr);
            Ast_Expr *expr = parse_expr(lexer);
            if (!expr) {
                return nullptr;
            }
            EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
            Ast_Directive_Print *print = SIF_NEW_CLONE(Ast_Directive_Print(expr, root_token.location));
            return print;
        }

        case TK_DIRECTIVE_C_CODE: {
            eat_next_token(lexer);
            Token code;
            EXPECT(lexer, TK_STRING, &code);
            Ast_Directive_C_Code *c_code = SIF_NEW_CLONE(Ast_Directive_C_Code(code.text, root_token.location));
            return c_code;
        }

        case TK_SEMICOLON: {
            if (eat_semicolon) {
                eat_next_token(lexer);
            }
            Ast_Empty_Statement *stmt = SIF_NEW_CLONE(Ast_Empty_Statement(root_token.location));
            return stmt;
        }

        case TK_FOR: {
            eat_next_token(lexer);
            if (g_currently_parsing_proc == nullptr) {
                report_error(root_token.location, "Cannot use `for` at file scope.");
                return nullptr;
            }

            Ast_For_Loop *for_loop = SIF_NEW_CLONE(Ast_For_Loop(root_token.location));
            assert(g_currently_parsing_proc != nullptr);
            Ast_Node *old_loop = g_currently_parsing_proc->current_parsing_loop;
            g_currently_parsing_proc->current_parsing_loop = for_loop;
            defer(g_currently_parsing_proc->current_parsing_loop = old_loop);

            {
                Ast_Block *block = SIF_NEW_CLONE(Ast_Block(lexer->location));
                Ast_Block *old_block = push_ast_block(block);
                defer(pop_ast_block(old_block));

                EXPECT(lexer, TK_LEFT_PAREN, nullptr);
                for_loop->pre = parse_single_statement(lexer, false);
                if (!for_loop->pre) {
                    return nullptr;
                }
                EXPECT(lexer, TK_SEMICOLON, nullptr);
                for_loop->condition = parse_expr(lexer);
                if (!for_loop->condition) {
                    return nullptr;
                }
                EXPECT(lexer, TK_SEMICOLON, nullptr);
                for_loop->post = parse_single_statement(lexer, false);
                if (!for_loop->post) {
                    return nullptr;
                }
                EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
                for_loop->body = parse_block_including_curly_brackets(lexer);
                if (!for_loop->body) {
                    return nullptr;
                }
            }
            return for_loop;
        }

        case TK_WHILE: {
            eat_next_token(lexer);
            if (g_currently_parsing_proc == nullptr) {
                report_error(root_token.location, "Cannot use `while` at file scope.");
                return nullptr;
            }
            Ast_While_Loop *while_loop = SIF_NEW_CLONE(Ast_While_Loop(root_token.location));
            Ast_Node *old_loop = g_currently_parsing_proc->current_parsing_loop;
            g_currently_parsing_proc->current_parsing_loop = while_loop;
            defer(g_currently_parsing_proc->current_parsing_loop = old_loop);

            EXPECT(lexer, TK_LEFT_PAREN, nullptr);
            while_loop->condition = parse_expr(lexer);
            if (!while_loop->condition) {
                return nullptr;
            }

            EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
            while_loop->body = parse_block_including_curly_brackets(lexer);
            if (!while_loop->body) {
                return nullptr;
            }
            return while_loop;
        }

        case TK_IF: {
            eat_next_token(lexer);
            if (g_currently_parsing_proc == nullptr) {
                report_error(root_token.location, "Cannot use `if` at file scope.");
                return nullptr;
            }
            EXPECT(lexer, TK_LEFT_PAREN, nullptr);
            Ast_Expr *condition = parse_expr(lexer);
            if (!condition) {
                return nullptr;
            }
            EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
            Ast_Block *body = parse_block_including_curly_brackets(lexer);
            if (body == nullptr) {
                return nullptr;
            }
            Token else_token;
            if (!peek_next_token(lexer, &else_token)) {
                assert(false && "unexpected EOF");
                return nullptr;
            }
            Ast_Block *else_body = nullptr;
            if (else_token.kind == TK_ELSE) {
                eat_next_token(lexer);
                else_body = parse_block_including_curly_brackets(lexer);
            }
            Ast_If *ast_if = SIF_NEW_CLONE(Ast_If(condition, body, else_body, root_token.location));
            return ast_if;
        }

        case TK_RETURN: {
            eat_next_token(lexer);
            if (g_currently_parsing_proc == nullptr) {
                report_error(root_token.location, "Cannot use `return` at file scope.");
                return nullptr;
            }
            Token semicolon;
            if (!peek_next_token(lexer, &semicolon)) {
                assert(false && "unexpected EOF");
                return nullptr;
            }
            Ast_Expr *return_expr = nullptr;
            if (semicolon.kind == TK_SEMICOLON) {
                eat_next_token(lexer);
            }
            else {
                return_expr = parse_expr(lexer);
                if (!return_expr) {
                    return nullptr;
                }
                EXPECT(lexer, TK_SEMICOLON, nullptr);
            }
            assert(g_currently_parsing_proc != nullptr);
            return SIF_NEW_CLONE(Ast_Return(g_currently_parsing_proc, return_expr, root_token.location));
        }

        case TK_CONTINUE: {
            eat_next_token(lexer);
            if (g_currently_parsing_proc == nullptr) {
                report_error(root_token.location, "Cannot use `continue` at file scope.");
                return nullptr;
            }
            if (g_currently_parsing_proc->current_parsing_loop == nullptr) {
                report_error(root_token.location, "`continue` must be used from within a loop.");
                return nullptr;
            }
            EXPECT(lexer, TK_SEMICOLON, nullptr);
            return SIF_NEW_CLONE(Ast_Continue(g_currently_parsing_proc->current_parsing_loop, root_token.location));
        }

        case TK_BREAK: {
            eat_next_token(lexer);
            if (g_currently_parsing_proc == nullptr) {
                report_error(root_token.location, "Cannot use `break` at file scope.");
                return nullptr;
            }
            if (g_currently_parsing_proc->current_parsing_loop == nullptr) {
                report_error(root_token.location, "`break` must be used from within a loop.");
                return nullptr;
            }
            EXPECT(lexer, TK_SEMICOLON, nullptr);
            return SIF_NEW_CLONE(Ast_Break(g_currently_parsing_proc->current_parsing_loop, root_token.location));
        }

        default: {
            Ast_Expr *expr = parse_expr(lexer);
            if (expr == nullptr) {
                return nullptr;
            }

            Token next_token;
            if (!peek_next_token(lexer, &next_token)) {
                assert(false && "unexpected EOF");
                return nullptr;
            }
            switch (next_token.kind) {
                case TK_SEMICOLON: {
                    if (eat_semicolon) {
                        eat_next_token(lexer);
                    }
                    return SIF_NEW_CLONE(Ast_Statement_Expr(expr, expr->location));
                }

                case TK_PLUS_ASSIGN:     // fallthrough
                case TK_MINUS_ASSIGN:    // fallthrough
                case TK_MULTIPLY_ASSIGN: // fallthrough
                case TK_DIVIDE_ASSIGN:   // fallthrough
                case TK_ASSIGN: { // todo(josh): <<=, &&=, etc
                    Token op;
                    assert(get_next_token(lexer, &op));
                    Ast_Expr *rhs = parse_expr(lexer);
                    if (rhs == nullptr) {
                        return nullptr;
                    }
                    if (eat_semicolon) {
                        EXPECT(lexer, TK_SEMICOLON, nullptr);
                    }
                    return SIF_NEW_CLONE(Ast_Assign(op.kind, expr, rhs, expr->location));
                }
                default: {
                    unexpected_token(lexer, next_token);
                    return nullptr;
                }
            }
        }
    }
    assert(false && "unreachable");
    return nullptr;
}

Ast_Block *parse_block(Lexer *lexer, bool only_parse_one_statement, bool push_new_block) {
    Ast_Block *old_block = nullptr;
    if (push_new_block) {
        Ast_Block *block = SIF_NEW_CLONE(Ast_Block(lexer->location));
        old_block = push_ast_block(block);
    }
    defer(if (push_new_block) {
        pop_ast_block(old_block);
    });

    Token root_token;
    while (peek_next_token(lexer, &root_token) && root_token.kind != TK_RIGHT_CURLY) {
        Ast_Node *node = parse_single_statement(lexer);
        if (node == nullptr) {
            return nullptr;
        }
        switch (node->ast_kind) {
            case AST_DIRECTIVE_ASSERT: {
                Ast_Directive_Assert *directive = (Ast_Directive_Assert *)node;
                g_all_assert_directives.append(directive);
                break;
            }
            case AST_DIRECTIVE_PRINT: {
                Ast_Directive_Print *directive = (Ast_Directive_Print *)node;
                g_all_print_directives.append(directive);
                break;
            }
            case AST_DIRECTIVE_C_CODE: {
                Ast_Directive_C_Code *directive = (Ast_Directive_C_Code *)node;
                g_all_c_code_directives.append(directive);
                break;
            }
            case AST_DIRECTIVE_FOREIGN_IMPORT: {
                Ast_Directive_Foreign_Import *directive = (Ast_Directive_Foreign_Import *)node;
                g_all_foreign_import_directives.append(directive);
                break;
            }
            case AST_EMPTY_STATEMENT: {
                // note(josh): do nothing
                break;
            }
            default: {
                current_block->nodes.append(node);
            }
        }

        if (only_parse_one_statement) {
            break;
        }
    }
    return current_block;
}

bool parse_file(const char *filename, Location include_location) {
    String_Builder include_sb = make_string_builder(g_global_linear_allocator, 128);
    if (starts_with(filename, "core:")) {
        filename = filename + 5;
        include_sb.printf("%s/%s", sif_core_lib_path, filename);
        filename = include_sb.string();
    }

    char *absolute_path = intern_string(get_absolute_path(filename, g_global_linear_allocator));

    For (idx, g_all_included_files) {
        if (g_all_included_files[idx] == absolute_path) {
            // we've already included this file, no need to do it again
            return true;
        }
    }

    g_all_included_files.append(absolute_path);

    int len = 0;
    char *root_file_text = read_entire_file(filename, &len);
    if (root_file_text == nullptr) {
        report_error(include_location, "Couldn't find file '%s'.", filename);
        return false;
    }
    int length = strlen(filename);
    char *clean_filename = (char *)alloc(g_global_linear_allocator, length+1);
    memcpy(clean_filename, filename, length);
    for (char *c = clean_filename; *c != '\0'; c++) {
        if (*c == '\\') {
            *c = './';
        }
    }
    Lexer lexer(clean_filename, root_file_text);
    Ast_Block *block = parse_block(&lexer, false, false);
    if (!block) {
        return false;
    }
    return true;
}

Ast_Block *begin_parsing(const char *filename) {
    assert(current_block == nullptr);
    Ast_Block *global_scope = SIF_NEW_CLONE(Ast_Block({}));
    global_scope->flags |= BF_IS_GLOBAL_SCOPE;
    Ast_Block *old_block = push_ast_block(global_scope);

    bool ok = parse_file("core:runtime.sif", {});
    assert(ok);

    ok = parse_file(filename, {});

    pop_ast_block(old_block);
    if (!ok) {
        return nullptr;
    }
    return global_scope;
}



bool is_or_op(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token)) {
        assert(false && "unexpected EOF");
        return false;
    }
    switch (token.kind) {
        case TK_BOOLEAN_OR: {
            return true;
        }
    }
    return false;
}

bool is_and_op(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token)) {
        assert(false && "unexpected EOF");
        return false;
    }
    return is_and_op(token.kind);
}
bool is_and_op(Token_Kind kind) {
    switch (kind) {
        case TK_BOOLEAN_AND: {
            return true;
        }
    }
    return false;
}

bool is_cmp_op(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token)) {
        assert(false && "unexpected EOF");
        return false;
    }
    return is_cmp_op(token.kind);
}
bool is_cmp_op(Token_Kind kind) {
    switch (kind) {
        case TK_LESS_THAN:
        case TK_LESS_THAN_OR_EQUAL:
        case TK_GREATER_THAN:
        case TK_GREATER_THAN_OR_EQUAL:
        case TK_NOT_EQUAL_TO:
        case TK_EQUAL_TO: {
            return true;
        }
    }
    return false;
}

bool is_add_op(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token)) {
        assert(false && "unexpected EOF");
        return false;
    }
    return is_add_op(token.kind);
}
bool is_add_op(Token_Kind kind) {
    switch (kind) {
        case TK_BIT_OR: // todo(josh): is this a good precedence for this?
        case TK_PLUS:
        case TK_MINUS: {
            return true;
        }
    }
    return false;
}

bool is_mul_op(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token)) {
        assert(false && "unexpected EOF");
        return false;
    }
    return is_mul_op(token.kind);
}
bool is_mul_op(Token_Kind kind) {
    switch (kind) {
        case TK_MULTIPLY:
        case TK_DIVIDE:
        case TK_AMPERSAND: // bitwise AND
        case TK_LEFT_SHIFT:
        case TK_RIGHT_SHIFT: {
            return true;
        }
    }
    return false;
}

bool is_unary_op(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token)) {
        assert(false && "unexpected EOF");
        return false;
    }
    return is_unary_op(token.kind);
}
bool is_unary_op(Token_Kind kind) {
    switch (kind) {
        case TK_MINUS:
        case TK_PLUS:
        case TK_NOT:
        case TK_BIT_NOT:
        case TK_DOT_DOT:
        case TK_SIZEOF:
        case TK_TYPEOF:
        case TK_AMPERSAND: { // address-of
            return true;
        }
    }
    return false;
}

bool is_postfix_op(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token)) {
        assert(false && "unexpected EOF");
        return false;
    }
    return is_postfix_op(token.kind);
}
bool is_postfix_op(Token_Kind kind) {
    switch (kind) {
        case TK_LEFT_PAREN:
        case TK_LEFT_SQUARE:
        case TK_LEFT_CURLY:
        case TK_DOT:
        case TK_NOT:
        case TK_CARET: { // dereference
            return true;
        }
    }
    return false;
}


Ast_Expr *parse_expr(Lexer *lexer);
Ast_Expr *parse_or_expr(Lexer *lexer);
Ast_Expr *parse_and_expr(Lexer *lexer);
Ast_Expr *parse_cmp_expr(Lexer *lexer);
Ast_Expr *parse_add_expr(Lexer *lexer);
Ast_Expr *parse_mul_expr(Lexer *lexer);
Ast_Expr *parse_unary_expr(Lexer *lexer);
Ast_Expr *parse_postfix_expr(Lexer *lexer);
Ast_Expr *parse_base_expr(Lexer *lexer);

Ast_Expr *parse_expr(Lexer *lexer) {
    return parse_or_expr(lexer);
}

Ast_Expr *parse_or_expr(Lexer *lexer) {
    Ast_Expr *expr = parse_and_expr(lexer);
    if (!expr) {
        return nullptr;
    }
    while (is_or_op(lexer)) {
        Token op = {};
        if (!get_next_token(lexer, &op)) {
            return nullptr;
        }
        Ast_Expr *lhs = expr;
        Ast_Expr *rhs = parse_and_expr(lexer);
        if (!rhs) {
            return nullptr;
        }
        expr = SIF_NEW_CLONE(Expr_Binary(op.kind, lhs, rhs));
    }
    return expr;
}

Ast_Expr *parse_and_expr(Lexer *lexer) {
    Ast_Expr *expr = parse_cmp_expr(lexer);
    if (!expr) {
        return nullptr;
    }
    while (is_and_op(lexer)) {
        Token op = {};
        if (!get_next_token(lexer, &op)) {
            return nullptr;
        }
        Ast_Expr *lhs = expr;
        Ast_Expr *rhs = parse_cmp_expr(lexer);
        if (!rhs) {
            return nullptr;
        }
        expr = SIF_NEW_CLONE(Expr_Binary(op.kind, lhs, rhs));
    }
    return expr;
}

Ast_Expr *parse_cmp_expr(Lexer *lexer) {
    Ast_Expr *expr = parse_add_expr(lexer);
    if (!expr) {
        return nullptr;
    }
    while (is_cmp_op(lexer)) {
        Token op = {};
        if (!get_next_token(lexer, &op)) {
            return nullptr;
        }
        Ast_Expr *lhs = expr;
        Ast_Expr *rhs = parse_add_expr(lexer);
        if (!rhs) {
            return nullptr;
        }
        expr = SIF_NEW_CLONE(Expr_Binary(op.kind, lhs, rhs));
    }
    return expr;
}

Ast_Expr *parse_add_expr(Lexer *lexer) {
    Ast_Expr *expr = parse_mul_expr(lexer);
    if (!expr) {
        return nullptr;
    }
    while (is_add_op(lexer)) {
        Token op = {};
        if (!get_next_token(lexer, &op)) {
            return nullptr;
        }
        Ast_Expr *lhs = expr;
        Ast_Expr *rhs = parse_mul_expr(lexer);
        if (!rhs) {
            return nullptr;
        }
        expr = SIF_NEW_CLONE(Expr_Binary(op.kind, lhs, rhs));
    }
    return expr;
}

Ast_Expr *parse_mul_expr(Lexer *lexer) {
    Ast_Expr *expr = parse_unary_expr(lexer);
    if (!expr) {
        return nullptr;
    }
    while (is_mul_op(lexer)) {
        Token op = {};
        if (!get_next_token(lexer, &op)) {
            return nullptr;
        }
        Ast_Expr *lhs = expr;
        Ast_Expr *rhs = parse_unary_expr(lexer);
        if (!rhs) {
            return nullptr;
        }
        expr = SIF_NEW_CLONE(Expr_Binary(op.kind, lhs, rhs));
    }
    return expr;
}

Ast_Expr *parse_unary_expr(Lexer *lexer) {
    while (is_unary_op(lexer)) {
        Token op = {};
        if (!get_next_token(lexer, &op)) {
            return nullptr;
        }

        switch (op.kind) {
            case TK_AMPERSAND: {
                Ast_Expr *rhs = parse_unary_expr(lexer);
                if (!rhs) {
                    return nullptr;
                }
                return SIF_NEW_CLONE(Expr_Address_Of(rhs, op.location));
            }
            case TK_MINUS:
            case TK_PLUS:
            case TK_BIT_NOT:
            case TK_NOT: {
                Ast_Expr *rhs = parse_unary_expr(lexer);
                if (!rhs) {
                    return nullptr;
                }
                return SIF_NEW_CLONE(Expr_Unary(op.kind, rhs, op.location));
            }
            case TK_DOT_DOT: {
                Ast_Expr *rhs = parse_postfix_expr(lexer);
                if (!rhs) {
                    return nullptr;
                }
                return SIF_NEW_CLONE(Expr_Spread(rhs, op.location));
            }
            case TK_SIZEOF: {
                EXPECT(lexer, TK_LEFT_PAREN, nullptr);
                Ast_Expr *expr = parse_expr(lexer);
                if (!expr) {
                    return nullptr;
                }
                EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
                return SIF_NEW_CLONE(Expr_Sizeof(expr, op.location));
            }
            case TK_TYPEOF: {
                EXPECT(lexer, TK_LEFT_PAREN, nullptr);
                Ast_Expr *expr = parse_expr(lexer);
                if (!expr) {
                    return nullptr;
                }
                EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
                return SIF_NEW_CLONE(Expr_Typeof(expr, op.location));
            }
            default: {
                assert(false);
            }
        }
    }

    return parse_postfix_expr(lexer);
}

Ast_Expr *parse_postfix_expr(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token)) {
        assert(false && "unexpected EOF");
        return nullptr;
    }

    Ast_Expr *base_expr = parse_base_expr(lexer);
    if (!base_expr) {
        return nullptr;
    }

    while (is_postfix_op(lexer)) {
        Token op = {};
        assert(peek_next_token(lexer, &op));

        switch (op.kind) {
            case TK_LEFT_PAREN: {
                eat_next_token(lexer);
                Array<Ast_Expr *> parameters = {};
                parameters.allocator = g_global_linear_allocator;
                bool first = true;
                Token token;
                while (peek_next_token(lexer, &token) && token.kind != TK_RIGHT_PAREN) {
                    if (!first) {
                        EXPECT(lexer, TK_COMMA, nullptr);
                    }
                    Ast_Expr *expr = parse_expr(lexer);
                    if (!expr) {
                        return nullptr;
                    }
                    parameters.append(expr);
                    first = false;
                }
                EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
                base_expr = SIF_NEW_CLONE(Expr_Procedure_Call(base_expr, parameters, base_expr->location));
                break;
            }
            case TK_LEFT_CURLY: {
                if (!lexer->allow_compound_literals) {
                    return base_expr; // todo(josh): this feels a bit sketchy, maybe it's fine
                }
                eat_next_token(lexer);
                Array<Ast_Expr *> exprs = {};
                exprs.allocator = g_global_linear_allocator;
                bool first = true;
                Token token;
                while (peek_next_token(lexer, &token) && token.kind != TK_RIGHT_CURLY) {
                    if (!first) {
                        EXPECT(lexer, TK_COMMA, nullptr);
                    }
                    Ast_Expr *expr = parse_expr(lexer);
                    if (!expr) {
                        return nullptr;
                    }
                    exprs.append(expr);
                    first = false;
                }
                EXPECT(lexer, TK_RIGHT_CURLY, nullptr);
                assert(base_expr != nullptr);
                base_expr = SIF_NEW_CLONE(Expr_Compound_Literal(base_expr, exprs, base_expr->location));
                break;
            }
            case TK_LEFT_SQUARE: {
                eat_next_token(lexer);
                Ast_Expr *index = parse_expr(lexer);
                if (!index) {
                    return nullptr;
                }
                EXPECT(lexer, TK_RIGHT_SQUARE, nullptr);

                base_expr = SIF_NEW_CLONE(Expr_Subscript(base_expr, index, base_expr->location));
                break;
            }
            case TK_CARET: {
                eat_next_token(lexer);
                base_expr = SIF_NEW_CLONE(Expr_Dereference(base_expr, base_expr->location));
                break;
            }
            case TK_NOT: {
                eat_next_token(lexer);
                EXPECT(lexer, TK_LEFT_PAREN, nullptr);
                Array<Ast_Expr *> parameters;
                parameters.allocator = g_global_linear_allocator;
                bool first = true;
                while (peek_next_token(lexer, &token) && token.kind != TK_RIGHT_PAREN) {
                    if (!first) {
                        EXPECT(lexer, TK_COMMA, nullptr);
                    }

                    Ast_Expr *expr = parse_expr(lexer);
                    if (!expr) {
                        return nullptr;
                    }
                    parameters.append(expr);
                    first = false;
                }
                EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
                base_expr = SIF_NEW_CLONE(Expr_Polymorphic_Type(base_expr, parameters, base_expr->location));
                break;
            }
            case TK_DOT: {
                eat_next_token(lexer);
                Token name_token;
                EXPECT(lexer, TK_IDENTIFIER, &name_token);
                base_expr = SIF_NEW_CLONE(Expr_Selector(base_expr, name_token.text, base_expr->location));
                break;
            }
            default: {
                unexpected_token(lexer, op);
                return nullptr;
            }
        }
    }
    // hey future Josh if you add code down here be sure to check the
    // early-out in the TK_LEFT_CURLY for handling compound literals
    return base_expr;
}

Ast_Expr *parse_base_expr(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token)) {
        assert(false && "unexpected EOF");
        return nullptr;
    }

    switch (token.kind) {
        case TK_NULL: {
            eat_next_token(lexer);
            return SIF_NEW_CLONE(Expr_Null(token.location));
        }
        case TK_TRUE: {
            eat_next_token(lexer);
            return SIF_NEW_CLONE(Expr_True(token.location));
        }
        case TK_FALSE: {
            eat_next_token(lexer);
            return SIF_NEW_CLONE(Expr_False(token.location));
        }
        case TK_IDENTIFIER: {
            eat_next_token(lexer);
            char *ident_name = token.text;
            Expr_Identifier *ident = SIF_NEW_CLONE(Expr_Identifier(ident_name, token.location));
            g_identifiers_to_resolve.append(ident);
            return ident;
        }
        case TK_NUMBER: {
            eat_next_token(lexer);
            return SIF_NEW_CLONE(Expr_Number_Literal(token.uint_value, token.int_value, token.float_value, token.has_a_dot, token.location));
        }
        case TK_CHAR: {
            eat_next_token(lexer);
            if (token.escaped_length != 1) {
                printf("%d %d\n", token.escaped_length, token.scanner_length);
                report_error(token.location, "Character literal must be length 1.");
                return nullptr;
            }
            return SIF_NEW_CLONE(Expr_Char_Literal(token.escaped_text[0], token.location));
        }
        case TK_STRING: {
            eat_next_token(lexer);
            return SIF_NEW_CLONE(Expr_String_Literal(token.text, token.scanner_length, token.escaped_text, token.escaped_length, token.location));
        }
        case TK_LEFT_PAREN: {
            eat_next_token(lexer);
            Ast_Expr *nested = parse_expr(lexer);
            EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
            return SIF_NEW_CLONE(Expr_Paren(nested, token.location));
        }
        case TK_PROC: {
            Ast_Proc_Header *header = parse_proc_header(lexer);
            return SIF_NEW_CLONE(Expr_Procedure_Type(header, token.location));
        }
        case TK_STRUCT: // note(josh): fallthrough
        case TK_UNION: {
            Ast_Struct *structure = parse_struct_or_union(lexer);
            return SIF_NEW_CLONE(Expr_Struct_Type(structure, token.location));
        }
        case TK_CAST: {
            eat_next_token(lexer);
            EXPECT(lexer, TK_LEFT_PAREN, nullptr);
            Ast_Expr *type_expr = parse_expr(lexer);
            if (!type_expr) {
                return nullptr;
            }
            EXPECT(lexer, TK_COMMA, nullptr);
            Ast_Expr *rhs = parse_expr(lexer);
            EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
            return SIF_NEW_CLONE(Expr_Cast(type_expr, rhs, token.location));
        }
        case TK_TRANSMUTE: {
            eat_next_token(lexer);
            EXPECT(lexer, TK_LEFT_PAREN, nullptr);
            Ast_Expr *type_expr = parse_expr(lexer);
            if (!type_expr) {
                return nullptr;
            }
            EXPECT(lexer, TK_COMMA, nullptr);
            Ast_Expr *rhs = parse_expr(lexer);
            EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
            return SIF_NEW_CLONE(Expr_Transmute(type_expr, rhs, token.location));
        }
        case TK_DOLLAR: {
            eat_next_token(lexer);
            Ast_Expr *ident_expr = parse_expr(lexer);
            if (ident_expr->expr_kind != EXPR_IDENTIFIER) {
                report_error(ident_expr->location, "Polymorphic variable must be an identifier.");
                return nullptr;
            }
            Expr_Identifier *ident = (Expr_Identifier *)ident_expr;
            did_parse_polymorphic_thing = true;
            Polymorphic_Declaration *poly_decl = SIF_NEW_CLONE(Polymorphic_Declaration(ident->name, nullptr, current_block, token.location));
            if (!register_declaration(current_block, poly_decl)) {
                return nullptr;
            }
            return SIF_NEW_CLONE(Expr_Polymorphic_Variable((Expr_Identifier *)ident, poly_decl, token.location));
        }
        case TK_LEFT_SQUARE: {
            eat_next_token(lexer);
            Token maybe_right_square;
            if (!peek_next_token(lexer, &maybe_right_square)) {
                assert(false && "unexpected EOF");
                return nullptr;
            }
            if (maybe_right_square.kind == TK_RIGHT_SQUARE) {
                // it's a slice
                EXPECT(lexer, TK_RIGHT_SQUARE, nullptr);
                Ast_Expr *slice_of = parse_expr(lexer);
                if (!slice_of) {
                    return nullptr;
                }
                return SIF_NEW_CLONE(Expr_Slice_Type(slice_of, token.location));
            }
            else {
                // it's an array
                Ast_Expr *length = parse_expr(lexer);
                if (!length) {
                    return nullptr;
                }
                EXPECT(lexer, TK_RIGHT_SQUARE, nullptr);
                Ast_Expr *array_of = parse_expr(lexer);
                if (!array_of) {
                    return nullptr;
                }
                return SIF_NEW_CLONE(Expr_Array_Type(array_of, length, token.location));
            }
            assert(false && "unreachable");
        }
        case TK_CARET: {
            eat_next_token(lexer);
            Ast_Expr *pointer_to = parse_expr(lexer);
            if (!pointer_to) {
                return nullptr;
            }
            return SIF_NEW_CLONE(Expr_Pointer_Type(pointer_to, token.location));
        }
        case TK_GREATER_THAN: {
            eat_next_token(lexer);
            Ast_Expr *reference_to = parse_expr(lexer);
            if (!reference_to) {
                return nullptr;
            }
            return SIF_NEW_CLONE(Expr_Reference_Type(reference_to, token.location));
        }
        default: {
            unexpected_token(lexer, token);
            return nullptr;
        }
    }
    assert(false && "unreachable");
    return nullptr;
}
