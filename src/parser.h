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
    AST_EXPR_LIST,
    AST_EMPTY_STATEMENT,
    AST_STATEMENT_EXPR,
    AST_BLOCK_STATEMENT,
    AST_DIRECTIVE_INCLUDE,
    AST_DIRECTIVE_ASSERT,
    AST_DIRECTIVE_PRINT,
    AST_DIRECTIVE_FOREIGN_IMPORT,
};

struct Ast_Expr;
struct Ast_Expr_List;
struct Ast_Var;
struct Ast_Proc;
struct Ast_Block;
struct Ast_Struct;
struct Ast_Statement_Expr;
struct Ast_Defer;
struct Ast_Proc_Header;
struct Ast_Enum;

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

struct Ast_Basic_Block {
    Array<Ast_Basic_Block *> from = {};
    Array<Ast_Basic_Block *> to = {};
    Array<Ast_Node *> nodes = {};
    bool has_return = {};
    Location location = {};
    Ast_Basic_Block(Allocator allocator, Location location)
    : location(location)
    {
        from.allocator = allocator;
        to.allocator = allocator;
        nodes.allocator = allocator;
    }
};

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
    Ast_Node(Ast_Kind ast_kind, Allocator allocator, Ast_Block *current_block, Location location)
    : ast_kind(ast_kind)
    , parent_block(current_block)
    , location(location)
    {
        polymorphs.allocator = allocator;
    }
};

enum Block_Flags {
    BF_IS_GLOBAL_SCOPE = 1 << 0,
    BF_IS_FILE_SCOPE   = 1 << 1,
};

struct Ast_Block : public Ast_Node {
    Array<Ast_Node *> nodes = {};
    Array<Declaration *> declarations = {};
    u64 flags = {};
    Hashtable<const char *, Declaration *> declarations_lookup = {};
    Array<Ast_Defer *> c_gen_defer_stack = {};
    Ast_Block(Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_BLOCK, allocator, current_block, location)
    {
        nodes.allocator = allocator;
        declarations.allocator = allocator;
        declarations_lookup = make_hashtable<const char *, Declaration *>(allocator, 16);
        c_gen_defer_stack.allocator = allocator;
    }
};

struct Ast_Proc_Header : public Ast_Node {
    char *name = nullptr;
    Array<Ast_Var *> parameters = {};
    Ast_Expr *return_type_expr = {};
    Type_Procedure *type = nullptr;
    Ast_Block *procedure_block = {}; // note(josh): NOT the same as the body. parameters live in this scope and it is the parent scope of the body
    bool is_foreign = {};
    Operand operand = {};
    Token_Kind operator_to_overload = {};
    bool is_polymorphic = {};
    Array<int> polymorphic_parameter_indices = {};
    Ast_Proc *procedure = {};
    Proc_Declaration *declaration = {};
    Ast_Node *current_parsing_loop = {};
    Array<Ast_Defer *> defers = {};
    Declaration *parent_declaration = {};
    Ast_Basic_Block *root_basic_block = {};
    Ast_Basic_Block *current_basic_block = {};
    Ast_Proc_Header(Declaration *parent_declaration, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_PROC_HEADER, allocator, current_block, location)
    , parent_declaration(parent_declaration)
    {
        polymorphic_parameter_indices.allocator = allocator;
        parameters.allocator = allocator;
        defers.allocator = allocator;
    }
};

struct Ast_Proc : public Ast_Node {
    Ast_Proc_Header *header = {};
    Ast_Block *body = nullptr;
    Ast_Proc(Ast_Proc_Header *header, Ast_Block *body, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_PROC, allocator, current_block, location)
    , header(header)
    , body(body)
    {}
};

struct Ast_Assign : public Ast_Node {
    Token_Kind op = {};
    Ast_Expr_List *lhs = {};
    Ast_Expr_List *rhs = {};
    Ast_Assign(Token_Kind op, Ast_Expr_List *lhs, Ast_Expr_List *rhs, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_ASSIGN, allocator, current_block, location)
    , op(op)
    , lhs(lhs)
    , rhs(rhs)
    {}
};

