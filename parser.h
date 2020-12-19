#pragma once

#include "common.h"
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
struct Ast_Var;
struct Ast_Proc;
struct Ast_Block;
struct Ast_Struct;

struct Declaration;

struct Type;

static Ast_Block *current_block;

struct Ast_Node {
    Ast_Block *parent_block = {};
    Ast_Kind ast_kind = AST_INVALID;
    Location location = {};
    Ast_Node(Ast_Kind ast_kind, Location location)
    : ast_kind(ast_kind)
    , parent_block(current_block)
    , location(location)
    {}
};

struct Ast_Block : public Ast_Node {
    Array<Ast_Node *> nodes = {};
    Array<Declaration *> declarations = {};

    Ast_Block(Location location)
    : Ast_Node(AST_BLOCK, location)
    {
        nodes.allocator = default_allocator();
        declarations.allocator = default_allocator();
    }
};

struct Ast_Proc : public Ast_Node {
    char *name = nullptr;
    Ast_Block *procedure_block = {}; // note(josh): NOT the same as the body. parameters live in this scope and it is the parent scope of the body
    Array<Ast_Var *> parameters = {};
    Ast_Block *body = nullptr;
    Type *type = nullptr;
    Ast_Proc(Location location)
    : Ast_Node(AST_PROC, location)
    {
        parameters.allocator = default_allocator();
    }
};

struct Ast_Var : public Ast_Node {
    char *name = nullptr;
    Ast_Expr *type_expr = nullptr;
    Ast_Expr *expr = nullptr;
    Type *type = nullptr;
    Ast_Var(Location location)
    : Ast_Node(AST_VAR, location)
    {}
};

struct Ast_Struct : public Ast_Node {
    char *name = nullptr;
    Type *type = nullptr;
    Array<Ast_Var *> fields = {};
    Ast_Block *body = {};
    Ast_Struct(Location location)
    : Ast_Node(AST_STRUCT, location)
    {
        fields.allocator = default_allocator();
    }
};



enum Expr_Kind {
    EXPR_INVALID,
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_ADDRESS_OF,

    EXPR_SUBSCRIPT,
    EXPR_DEREFERENCE,
    EXPR_PROCEDURE_CALL,
    EXPR_SELECTOR,

    EXPR_IDENTIFIER,
    EXPR_NUMBER_LITERAL,
    EXPR_STRING_LITERAL,

    EXPR_NULL,
    EXPR_TRUE,
    EXPR_FALSE,

    EXPR_POINTER_TYPE,
    EXPR_ARRAY_TYPE,

    EXPR_PAREN,

    EXPR_COUNT,
};

struct Ast_Expr : public Ast_Node {
    Expr_Kind expr_kind = EXPR_INVALID;
    Ast_Expr(Expr_Kind kind, Location location)
    : Ast_Node(AST_EXPR, location)
    , expr_kind(kind)
    {}
};

struct Expr_Binary : public Ast_Expr {
    Token_Kind op = TK_INVALID;
    Ast_Expr *lhs = nullptr;
    Ast_Expr *rhs = nullptr;
    Expr_Binary(Token_Kind op, Ast_Expr *lhs, Ast_Expr *rhs)
    : Ast_Expr(EXPR_BINARY, lhs->location)
    , op(op)
    , lhs(lhs)
    , rhs(rhs)
    {}
};

struct Expr_Address_Of : public Ast_Expr {
    Ast_Expr *rhs = nullptr;
    Expr_Address_Of(Ast_Expr *rhs, Location location)
    : Ast_Expr(EXPR_ADDRESS_OF, location)
    , rhs(rhs)
    {}
};

struct Expr_Unary : public Ast_Expr {
    Token_Kind op = TK_INVALID;
    Ast_Expr *rhs = nullptr;
    Expr_Unary(Token_Kind op, Ast_Expr *rhs, Location location)
    : Ast_Expr(EXPR_UNARY, location)
    , op(op)
    , rhs(rhs)
    {}
};

struct Expr_Identifier : public Ast_Expr {
    char *name = nullptr;
    Declaration *resolved_declaration = nullptr;
    Expr_Identifier(char *name, Location location)
    : Ast_Expr(EXPR_IDENTIFIER, location)
    , name(name)
    {}
};

struct Expr_Number_Literal : public Ast_Expr {
    char *number_string = nullptr;
    Expr_Number_Literal(char *number_string, Location location)
    : Ast_Expr(EXPR_NUMBER_LITERAL, location)
    , number_string(number_string)
    {}
};

