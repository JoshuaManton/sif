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
    AST_USING,
    AST_DEFER,
    AST_STRUCT,
    AST_ENUM,
    AST_IF,
    AST_FOR_LOOP,
    AST_WHILE_LOOP,
    AST_RETURN,
    AST_BREAK,
    AST_CONTINUE,
    AST_ASSIGN,
    AST_EXPR,
    AST_EMPTY_STATEMENT,
    AST_STATEMENT_EXPR,
    AST_BLOCK_STATEMENT,
    AST_DIRECTIVE_INCLUDE,
    AST_DIRECTIVE_ASSERT,
    AST_DIRECTIVE_PRINT,
    AST_DIRECTIVE_C_CODE,
    AST_DIRECTIVE_FOREIGN_IMPORT,
};

struct Ast_Expr;
struct Ast_Var;
struct Ast_Proc;
struct Ast_Block;
struct Ast_Struct;
struct Ast_Statement_Expr;
struct Ast_Defer;
struct Ast_Proc_Header;

struct Declaration;
struct Type_Declaration;
struct Struct_Declaration;
struct Struct_Member_Declaration;
struct Enum_Declaration;
struct Proc_Declaration;
struct Var_Declaration;
struct Polymorphic_Declaration;

struct Type;
struct Type_Procedure;
struct Type_Struct;
struct Type_Array;
struct Type_Enum;

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

    u64 uint_value   = {};
    i64 int_value    = {};
    f64 float_value  = {};
    bool bool_value  = {};
    Type *type_value = {};
    Ast_Proc_Header *proc_value = {};
    char *scanned_string_value = {};
    int scanned_string_length = {};
    char *escaped_string_value = {};
    int escaped_string_length = {};
    Declaration *referenced_declaration = {};

    Type *reference_type = {};

    Operand()
    {}

    Operand(Location location)
    : location(location)
    {}
};

struct Ast_Node;

struct Node_Polymorph_Parameter {
    bool is_polymorphic_value = {};
    bool is_polymorphic_type = {};

    Operand value = {};
    Type *type = {};
};

struct Node_Polymorph {
    Ast_Node *polymorphed_node = {};
    Array<Node_Polymorph_Parameter> polymorph_values = {};
};

struct Ast_Node {
    Ast_Block *parent_block = {};
    Ast_Kind ast_kind = AST_INVALID;
    Location location = {};
    Array<Node_Polymorph> polymorphs = {};
    Ast_Node(Ast_Kind ast_kind, Ast_Block *current_block, Location location)
    : ast_kind(ast_kind)
    , parent_block(current_block)
    , location(location)
    {
        polymorphs.allocator = g_global_linear_allocator;
    }
};

enum Block_Flags {
    BF_IS_GLOBAL_SCOPE = 1 << 0,
};

struct Ast_Block : public Ast_Node {
    Array<Ast_Node *> nodes = {};
    Array<Declaration *> declarations = {};
    u64 flags = {};
    Hashtable<const char *, Declaration *> declarations_lookup = {};
    Array<Ast_Defer *> c_gen_defer_stack = {};
    Ast_Block(Ast_Block *current_block, Location location)
    : Ast_Node(AST_BLOCK, current_block, location)
    {
        nodes.allocator = g_global_linear_allocator;
        declarations.allocator = g_global_linear_allocator;
        declarations_lookup = make_hashtable<const char *, Declaration *>(g_global_linear_allocator, 16);
        c_gen_defer_stack.allocator = g_global_linear_allocator;
    }
};