struct Ast_Directive_Include : public Ast_Node {
    char *filename = {};
    Ast_Directive_Include(char *filename, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_DIRECTIVE_INCLUDE, allocator, current_block, location)
    , filename(filename)
    {}
};

struct Ast_Directive_Assert : public Ast_Node {
    Ast_Expr *expr = {};
    Ast_Directive_Assert(Ast_Expr *expr, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_DIRECTIVE_ASSERT, allocator, current_block, location)
    , expr(expr)
    {}
};

struct Ast_Directive_Print : public Ast_Node {
    Ast_Expr *expr = {};
    Ast_Directive_Print(Ast_Expr *expr, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_DIRECTIVE_PRINT, allocator, current_block, location)
    , expr(expr)
    {}
};

struct Ast_Directive_Foreign_Import : public Ast_Node {
    const char *name = {};
    const char *path = {};
    Ast_Directive_Foreign_Import(const char *name, const char *path, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_DIRECTIVE_FOREIGN_IMPORT, allocator, current_block, location)
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
    Ast_Proc_Header *is_parameter_for_procedure = {}; // note(josh): may be null
    Ast_Struct *belongs_to_struct = {}; // note(josh): may be null
    Struct_Member_Declaration *struct_member = {}; // note(josh): only set if belongs_to_struct is set
    Ast_Var(char *name, Ast_Expr *name_expr, Ast_Expr *type_expr, Ast_Expr *expr, bool is_constant, bool is_polymorphic_value, bool is_polymorphic_type, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_VAR, allocator, current_block, location)
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
    Ast_Using(Ast_Expr *expr, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_USING, allocator, current_block, location)
    , expr(expr)
    {}
};

struct Ast_Defer : public Ast_Node {
    Ast_Node *node_to_defer = {};
    Ast_Defer(Ast_Node *node_to_defer, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_DEFER, allocator, current_block, location)
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
    Array<Ast_Struct *> local_structs = {};
    Array<Ast_Enum *> local_enums = {};
    Array<Ast_Var *> polymorphic_parameters = {};
    Declaration *parent_declaration = {};
    Ast_Struct(Declaration *parent_declaration, bool is_union, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_STRUCT, allocator, current_block, location)
    , parent_declaration(parent_declaration)
    , is_union(is_union)
    {
        fields.allocator = allocator;
        operator_overloads.allocator = allocator;
        polymorphic_parameters.allocator = allocator;
        procedures.allocator = allocator;
        local_structs.allocator = allocator;
        local_enums.allocator = allocator;
    }
};

struct Ast_If : public Ast_Node {
    Ast_Node *pre_statement = {};
    Ast_Block *if_block = {}; // note(josh): this is the lexical scope of the entire if statement, NOT the body
    Ast_Expr *condition = {};
    Ast_Block *body = {};
    Ast_Block *else_body = {};
    Ast_If(Ast_Block *if_block, Ast_Node *pre_statement, Ast_Expr *condition, Ast_Block *body, Ast_Block *else_body, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_IF, allocator, current_block, location)
    , if_block(if_block)
    , pre_statement(pre_statement)
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
    Ast_For_Loop(Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_FOR_LOOP, allocator, current_block, location)
    {}
};

struct Ast_While_Loop : public Ast_Node {
    Ast_Expr *condition = {};
    Ast_Block *body = {};
    Ast_While_Loop(Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_WHILE_LOOP, allocator, current_block, location)
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
    Declaration *parent_declaration = {};
    Ast_Enum(char *name, Declaration *parent_declaration, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_ENUM, allocator, current_block, location)
    , name(name)
    , parent_declaration(parent_declaration)
    {}
};

struct Ast_Return : public Ast_Node {
    Ast_Proc_Header *matching_procedure = {};
    Ast_Expr *expr = {};
    Ast_Return(Ast_Proc_Header *matching_procedure, Ast_Expr *expr, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_RETURN, allocator, current_block, location)
    , matching_procedure(matching_procedure)
    , expr(expr)
    {}
};

struct Ast_Break : public Ast_Node {
    Ast_Node *matching_loop = {};
    Ast_Break(Ast_Node *matching_loop, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_BREAK, allocator, current_block, location)
    , matching_loop(matching_loop)
    {}
};

