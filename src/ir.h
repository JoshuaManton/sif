#pragma once

#include "parser.h"
#include "checker.h"

struct IR_Statement;
struct IR_Temporary;

struct IR_Proc {
    Array<IR_Var *> parameters = {};
    Array<IR_Var *> all_variables = {}; // parameters + local_variables
    Array<IR_Statement *> statements = {};
    IR_Proc()
    {
        parameters.allocator = g_global_linear_allocator;
        all_variables.allocator = g_global_linear_allocator;
        statements.allocator = g_global_linear_allocator;
    }
};

struct IR {
    IR_Proc *current_proc = {};
};



enum IR_Storage_Kind {
    IR_STORAGE_INVALID,
    IR_STORAGE_STACK,
    IR_STORAGE_GLOBAL,
    IR_STORAGE_INDIRECT,
    IR_STORAGE_REGISTER,

    IR_STORAGE_COUNT,
};

struct IR_Storage {
    IR_Storage_Kind kind = {};
    Type *type = {};
    IR_Storage(IR_Storage_Kind kind, Type *type)
    : kind(kind)
    , type(type)
    {}
};

struct IR_Stack_Storage : public IR_Storage {
    IR_Proc *procedure = {};
    IR_Stack_Storage(IR_Proc *procedure, Type *type)
    : IR_Storage(IR_STORAGE_STACK, type)
    , procedure(procedure)
    {}
};

struct IR_Global_Storage : public IR_Storage {
    IR_Global_Storage(Type *type)
    : IR_Storage(IR_STORAGE_GLOBAL, type)
    {}
};

struct IR_Indirect_Storage : public IR_Storage {
    IR_Storage *points_to = {};
    IR_Indirect_Storage(IR_Storage *points_to, Type *type)
    : IR_Storage(IR_STORAGE_INDIRECT, type)
    , points_to(points_to)
    {}
};

struct IR_Register_Storage : public IR_Storage {
    IR_Temporary *temporary = {};
    IR_Register_Storage(IR_Temporary *temporary, Type *type)
    : IR_Storage(IR_STORAGE_REGISTER, type)
    , temporary(temporary)
    {}
};



struct IR_Var {
    IR_Storage *storage = {};
    Type *type = {};
};

struct IR_Temporary {
    int reg = {};
};

enum IR_Statement_Kind {
    IRS_INVALID,
    IRS_BINARY,
    IRS_MOVE_CONSTANT,
    IRS_STORE,
    IRS_LOAD,
    IRS_INDIRECTION,

    IRS_COUNT,
};

struct IR_Statement {
    IR_Statement_Kind kind = {};
    IR_Statement(IR_Statement_Kind kind)
    : kind(kind)
    {}
};

enum IR_Op {
    IRO_INVALID,
    IRO_ADD,
    IRO_SUB,
    IRO_MUL,
    IRO_DIV,

    IRO_COUNT,
};

struct IR_Statement_Binary : public IR_Statement {
    IR_Storage *lhs = {};
    IR_Storage *rhs = {};
    IR_Register_Storage *result = {};
    IR_Op op = {};
    IR_Statement_Binary()
    : IR_Statement(IRS_BINARY)
    {}
};

struct IR_Statement_Move_Constant : public IR_Statement {
    Operand operand = {};
    IR_Register_Storage *result = {};
    IR_Statement_Move_Constant()
    : IR_Statement(IRS_MOVE_CONSTANT)
    {}
};

struct IR_Statement_Store : public IR_Statement {
    IR_Storage *dst = {};
    IR_Storage *src = {};
    IR_Statement_Store()
    : IR_Statement(IRS_STORE)
    {}
};

struct IR_Statement_Load : public IR_Statement {
    IR_Storage *dst = {};
    IR_Storage *src = {};
    IR_Statement_Load()
    : IR_Statement(IRS_LOAD)
    {}
};

struct IR_Statement_Indirection : public IR_Statement {
    IR_Storage *storage_to_indirect = {};
    IR_Storage *result = {};
    IR_Statement_Indirection()
    : IR_Statement(IRS_INDIRECTION)
    {}
};

IR_Proc *generate_ir_proc(IR *ir, Ast_Proc *procedure);
void printf_ir_statement(IR_Statement *statement);