#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>

#include "parser.h"

Array<Expr_Identifier *>      g_identifiers_to_resolve;
Array<Declaration *>          g_all_declarations;
Array<Ast_Directive_Assert *> g_all_assert_directives;
Array<Ast_Directive_Print *>  g_all_print_directives;
Array<Ast_Directive_C_Code *> g_all_c_code_directives;

Ast_Proc_Header *g_currently_parsing_proc;

void init_parser() {
    g_identifiers_to_resolve.allocator = default_allocator();
    g_all_declarations.allocator       = default_allocator();
    g_all_assert_directives.allocator  = default_allocator();
    g_all_print_directives.allocator   = default_allocator();
    g_all_c_code_directives.allocator  = default_allocator();
}

void resolve_identifiers() {
    For (idx, g_identifiers_to_resolve) {
        Expr_Identifier *ident = g_identifiers_to_resolve[idx];
        Ast_Block *block = ident->parent_block;
        while (block != nullptr) {
            For (decl_idx, block->declarations) {
                Declaration *decl = block->declarations[decl_idx];
                if (strcmp(decl->name, ident->name) == 0) {
                    ident->resolved_declaration = decl;
                    break;
                }
            }

            if (ident->resolved_declaration) {
                break;
            }
            block = block->parent_block;
        }

        if (!ident->resolved_declaration) {
            assert(ident->resolved_declaration != nullptr && "Unresolved identifier");
            g_reported_error = true;
        }
    }
}

Ast_Block *push_ast_block(Ast_Block *block) {
    Ast_Block *old_block = current_block;
    current_block = block;
    return old_block;
}

void pop_ast_block(Ast_Block *old_block) {
    current_block = old_block;
}

void register_declaration(Declaration *new_declaration) {
    assert(new_declaration->parent_block != nullptr);
    For (decl_idx, new_declaration->parent_block->declarations) {
        Declaration *decl = new_declaration->parent_block->declarations[decl_idx];
        if (strcmp(decl->name, new_declaration->name) == 0) {
            report_error(new_declaration->location, "Name collision.");
            return;
        }
    }
    new_declaration->parent_block->declarations.append(new_declaration);
    g_all_declarations.append(new_declaration);
}



#define EXPECT(_lexer, _token_kind, _token_ptr) if (!expect_token(_lexer, _token_kind, _token_ptr)) { return nullptr; }