struct Ast_Proc_Header : public Ast_Node {
    char *name = nullptr;
    Array<Ast_Var *> parameters = {};
    Ast_Expr *return_type_expr = {};
    Type_Procedure *type = nullptr;
    Ast_Block *procedure_block = {}; // note(josh): NOT the same as the body. parameters live in this scope and it is the parent scope of the body
    Ast_Struct *lives_in_struct = {}; // null if proc doesn't live in a struct
    bool is_foreign = {};
    Operand operand = {};
    Token_Kind operator_to_overload = {};
    Ast_Struct *struct_to_operator_overload = {};
    bool is_polymorphic = {};
    Array<int> polymorphic_parameter_indices = {};
    Ast_Proc *procedure = {};
    Proc_Declaration *declaration = {};
    Ast_Node *current_parsing_loop = {};
    Array<Ast_Defer *> defers = {};
    Ast_Proc_Header(char *name, Ast_Block *procedure_block, Array<Ast_Var *> parameters, Ast_Expr *return_type_expr, bool is_foreign, Token_Kind operator_to_overload, Array<int> polymorphic_parameter_indices, Ast_Block *current_block, Location location)
    : Ast_Node(AST_PROC_HEADER, current_block, location)
    , name(name)
    , procedure_block(procedure_block)
    , parameters(parameters)
    , return_type_expr(return_type_expr)
    , is_foreign(is_foreign)
    , operator_to_overload(operator_to_overload)
    , is_polymorphic(polymorphic_parameter_indices.count > 0)
    , polymorphic_parameter_indices(polymorphic_parameter_indices)
    {
        parameters.allocator = g_global_linear_allocator;
        defers.allocator = g_global_linear_allocator;
    }
};

struct Ast_Proc : public Ast_Node {
    Ast_Proc_Header *header = {};
    Ast_Block *body = nullptr;
    Ast_Proc(Ast_Proc_Header *header, Ast_Block *body, Ast_Block *current_block, Location location)
    : Ast_Node(AST_PROC, current_block, location)
    , header(header)
    , body(body)
    {}
};

struct Ast_Assign : public Ast_Node {
    Token_Kind op = {};
    Ast_Expr *lhs = {};
    Ast_Expr *rhs = {};
    Ast_Assign(Token_Kind op, Ast_Expr *lhs, Ast_Expr *rhs, Ast_Block *current_block, Location location)
    : Ast_Node(AST_ASSIGN, current_block, location)
    , op(op)
    , lhs(lhs)
    , rhs(rhs)
    {}
};

struct Ast_Directive_Include : public Ast_Node {
    char *filename = {};
    Ast_Directive_Include(char *filename, Ast_Block *current_block, Location location)
    : Ast_Node(AST_DIRECTIVE_INCLUDE, current_block, location)
    , filename(filename)
    {}
};

struct Ast_Directive_Assert : public Ast_Node {
    Ast_Expr *expr = {};
    Ast_Directive_Assert(Ast_Expr *expr, Ast_Block *current_block, Location location)
    : Ast_Node(AST_DIRECTIVE_ASSERT, current_block, location)
    , expr(expr)
    {}
};

struct Ast_Directive_Print : public Ast_Node {
    Ast_Expr *expr = {};
    Ast_Directive_Print(Ast_Expr *expr, Ast_Block *current_block, Location location)
    : Ast_Node(AST_DIRECTIVE_PRINT, current_block, location)
    , expr(expr)
    {}
};

struct Ast_Directive_C_Code : public Ast_Node {
    char *text = {};
    Ast_Directive_C_Code(char *text, Ast_Block *current_block, Location location)
    : Ast_Node(AST_DIRECTIVE_C_CODE, current_block, location)
    , text(text)
    {}
};

struct Ast_Directive_Foreign_Import : public Ast_Node {
    const char *name = {};
    const char *path = {};
    Ast_Directive_Foreign_Import(const char *name, const char *path, Ast_Block *current_block, Location location)
    : Ast_Node(AST_DIRECTIVE_FOREIGN_IMPORT, current_block, location)
    , name(name)
    , path(path)
    {}
};

struct Ast_Var : public Ast_Node {
    char *name = {};
    Ast_Expr *name_expr = {};
    Ast_Expr *type_expr = {};
    Ast_Expr *expr = {};
    Type *type = {};
    bool is_constant = {};
    Var_Declaration *declaration = {};
    Operand constant_operand = {};
    bool is_polymorphic_value = {};
    bool is_polymorphic_type = {};
    bool is_using = {};
    Ast_Struct *belongs_to_struct = {}; // note(josh): may be null
    Struct_Member_Declaration *struct_member = {}; // note(josh): only set if belongs_to_struct is set
    Ast_Var(char *name, Ast_Expr *name_expr, Ast_Expr *type_expr, Ast_Expr *expr, bool is_constant, bool is_polymorphic_value, bool is_polymorphic_type, Ast_Block *current_block, Location location)
    : Ast_Node(AST_VAR, current_block, location)
    , name(name)
    , name_expr(name_expr)
    , type_expr(type_expr)
    , expr(expr)
    , is_constant(is_constant)
    , is_polymorphic_value(is_polymorphic_value)
    , is_polymorphic_type(is_polymorphic_type)
    {}
};

