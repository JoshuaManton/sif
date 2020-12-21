#pragma once

#include "common.h"
#include "lexer.h"
#include "basic.h"

enum Ast_Kind {
    AST_INVALID,
    AST_FILE,
    AST_BLOCK,
    AST_PROC_HEADER,
    AST_PROC,
    AST_VAR,
    AST_STRUCT,
    AST_ASSIGN,
    AST_EXPR,
    AST_STATEMENT_EXPR,
    AST_DIRECTIVE_ASSERT,
    AST_DIRECTIVE_PRINT,
};

struct Ast_Expr;
struct Ast_Var;
struct Ast_Proc;
struct Ast_Block;
struct Ast_Struct;
struct Ast_Statement_Expr;

struct Declaration;
struct Type_Declaration;
struct Struct_Declaration;
struct Proc_Declaration;
struct Var_Declaration;

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

enum Block_Flags {
    BF_IS_GLOBAL_SCOPE = 1 << 0,
};

struct Ast_Block : public Ast_Node {
    Array<Ast_Node *> nodes = {};
    Array<Declaration *> declarations = {};
    u64 flags = {};
    Ast_Block(Location location)
    : Ast_Node(AST_BLOCK, location)
    {
        nodes.allocator = default_allocator();
        declarations.allocator = default_allocator();
    }
};

struct Ast_Proc_Header : public Ast_Node {
    char *name = nullptr;
    Array<Ast_Var *> parameters = {};
    Ast_Expr *return_type_expr = {};
    Type *type = nullptr;
    Ast_Block *procedure_block = {}; // note(josh): NOT the same as the body. parameters live in this scope and it is the parent scope of the body
    Ast_Proc_Header(char *name, Ast_Block *procedure_block, Array<Ast_Var *> parameters, Ast_Expr *return_type_expr, Location location)
    : Ast_Node(AST_PROC_HEADER, location)
    , name(name)
    , procedure_block(procedure_block)
    , parameters(parameters)
    , return_type_expr(return_type_expr)
    {
        parameters.allocator = default_allocator();
    }
};

struct Ast_Proc : public Ast_Node {
    Ast_Proc_Header *header = {};
    Ast_Block *body = nullptr;
    Proc_Declaration *declaration = {};
    Ast_Proc(Ast_Proc_Header *header, Ast_Block *body, Location location)
    : Ast_Node(AST_PROC, location)
    , header(header)
    , body(body)
    {}
};

struct Ast_Assign : public Ast_Node {
    Ast_Expr *lhs = {};
    Ast_Expr *rhs = {};
    // todo(josh): +=, -=, etc
    Ast_Assign(Ast_Expr *lhs, Ast_Expr *rhs, Location location)
    : Ast_Node(AST_ASSIGN, location)
    , lhs(lhs)
    , rhs(rhs)
    {}
};

struct Ast_Directive_Assert : public Ast_Node {
    Ast_Expr *expr = {};
    Ast_Directive_Assert(Ast_Expr *expr, Location location)
    : Ast_Node(AST_DIRECTIVE_ASSERT, location)
    , expr(expr)
    {}
};

struct Ast_Directive_Print : public Ast_Node {
    Ast_Expr *expr = {};
    Ast_Directive_Print(Ast_Expr *expr, Location location)
    : Ast_Node(AST_DIRECTIVE_PRINT, location)
    , expr(expr)
    {}
};

struct Ast_Var : public Ast_Node {
    char *name = nullptr;
    Ast_Expr *type_expr = nullptr;
    Ast_Expr *expr = nullptr;
    Type *type = nullptr;
    Var_Declaration *declaration = {};
    Ast_Var(Location location)
    : Ast_Node(AST_VAR, location)
    {}
};

struct Ast_Struct : public Ast_Node {
    char *name = nullptr;
    Type *type = nullptr;
    Array<Ast_Var *> fields = {};
    Ast_Block *body = {};
    Struct_Declaration *declaration = {};
    Ast_Struct(Location location)
    : Ast_Node(AST_STRUCT, location)
    {
        fields.allocator = default_allocator();
    }
};

struct Ast_Statement_Expr : public Ast_Node {
    Ast_Expr *expr = {};
    Ast_Statement_Expr(Ast_Expr *expr, Location location)
    : Ast_Node(AST_STATEMENT_EXPR, location)
    , expr(expr)
    {}
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

    EXPR_SIZEOF,
    EXPR_TYPEOF,

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

struct Expr_Sizeof : public Ast_Expr {
    Ast_Expr *expr = {};
    Expr_Sizeof(Ast_Expr *expr, Location location)
    : Ast_Expr(EXPR_SIZEOF, location)
    , expr(expr)
    {}
};

struct Expr_Typeof : public Ast_Expr {
    Ast_Expr *expr = {};
    Expr_Typeof(Ast_Expr *expr, Location location)
    : Ast_Expr(EXPR_TYPEOF, location)
    , expr(expr)
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

enum Declaration_Check_State {
    DCS_UNCHECKED,
    DCS_CHECKING,
    DCS_CHECKED,
};

enum Operand_Flags {
    OPERAND_CONSTANT = 1 << 0,
    OPERAND_TYPE     = 1 << 1,
    OPERAND_LVALUE   = 1 << 2,
    OPERAND_RVALUE   = 1 << 3,
    OPERAND_NO_VALUE = 1 << 4, // note(josh): procedures that don't return anything
};

struct Operand {
    Location location = {};

    u64 flags = {};
    Type *type = {};

    i64 int_value      = {};
    f64 float_value    = {};
    char *string_value = {};
    bool bool_value    = {};
    Type *type_value   = {};

    Operand()
    {}

    Operand(Location location)
    : location(location)
    {}
};

struct Declaration {
    Ast_Block *parent_block = {};
    Declaration_Check_State check_state = {};
    char *name = {};
    Declaration_Kind kind = {};
    Operand operand = {};
    Declaration(char *name, Declaration_Kind kind, Ast_Block *parent_block)
    : name(name)
    , kind(kind)
    , parent_block(parent_block)
    {}
};

struct Type_Declaration : Declaration {
    Type *type = {};
    Type_Declaration(char *name, Type *type, Ast_Block *parent_block)
    : Declaration(name, DECL_TYPE, parent_block)
    , type(type)
    {}
};

struct Struct_Declaration : Declaration {
    Ast_Struct *structure = {};
    Struct_Declaration(Ast_Struct *structure, Ast_Block *parent_block)
    : Declaration(structure->name, DECL_STRUCT, parent_block)
    , structure(structure)
    {}
};

struct Proc_Declaration : Declaration {
    Ast_Proc *procedure = {};
    Proc_Declaration(Ast_Proc *procedure, Ast_Block *parent_block)
    : Declaration(procedure->header->name, DECL_PROC, parent_block)
    , procedure(procedure)
    {}
};

struct Var_Declaration : Declaration {
    Ast_Var *var = {};
    Var_Declaration(Ast_Var *var, Ast_Block *parent_block)
    : Declaration(var->name, DECL_VAR, parent_block)
    , var(var)
    {}
};

extern Array<Declaration *> g_all_declarations;
extern Array<Ast_Directive_Assert *> g_all_assert_directives;
extern Array<Ast_Directive_Print *>  g_all_print_directives;

void init_parser();
void resolve_identifiers();
void register_declaration(Declaration *new_declaration);
Ast_Expr *parse_expr(Lexer *lexer);
Ast_Var *parse_var(Lexer *lexer);
Ast_Block *parse_block(Lexer *lexer);
Ast_Node *parse_statement(Lexer *lexer);