Ast_Var *parse_var(Lexer *lexer) {
    Token var_token;
    EXPECT(lexer, TK_VAR, &var_token);

    Token var_name_token;
    EXPECT(lexer, TK_IDENTIFIER, &var_name_token);
    char *var_name = var_name_token.text;

    Token colon;
    if (!peek_next_token(lexer, &colon)) {
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

    Ast_Var *var = new Ast_Var(var_name, type_expr, expr, var_token.location);
    var->declaration = new Var_Declaration(var, current_block);
    register_declaration(var->declaration);
    return var;
}

Ast_Proc_Header *parse_proc_header(Lexer *lexer) {
    Token token;
    EXPECT(lexer, TK_PROC, &token);
    Location proc_location = token.location;
    Ast_Block *procedure_block = new Ast_Block(token.location);
    Ast_Block *old_block = push_ast_block(procedure_block);
    defer(pop_ast_block(old_block));

    // name
    EXPECT(lexer, TK_IDENTIFIER, &token);

    char *proc_name = token.text;

    // parameter list
    EXPECT(lexer, TK_LEFT_PAREN, nullptr);
    Array<Ast_Var *> parameters = {};
    parameters.allocator = default_allocator();
    bool first = true;
    while (peek_next_token(lexer, &token) && token.kind != TK_RIGHT_PAREN) {
        if (!first) {
            EXPECT(lexer, TK_COMMA, nullptr);
        }

        Ast_Var *var = parse_var(lexer);
        if (!var) {
            return nullptr;
        }
        parameters.append(var);
        first = false;
    }
    EXPECT(lexer, TK_RIGHT_PAREN, nullptr);

    // return type
    Ast_Expr *return_type_expr = {};
    Token colon = {};
    if (!peek_next_token(lexer, &colon)) {
        return nullptr;
    }
    if (colon.kind == TK_COLON) {
        eat_next_token(lexer);
        return_type_expr = parse_expr(lexer);
        if (!return_type_expr) {
            return nullptr;
        }
    }

    Token foreign;
    bool is_foreign = false;
    if (!peek_next_token(lexer, &foreign)) {
        return nullptr;
    }
    if (foreign.kind == TK_DIRECTIVE_FOREIGN) {
        eat_next_token(lexer);
        is_foreign = true;
        EXPECT(lexer, TK_SEMICOLON, nullptr); // note(josh): this isn't really necessary but it looks nicer
    }

    return new Ast_Proc_Header(proc_name, procedure_block, parameters, return_type_expr, is_foreign, proc_location);
}

Ast_Proc *parse_proc(Lexer *lexer) {
    Ast_Proc_Header *header = parse_proc_header(lexer);

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

    Ast_Proc *proc = new Ast_Proc(header, body, header->location);
    proc->declaration = new Proc_Declaration(proc, current_block);
    register_declaration(proc->declaration);
    return proc;
}

Ast_Struct *parse_struct(Lexer *lexer) {
    Token token;
    EXPECT(lexer, TK_STRUCT, &token);
    Ast_Struct *structure = new Ast_Struct(token.location);

    // name
    EXPECT(lexer, TK_IDENTIFIER, &token);
    structure->name = token.text;

    // fields
    EXPECT(lexer, TK_LEFT_CURLY, &token);

    structure->body = parse_block(lexer);
    if (!structure->body) {
        return nullptr;
    }
    EXPECT(lexer, TK_RIGHT_CURLY, nullptr);
    For (idx, structure->body->nodes) {
        Ast_Node *node = structure->body->nodes[idx];
        assert(node->ast_kind == AST_VAR);
        Ast_Var *var = (Ast_Var *)node;
        structure->fields.append(var);
    }

    structure->declaration = new Struct_Declaration(structure, current_block);
    register_declaration(structure->declaration);
    return structure;
}

Ast_Block *parse_block_including_curly_brackets(Lexer *lexer) {
    Token open_curly;
    if (!peek_next_token(lexer, &open_curly)) {
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

Ast_Node *parse_single_statement(Lexer *lexer, bool eat_semicolon = true) {
    Token root_token;
    if (!peek_next_token(lexer, &root_token)) {
        return nullptr;
    }

    switch (root_token.kind) {
        case TK_VAR: {
            Ast_Var *var = parse_var(lexer);
            if (!var) {
                return nullptr;
            }
            if (eat_semicolon) {
                EXPECT(lexer, TK_SEMICOLON, nullptr);
            }
            return var;
        }

        case TK_PROC: {
            Ast_Proc *proc = parse_proc(lexer);
            if (!proc) {
                return nullptr;
            }
            return proc;
        }

        case TK_STRUCT: {
            Ast_Struct *structure = parse_struct(lexer);
            if (!structure) {
                return nullptr;
            }
            return structure;
        }

        case TK_DIRECTIVE_ASSERT: {
            eat_next_token(lexer);
            EXPECT(lexer, TK_LEFT_PAREN, nullptr);
            Ast_Expr *expr = parse_expr(lexer);
            if (!expr) {
                return nullptr;
            }
            EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
            return new Ast_Directive_Assert(expr, root_token.location);
        }

        case TK_DIRECTIVE_PRINT: {
            eat_next_token(lexer);
            EXPECT(lexer, TK_LEFT_PAREN, nullptr);
            Ast_Expr *expr = parse_expr(lexer);
            if (!expr) {
                return nullptr;
            }
            EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
            return new Ast_Directive_Print(expr, root_token.location);
        }

        case TK_DIRECTIVE_C_CODE: {
            eat_next_token(lexer);
            Token code;
            EXPECT(lexer, TK_STRING, &code);
            return new Ast_Directive_C_Code(code.escaped_text, root_token.location);
        }

        case TK_SEMICOLON: {
            if (eat_semicolon) {
                eat_next_token(lexer);
            }
            return new Ast_Empty_Statement(root_token.location);
        }

        case TK_FOR: {
            Ast_Block *block = new Ast_Block(lexer->location);
            Ast_Block *old_block = push_ast_block(block);

            eat_next_token(lexer);
            EXPECT(lexer, TK_LEFT_PAREN, nullptr);
            Ast_Node *pre = parse_single_statement(lexer, false);
            EXPECT(lexer, TK_SEMICOLON, nullptr);
            Ast_Expr *condition = parse_expr(lexer);
            EXPECT(lexer, TK_SEMICOLON, nullptr);
            Ast_Node *post = parse_single_statement(lexer, false);
            EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
            Ast_Block *body = parse_block_including_curly_brackets(lexer);
            pop_ast_block(old_block);

            return new Ast_For_Loop(pre, condition, post, body, root_token.location);
        }

        case TK_IF: {
            eat_next_token(lexer);
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
                return nullptr;
            }
            Ast_Block *else_body = nullptr;
            if (else_token.kind == TK_ELSE) {
                eat_next_token(lexer);
                else_body = parse_block_including_curly_brackets(lexer);
            }
            return new Ast_If(condition, body, else_body, root_token.location);
        }

        case TK_RETURN: {
            eat_next_token(lexer);
            Token semicolon;
            if (!peek_next_token(lexer, &semicolon)) {
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
            return new Ast_Return(g_currently_parsing_proc, return_expr, root_token.location);
        }

        default: {
            Ast_Expr *expr = parse_expr(lexer);
            if (expr == nullptr) {
                return nullptr;
            }

            Token next_token;
            if (!peek_next_token(lexer, &next_token)) {
                assert(false);
                return nullptr;
            }
            switch (next_token.kind) {
                case TK_SEMICOLON: {
                    if (eat_semicolon) {
                        eat_next_token(lexer);
                    }
                    return new Ast_Statement_Expr(expr, expr->location);
                }
                case TK_ASSIGN: {
                    eat_next_token(lexer);
                    Ast_Expr *rhs = parse_expr(lexer);
                    if (rhs == nullptr) {
                        return nullptr;
                    }
                    if (eat_semicolon) {
                        EXPECT(lexer, TK_SEMICOLON, nullptr);
                    }
                    return new Ast_Assign(expr, rhs, expr->location);
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
    Ast_Block *block = new Ast_Block(lexer->location);
    Ast_Block *old_block = push_ast_block(block);
    defer(pop_ast_block(old_block));

    if (old_block == nullptr) {
        block->flags |= BF_IS_GLOBAL_SCOPE;
    }

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
            case AST_EMPTY_STATEMENT: {
                // note(josh): do nothing
                break;
            }
            default: {
                block->nodes.append(node);
            }
        }

        if (only_parse_one_statement) {
            break;
        }
    }
    return block;
}



bool is_or_op(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token))  return false;
    switch (token.kind) {
        case TK_BOOLEAN_OR: {
            return true;
        }
    }
    return false;
}

bool is_and_op(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token))  return false;
    switch (token.kind) {
        case TK_BOOLEAN_AND: {
            return true;
        }
    }
    return false;
}

bool is_cmp_op(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token))  return false;
    switch (token.kind) {
        case TK_NOT_EQUAL_TO:
        case TK_LESS_THAN:
        case TK_LESS_THAN_OR_EQUAL:
        case TK_GREATER_THAN:
        case TK_GREATER_THAN_OR_EQUAL:
        case TK_EQUAL_TO:
        case TK_BOOLEAN_AND:
        case TK_BOOLEAN_AND_EQUALS:
        case TK_BOOLEAN_OR:
        case TK_BOOLEAN_OR_EQUALS: {
            return true;
        }
    }
    return false;
}