struct Ast_Using : public Ast_Node {
    Ast_Expr *expr = {};
    Ast_Using(Ast_Expr *expr, Ast_Block *current_block, Location location)
    : Ast_Node(AST_USING, current_block, location)
    , expr(expr)
    {}
};

struct Ast_Defer : public Ast_Node {
    Ast_Node *node_to_defer = {};
    Ast_Defer(Ast_Node *node_to_defer, Ast_Block *current_block, Location location)
    : Ast_Node(AST_DEFER, current_block, location)
    , node_to_defer(node_to_defer)
    {}
};

struct Ast_Struct : public Ast_Node {
    bool is_union = {};
    char *name = nullptr;
    Type_Struct *type = nullptr;
    Ast_Block *body = {};
    Array<Ast_Var *> fields = {}; // todo(josh): delete this and just use the variables/constants block
    Ast_Block *struct_block = {};
    Struct_Declaration *declaration = {};
    Array<Ast_Proc *> operator_overloads = {};
    Array<Ast_Proc *> procedures = {}; // todo(josh): maybe combine this with the operator_overloads above?
    Array<Ast_Var *> polymorphic_parameters = {};
    Ast_Struct(bool is_union, Ast_Block *current_block, Location location)
    : Ast_Node(AST_STRUCT, current_block, location)
    , is_union(is_union)
    {
        fields.allocator = g_global_linear_allocator;
        operator_overloads.allocator = g_global_linear_allocator;
        polymorphic_parameters.allocator = g_global_linear_allocator;
        procedures.allocator = g_global_linear_allocator;
    }
};

struct Ast_If : public Ast_Node {
    Ast_Expr *condition = {};
    Ast_Block *body = {};
    Ast_Block *else_body = {};
    Ast_If(Ast_Expr *condition, Ast_Block *body, Ast_Block *else_body, Ast_Block *current_block, Location location)
    : Ast_Node(AST_IF, current_block, location)
    , condition(condition)
    , body(body)
    , else_body(else_body)
    {}
};

struct Ast_For_Loop : public Ast_Node {
    Ast_Node *pre = {};
    Ast_Expr *condition = {};
    Ast_Node *post = {};
    Ast_Block *body = {};
    Ast_For_Loop(Ast_Block *current_block, Location location)
    : Ast_Node(AST_FOR_LOOP, current_block, location)
    {}
};

struct Ast_While_Loop : public Ast_Node {
    Ast_Expr *condition = {};
    Ast_Block *body = {};
    Ast_While_Loop(Ast_Block *current_block, Location location)
    : Ast_Node(AST_WHILE_LOOP, current_block, location)
    {}
};

struct Enum_Field {
    char *name = {};
    Ast_Expr *expr = {};
    bool resolved = {};
    Location location = {};
};
struct Ast_Enum : public Ast_Node {
    char *name = {};
    Ast_Block *enum_block = {};
    Array<Enum_Field> fields = {};
    Type_Enum *type = nullptr;
    Enum_Declaration *declaration = {};
    Ast_Expr *base_type_expr = {};
    Ast_Enum(char *name, Ast_Block *current_block, Location location)
    : Ast_Node(AST_ENUM, current_block, location)
    , name(name)
    {}
};

struct Ast_Return : public Ast_Node {
    Ast_Proc_Header *matching_procedure = {};
    Ast_Expr *expr = {};
    Ast_Return(Ast_Proc_Header *matching_procedure, Ast_Expr *expr, Ast_Block *current_block, Location location)
    : Ast_Node(AST_RETURN, current_block, location)
    , matching_procedure(matching_procedure)
    , expr(expr)
    {}
};

struct Ast_Break : public Ast_Node {
    Ast_Node *matching_loop = {};
    Ast_Break(Ast_Node *matching_loop, Ast_Block *current_block, Location location)
    : Ast_Node(AST_BREAK, current_block, location)
    , matching_loop(matching_loop)
    {}
};