struct Ast_Continue : public Ast_Node {
    Ast_Node *matching_loop = {};
    Ast_Continue(Ast_Node *matching_loop, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_CONTINUE, allocator, current_block, location)
    , matching_loop(matching_loop)
    {}
};

struct Ast_Statement_Expr : public Ast_Node {
    Ast_Expr *expr = {};
    Ast_Statement_Expr(Ast_Expr *expr, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_STATEMENT_EXPR, allocator, current_block, location)
    , expr(expr)
    {}
};

struct Ast_Block_Statement : public Ast_Node {
    Ast_Block *block = {};
    Ast_Block_Statement(Ast_Block *block, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_BLOCK_STATEMENT, allocator, current_block, location)
    , block(block)
    {}
};

struct Ast_Empty_Statement : public Ast_Node {
    Ast_Empty_Statement(Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_EMPTY_STATEMENT, allocator, current_block, location)
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
    EXPR_TYPEOFELEMENT,
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
    Ast_Expr(Expr_Kind kind, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_EXPR, allocator, current_block, location)
    , expr_kind(kind)
    {}
};

struct Ast_Expr_List : public Ast_Node {
    Array<Ast_Expr *> exprs = {};
    Ast_Expr_List(Array<Ast_Expr *> exprs, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Node(AST_EXPR_LIST, allocator, current_block, location)
    , exprs(exprs)
    {}
};

struct Expr_Binary : public Ast_Expr {
    Token_Kind op = TK_INVALID;
    Ast_Expr *lhs = nullptr;
    Ast_Expr *rhs = nullptr;
    Expr_Binary(Token_Kind op, Ast_Expr *lhs, Ast_Expr *rhs, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_BINARY, allocator, current_block, location)
    , op(op)
    , lhs(lhs)
    , rhs(rhs)
    {}
};

struct Expr_Address_Of : public Ast_Expr {
    Ast_Expr *rhs = nullptr;
    Expr_Address_Of(Ast_Expr *rhs, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_ADDRESS_OF, allocator, current_block, location)
    , rhs(rhs)
    {}
};

struct Expr_Unary : public Ast_Expr {
    Token_Kind op = TK_INVALID;
    Ast_Expr *rhs = nullptr;
    Expr_Unary(Token_Kind op, Ast_Expr *rhs, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_UNARY, allocator, current_block, location)
    , op(op)
    , rhs(rhs)
    {}
};

struct Expr_Identifier : public Ast_Expr {
    char *name = nullptr;
    Declaration *resolved_declaration = nullptr;
    Expr_Identifier(char *name, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_IDENTIFIER, allocator, current_block, location)
    , name(name)
    {}
};

struct Expr_Number_Literal : public Ast_Expr {
    u64 uint_value = {};
    i64 int_value = {};
    f64 float_value = {};
    bool has_a_dot = false;
    Expr_Number_Literal(u64 uint_value, i64 int_value, f64 float_value, bool has_a_dot, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_NUMBER_LITERAL, allocator, current_block, location)
    , uint_value(uint_value)
    , int_value(int_value)
    , float_value(float_value)
    , has_a_dot(has_a_dot)
    {}
};

struct Expr_Char_Literal : public Ast_Expr {
    char c = {};
    Expr_Char_Literal(char c, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_CHAR_LITERAL, allocator, current_block, location)
    , c(c)
    {}
};

struct Expr_String_Literal : public Ast_Expr {
    char *text = {};
    int scanner_length = {};
    char *escaped_text = {};
    int escaped_length = {};
    Expr_String_Literal(char *text, int scanner_length, char *escaped_text, int escaped_length, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_STRING_LITERAL, allocator, current_block, location)
    , text(text)
    , scanner_length(scanner_length)
    , escaped_text(escaped_text)
    , escaped_length(escaped_length)
    {}
};

struct Expr_Subscript : public Ast_Expr {
    Ast_Expr *lhs = {};
    Ast_Expr *index = {};
    Expr_Subscript(Ast_Expr *lhs, Ast_Expr *index, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_SUBSCRIPT, allocator, current_block, location)
    , lhs(lhs)
    , index(index)
    {}
};

struct Expr_Polymorphic_Variable : public Ast_Expr {
    Expr_Identifier *ident = {};
    Declaration *inserted_declaration = {};
    Expr_Polymorphic_Variable(Expr_Identifier *ident, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_POLYMORPHIC_VARIABLE, allocator, current_block, location)
    , ident(ident)
    {}
};

struct Expr_Dereference : public Ast_Expr {
    Ast_Expr *lhs = nullptr;
    Expr_Dereference(Ast_Expr *lhs, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_DEREFERENCE, allocator, current_block, location)
    , lhs(lhs)
    {}
};

struct Expr_Procedure_Call : public Ast_Expr {
    Ast_Expr *lhs = nullptr;
    Array<Ast_Expr *> parameters = {};
    Type_Procedure *target_procedure_type = {};
    Expr_Procedure_Call(Ast_Expr *lhs, Array<Ast_Expr *> parameters, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_PROCEDURE_CALL, allocator, current_block, location)
    , lhs(lhs)
    , parameters(parameters)
    {}
};



struct Struct_Field {
    const char *name = {};
    Operand operand = {};
    int offset = {}; // -1 if is_constant
    Array<char *> notes = {};
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
    Expr_Selector(Ast_Expr *lhs, char *field_name, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_SELECTOR, allocator, current_block, location)
    , lhs(lhs)
    , field_name(field_name)
    {}
};

struct Expr_Implicit_Enum_Selector : public Ast_Expr {
    const char *field = nullptr;
    Expr_Implicit_Enum_Selector(const char *field, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_IMPLICIT_ENUM_SELECTOR, allocator, current_block, location)
    , field(field)
    {}
};

struct Expr_Compound_Literal : public Ast_Expr {
    Ast_Expr *type_expr = {};
    Array<Ast_Expr *> exprs = {};
    bool is_partial = {};
    Expr_Compound_Literal(Ast_Expr *type_expr, Array<Ast_Expr *> exprs, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_COMPOUND_LITERAL, allocator, current_block, location)
    , type_expr(type_expr)
    , exprs(exprs)
    {}
};

struct Expr_Paren : public Ast_Expr {
    Ast_Expr *nested = nullptr;
    Expr_Paren(Ast_Expr *nested, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_PAREN, allocator, current_block, location)
    , nested(nested)
    {}
};

struct Expr_Cast : public Ast_Expr {
    Ast_Expr *type_expr = {};
    Ast_Expr *rhs;
    Expr_Cast(Ast_Expr *type_expr, Ast_Expr *rhs, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_CAST, allocator, current_block, location)
    , type_expr(type_expr)
    , rhs(rhs)
    {}
};

struct Expr_Transmute : public Ast_Expr {
    Ast_Expr *type_expr = {};
    Ast_Expr *rhs;
    Expr_Transmute(Ast_Expr *type_expr, Ast_Expr *rhs, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_TRANSMUTE, allocator, current_block, location)
    , type_expr(type_expr)
    , rhs(rhs)
    {}
};

struct Expr_Sizeof : public Ast_Expr {
    Ast_Expr *expr = {};
    Expr_Sizeof(Ast_Expr *expr, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_SIZEOF, allocator, current_block, location)
    , expr(expr)
    {}
};

struct Expr_Typeof : public Ast_Expr {
    Ast_Expr *expr = {};
    Expr_Typeof(Ast_Expr *expr, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_TYPEOF, allocator, current_block, location)
    , expr(expr)
    {}
};

struct Expr_Typeofelement : public Ast_Expr {
    Ast_Expr *expr = {};
    Expr_Typeofelement(Ast_Expr *expr, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_TYPEOFELEMENT, allocator, current_block, location)
    , expr(expr)
    {}
};

struct Expr_Null : public Ast_Expr {
    Expr_Null(Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_NULL, allocator, current_block, location)
    {}
};

struct Expr_True : public Ast_Expr {
    Expr_True(Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_TRUE, allocator, current_block, location)
    {}
};

struct Expr_False : public Ast_Expr {
    Expr_False(Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_FALSE, allocator, current_block, location)
    {}
};

struct Expr_Pointer_Type : public Ast_Expr {
    Ast_Expr *pointer_to = nullptr;
    Expr_Pointer_Type(Ast_Expr *pointer_to, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_POINTER_TYPE, allocator, current_block, location)
    , pointer_to(pointer_to)
    {}
};

struct Expr_Reference_Type : public Ast_Expr {
    Ast_Expr *reference_to = nullptr;
    Expr_Reference_Type(Ast_Expr *reference_to, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_REFERENCE_TYPE, allocator, current_block, location)
    , reference_to(reference_to)
    {}
};

struct Expr_Polymorphic_Type : public Ast_Expr {
    Ast_Expr *type_expr = {};
    Array<Ast_Expr *> parameters = {};
    Expr_Polymorphic_Type(Ast_Expr *type_expr, Array<Ast_Expr *> parameters, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_POLYMORPHIC_TYPE, allocator, current_block, location)
    , type_expr(type_expr)
    , parameters(parameters)
    {}
};

struct Expr_Array_Type : public Ast_Expr {
    Ast_Expr *array_of = nullptr;
    Ast_Expr *count_expr = nullptr;
    Expr_Array_Type(Ast_Expr *array_of, Ast_Expr *count_expr, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_ARRAY_TYPE, allocator, current_block, location)
    , array_of(array_of)
    , count_expr(count_expr)
    {}
};

struct Expr_Slice_Type : public Ast_Expr {
    Ast_Expr *slice_of = nullptr;
    Expr_Slice_Type(Ast_Expr *slice_of, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_SLICE_TYPE, allocator, current_block, location)
    , slice_of(slice_of)
    {}
};

struct Expr_Procedure_Type : public Ast_Expr {
    Ast_Proc_Header *header = {};
    Expr_Procedure_Type(Ast_Proc_Header *header, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_PROCEDURE_TYPE, allocator, current_block, location)
    , header(header)
    {}
};

struct Expr_Struct_Type : public Ast_Expr {
    Ast_Struct *structure = {};
    Expr_Struct_Type(Ast_Struct *structure, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_STRUCT_TYPE, allocator, current_block, location)
    , structure(structure)
    {}
};

struct Expr_Spread : public Ast_Expr {
    Ast_Expr *rhs = nullptr;
    bool is_c_varargs = {};
    Expr_Spread(Ast_Expr *rhs, Allocator allocator, Ast_Block *current_block, Location location)
    : Ast_Expr(EXPR_SPREAD, allocator, current_block, location)
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

extern Array<Ast_Directive_Foreign_Import *> g_all_foreign_import_directives;
extern int g_total_lines_parsed;

void init_parser();
Ast_Block *push_ast_block(Lexer *lexer, Ast_Block *block);
void pop_ast_block(Lexer *lexer, Ast_Block *old_block);
bool register_declaration(Ast_Block *block, Declaration *new_declaration);
void parse_file(const char *requested_filename, Location include_location);
Ast_Block *begin_parsing(const char *filename);
Ast_Node *parse_single_statement(Lexer *lexer, bool eat_semicolon = true, char *name_override = nullptr);
Ast_Expr *unparen_expr(Ast_Expr *expr);
Ast_Expr *parse_expr(Lexer *lexer);
Ast_Var *parse_var(Lexer *lexer);
Ast_Proc_Header *parse_proc_header(Lexer *lexer, char *name_override = nullptr);
Ast_Proc *parse_proc(Lexer *lexer, char *name_override = nullptr);
Ast_Struct *parse_struct_or_union(Lexer *lexer, char *name_override = nullptr);
Ast_Block *parse_block(Lexer *lexer, bool only_parse_one_statement = false);
Ast_Node *parse_statement(Lexer *lexer);

bool is_and_op(Token_Kind kind);
bool is_cmp_op(Token_Kind kind);
bool is_add_op(Token_Kind kind);
bool is_mul_op(Token_Kind kind);
bool is_unary_op(Token_Kind kind);
bool is_postfix_op(Token_Kind kind);