bool is_add_op(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token))  return false;
    switch (token.kind) {
        case TK_PLUS:
        case TK_MINUS: {
            return true;
        }
    }
    return false;
}

bool is_mul_op(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token))  return false;
    switch (token.kind) {
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
    if (!peek_next_token(lexer, &token))  return false;
    switch (token.kind) {
        case TK_MINUS:
        case TK_PLUS:
        case TK_NOT:
        case TK_SIZEOF:
        case TK_TYPEOF:
        case TK_CAST:
        case TK_AMPERSAND: { // address-of
            return true;
        }
    }
    return false;
}

bool is_postfix_op(Lexer *lexer) {
    Token token;
    if (!peek_next_token(lexer, &token))  return false;
    switch (token.kind) {
        case TK_LEFT_PAREN:
        case TK_LEFT_SQUARE:
        case TK_DOT:
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
        expr = new Expr_Binary(op.kind, lhs, rhs);
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
        expr = new Expr_Binary(op.kind, lhs, rhs);
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
        expr = new Expr_Binary(op.kind, lhs, rhs);
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
        expr = new Expr_Binary(op.kind, lhs, rhs);
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
        expr = new Expr_Binary(op.kind, lhs, rhs);
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
                return new Expr_Address_Of(rhs, op.location);
            }
            case TK_MINUS:
            case TK_PLUS:
            case TK_NOT: {
                Ast_Expr *rhs = parse_unary_expr(lexer);
                if (!rhs) {
                    return nullptr;
                }
                return new Expr_Unary(op.kind, rhs, op.location);
            }
            case TK_SIZEOF: {
                EXPECT(lexer, TK_LEFT_PAREN, nullptr);
                Ast_Expr *expr = parse_expr(lexer);
                if (!expr) {
                    return nullptr;
                }
                EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
                return new Expr_Sizeof(expr, op.location);
            }
            case TK_TYPEOF: {
                EXPECT(lexer, TK_LEFT_PAREN, nullptr);
                Ast_Expr *expr = parse_expr(lexer);
                if (!expr) {
                    return nullptr;
                }
                EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
                return new Expr_Typeof(expr, op.location);
            }
            case TK_CAST: {
                EXPECT(lexer, TK_LEFT_PAREN, nullptr);
                Ast_Expr *type_expr = parse_expr(lexer);
                if (!type_expr) {
                    return nullptr;
                }
                EXPECT(lexer, TK_COMMA, nullptr);
                Ast_Expr *rhs = parse_expr(lexer);
                EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
                return new Expr_Cast(type_expr, rhs, op.location);
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
        return nullptr;
    }

    Ast_Expr *base_expr = parse_base_expr(lexer);
    if (!base_expr) {
        return nullptr;
    }

    while (is_postfix_op(lexer)) {
        Token op = {};
        if (!get_next_token(lexer, &op)) {
            return nullptr;
        }

        switch (op.kind) {
            case TK_LEFT_PAREN: {
                Array<Ast_Expr *> parameters = {};
                parameters.allocator = default_allocator();
                Token token;
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

                base_expr = new Expr_Procedure_Call(base_expr, parameters, op.location);
                break;
            }
            case TK_LEFT_SQUARE: {
                Ast_Expr *index = parse_expr(lexer);
                if (!index) {
                    return nullptr;
                }
                EXPECT(lexer, TK_RIGHT_SQUARE, nullptr);

                base_expr = new Expr_Subscript(base_expr, index, op.location);
                break;
            }
            case TK_CARET: {
                base_expr = new Expr_Dereference(base_expr, op.location);
                break;
            }
            case TK_DOT: {
                Token name_token;
                EXPECT(lexer, TK_IDENTIFIER, &name_token);
                base_expr = new Expr_Selector(base_expr, name_token.text, op.location);
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
        return nullptr;
    }

    switch (token.kind) {
        case TK_NULL: {
            eat_next_token(lexer);
            return new Expr_Null(token.location);
        }
        case TK_TRUE: {
            eat_next_token(lexer);
            return new Expr_True(token.location);
        }
        case TK_FALSE: {
            eat_next_token(lexer);
            return new Expr_False(token.location);
        }
        case TK_IDENTIFIER: {
            eat_next_token(lexer);
            Expr_Identifier *ident = new Expr_Identifier(token.text, token.location);
            g_identifiers_to_resolve.append(ident);
            return ident;
        }
        case TK_NUMBER: {
            eat_next_token(lexer);
            return new Expr_Number_Literal(token.text, token.has_a_dot, token.location);
        }
        case TK_STRING: {
            eat_next_token(lexer);
            // todo(josh): unescape the string
            return new Expr_String_Literal(token.text, token.scanner_length, token.escaped_text, token.escaped_length, token.location);
        }
        case TK_LEFT_PAREN: {
            eat_next_token(lexer);
            Ast_Expr *nested = parse_expr(lexer);
            EXPECT(lexer, TK_RIGHT_PAREN, nullptr);
            return new Expr_Paren(nested, token.location);
        }
        case TK_LEFT_SQUARE: {
            eat_next_token(lexer);
            Ast_Expr *length = parse_expr(lexer);
            if (!length) {
                return nullptr;
            }
            EXPECT(lexer, TK_RIGHT_SQUARE, nullptr);
            Ast_Expr *array_of = parse_expr(lexer);
            if (!array_of) {
                return nullptr;
            }
            return new Expr_Array_Type(array_of, length, token.location);
        }
        case TK_CARET: {
            eat_next_token(lexer);
            Ast_Expr *pointer_to = parse_expr(lexer);
            if (!pointer_to) {
                return nullptr;
            }
            return new Expr_Pointer_Type(pointer_to, token.location);
        }
        default: {
            unexpected_token(lexer, token);
            return nullptr;
        }
    }
    assert(false && "unreachable");
    return nullptr;
}