struct Ast_Continue : public Ast_Node {
    Ast_Node *matching_loop = {};
    Ast_Continue(Ast_Node *matching_loop, Ast_Block *current_block, Location location)
    : Ast_Node(AST_CONTINUE, current_block, location)
    , matching_loop(matching_loop)
    {}
};

struct Ast_Statement_Expr : public Ast_Node {
    Ast_Expr *expr = {};
    Ast_Statement_Expr(Ast_Expr *expr, Ast_Block *current_block, Location location)
    : Ast_Node(AST_STATEMENT_EXPR, current_block, location)
    , expr(expr)
    {}
};

struct Ast_Block_Statement : public Ast_Node {
    Ast_Block *block = {};
    Ast_Block_Statement(Ast_Block *block, Ast_Block *current_block, Location location)
    : Ast_Node(AST_BLOCK_STATEMENT, current_block, location)
    , block(block)
    {}
};

struct Ast_Empty_Statement : public Ast_Node {
    Ast_Empty_Statement(Ast_Block *current_block, Location location)
    : Ast_Node(AST_EMPTY_STATEMENT, current_block, location)
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
    EXPR_IMPLICIT_ENUM_SELECTOR,

    EXPR_COMPOUND_LITERAL,

    EXPR_IDENTIFIER,
    EXPR_NUMBER_LITERAL,
    EXPR_STRING_LITERAL,
    EXPR_CHAR_LITERAL,

    EXPR_POLYMORPHIC_VARIABLE,

    EXPR_NULL,
    EXPR_TRUE,
    EXPR_FALSE,

    EXPR_SIZEOF,
    EXPR_TYPEOF,
    EXPR_CAST,
    EXPR_TRANSMUTE,

    EXPR_SPREAD,

    EXPR_POLYMORPHIC_TYPE,
    EXPR_POINTER_TYPE,
    EXPR_REFERENCE_TYPE,
    EXPR_ARRAY_TYPE,
    EXPR_SLICE_TYPE,
    EXPR_PROCEDURE_TYPE,
    EXPR_STRUCT_TYPE,

    EXPR_PAREN,

    EXPR_COUNT,
};


struct Procedure_Call_Parameter {
    Array<Ast_Expr *> exprs = {}; // this is an array because varargs
};
struct Ast_Expr : public Ast_Node {
    Expr_Kind expr_kind = EXPR_INVALID;
    Operand operand = {};
    Ast_Proc *desugared_procedure_to_call = {};
    Array<Procedure_Call_Parameter> processed_procedure_call_parameters = {};
    Ast_Expr(Expr_Kind kind, Ast_Block *current_block, Location location)
    : Ast_Node(AST_EXPR, current_block, location)
    , expr_kind(kind)
    {}
};

struct Expr_Binary : public Ast_Expr {
    Token_Kind op = TK_INVALID;
    Ast_Expr *lhs = nullptr;
    Ast_Expr *rhs = nullptr;
    Expr_Binary(Token_Kind op, Ast_Expr *lhs, Ast_Expr *rhs, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_BINARY, current_block, location)
    , op(op)
    , lhs(lhs)
    , rhs(rhs)
    {}
};

struct Expr_Address_Of : public Ast_Expr {
    Ast_Expr *rhs = nullptr;
    Expr_Address_Of(Ast_Expr *rhs, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_ADDRESS_OF, current_block, location)
    , rhs(rhs)
    {}
};

struct Expr_Unary : public Ast_Expr {
    Token_Kind op = TK_INVALID;
    Ast_Expr *rhs = nullptr;
    Expr_Unary(Token_Kind op, Ast_Expr *rhs, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_UNARY, current_block, location)
    , op(op)
    , rhs(rhs)
    {}
};

struct Expr_Identifier : public Ast_Expr {
    char *name = nullptr;
    Declaration *resolved_declaration = nullptr;
    Expr_Identifier(char *name, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_IDENTIFIER, current_block, location)
    , name(name)
    {}
};

struct Expr_Number_Literal : public Ast_Expr {
    u64 uint_value = {};
    i64 int_value = {};
    f64 float_value = {};
    bool has_a_dot = false;
    Expr_Number_Literal(u64 uint_value, i64 int_value, f64 float_value, bool has_a_dot, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_NUMBER_LITERAL, current_block, location)
    , uint_value(uint_value)
    , int_value(int_value)
    , float_value(float_value)
    , has_a_dot(has_a_dot)
    {}
};