struct Expr_String_Literal : public Ast_Expr {
    char *text = nullptr;
    Expr_String_Literal(char *text, Location location)
    : Ast_Expr(EXPR_STRING_LITERAL, location)
    , text(text)
    {}
};

struct Expr_Subscript : public Ast_Expr {
    Ast_Expr *lhs = nullptr;
    Ast_Expr *index = nullptr;
    Expr_Subscript(Ast_Expr *lhs, Ast_Expr *index, Location location)
    : Ast_Expr(EXPR_SUBSCRIPT, location)
    , lhs(lhs)
    , index(index)
    {}
};

struct Expr_Dereference : public Ast_Expr {
    Ast_Expr *lhs = nullptr;
    Expr_Dereference(Ast_Expr *lhs, Location location)
    : Ast_Expr(EXPR_DEREFERENCE, location)
    , lhs(lhs)
    {}
};

struct Expr_Procedure_Call : public Ast_Expr {
    Ast_Expr *lhs = nullptr;
    Array<Ast_Expr *> parameters = {};
    Expr_Procedure_Call(Ast_Expr *lhs, Array<Ast_Expr *> parameters, Location location)
    : Ast_Expr(EXPR_PROCEDURE_CALL, location)
    , lhs(lhs)
    , parameters(parameters)
    {}
};

struct Expr_Selector : public Ast_Expr {
    Ast_Expr *lhs = nullptr;
    char *field_name = nullptr;
    Expr_Selector(Ast_Expr *lhs, char *field_name, Location location)
    : Ast_Expr(EXPR_SELECTOR, location)
    , lhs(lhs)
    , field_name(field_name)
    {}
};

struct Expr_Paren : public Ast_Expr {
    Ast_Expr *nested = nullptr;
    Expr_Paren(Ast_Expr *nested, Location location)
    : Ast_Expr(EXPR_PAREN, location)
    , nested(nested)
    {}
};

struct Expr_Null : public Ast_Expr {
    Expr_Null(Location location)
    : Ast_Expr(EXPR_NULL, location)
    {}
};

struct Expr_True : public Ast_Expr {
    Expr_True(Location location)
    : Ast_Expr(EXPR_TRUE, location)
    {}
};

struct Expr_False : public Ast_Expr {
    Expr_False(Location location)
    : Ast_Expr(EXPR_FALSE, location)
    {}
};

struct Expr_Pointer_Type : public Ast_Expr {
    Ast_Expr *pointer_to = nullptr;
    Expr_Pointer_Type(Ast_Expr *pointer_to, Location location)
    : Ast_Expr(EXPR_POINTER_TYPE, location)
    , pointer_to(pointer_to)
    {}
};

struct Expr_Array_Type : public Ast_Expr {
    Ast_Expr *array_of = nullptr;
    Ast_Expr *count_expr = nullptr;
    Expr_Array_Type(Ast_Expr *array_of, Ast_Expr *count_expr, Location location)
    : Ast_Expr(EXPR_ARRAY_TYPE, location)
    , array_of(array_of)
    , count_expr(count_expr)
    {}
};

enum Declaration_Kind {
    DECL_INVALID,
    DECL_TYPE,
    DECL_STRUCT,
    DECL_VAR,
    DECL_PROC,
    DECL_COUNT,
};

struct Declaration {
    char *name = {};
    Declaration_Kind kind = {};
    Declaration(char *name, Declaration_Kind kind)
    : name(name)
    , kind(kind)
    {}
};

struct Type_Declaration : Declaration {
    Type *type = {};
    Type_Declaration(char *name, Type *type)
    : Declaration(name, DECL_TYPE)
    , type(type)
    {}
};

struct Struct_Declaration : Declaration {
    Ast_Struct *structure = {};
    Struct_Declaration(Ast_Struct *structure)
    : Declaration(structure->name, DECL_STRUCT)
    , structure(structure)
    {}
};

struct Proc_Declaration : Declaration {
    Ast_Proc *procedure = {};
    Proc_Declaration(Ast_Proc *procedure)
    : Declaration(procedure->name, DECL_PROC)
    , procedure(procedure)
    {}
};

struct Var_Declaration : Declaration {
    Ast_Var *var = {};
    Var_Declaration(Ast_Var *var)
    : Declaration(var->name, DECL_VAR)
    , var(var)
    {}
};

void init_parser();
void resolve_identifiers();
Ast_Expr *parse_expr(Lexer *lexer);
Ast_Var *parse_var(Lexer *lexer);
Ast_Block *parse_block(Lexer *lexer);
Ast_Node *parse_statement(Lexer *lexer);
