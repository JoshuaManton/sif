#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>

#include "parser.h"

Ast_Var *parse_var(Lexer *lexer) {
    if (!expect_token(lexer, TK_VAR)) {
        return nullptr;
    }
    Ast_Var *var = new Ast_Var;
    Token token;

    if (!expect_token(lexer, TK_IDENTIFIER, &token)) {
        return nullptr;
    }
    var->name = token.text;

    if (!expect_token(lexer, TK_COLON)) {
        return nullptr;
    }

    if (!expect_token(lexer, TK_IDENTIFIER, &token)) {
        return nullptr;
    }
    var->type_name = token.text;

    if (peek_next_token(lexer, &token) && token.kind == TK_ASSIGN) {
        get_next_token(lexer, &token);
        var->expr = parse_expr(lexer);
    }

    return var;
}

Ast_Block *parse_block(Lexer *lexer) {
    Ast_Block *block = new Ast_Block;
    Token token;
    while (peek_next_token(lexer, &token) && token.kind != TK_RIGHT_CURLY) {
        switch (token.kind) {
            case TK_VAR: {
                Ast_Var *var = parse_var(lexer);
                block->nodes.append(var);
                if (!expect_token(lexer, TK_SEMICOLON)) {
                    printf("Missing semicolon\n");
                    return nullptr;
                }
                break;
            }
            case AST_PROC: {
                break;
            }
            default: {
                printf("Unexpected token: %s\n", token_string(token.kind));
                assert(false);
            }
        }
    }
    return block;
}

Ast_Node *parse_statement(Lexer *lexer) {
    return nullptr;
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
    while (is_or_op(lexer)) {
        Token op = {};
        if (!get_next_token(lexer, &op)) {
            return nullptr;
        }
        Ast_Expr *lhs = expr;
        Ast_Expr *rhs = parse_and_expr(lexer);
        expr = new Expr_Binary(op.kind, lhs, rhs);
    }
    return expr;
}

Ast_Expr *parse_and_expr(Lexer *lexer) {
    Ast_Expr *expr = parse_cmp_expr(lexer);
    while (is_and_op(lexer)) {
        Token op = {};
        if (!get_next_token(lexer, &op)) {
            return nullptr;
        }
        Ast_Expr *lhs = expr;
        Ast_Expr *rhs = parse_cmp_expr(lexer);
        expr = new Expr_Binary(op.kind, lhs, rhs);
    }
    return expr;
}

Ast_Expr *parse_cmp_expr(Lexer *lexer) {
    Ast_Expr *expr = parse_add_expr(lexer);
    while (is_cmp_op(lexer)) {
        Token op = {};
        if (!get_next_token(lexer, &op)) {
            return nullptr;
        }
        Ast_Expr *lhs = expr;
        Ast_Expr *rhs = parse_add_expr(lexer);
        expr = new Expr_Binary(op.kind, lhs, rhs);
    }
    return expr;
}

Ast_Expr *parse_add_expr(Lexer *lexer) {
    Ast_Expr *expr = parse_mul_expr(lexer);
    while (is_add_op(lexer)) {
        Token op = {};
        if (!get_next_token(lexer, &op)) {
            return nullptr;
        }
        Ast_Expr *lhs = expr;
        Ast_Expr *rhs = parse_mul_expr(lexer);
        expr = new Expr_Binary(op.kind, lhs, rhs);
        printf("%d\n", ((Expr_Binary *)expr)->op);
    }
    return expr;
}

Ast_Expr *parse_mul_expr(Lexer *lexer) {
    Ast_Expr *expr = parse_unary_expr(lexer);
    while (is_mul_op(lexer)) {
        Token op = {};
        if (!get_next_token(lexer, &op)) {
            return nullptr;
        }
        Ast_Expr *lhs = expr;
        Ast_Expr *rhs = parse_unary_expr(lexer);
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
                return new Expr_Address_Of(rhs);
            }
            case TK_MINUS:
            case TK_PLUS:
            case TK_NOT: {
                Ast_Expr *rhs = parse_unary_expr(lexer);
                return new Expr_Unary(op.kind, rhs);
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

    while (is_postfix_op(lexer)) {
        Token op = {};
        if (!get_next_token(lexer, &op)) {
            return nullptr;
        }

        switch (op.kind) {
            case TK_LEFT_PAREN: {
                // proc call
                // params: [dynamic]^Ast_Expr;
                // first := true;
                // for {
                //     defer first = false;
                //     token, ok := peek(lexer);
                //     assert(ok);
                //     if token.kind != .RParen {
                //         if !first do expect(lexer, .Comma);
                //         param := parse_expr(lexer);
                //         append(&params, param);
                //     }
                //     else {
                //         break;
                //     }
                // }
                // expect(lexer, .RParen);

                // call := make_node(Ast_Expr);
                // call.kind = Expr_Call{base_expr, params[:]};
                // base_expr = call;
                assert(false);
                return nullptr;
            }
            case TK_LEFT_SQUARE: {
                // index := parse_expr(lexer);
                // expect(lexer, .RSquare);
                // subscript := make_node(Ast_Expr);
                // subscript.kind = Expr_Subscript{base_expr, index};
                // base_expr = subscript;
                assert(false);
                return nullptr;
            }
            case TK_CARET: {
                // deref := make_node(Ast_Expr);
                // deref.kind = Expr_Dereference{base_expr};
                // base_expr = deref;
                assert(false);
                return nullptr;
            }
            case TK_DOT: {
                // name := expect(lexer, .Identifier);
                // assert(ok);
                // ident := make_node(Ast_Identifier);
                // ident.name = name.slice;
                // selector := make_node(Ast_Expr);
                // selector.kind = Expr_Selector{base_expr, ident};
                // base_expr = selector;
                assert(false);
                return nullptr;
            }
            default: {
                assert(false);
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
            return new Expr_Null;
        }
        case TK_TRUE: {
            get_next_token(lexer, &token);
            return new Expr_True;
        }
        case TK_FALSE: {
            get_next_token(lexer, &token);
            return new Expr_False;
        }
        case TK_IDENTIFIER: {
            get_next_token(lexer, &token);
            // todo(josh): queue for resolving
            return new Expr_Identifier(token.text);
        }
        case TK_NUMBER: {
            get_next_token(lexer, &token);
            return new Expr_Number_Literal(token.text);
        }
        case TK_STRING: {
            get_next_token(lexer, &token);
            // todo(josh): unescape the string
            return new Expr_String_Literal(token.text);
        }
        case TK_LEFT_PAREN: {
            get_next_token(lexer, &token);
            Ast_Expr *nested = parse_expr(lexer);
            if (!expect_token(lexer, TK_RIGHT_PAREN)) {
                assert(false);
                return nullptr;
            }
            return new Expr_Paren(nested);
        }
        case TK_LEFT_SQUARE: {
            assert(false && "todo(josh): parse typespecs");
        }
        case TK_CARET: {
            assert(false && "todo(josh): parse typespecs");
        }
        default: {
            assert(false);
        }
    }
    assert(false && "unreachable");
    return nullptr;
}