struct Expr_Char_Literal : public Ast_Expr {
    char c = {};
    Expr_Char_Literal(char c, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_CHAR_LITERAL, current_block, location)
    , c(c)
    {}
};

struct Expr_String_Literal : public Ast_Expr {
    char *text = {};
    int scanner_length = {};
    char *escaped_text = {};
    int escaped_length = {};
    Expr_String_Literal(char *text, int scanner_length, char *escaped_text, int escaped_length, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_STRING_LITERAL, current_block, location)
    , text(text)
    , scanner_length(scanner_length)
    , escaped_text(escaped_text)
    , escaped_length(escaped_length)
    {}
};

struct Expr_Subscript : public Ast_Expr {
    Ast_Expr *lhs = {};
    Ast_Expr *index = {};
    Expr_Subscript(Ast_Expr *lhs, Ast_Expr *index, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_SUBSCRIPT, current_block, location)
    , lhs(lhs)
    , index(index)
    {}
};

struct Expr_Polymorphic_Variable : public Ast_Expr {
    Expr_Identifier *ident = {};
    Declaration *inserted_declaration = {};
    Expr_Polymorphic_Variable(Expr_Identifier *ident, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_POLYMORPHIC_VARIABLE, current_block, location)
    , ident(ident)
    {}
};

struct Expr_Dereference : public Ast_Expr {
    Ast_Expr *lhs = nullptr;
    Expr_Dereference(Ast_Expr *lhs, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_DEREFERENCE, current_block, location)
    , lhs(lhs)
    {}
};

struct Expr_Procedure_Call : public Ast_Expr {
    Ast_Expr *lhs = nullptr;
    Array<Ast_Expr *> parameters = {};
    Type_Procedure *target_procedure_type = {};
    Expr_Procedure_Call(Ast_Expr *lhs, Array<Ast_Expr *> parameters, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_PROCEDURE_CALL, current_block, location)
    , lhs(lhs)
    , parameters(parameters)
    {}
};



struct Struct_Field {
    const char *name = {};
    Operand operand = {};
    int offset = {}; // -1 if is_constant
};

struct Selector_Expression_Lookup_Result {
    Ast_Expr *lhs = {};
    Declaration *declaration = {};
    Operand operand = {};
    Type *type_with_field = {};
};

struct Expr_Selector : public Ast_Expr {
    Ast_Expr *lhs = nullptr;
    char *field_name = nullptr;
    Selector_Expression_Lookup_Result lookup = {};
    Expr_Selector(Ast_Expr *lhs, char *field_name, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_SELECTOR, current_block, location)
    , lhs(lhs)
    , field_name(field_name)
    {}
};

struct Expr_Implicit_Enum_Selector : public Ast_Expr {
    const char *field = nullptr;
    Expr_Implicit_Enum_Selector(const char *field, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_IMPLICIT_ENUM_SELECTOR, current_block, location)
    , field(field)
    {}
};

struct Expr_Compound_Literal : public Ast_Expr {
    Ast_Expr *type_expr = {};
    Array<Ast_Expr *> exprs = {};
    bool is_partial = {};
    Expr_Compound_Literal(Ast_Expr *type_expr, Array<Ast_Expr *> exprs, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_COMPOUND_LITERAL, current_block, location)
    , type_expr(type_expr)
    , exprs(exprs)
    {}
};

struct Expr_Paren : public Ast_Expr {
    Ast_Expr *nested = nullptr;
    Expr_Paren(Ast_Expr *nested, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_PAREN, current_block, location)
    , nested(nested)
    {}
};

struct Expr_Cast : public Ast_Expr {
    Ast_Expr *type_expr = {};
    Ast_Expr *rhs;
    Expr_Cast(Ast_Expr *type_expr, Ast_Expr *rhs, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_CAST, current_block, location)
    , type_expr(type_expr)
    , rhs(rhs)
    {}
};

struct Expr_Transmute : public Ast_Expr {
    Ast_Expr *type_expr = {};
    Ast_Expr *rhs;
    Expr_Transmute(Ast_Expr *type_expr, Ast_Expr *rhs, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_TRANSMUTE, current_block, location)
    , type_expr(type_expr)
    , rhs(rhs)
    {}
};

