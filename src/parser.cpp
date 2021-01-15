#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>

#include "parser.h"
#include "os_windows.h"
#include "spinlock.h"

#define NUM_PARSER_THREADS 4

int g_files_done_parsing;

Spinlock g_all_file_blocks_spinlock;
Array<Ast_Block *> g_all_file_blocks;
int g_total_lines_parsed;

Spinlock g_all_foreign_imports_spinlock;
Array<Ast_Directive_Foreign_Import *> g_all_foreign_import_directives;

Spinlock g_lexers_to_process_spinlock;
Array<Lexer> g_lexers_to_process;
Array<char *> g_all_included_files;

// todo(josh): @Multithreading this is a concern
int g_num_anonymous_structs = 0;

// todo(josh): @Multithreading this is a concern
int g_num_anonymous_procedures = 0;

Thread g_parser_threads[NUM_PARSER_THREADS];

u32 parser_worker_thread(void *);

void init_parser() {
    g_all_foreign_import_directives.allocator = g_global_linear_allocator;
    g_all_included_files.allocator = g_global_linear_allocator;
    g_all_file_blocks.allocator = g_global_linear_allocator;
    g_lexers_to_process.allocator = g_global_linear_allocator;
}

Ast_Block *push_ast_block(Lexer *lexer, Ast_Block *block) {
    Ast_Block *old_block = lexer->current_block;
    lexer->current_block = block;
    return old_block;
}

void pop_ast_block(Lexer *lexer, Ast_Block *old_block) {
    lexer->current_block = old_block;
}

bool register_declaration(Ast_Block *block, Declaration *new_declaration) {
    assert(new_declaration->name != nullptr);
    // g_register_declaration_spinlock.lock();
    // defer(g_register_declaration_spinlock.unlock());
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
    return true;
}



#define EXPECT(_lexer, _token_kind, _token_ptr) if (!expect_token(_lexer, _token_kind, _token_ptr)) { return nullptr; }



Array<char *> parse_notes(Lexer *lexer) {
    Array<char *> notes = {};
    notes.allocator = lexer->allocator;
    Token note_token;
    while (peek_next_token(lexer, &note_token) && note_token.kind == TK_NOTE) {
        eat_next_token(lexer);
        assert(note_token.text[0] != '@');
        notes.append(note_token.text);
    }
    return notes;
}

Ast_Expr_List *parse_expr_list(Lexer *lexer) {
    Array<Ast_Expr *> exprs = {};
    exprs.allocator = lexer->allocator;
    Ast_Expr *root_expr = parse_expr(lexer);
    if (!root_expr) {
        return nullptr;
    }
    exprs.append(root_expr);
    Token comma;
    // todo(josh): parameterize delimiter/end token
    while (peek_next_token(lexer, &comma) && comma.kind == TK_COMMA) {
        EXPECT(lexer, TK_COMMA, nullptr);
        Ast_Expr *expr = parse_expr(lexer);
        if (!expr) {
            return nullptr;
        }
        exprs.append(expr);
    }
    assert(exprs.count > 0);
    return SIF_NEW_CLONE(Ast_Expr_List(exprs, lexer->allocator, lexer->current_block, root_expr->location), lexer->allocator);
}



