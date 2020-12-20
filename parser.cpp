#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>

#include "parser.h"

Array<Expr_Identifier *> g_identifiers_to_resolve;
Array<Declaration *> g_all_declarations;

void init_parser() {
    g_identifiers_to_resolve.allocator = default_allocator();
    g_all_declarations.allocator = default_allocator();
}

void queue_identifier_for_resolving(Expr_Identifier *ident) {
    g_identifiers_to_resolve.append(ident);
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
            printf("%s\n", ident->name);
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

void register_declaration(Ast_Block *block, Declaration *new_declaration) {
    For (decl_idx, block->declarations) {
        Declaration *decl = block->declarations[decl_idx];
        if (strcmp(decl->name, new_declaration->name) == 0) {
            assert(false && "name collision");
        }
    }
    block->declarations.append(new_declaration);
    g_all_declarations.append(new_declaration);
}



Ast_Var *parse_var(Lexer *lexer) {
    Token token;
    if (!expect_token(lexer, TK_VAR, &token)) {
        return nullptr;
    }
    Ast_Var *var = new Ast_Var(token.location);

    if (!expect_token(lexer, TK_IDENTIFIER, &token)) {
        return nullptr;
    }
    var->name = token.text;

    if (!expect_token(lexer, TK_COLON)) {
        return nullptr;
    }

    var->type_expr = parse_expr(lexer);
    if (!var->type_expr) {
        return nullptr;
    }

    if (peek_next_token(lexer, &token) && token.kind == TK_ASSIGN) {
        get_next_token(lexer, &token);
        var->expr = parse_expr(lexer);
    }

    register_declaration(current_block, new Var_Declaration(var));
    return var;
}

Ast_Proc *parse_proc(Lexer *lexer) {
    Token token;
    if (!expect_token(lexer, TK_PROC, &token)) {
        return nullptr;
    }
    Ast_Proc *proc = new Ast_Proc(token.location);
    proc->procedure_block = new Ast_Block(token.location); // todo(josh): this location is incorrect, it should be the location of the body
    Ast_Block *old_block = push_ast_block(proc->procedure_block);
    defer(pop_ast_block(old_block));

    // name
    if (!expect_token(lexer, TK_IDENTIFIER, &token)) {
        return nullptr;
    }
    proc->name = token.text;

    // parameter list
    if (!expect_token(lexer, TK_LEFT_PAREN)) {
        return nullptr;
    }
    bool first = true;
    while (peek_next_token(lexer, &token) && token.kind != TK_RIGHT_PAREN) {
        if (!first) {
            if (!expect_token(lexer, TK_COMMA)) {
                return nullptr;
            }
        }

        Ast_Var *var = parse_var(lexer);
        if (!var) {
            return nullptr;
        }
        proc->parameters.append(var);
        first = false;
    }
    if (!expect_token(lexer, TK_RIGHT_PAREN)) {
        return nullptr;
    }

    // body
    if (!expect_token(lexer, TK_LEFT_CURLY)) {
        return nullptr;
    }

    proc->body = parse_block(lexer);

    if (!expect_token(lexer, TK_RIGHT_CURLY)) {
        return nullptr;
    }

    register_declaration(current_block, new Proc_Declaration(proc));
    return proc;
}

Ast_Struct *parse_struct(Lexer *lexer) {
    Token token;
    if (!expect_token(lexer, TK_STRUCT, &token)) {
        return nullptr;
    }
    Ast_Struct *structure = new Ast_Struct(token.location);

    // name
    if (!expect_token(lexer, TK_IDENTIFIER, &token)) {
        return nullptr;
    }
    structure->name = token.text;

    // fields
    if (!expect_token(lexer, TK_LEFT_CURLY, &token)) {
        return nullptr;
    }

    structure->body = parse_block(lexer);
    if (!structure->body) {
        return nullptr;
    }
    if (!expect_token(lexer, TK_RIGHT_CURLY)) {
        return nullptr;
    }
    For (idx, structure->body->nodes) {
        Ast_Node *node = structure->body->nodes[idx];
        assert(node->ast_kind == AST_VAR);
        Ast_Var *var = (Ast_Var *)node;
        structure->fields.append(var);
    }

    register_declaration(current_block, new Struct_Declaration(structure));
    return structure;
}

Ast_Block *parse_block(Lexer *lexer) {
    Ast_Block *block = new Ast_Block(lexer->location);
    Ast_Block *old_block = push_ast_block(block);
    defer(pop_ast_block(old_block));

    Token token;
    while (peek_next_token(lexer, &token) && token.kind != TK_RIGHT_CURLY) {
        switch (token.kind) {
            case TK_VAR: {
                Ast_Var *var = parse_var(lexer);
                if (!var) {
                    return nullptr;
                }

                block->nodes.append(var);
                if (!expect_token(lexer, TK_SEMICOLON)) {
                    return nullptr;
                }
                break;
            }

            case TK_PROC: {
                Ast_Proc *proc = parse_proc(lexer);
                if (!proc) {
                    return nullptr;
                }

                block->nodes.append(proc);
                break;
            }

            case TK_STRUCT: {
                Ast_Struct *structure = parse_struct(lexer);
                if (!structure) {
                    return nullptr;
                }

                block->nodes.append(structure);
                break;
            }

            case TK_DIRECTIVE_ASSERT: {
                assert(expect_token(lexer, TK_DIRECTIVE_ASSERT));
                if (!expect_token(lexer, TK_LEFT_PAREN)) {
                    return nullptr;
                }
                Ast_Expr *expr = parse_expr(lexer);
                if (!expr) {
                    return nullptr;
                }
                if (!expect_token(lexer, TK_RIGHT_PAREN)) {
                    return nullptr;
                }
                block->nodes.append(new Ast_Directive_Assert(expr, token.location));
                break;
            }

            case TK_DIRECTIVE_PRINT: {
                assert(expect_token(lexer, TK_DIRECTIVE_PRINT));
                if (!expect_token(lexer, TK_LEFT_PAREN)) {
                    return nullptr;
                }
                Ast_Expr *expr = parse_expr(lexer);
                if (!expr) {
                    return nullptr;
                }
                if (!expect_token(lexer, TK_RIGHT_PAREN)) {
                    return nullptr;
                }
                block->nodes.append(new Ast_Directive_Print(expr, token.location));
                break;
            }

            default: {
                unexpected_token(lexer, token);
                return nullptr;
            }
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
                if (!expect_token(lexer, TK_LEFT_PAREN)) {
                    return nullptr;
                }
                Ast_Expr *expr = parse_expr(lexer);
                if (!expr) {
                    return nullptr;
                }
                if (!expect_token(lexer, TK_RIGHT_PAREN)) {
                    return nullptr;
                }
                return new Expr_Sizeof(expr, op.location);
            }
            case TK_TYPEOF: {
                if (!expect_token(lexer, TK_LEFT_PAREN)) {
                    return nullptr;
                }
                Ast_Expr *expr = parse_expr(lexer);
                if (!expr) {
                    return nullptr;
                }
                if (!expect_token(lexer, TK_RIGHT_PAREN)) {
                    return nullptr;
                }
                return new Expr_Typeof(expr, op.location);
            }
            // case .Cast: {
            //     expect(lexer, .LParen);
            //     typespec := parse_typespec(lexer);
            //     expect(lexer, .RParen);
            //     rhs := parse_unary_expr(lexer);
            //     expr.kind = Expr_Cast{typespec, rhs};
            // }
            // case .Size_Of: {
            //     expect(lexer, .LParen);
            //     thing_to_get_the_size_of := parse_expr(lexer);
            //     expect(lexer, .RParen);
            //     expr.kind = Expr_Size_Of{thing_to_get_the_size_of};
            // }
            // case .Type_Of: {
            //     expect(lexer, .LParen);
            //     thing_to_get_the_type_of := parse_expr(lexer);
            //     expect(lexer, .RParen);
            //     expr.kind = Expr_Type_Of{thing_to_get_the_type_of};
            // }
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
                        if (!expect_token(lexer, TK_COMMA)) {
                            return nullptr;
                        }
                    }
                    Ast_Expr *expr = parse_expr(lexer);
                    if (!expr) {
                        return nullptr;
                    }
                    parameters.append(expr);
                    first = false;
                }
                if (!expect_token(lexer, TK_RIGHT_PAREN)) {
                    return nullptr;
                }

                base_expr = new Expr_Procedure_Call(base_expr, parameters, op.location);
                break;
            }
            case TK_LEFT_SQUARE: {
                Ast_Expr *index = parse_expr(lexer);
                if (!index) {
                    return nullptr;
                }
                if (!expect_token(lexer, TK_RIGHT_SQUARE)) {
                    return nullptr;
                }

                base_expr = new Expr_Subscript(base_expr, index, op.location);
                break;
            }
            case TK_CARET: {
                base_expr = new Expr_Dereference(base_expr, op.location);
                break;
            }
            case TK_DOT: {
                Token name_token;
                if (!expect_token(lexer, TK_IDENTIFIER, &name_token)) {
                    return nullptr;
                }
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
            get_next_token(lexer, &token);
            return new Expr_Null(token.location);
        }
        case TK_TRUE: {
            get_next_token(lexer, &token);
            return new Expr_True(token.location);
        }
        case TK_FALSE: {
            get_next_token(lexer, &token);
            return new Expr_False(token.location);
        }
        case TK_IDENTIFIER: {
            get_next_token(lexer, &token);
            Expr_Identifier *ident = new Expr_Identifier(token.text, token.location);
            queue_identifier_for_resolving(ident);
            return ident;
        }
        case TK_NUMBER: {
            get_next_token(lexer, &token);
            return new Expr_Number_Literal(token.text, token.location);
        }
        case TK_STRING: {
            get_next_token(lexer, &token);
            // todo(josh): unescape the string
            return new Expr_String_Literal(token.text, token.location);
        }
        case TK_LEFT_PAREN: {
            get_next_token(lexer, &token);
            Ast_Expr *nested = parse_expr(lexer);
            if (!expect_token(lexer, TK_RIGHT_PAREN)) {
                return nullptr;
            }
            return new Expr_Paren(nested, token.location);
        }
        case TK_LEFT_SQUARE: {
            get_next_token(lexer, &token);
            Ast_Expr *length = parse_expr(lexer);
            if (!length) {
                return nullptr;
            }
            if (!expect_token(lexer, TK_RIGHT_SQUARE)) {
                return nullptr;
            }
            Ast_Expr *array_of = parse_expr(lexer);
            if (!array_of) {
                return nullptr;
            }
            return new Expr_Array_Type(array_of, length, token.location);
        }
        case TK_CARET: {
            get_next_token(lexer, &token);
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