struct Expr_Sizeof : public Ast_Expr {
    Ast_Expr *expr = {};
    Expr_Sizeof(Ast_Expr *expr, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_SIZEOF, current_block, location)
    , expr(expr)
    {}
};

struct Expr_Typeof : public Ast_Expr {
    Ast_Expr *expr = {};
    Expr_Typeof(Ast_Expr *expr, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_TYPEOF, current_block, location)
    , expr(expr)
    {}
};

struct Expr_Null : public Ast_Expr {
    Expr_Null(Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_NULL, current_block, location)
    {}
};

struct Expr_True : public Ast_Expr {
    Expr_True(Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_TRUE, current_block, location)
    {}
};

struct Expr_False : public Ast_Expr {
    Expr_False(Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_FALSE, current_block, location)
    {}
};

struct Expr_Pointer_Type : public Ast_Expr {
    Ast_Expr *pointer_to = nullptr;
    Expr_Pointer_Type(Ast_Expr *pointer_to, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_POINTER_TYPE, current_block, location)
    , pointer_to(pointer_to)
    {}
};

struct Expr_Reference_Type : public Ast_Expr {
    Ast_Expr *reference_to = nullptr;
    Expr_Reference_Type(Ast_Expr *reference_to, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_REFERENCE_TYPE, current_block, location)
    , reference_to(reference_to)
    {}
};

struct Expr_Polymorphic_Type : public Ast_Expr {
    Ast_Expr *type_expr = {};
    Array<Ast_Expr *> parameters = {};
    Expr_Polymorphic_Type(Ast_Expr *type_expr, Array<Ast_Expr *> parameters, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_POLYMORPHIC_TYPE, current_block, location)
    , type_expr(type_expr)
    , parameters(parameters)
    {}
};

struct Expr_Array_Type : public Ast_Expr {
    Ast_Expr *array_of = nullptr;
    Ast_Expr *count_expr = nullptr;
    Expr_Array_Type(Ast_Expr *array_of, Ast_Expr *count_expr, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_ARRAY_TYPE, current_block, location)
    , array_of(array_of)
    , count_expr(count_expr)
    {}
};

struct Expr_Slice_Type : public Ast_Expr {
    Ast_Expr *slice_of = nullptr;
    Expr_Slice_Type(Ast_Expr *slice_of, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_SLICE_TYPE, current_block, location)
    , slice_of(slice_of)
    {}
};

struct Expr_Procedure_Type : public Ast_Expr {
    Ast_Proc_Header *header = {};
    Expr_Procedure_Type(Ast_Proc_Header *header, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_PROCEDURE_TYPE, current_block, location)
    , header(header)
    {}
};

struct Expr_Struct_Type : public Ast_Expr {
    Ast_Struct *structure = {};
    Expr_Struct_Type(Ast_Struct *structure, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_STRUCT_TYPE, current_block, location)
    , structure(structure)
    {}
};

struct Expr_Spread : public Ast_Expr {
    Ast_Expr *rhs = nullptr;
    bool is_c_varargs = {};
    Expr_Spread(Ast_Expr *rhs, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_SPREAD, current_block, location)
    , rhs(rhs)
    {}
};

enum Declaration_Kind {
    DECL_INVALID,
    DECL_TYPE,
    DECL_STRUCT,
    DECL_USING,
    DECL_ENUM,
    DECL_VAR,
    DECL_STRUCT_MEMBER,
    DECL_CONSTANT_VALUE,
    DECL_PROC,
    DECL_COUNT,
};

enum Declaration_Check_State {
    DCS_UNCHECKED,
    DCS_CHECKING,
    DCS_CHECKED,
};

struct Declaration {
    Ast_Block *parent_block = {};
    Declaration_Check_State check_state = {};
    const char *name = {};
    const char *link_name = {};
    Declaration_Kind kind = {};
    Operand operand = {};
    Location location = {};
    bool is_polymorphic = {};
    Array<char *> notes = {};
    Declaration *from_using = {};
    Declaration(const char *name, Declaration_Kind kind, Ast_Block *parent_block, Location location)
    : name(name)
    , kind(kind)
    , parent_block(parent_block)
    , location(location)
    {}
};