Ast_Var *parse_var(Lexer *lexer, bool require_var = true) {
    Token root_token;
    if (!peek_next_token(lexer, &root_token)) {
        report_error(lexer->location, "Unexpected end of file.");
        return nullptr;
    }
    bool is_using = false;
    if (root_token.kind == TK_USING) {
        eat_next_token(lexer);
        is_using = true;
    }
    Token var_or_const_token;
    if (!peek_next_token(lexer, &var_or_const_token)) {
        report_error(lexer->location, "Unexpected end of file.");
        return nullptr;
    }
    bool had_var_or_const = true;
    if (var_or_const_token.kind != TK_VAR) {
        if (var_or_const_token.kind != TK_CONST) {
            if (require_var) {
                report_error(var_or_const_token.location, "Unexpected token %s, expected `var` or `const`.", token_string(var_or_const_token.kind));
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

    bool is_constant = var_or_const_token.kind == TK_CONST;

    int polymorph_count_before_name = lexer->num_polymorphic_variables_parsed;
    Ast_Expr *name_expr = parse_expr(lexer);
    if (name_expr == nullptr) {
        return nullptr;
    }
    bool is_polymorphic_value = polymorph_count_before_name != lexer->num_polymorphic_variables_parsed;
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
            break;
        }
        default: {
            report_error(name_expr->location, "Expected a variable name."); // todo(josh): @ErrorMessage
            return nullptr;
        }
    }
    assert(var_name != nullptr);

    Token colon;
    if (!peek_next_token(lexer, &colon)) {
        report_error(lexer->location, "Unexpected end of file.");
        return nullptr;
    }
    int polymorph_count_before_type = lexer->num_polymorphic_variables_parsed;
    Ast_Expr *type_expr = nullptr;
    if (colon.kind == TK_COLON) {
        eat_next_token(lexer);
        type_expr = parse_expr(lexer);
        if (!type_expr) {
            return nullptr;
        }
    }
    bool is_polymorphic_type = polymorph_count_before_type != lexer->num_polymorphic_variables_parsed;

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

    Array<char *> notes = parse_notes(lexer);

    Ast_Var *var = SIF_NEW_CLONE(Ast_Var(var_name, name_expr, type_expr, expr, is_constant, is_polymorphic_value, is_polymorphic_type, lexer->allocator, lexer->current_block, root_token.location), lexer->allocator);
    var->is_using = is_using;
    var->declaration = SIF_NEW_CLONE(Var_Declaration(var, lexer->current_block), lexer->allocator);
    var->declaration->notes = notes;
    var->is_polymorphic_value = is_polymorphic_value;
    if (!var->is_polymorphic_value) {
        if (!register_declaration(lexer->current_block, var->declaration)) {
            return nullptr;
        }
    }
    return var;
}

Ast_Proc_Header *parse_proc_header(Lexer *lexer, char *name_override) {
    Token root_token;
    if (!peek_next_token(lexer, &root_token)) {
        report_error(lexer->location, "Unexpected end of file.");
        return nullptr;
    }

    Ast_Proc_Header *header = SIF_NEW_CLONE(Ast_Proc_Header(lexer->current_toplevel_declaration, lexer->allocator, lexer->current_block, root_token.location), lexer->allocator);
    header->declaration = SIF_NEW_CLONE(Proc_Declaration(header, lexer->current_block), lexer->allocator);
    {
        header->procedure_block = SIF_NEW_CLONE(Ast_Block(lexer->allocator, lexer->current_block, root_token.location), lexer->allocator);
        Ast_Block *old_block = push_ast_block(lexer, header->procedure_block);
        defer(pop_ast_block(lexer, old_block));

        bool is_operator_overload = false;
        Token_Kind operator_to_overload = TK_INVALID;
        char *proc_name = nullptr;
        if (root_token.kind == TK_PROC) {
            eat_next_token(lexer, nullptr);
            Token name_token = {};
            if (!peek_next_token(lexer, &name_token)) {
                report_error(lexer->location, "Unexpected end of file.");
                return nullptr;
            }
            if (name_token.kind != TK_LEFT_PAREN) {
                EXPECT(lexer, TK_IDENTIFIER, &name_token);
                proc_name = name_token.text;
            }
            else {
                g_num_anonymous_procedures += 1;
                String_Builder name_sb = make_string_builder(lexer->allocator, 128);
                name_sb.printf("anonymous_procedure_%d", g_num_anonymous_procedures);
                proc_name = name_sb.string();
            }
        }
        else if (root_token.kind == TK_OPERATOR) {
            is_operator_overload = true;
            eat_next_token(lexer, nullptr);
            Token operator_token;
            if (!peek_next_token(lexer, &operator_token)) {
                report_error(lexer->location, "Unexpected end of file.");
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
                case TK_MOD: {
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

        if (name_override != nullptr) {
            proc_name = intern_string(name_override);
        }

        header->name = proc_name;
        header->operator_to_overload = operator_to_overload;

        // parameter list
        EXPECT(lexer, TK_LEFT_PAREN, nullptr);
        bool first = true;
        Token right_paren;
        while (peek_next_token(lexer, &right_paren) && right_paren.kind != TK_RIGHT_PAREN) {
            if (!first) {
                EXPECT(lexer, TK_COMMA, nullptr);
            }

            Ast_Var *var = parse_var(lexer, false);
            if (!var) {
                return nullptr;
            }
            if (var->is_polymorphic_value || var->is_polymorphic_type) {
                header->polymorphic_parameter_indices.append(header->parameters.count);
            }
            var->is_parameter_for_procedure = header;
            header->parameters.append(var);
            first = false;
        }
        EXPECT(lexer, TK_RIGHT_PAREN, nullptr);

        header->is_polymorphic = header->polymorphic_parameter_indices.count > 0;

        // return type
        Token colon = {};
        if (!peek_next_token(lexer, &colon)) {
            report_error(lexer->location, "Unexpected end of file.");
            return nullptr;
        }
        if (colon.kind == TK_COLON) {
            eat_next_token(lexer);
            assert(lexer->allow_compound_literals);
            lexer->allow_compound_literals = false;
            header->return_type_expr = parse_expr(lexer);
            lexer->allow_compound_literals = true;
            if (!header->return_type_expr) {
                return nullptr;
            }
        }

        Token foreign;
        if (!peek_next_token(lexer, &foreign)) {
            report_error(lexer->location, "Unexpected end of file.");
            return nullptr;
        }
        if (foreign.kind == TK_DIRECTIVE_FOREIGN) {
            eat_next_token(lexer);
            header->is_foreign = true;
            EXPECT(lexer, TK_SEMICOLON, nullptr); // note(josh): this isn't really necessary but it looks nicer
        }
    }

    header->declaration->name = header->name;
    header->declaration->is_polymorphic = header->is_polymorphic;
    if (header->declaration->name != nullptr) {
        if (!register_declaration(lexer->current_block, header->declaration)) {
            return nullptr;
        }
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

        assert(header->declaration);
        Declaration *old_toplevel_declaration = lexer->current_toplevel_declaration;
        lexer->current_toplevel_declaration = header->declaration;
        Ast_Block *old_block = push_ast_block(lexer, header->procedure_block);
        body = parse_block(lexer);
        pop_ast_block(lexer, old_block);
        lexer->current_toplevel_declaration = old_toplevel_declaration;
        if (body == nullptr) {
            return nullptr;
        }

        EXPECT(lexer, TK_RIGHT_CURLY, nullptr);
    }

    header->declaration->notes = parse_notes(lexer);

    Ast_Proc *proc = SIF_NEW_CLONE(Ast_Proc(header, body, lexer->allocator, lexer->current_block, header->location), lexer->allocator);
    header->procedure = proc;
    return proc;
}

Ast_Struct *parse_struct_or_union(Lexer *lexer, char *name_override) {
    Token token;
    if (!peek_next_token(lexer, &token)) {
        report_error(lexer->location, "Unexpected end of file.");
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

    Ast_Struct *structure = SIF_NEW_CLONE(Ast_Struct(lexer->current_toplevel_declaration, is_union, lexer->allocator, lexer->current_block, token.location), lexer->allocator);
    structure->declaration = SIF_NEW_CLONE(Struct_Declaration(structure, lexer->current_block), lexer->allocator);
    Declaration *old_toplevel_declaration = lexer->current_toplevel_declaration;
    lexer->current_toplevel_declaration = structure->declaration;
    defer(lexer->current_toplevel_declaration = old_toplevel_declaration);

    bool is_polymorphic = false;
    {
        structure->struct_block = SIF_NEW_CLONE(Ast_Block(lexer->allocator, lexer->current_block, token.location), lexer->allocator);
        Ast_Block *old_block1 = push_ast_block(lexer, structure->struct_block);
        defer(pop_ast_block(lexer, old_block1));

        // name
        Token name_token;
        if (!peek_next_token(lexer, &name_token)) {
            report_error(lexer->location, "Unexpected end of file.");
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
            g_num_anonymous_structs += 1;
            String_Builder name_sb = make_string_builder(lexer->allocator, 128);
            name_sb.printf("Anonymous_Struct_%d", g_num_anonymous_structs);
            structure->name = name_sb.string();
        }

        // polymorphic parameters
        Token maybe_poly;
        if (!peek_next_token(lexer, &maybe_poly)) {
            report_error(lexer->location, "Unexpected end of file.");
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
                        structure->operator_overloads.append(proc);
                        break;
                    }
                    else {
                        assert(proc->header->name != nullptr);
                        structure->procedures.append(proc);
                        break;
                    }
                }
                case AST_STRUCT: {
                    Ast_Struct *local_struct = (Ast_Struct *)node;
                    structure->local_structs.append(local_struct);
                    break;
                }
                case AST_ENUM: {
                    Ast_Enum *local_enum = (Ast_Enum *)node;
                    structure->local_enums.append(local_enum);
                    break;
                }
                default: {
                    report_error(node->location, "Only variables, constants, procedures, structs, and enums are allowed in structs.");
                    return nullptr;
                }
            }
        }
    }
    structure->declaration->name = structure->name;
    structure->declaration->notes = parse_notes(lexer);
    structure->declaration->is_polymorphic = is_polymorphic;
    if (!register_declaration(lexer->current_block, structure->declaration)) {
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

    Ast_Enum *ast_enum = SIF_NEW_CLONE(Ast_Enum(name_token.text, lexer->current_toplevel_declaration, lexer->allocator, lexer->current_block, root_token.location), lexer->allocator);
    ast_enum->declaration = SIF_NEW_CLONE(Enum_Declaration(ast_enum, lexer->current_block), lexer->allocator);
    Declaration *old_toplevel_declaration = lexer->current_toplevel_declaration;
    lexer->current_toplevel_declaration = ast_enum->declaration;
    defer(lexer->current_toplevel_declaration = old_toplevel_declaration);

    Array<Enum_Field> enum_fields = {};
    enum_fields.allocator = lexer->allocator;

    Token maybe_type;
    if (!peek_next_token(lexer, &maybe_type)) {
        report_error(lexer->location, "Unexpected end of file.");
        return nullptr;
    }
    if (maybe_type.kind != TK_LEFT_CURLY) {
        ast_enum->base_type_expr = parse_expr(lexer);
    }

    EXPECT(lexer, TK_LEFT_CURLY, nullptr);
    {
        ast_enum->enum_block = SIF_NEW_CLONE(Ast_Block(lexer->allocator, lexer->current_block, lexer->location), lexer->allocator);
        Ast_Block *old_block = push_ast_block(lexer, ast_enum->enum_block);
        defer(pop_ast_block(lexer, old_block));

        Token right_curly;
        while (peek_next_token(lexer, &right_curly) && right_curly.kind != TK_RIGHT_CURLY) {
            Token ident_token;
            EXPECT(lexer, TK_IDENTIFIER, &ident_token);

            Token maybe_equals;
            if (!peek_next_token(lexer, &maybe_equals)) {
                report_error(lexer->location, "Unexpected end of file.");
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

    ast_enum->declaration->name = ast_enum->name;
    ast_enum->fields = enum_fields;
    ast_enum->declaration->notes = parse_notes(lexer);
    if (!register_declaration(lexer->current_block, ast_enum->declaration)) {
        return nullptr;
    }
    return ast_enum;
}

Ast_Block *parse_block_including_curly_brackets(Lexer *lexer) {
    Token open_curly;
    if (!peek_next_token(lexer, &open_curly)) {
        report_error(lexer->location, "Unexpected end of file.");
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

const char *relative_to_absolute_path(const char *relative_path, const char *relative_to, Allocator allocator) {
    const char *dir_of_relative_to = path_directory(relative_to, allocator);
    String_Builder sb = make_string_builder(allocator, 128);
    sb.printf("%s\\%s", dir_of_relative_to, relative_path);
    return sb.string();
}

Ast_Node *parse_single_statement(Lexer *lexer, bool eat_semicolon, char *name_override) {
    Token root_token;
    if (!peek_next_token(lexer, &root_token)) {
        report_error(lexer->location, "Unexpected end of file.");
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
                report_error(lexer->location, "Unexpected end of file.");
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
            return SIF_NEW_CLONE(Ast_Using(expr, lexer->allocator, lexer->current_block, using_token.location), lexer->allocator);
        }

        case TK_DEFER: {
            Token defer_token;
            eat_next_token(lexer, &defer_token);
            Ast_Node *stmt = parse_single_statement(lexer);
            if (!stmt) {
                return nullptr;
            }
            if (!(lexer->current_toplevel_declaration && lexer->current_toplevel_declaration->kind == DECL_PROC)) {
                report_error(defer_token.location, "defer statements must be inside a procedure.");
                return nullptr;
            }
            return SIF_NEW_CLONE(Ast_Defer(stmt, lexer->allocator, lexer->current_block, defer_token.location), lexer->allocator);
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
            Ast_Block_Statement *stmt = SIF_NEW_CLONE(Ast_Block_Statement(block, lexer->allocator, lexer->current_block, block->location), lexer->allocator);
            return stmt;
        }

        case TK_DIRECTIVE_INCLUDE: {
            eat_next_token(lexer);
            Token filename_token;
            EXPECT(lexer, TK_STRING, &filename_token);
            parse_file(filename_token.text, filename_token.location);
            Ast_Directive_Include *include = SIF_NEW_CLONE(Ast_Directive_Include(filename_token.text, lexer->allocator, lexer->current_block, root_token.location), lexer->allocator);
            return include;
        }

        case TK_DIRECTIVE_FOREIGN_IMPORT: {
            eat_next_token(lexer);
            Token path_token;
            EXPECT(lexer, TK_STRING, &path_token);
            const char *absolute = relative_to_absolute_path(path_token.text, root_token.location.filepath, lexer->allocator);
            Ast_Directive_Foreign_Import *foreign_import = SIF_NEW_CLONE(Ast_Directive_Foreign_Import(nullptr, absolute, lexer->allocator, lexer->current_block, root_token.location), lexer->allocator);
            return foreign_import;
        }

        case TK_DIRECTIVE_FOREIGN_SYSTEM_IMPORT: {
            eat_next_token(lexer);
            Token path_token;
            EXPECT(lexer, TK_STRING, &path_token);
            Ast_Directive_Foreign_Import *foreign_import = SIF_NEW_CLONE(Ast_Directive_Foreign_Import(nullptr, path_token.text, lexer->allocator, lexer->current_block, root_token.location), lexer->allocator);
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
            Ast_Directive_Assert *assert = SIF_NEW_CLONE(Ast_Directive_Assert(expr, lexer->allocator, lexer->current_block, root_token.location), lexer->allocator);
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
            Ast_Directive_Print *print = SIF_NEW_CLONE(Ast_Directive_Print(expr, lexer->allocator, lexer->current_block, root_token.location), lexer->allocator);
            return print;
        }

        case TK_SEMICOLON: {
            if (eat_semicolon) {
                eat_next_token(lexer);
            }
            Ast_Empty_Statement *stmt = SIF_NEW_CLONE(Ast_Empty_Statement(lexer->allocator, lexer->current_block, root_token.location), lexer->allocator);
            return stmt;
        }

        case TK_FOR: {
            eat_next_token(lexer);
            if (!(lexer->current_toplevel_declaration && lexer->current_toplevel_declaration->kind == DECL_PROC)) {
                report_error(root_token.location, "Can only use `for` in a procedure.");
                return nullptr;
            }

            Ast_For_Loop *for_loop = SIF_NEW_CLONE(Ast_For_Loop(lexer->allocator, lexer->current_block, root_token.location), lexer->allocator);
            assert(lexer->current_toplevel_declaration->kind == DECL_PROC);
            Ast_Proc_Header *current_procedure = ((Proc_Declaration *)lexer->current_toplevel_declaration)->header;
            assert(current_procedure != nullptr);
            Ast_Node *old_loop = current_procedure->current_parsing_loop;
            current_procedure->current_parsing_loop = for_loop;
            defer(current_procedure->current_parsing_loop = old_loop);

            {
                Ast_Block *block = SIF_NEW_CLONE(Ast_Block(lexer->allocator, lexer->current_block, lexer->location), lexer->allocator);
                Ast_Block *old_block = push_ast_block(lexer, block);
                defer(pop_ast_block(lexer, old_block));

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
            if (!(lexer->current_toplevel_declaration && lexer->current_toplevel_declaration->kind == DECL_PROC)) {
                report_error(root_token.location, "Can only use `while` in a procedure.");
                return nullptr;
            }
            Ast_While_Loop *while_loop = SIF_NEW_CLONE(Ast_While_Loop(lexer->allocator, lexer->current_block, root_token.location), lexer->allocator);
            assert(lexer->current_toplevel_declaration->kind == DECL_PROC);
            Ast_Proc_Header *current_procedure = ((Proc_Declaration *)lexer->current_toplevel_declaration)->header;
            assert(current_procedure != nullptr);
            Ast_Node *old_loop = current_procedure->current_parsing_loop;
            current_procedure->current_parsing_loop = while_loop;
            defer(current_procedure->current_parsing_loop = old_loop);

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
            Ast_Block *if_block = nullptr;
            Ast_Node *pre_statement = nullptr;
            Ast_Block *body = nullptr;
            Ast_Expr *condition = nullptr;
            {
                if_block = SIF_NEW_CLONE(Ast_Block(lexer->allocator, lexer->current_block, lexer->location), lexer->allocator);
                Ast_Block *old_block = push_ast_block(lexer, if_block);
                defer(pop_ast_block(lexer, old_block));

                eat_next_token(lexer);
                if (!(lexer->current_toplevel_declaration && lexer->current_toplevel_declaration->kind == DECL_PROC)) {
                    report_error(root_token.location, "Can only use `if` inside a procedure.");
                    return nullptr;
                }
                Token maybe_left_paren;
                if (!peek_next_token(lexer, &maybe_left_paren)) {
                    report_error(lexer->location, "Unexpected end of file.");
                    return nullptr;
                }
                if (maybe_left_paren.kind != TK_LEFT_PAREN) {
                    pre_statement = parse_single_statement(lexer);
                }

                EXPECT(lexer, TK_LEFT_PAREN, nullptr);
                condition = parse_expr(lexer);
                if (!condition) {
                    return nullptr;
                }
                EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
                body = parse_block_including_curly_brackets(lexer);
                if (body == nullptr) {
                    return nullptr;
                }
            }
            Token else_token;
            if (!peek_next_token(lexer, &else_token)) {
                report_error(lexer->location, "Unexpected end of file.");
                return nullptr;
            }
            Ast_Block *else_body = nullptr;
            if (else_token.kind == TK_ELSE) {
                eat_next_token(lexer);
                else_body = parse_block_including_curly_brackets(lexer);
            }
            Ast_If *ast_if = SIF_NEW_CLONE(Ast_If(if_block, pre_statement, condition, body, else_body, lexer->allocator, lexer->current_block, root_token.location), lexer->allocator);
            return ast_if;
        }

        case TK_RETURN: {
            eat_next_token(lexer);
            if (!(lexer->current_toplevel_declaration && lexer->current_toplevel_declaration->kind == DECL_PROC)) {
                report_error(root_token.location, "Can only use `return` in a procedure.");
                return nullptr;
            }
            Token semicolon;
            if (!peek_next_token(lexer, &semicolon)) {
                report_error(lexer->location, "Unexpected end of file.");
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
            assert(lexer->current_toplevel_declaration->kind == DECL_PROC);
            Ast_Proc_Header *current_procedure = ((Proc_Declaration *)lexer->current_toplevel_declaration)->header;
            assert(current_procedure != nullptr);
            return SIF_NEW_CLONE(Ast_Return(current_procedure, return_expr, lexer->allocator, lexer->current_block, root_token.location), lexer->allocator);
        }

        case TK_CONTINUE: {
            eat_next_token(lexer);
            if (!(lexer->current_toplevel_declaration && lexer->current_toplevel_declaration->kind == DECL_PROC)) {
                report_error(root_token.location, "Can only use `continue` in a procedure.");
                return nullptr;
            }
            assert(lexer->current_toplevel_declaration->kind == DECL_PROC);
            Ast_Proc_Header *current_procedure = ((Proc_Declaration *)lexer->current_toplevel_declaration)->header;
            assert(current_procedure != nullptr);
            if (current_procedure->current_parsing_loop == nullptr) {
                report_error(root_token.location, "`continue` must be used from within a loop.");
                return nullptr;
            }
            EXPECT(lexer, TK_SEMICOLON, nullptr);
            return SIF_NEW_CLONE(Ast_Continue(current_procedure->current_parsing_loop, lexer->allocator, lexer->current_block, root_token.location), lexer->allocator);
        }

        case TK_BREAK: {
            eat_next_token(lexer);
            if (!(lexer->current_toplevel_declaration && lexer->current_toplevel_declaration->kind == DECL_PROC)) {
                report_error(root_token.location, "Can only use `break` in a procedure.");
                return nullptr;
            }
            assert(lexer->current_toplevel_declaration->kind == DECL_PROC);
            Ast_Proc_Header *current_procedure = ((Proc_Declaration *)lexer->current_toplevel_declaration)->header;
            assert(current_procedure != nullptr);
            if (current_procedure->current_parsing_loop == nullptr) {
                report_error(root_token.location, "`break` must be used from within a loop.");
                return nullptr;
            }
            EXPECT(lexer, TK_SEMICOLON, nullptr);
            return SIF_NEW_CLONE(Ast_Break(current_procedure->current_parsing_loop, lexer->allocator, lexer->current_block, root_token.location), lexer->allocator);
        }

        default: {
            Ast_Expr_List *lhs_list = parse_expr_list(lexer);
            if (lhs_list == nullptr) {
                return nullptr;
            }

            Token next_token;
            if (!peek_next_token(lexer, &next_token)) {
                report_error(lexer->location, "Unexpected end of file.");
                return nullptr;
            }
            switch (next_token.kind) {
                case TK_SEMICOLON: {
                    if (eat_semicolon) {
                        eat_next_token(lexer);
                    }
                    if (lhs_list->exprs.count != 1) {
                        report_error(lhs_list->location, "Expected 1 expression, got %d.", lhs_list->exprs.count);
                        return nullptr;
                    }
                    return SIF_NEW_CLONE(Ast_Statement_Expr(lhs_list->exprs[0], lexer->allocator, lexer->current_block, lhs_list->exprs[0]->location), lexer->allocator);
                }

                case TK_PLUS_ASSIGN:     // fallthrough
                case TK_MINUS_ASSIGN:    // fallthrough
                case TK_MULTIPLY_ASSIGN: // fallthrough
                case TK_DIVIDE_ASSIGN:   // fallthrough
                case TK_MOD_ASSIGN:      // fallthrough
                case TK_BIT_OR_ASSIGN:   // fallthrough
                case TK_ASSIGN: { // todo(josh): <<=, &&=, etc
                    Token op;
                    assert(get_next_token(lexer, &op));
                    Ast_Expr_List *rhs_list = parse_expr_list(lexer);
                    if (rhs_list == nullptr) {
                        return nullptr;
                    }
                    if (eat_semicolon) {
                        EXPECT(lexer, TK_SEMICOLON, nullptr);
                    }
                    return SIF_NEW_CLONE(Ast_Assign(op.kind, lhs_list, rhs_list, lexer->allocator, lexer->current_block, lhs_list->location), lexer->allocator);
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

Ast_Block *parse_block(Lexer *lexer, bool only_parse_one_statement) {
    Ast_Block *block = SIF_NEW_CLONE(Ast_Block(lexer->allocator, lexer->current_block, lexer->location), lexer->allocator);
    Ast_Block *old_block = push_ast_block(lexer, block);
    defer(pop_ast_block(lexer, old_block));

    Token root_token;
    while (peek_next_token(lexer, &root_token) && root_token.kind != TK_RIGHT_CURLY) {
        Ast_Node *node = parse_single_statement(lexer);
        if (node == nullptr) {
            return nullptr;
        }
        switch (node->ast_kind) {
            case AST_DIRECTIVE_FOREIGN_IMPORT: {
                Ast_Directive_Foreign_Import *directive = (Ast_Directive_Foreign_Import *)node;
                g_all_foreign_imports_spinlock.lock();
                g_all_foreign_import_directives.append(directive);
                g_all_foreign_imports_spinlock.unlock();
                break;
            }
            case AST_EMPTY_STATEMENT: {
                break;
            }
            default: {
                lexer->current_block->nodes.append(node);
                break;
            }
        }

        if (only_parse_one_statement) {
            break;
        }
    }
    return lexer->current_block;
}

void parse_file(const char *requested_filename, Location include_location) {
    String_Builder include_sb = make_string_builder(g_global_linear_allocator, 128);
    if (starts_with(requested_filename, "core:")) {
        requested_filename = requested_filename + 5;
        include_sb.printf("%s/%s", sif_core_lib_path, requested_filename);
    }
    else {
        if (include_location.filepath) { // the root file doesn't _have_ an include location
            const char *absolute = relative_to_absolute_path(requested_filename, include_location.filepath, g_global_linear_allocator);
            include_sb.printf("%s", absolute);
        }
        else {
            include_sb.print(requested_filename);
        }
    }

    const char *filename = include_sb.string();
    char *absolute_path = intern_string(get_absolute_path(filename, g_global_linear_allocator));
    int path_len = strlen(absolute_path);
    for (int i = 0; i < path_len; i += 1) {
        if (absolute_path[i] == '\\') {
            absolute_path[i] = '/';
        }
    }

    g_lexers_to_process_spinlock.lock();
    defer(g_lexers_to_process_spinlock.unlock());

    For (idx, g_all_included_files) {
        if (g_all_included_files[idx] == absolute_path) {
            // we've already included this file, no need to do it again
            return;
        }
    }

    g_all_included_files.append(absolute_path);

    Lexer lexer(absolute_path);
    Dynamic_Arena *arena = (Dynamic_Arena *)alloc(g_global_linear_allocator, sizeof(Dynamic_Arena), DEFAULT_ALIGNMENT);
    init_dynamic_arena(arena, 1 * 1024 * 1024, g_global_linear_allocator);
    lexer.allocator = dynamic_arena_allocator(arena);
    g_lexers_to_process.append(lexer);
}

u32 parser_worker_thread(void *userdata) {
    while (g_files_done_parsing < g_all_included_files.count) {
        g_lexers_to_process_spinlock.lock();
        if (g_lexers_to_process.count > 0) {
            Lexer lexer = g_lexers_to_process.pop();
            g_lexers_to_process_spinlock.unlock();

            int len = 0;
            char *root_file_text = read_entire_file(lexer.location.filepath, &len);
            if (root_file_text == nullptr) {
                report_error({}, "Couldn't find file '%s'.", lexer.location.filepath);
                return 0;
            }
            lexer.text = root_file_text;

            Ast_Block *block = parse_block(&lexer, false);
            if (!block) {
                return 0;
            }
            block->flags = BF_IS_FILE_SCOPE;

            g_all_file_blocks_spinlock.lock();
            g_all_file_blocks.append(block);
            g_files_done_parsing += 1;
            g_total_lines_parsed += lexer.location.line;
            g_all_file_blocks_spinlock.unlock();
        }
        else {
            g_lexers_to_process_spinlock.unlock();
        }
        // sleep(1);
    }
    return 0;
}

Ast_Block *begin_parsing(const char *filename) {
    parse_file("core:runtime.sif", {});
    parse_file(filename, {});

    for (int i = 0; i < NUM_PARSER_THREADS; i += 1) {
        g_parser_threads[i] = create_thread(parser_worker_thread, (void *)(i64)i);
    }

    for (int i = 0; i < NUM_PARSER_THREADS; i += 1) {
        wait_for_thread(g_parser_threads[i]);
    }

    Ast_Block *global_scope = SIF_NEW_CLONE(Ast_Block(g_global_linear_allocator, {}, {}), g_global_linear_allocator);
    global_scope->flags |= BF_IS_GLOBAL_SCOPE;

    For (idx, g_all_file_blocks) {
        Ast_Block *file_block = g_all_file_blocks[idx];
        file_block->parent_block = global_scope;
        For (decl_idx, file_block->declarations) {
            if (!register_declaration(global_scope, file_block->declarations[decl_idx])) {
                return nullptr;
            }
        }
    }
    return global_scope;
}



bool is_or_op(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token)) {
        report_error(lexer->location, "Unexpected end of file.");
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
        report_error(lexer->location, "Unexpected end of file.");
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
        report_error(lexer->location, "Unexpected end of file.");
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
        report_error(lexer->location, "Unexpected end of file.");
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
        report_error(lexer->location, "Unexpected end of file.");
        return false;
    }
    return is_mul_op(token.kind);
}
bool is_mul_op(Token_Kind kind) {
    switch (kind) {
        case TK_MULTIPLY:
        case TK_DIVIDE:
        case TK_MOD:
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
        report_error(lexer->location, "Unexpected end of file.");
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
        report_error(lexer->location, "Unexpected end of file.");
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

Ast_Expr *unparen_expr(Ast_Expr *expr) {
    assert(expr != nullptr);
    while (expr->expr_kind == EXPR_PAREN) {
        Expr_Paren *paren = (Expr_Paren *)expr;
        expr = paren->nested;
    }
    return expr;
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
        expr = SIF_NEW_CLONE(Expr_Binary(op.kind, lhs, rhs, lexer->allocator, lexer->current_block, op.location), lexer->allocator);
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
        expr = SIF_NEW_CLONE(Expr_Binary(op.kind, lhs, rhs, lexer->allocator, lexer->current_block, op.location), lexer->allocator);
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
        expr = SIF_NEW_CLONE(Expr_Binary(op.kind, lhs, rhs, lexer->allocator, lexer->current_block, op.location), lexer->allocator);
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
        expr = SIF_NEW_CLONE(Expr_Binary(op.kind, lhs, rhs, lexer->allocator, lexer->current_block, op.location), lexer->allocator);
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
        expr = SIF_NEW_CLONE(Expr_Binary(op.kind, lhs, rhs, lexer->allocator, lexer->current_block, op.location), lexer->allocator);
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
                return SIF_NEW_CLONE(Expr_Address_Of(rhs, lexer->allocator, lexer->current_block, op.location), lexer->allocator);
            }
            case TK_MINUS:
            case TK_PLUS:
            case TK_BIT_NOT:
            case TK_NOT: {
                Ast_Expr *rhs = parse_unary_expr(lexer);
                if (!rhs) {
                    return nullptr;
                }
                return SIF_NEW_CLONE(Expr_Unary(op.kind, rhs, lexer->allocator, lexer->current_block, op.location), lexer->allocator);
            }
            case TK_DOT_DOT: {
                Ast_Expr *rhs = parse_postfix_expr(lexer);
                if (!rhs) {
                    return nullptr;
                }
                return SIF_NEW_CLONE(Expr_Spread(rhs, lexer->allocator, lexer->current_block, op.location), lexer->allocator);
            }
            case TK_SIZEOF: {
                EXPECT(lexer, TK_LEFT_PAREN, nullptr);
                Ast_Expr *expr = parse_expr(lexer);
                if (!expr) {
                    return nullptr;
                }
                EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
                return SIF_NEW_CLONE(Expr_Sizeof(expr, lexer->allocator, lexer->current_block, op.location), lexer->allocator);
            }
            case TK_TYPEOF: {
                EXPECT(lexer, TK_LEFT_PAREN, nullptr);
                Ast_Expr *expr = parse_expr(lexer);
                if (!expr) {
                    return nullptr;
                }
                EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
                return SIF_NEW_CLONE(Expr_Typeof(expr, lexer->allocator, lexer->current_block, op.location), lexer->allocator);
            }
            default: {
                assert(false);
            }
        }
    }

    return parse_postfix_expr(lexer);
}

Expr_Compound_Literal *parse_compound_literal(Lexer *lexer, Ast_Expr *type_expr) {
    assert(lexer->allow_compound_literals);
    Token left_curly_token;
    EXPECT(lexer, TK_LEFT_CURLY, &left_curly_token);
    Location start_location = left_curly_token.location;
    if (type_expr != nullptr) {
        start_location = type_expr->location;
    }
    Array<Ast_Expr *> exprs = {};
    exprs.allocator = lexer->allocator;
    Token token;
    while (peek_next_token(lexer, &token) && token.kind != TK_RIGHT_CURLY) {
        Ast_Expr *expr = parse_expr(lexer);
        if (!expr) {
            return nullptr;
        }
        exprs.append(expr);

        Token maybe_comma;
        if (!peek_next_token(lexer, &maybe_comma)) {
            report_error(lexer->location, "Unexpected end of file.");
            return nullptr;
        }
        if (maybe_comma.kind == TK_RIGHT_CURLY) {
        }
        else {
            EXPECT(lexer, TK_COMMA, nullptr);
        }
    }
    EXPECT(lexer, TK_RIGHT_CURLY, nullptr);
    Expr_Compound_Literal *compound_literal = SIF_NEW_CLONE(Expr_Compound_Literal(type_expr, exprs, lexer->allocator, lexer->current_block, start_location), lexer->allocator);
    return compound_literal;
}

Ast_Expr *parse_postfix_expr(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token)) {
        report_error(lexer->location, "Unexpected end of file.");
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
                parameters.allocator = lexer->allocator;
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
                base_expr = SIF_NEW_CLONE(Expr_Procedure_Call(base_expr, parameters, lexer->allocator, lexer->current_block, base_expr->location), lexer->allocator);
                break;
            }
            case TK_LEFT_CURLY: {
                if (!lexer->allow_compound_literals) {
                    return base_expr;
                }
                base_expr = parse_compound_literal(lexer, base_expr);
                if (!base_expr) {
                    return nullptr;
                }
                break;
            }
            case TK_LEFT_SQUARE: {
                eat_next_token(lexer);
                Ast_Expr *index = parse_expr(lexer);
                if (!index) {
                    return nullptr;
                }
                EXPECT(lexer, TK_RIGHT_SQUARE, nullptr);

                base_expr = SIF_NEW_CLONE(Expr_Subscript(base_expr, index, lexer->allocator, lexer->current_block, base_expr->location), lexer->allocator);
                break;
            }
            case TK_CARET: {
                eat_next_token(lexer);
                base_expr = SIF_NEW_CLONE(Expr_Dereference(base_expr, lexer->allocator, lexer->current_block, base_expr->location), lexer->allocator);
                break;
            }
            case TK_NOT: {
                eat_next_token(lexer);
                EXPECT(lexer, TK_LEFT_PAREN, nullptr);
                Array<Ast_Expr *> parameters;
                parameters.allocator = lexer->allocator;
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
                base_expr = SIF_NEW_CLONE(Expr_Polymorphic_Type(base_expr, parameters, lexer->allocator, lexer->current_block, base_expr->location), lexer->allocator);
                break;
            }
            case TK_DOT: {
                eat_next_token(lexer);
                Token name_token;
                EXPECT(lexer, TK_IDENTIFIER, &name_token);
                base_expr = SIF_NEW_CLONE(Expr_Selector(base_expr, name_token.text, lexer->allocator, lexer->current_block, base_expr->location), lexer->allocator);
                break;
            }
            default: {
                unexpected_token(lexer, op);
                return nullptr;
            }
        }
    }
    return base_expr;
}

Ast_Expr *parse_base_expr(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token)) {
        report_error(lexer->location, "Unexpected end of file.");
        return nullptr;
    }

    switch (token.kind) {
        case TK_NULL: {
            eat_next_token(lexer);
            return SIF_NEW_CLONE(Expr_Null(lexer->allocator, lexer->current_block, token.location), lexer->allocator);
        }
        case TK_TRUE: {
            eat_next_token(lexer);
            return SIF_NEW_CLONE(Expr_True(lexer->allocator, lexer->current_block, token.location), lexer->allocator);
        }
        case TK_FALSE: {
            eat_next_token(lexer);
            return SIF_NEW_CLONE(Expr_False(lexer->allocator, lexer->current_block, token.location), lexer->allocator);
        }
        case TK_IDENTIFIER: {
            eat_next_token(lexer);
            char *ident_name = token.text;
            Expr_Identifier *ident = SIF_NEW_CLONE(Expr_Identifier(ident_name, lexer->allocator, lexer->current_block, token.location), lexer->allocator);
            return ident;
        }
        case TK_NUMBER: {
            eat_next_token(lexer);
            return SIF_NEW_CLONE(Expr_Number_Literal(token.uint_value, token.int_value, token.float_value, token.has_a_dot, lexer->allocator, lexer->current_block, token.location), lexer->allocator);
        }
        case TK_CHAR: {
            eat_next_token(lexer);
            if (token.escaped_length != 1) {
                printf("%d %d\n", token.escaped_length, token.scanner_length);
                report_error(token.location, "Character literal must be length 1.");
                return nullptr;
            }
            return SIF_NEW_CLONE(Expr_Char_Literal(token.escaped_text[0], lexer->allocator, lexer->current_block, token.location), lexer->allocator);
        }
        case TK_STRING: {
            eat_next_token(lexer);
            return SIF_NEW_CLONE(Expr_String_Literal(token.text, token.scanner_length, token.escaped_text, token.escaped_length, lexer->allocator, lexer->current_block, token.location), lexer->allocator);
        }
        case TK_LEFT_PAREN: {
            eat_next_token(lexer);
            Ast_Expr *nested = parse_expr(lexer);
            EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
            return SIF_NEW_CLONE(Expr_Paren(nested, lexer->allocator, lexer->current_block, token.location), lexer->allocator);
        }
        case TK_DIRECTIVE_C_VARARGS: {
            Token directive_token;
            eat_next_token(lexer, &directive_token);
            Ast_Expr *type = parse_expr(lexer);
            if (!type) {
                return nullptr;
            }
            if (unparen_expr(type)->expr_kind != EXPR_SPREAD) {
                report_error(directive_token.location, "#c_varargs can only be used on a '..type' expression.");
                return nullptr;
            }
            Expr_Spread *spread = (Expr_Spread *)type;
            spread->is_c_varargs = true;
            return spread;
        }
        case TK_DIRECTIVE_PARTIAL: {
            Token partial_token;
            eat_next_token(lexer, &partial_token);
            Ast_Expr *expr = parse_expr(lexer);
            if (expr == nullptr) {
                return nullptr;
            }
            if (unparen_expr(expr)->expr_kind != EXPR_COMPOUND_LITERAL) {
                report_error(partial_token.location, "#partial can only be used on compound literal expressions.");
                return nullptr;
            }
            Expr_Compound_Literal *compound_literal = (Expr_Compound_Literal *)expr;
            compound_literal->is_partial = true;
            return compound_literal;
        }
        case TK_LEFT_CURLY: {
            assert(lexer->allow_compound_literals);
            Expr_Compound_Literal *compound_literal = parse_compound_literal(lexer, nullptr);
            if (!compound_literal) {
                return nullptr;
            }
            return compound_literal;
        }
        case TK_DOT: {
            Token dot_token;
            eat_next_token(lexer, &dot_token);
            Token identifier_token;
            EXPECT(lexer, TK_IDENTIFIER, &identifier_token);
            return SIF_NEW_CLONE(Expr_Implicit_Enum_Selector(identifier_token.text, lexer->allocator, lexer->current_block, dot_token.location), lexer->allocator);
        }
        case TK_PROC: {
            Ast_Proc_Header *header = parse_proc_header(lexer);
            return SIF_NEW_CLONE(Expr_Procedure_Type(header, lexer->allocator, lexer->current_block, token.location), lexer->allocator);
        }
        case TK_STRUCT: // note(josh): fallthrough
        case TK_UNION: {
            Ast_Struct *structure = parse_struct_or_union(lexer);
            return SIF_NEW_CLONE(Expr_Struct_Type(structure, lexer->allocator, lexer->current_block, token.location), lexer->allocator);
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
            return SIF_NEW_CLONE(Expr_Cast(type_expr, rhs, lexer->allocator, lexer->current_block, token.location), lexer->allocator);
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
            return SIF_NEW_CLONE(Expr_Transmute(type_expr, rhs, lexer->allocator, lexer->current_block, token.location), lexer->allocator);
        }
        case TK_DOLLAR: {
            eat_next_token(lexer);
            Ast_Expr *ident_expr = parse_expr(lexer);
            if (ident_expr->expr_kind != EXPR_IDENTIFIER) {
                report_error(ident_expr->location, "Polymorphic variable must be an identifier.");
                return nullptr;
            }
            Expr_Identifier *ident = (Expr_Identifier *)ident_expr;
            lexer->num_polymorphic_variables_parsed += 1;
            return SIF_NEW_CLONE(Expr_Polymorphic_Variable((Expr_Identifier *)ident, lexer->allocator, lexer->current_block, token.location), lexer->allocator);
        }
        case TK_LEFT_SQUARE: {
            eat_next_token(lexer);
            Token maybe_right_square;
            if (!peek_next_token(lexer, &maybe_right_square)) {
                report_error(lexer->location, "Unexpected end of file.");
                return nullptr;
            }
            if (maybe_right_square.kind == TK_RIGHT_SQUARE) {
                // it's a slice
                EXPECT(lexer, TK_RIGHT_SQUARE, nullptr);
                bool old_allow_compound_literals = lexer->allow_compound_literals;
                lexer->allow_compound_literals = false;
                Ast_Expr *slice_of = parse_expr(lexer);
                lexer->allow_compound_literals = old_allow_compound_literals;
                if (!slice_of) {
                    return nullptr;
                }
                return SIF_NEW_CLONE(Expr_Slice_Type(slice_of, lexer->allocator, lexer->current_block, token.location), lexer->allocator);
            }
            else {
                // it's an array
                Ast_Expr *length = parse_expr(lexer);
                if (!length) {
                    return nullptr;
                }
                EXPECT(lexer, TK_RIGHT_SQUARE, nullptr);
                bool old_allow_compound_literals = lexer->allow_compound_literals;
                lexer->allow_compound_literals = false;
                Ast_Expr *array_of = parse_expr(lexer);
                lexer->allow_compound_literals = old_allow_compound_literals;
                if (!array_of) {
                    return nullptr;
                }
                return SIF_NEW_CLONE(Expr_Array_Type(array_of, length, lexer->allocator, lexer->current_block, token.location), lexer->allocator);
            }
            assert(false && "unreachable");
        }
        case TK_CARET: {
            eat_next_token(lexer);
            bool old_allow_compound_literals = lexer->allow_compound_literals;
            lexer->allow_compound_literals = false;
            Ast_Expr *pointer_to = parse_expr(lexer);
            lexer->allow_compound_literals = old_allow_compound_literals;
            if (!pointer_to) {
                return nullptr;
            }
            return SIF_NEW_CLONE(Expr_Pointer_Type(pointer_to, lexer->allocator, lexer->current_block, token.location), lexer->allocator);
        }
        case TK_GREATER_THAN: {
            eat_next_token(lexer);
            bool old_allow_compound_literals = lexer->allow_compound_literals;
            lexer->allow_compound_literals = false;
            Ast_Expr *reference_to = parse_expr(lexer);
            lexer->allow_compound_literals = old_allow_compound_literals;
            if (!reference_to) {
                return nullptr;
            }
            return SIF_NEW_CLONE(Expr_Reference_Type(reference_to, lexer->allocator, lexer->current_block, token.location), lexer->allocator);
        }
        default: {
            unexpected_token(lexer, token);
            return nullptr;
        }
    }
    assert(false && "unreachable");
    return nullptr;
}
