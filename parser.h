#pragma once

#include "lexer.h"
#include "basic.h"

enum Ast_Kind {
    AST_INVALID,
    AST_FILE,
    AST_BLOCK,
    AST_PROC,
    AST_VAR,
    AST_STRUCT,
    AST_EXPR,
};

struct Ast_Expr;

struct Ast_Node {
    Ast_Kind kind = AST_INVALID;
    Ast_Node(Ast_Kind kind) : kind(kind) {}
};

struct Ast_File : public Ast_Node {
    Ast_File() : Ast_Node(AST_FILE) {}
};

struct Ast_Block : public Ast_Node {
    Array<Ast_Node *> nodes = {};
    Ast_Block() : Ast_Node(AST_BLOCK) {
        nodes.allocator = default_allocator();
    }
};

struct Ast_Proc : public Ast_Node {
    Ast_Proc() : Ast_Node(AST_PROC) {}
};

struct Ast_Var : public Ast_Node {
    char *name = nullptr;
    char *type_name = nullptr;
    Ast_Expr *expr = nullptr;
    Ast_Var() : Ast_Node(AST_VAR) {}
};

struct Ast_Struct : public Ast_Node {
    Ast_Struct() : Ast_Node(AST_STRUCT) {}
};



enum Expr_Kind {
    EXPR_INVALID,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_ADDRESS_OF,

    EXPR_IDENTIFIER,
    EXPR_NUMBER_LITERAL,
    EXPR_STRING_LITERAL,

    EXPR_NULL,
    EXPR_TRUE,
    EXPR_FALSE,

    EXPR_PAREN,

    EXPR_COUNT,
};

struct Ast_Expr : public Ast_Node {
    Expr_Kind expr_kind = EXPR_INVALID;
    Ast_Expr(Expr_Kind kind)
    : Ast_Node(AST_EXPR)
    , expr_kind(kind)
    {}
};

struct Expr_Binary : public Ast_Expr {
    Token_Kind op = TK_INVALID;
    Ast_Expr *lhs = nullptr;
    Ast_Expr *rhs = nullptr;
    Expr_Binary(Token_Kind op, Ast_Expr *lhs, Ast_Expr *rhs)
    : Ast_Expr(EXPR_BINARY)
    , op(op)
    , lhs(lhs)
    , rhs(rhs)
    {}
};

struct Expr_Address_Of : public Ast_Expr {
    Ast_Expr *rhs = nullptr;
    Expr_Address_Of(Ast_Expr *rhs)
    : Ast_Expr(EXPR_ADDRESS_OF)
    , rhs(rhs)
    {}
};

struct Expr_Unary : public Ast_Expr {
    Token_Kind op = TK_INVALID;
    Ast_Expr *rhs = nullptr;
    Expr_Unary(Token_Kind op, Ast_Expr *rhs)
    : Ast_Expr(EXPR_UNARY)
    , op(op)
    , rhs(rhs)
    {}
};

struct Expr_Identifier : public Ast_Expr {
    char *name = nullptr;
    Expr_Identifier(char *name)
    : Ast_Expr(EXPR_IDENTIFIER)
    , name(name)
    {}
};

struct Expr_Number_Literal : public Ast_Expr {
    char *number_string = nullptr;
    Expr_Number_Literal(char *number_string)
    : Ast_Expr(EXPR_NUMBER_LITERAL)
    , number_string(number_string)
    {}
};

struct Expr_String_Literal : public Ast_Expr {
    char *text = nullptr;
    Expr_String_Literal(char *text)
    : Ast_Expr(EXPR_STRING_LITERAL)
    , text(text)
    {}
};

struct Expr_Paren : public Ast_Expr {
    Ast_Expr *nested = nullptr;
    Expr_Paren(Ast_Expr *nested)
    : Ast_Expr(EXPR_PAREN)
    , nested(nested)
    {}
};

struct Expr_Null : public Ast_Expr {
    Expr_Null()
    : Ast_Expr(EXPR_NULL)
    {}
};

struct Expr_True : public Ast_Expr {
    Expr_True()
    : Ast_Expr(EXPR_TRUE)
    {}
};

struct Expr_False : public Ast_Expr {
    Expr_False()
    : Ast_Expr(EXPR_FALSE)
    {}
};

Ast_Expr *parse_expr(Lexer *lexer);
Ast_Var *parse_var(Lexer *lexer);
Ast_Block *parse_block(Lexer *lexer);
Ast_Node *parse_statement(Lexer *lexer);