struct Type_Declaration : public Declaration {
    Type *type = {};
    Type_Declaration(char *name, Type *type, Ast_Block *parent_block)
    : Declaration(name, DECL_TYPE, parent_block, {})
    , type(type)
    {}
};

struct Struct_Declaration : public Declaration {
    Ast_Struct *structure = {};
    Struct_Declaration(Ast_Struct *structure, Ast_Block *parent_block)
    : Declaration(structure->name, DECL_STRUCT, parent_block, structure->location)
    , structure(structure)
    {}
};

struct Using_Declaration : public Declaration {
    Ast_Node *importer = {}; // todo(josh): we don't currently use this for anything, but that surprises me a little bit. investigate.
    Declaration *declaration = {};
    Using_Declaration(Ast_Node *importer, Declaration *declaration, Ast_Block *parent_block, Location location)
    : Declaration(declaration->name, DECL_USING, parent_block, location)
    , importer(importer)
    , declaration(declaration)
    {}
};

struct Enum_Declaration : public Declaration {
    Ast_Enum *ast_enum = {};
    Enum_Declaration(Ast_Enum *ast_enum, Ast_Block *parent_block)
    : Declaration(ast_enum->name, DECL_ENUM, parent_block, ast_enum->location)
    , ast_enum(ast_enum)
    {}
};

struct Proc_Declaration : public Declaration {
    Ast_Proc_Header *header = {};
    Proc_Declaration(Ast_Proc_Header *header, Ast_Block *parent_block)
    : Declaration(header->name, DECL_PROC, parent_block, header->location)
    , header(header)
    {}
};

struct Var_Declaration : public Declaration {
    Ast_Var *var = {};
    Var_Declaration(Ast_Var *var, Ast_Block *parent_block)
    : Declaration(var->name, DECL_VAR, parent_block, var->location)
    , var(var)
    {}
};

struct Struct_Member_Declaration : public Declaration {
    const char *name = {};
    Operand operand = {};
    int offset = {};
    Struct_Member_Declaration(const char *name, Operand operand, int offset, Ast_Block *parent_block, Location location)
    : Declaration(name, DECL_STRUCT_MEMBER, parent_block, location)
    , name(name)
    , operand(operand)
    , offset(offset)
    {}
};

struct Constant_Declaration : public Declaration {
    Operand operand = {};
    Constant_Declaration(char *name, Operand operand, Ast_Block *block, Location location)
    : Declaration(name, DECL_CONSTANT_VALUE, block, location)
    , operand(operand)
    {}
};

extern Array<Declaration *>                  g_all_declarations;
extern Array<Ast_Directive_Assert *>         g_all_assert_directives;
extern Array<Ast_Directive_Print *>          g_all_print_directives;
extern Array<Ast_Directive_C_Code *>         g_all_c_code_directives;
extern Array<Ast_Directive_Foreign_Import *> g_all_foreign_import_directives;

void init_parser();
Ast_Block *push_ast_block(Lexer *lexer, Ast_Block *block);
void pop_ast_block(Lexer *lexer, Ast_Block *old_block);
bool register_declaration(Ast_Block *block, Declaration *new_declaration);
bool parse_file(const char *requested_filename, Location include_location);
Ast_Block *begin_parsing(const char *filename);
Ast_Node *parse_single_statement(Lexer *lexer, bool eat_semicolon = true, char *name_override = nullptr);
Ast_Expr *unparen_expr(Ast_Expr *expr);
Ast_Expr *parse_expr(Lexer *lexer);
Ast_Var *parse_var(Lexer *lexer);
Ast_Proc_Header *parse_proc_header(Lexer *lexer, char *name_override = nullptr);
Ast_Proc *parse_proc(Lexer *lexer, char *name_override = nullptr);
Ast_Struct *parse_struct_or_union(Lexer *lexer, char *name_override = nullptr);
Ast_Block *parse_block(Lexer *lexer, bool only_parse_one_statement = false, bool push_new_block = true);
Ast_Node *parse_statement(Lexer *lexer);

bool is_and_op(Token_Kind kind);
bool is_cmp_op(Token_Kind kind);
bool is_add_op(Token_Kind kind);
bool is_mul_op(Token_Kind kind);
bool is_unary_op(Token_Kind kind);
bool is_postfix_op(Token_Kind kind);
