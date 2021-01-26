#pragma once

#include "parser.h"
#include "checker.h"

struct IR_Statement;

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

struct IR_Var {
    Type *type = {};
};

enum IR_Op {
    IRO_INVALID,
    IRO_ADD,
    IRO_SUB,
    IRO_MUL,
    IRO_DIV,

    IRO_COUNT,
};

struct IR_Temporary {
    int reg = {};
};

enum IR_Statement_Kind {
    IRS_INVALID,
    IRS_BINARY,
    IRS_MOVE_CONSTANT,
    IRS_STORE_TO_VAR,
    IRS_LOAD_FROM_VAR,

    IRS_COUNT,
};

struct IR_Statement {
    IR_Statement_Kind kind = {};
    IR_Statement(IR_Statement_Kind kind)
    : kind(kind)
    {}
};

struct IR_Statement_Binary : public IR_Statement {
    IR_Temporary *lhs = {};
    IR_Temporary *rhs = {};
    IR_Temporary *result = {};
    IR_Op op = {};
    IR_Statement_Binary()
    : IR_Statement(IRS_BINARY)
    {}
};

struct IR_Statement_Move_Constant : public IR_Statement {
    Operand operand = {};
    IR_Temporary *result = {};
    IR_Statement_Move_Constant()
    : IR_Statement(IRS_MOVE_CONSTANT)
    {}
};

struct IR_Statement_Store_To_Var : public IR_Statement {
    IR_Var *var = {};
    IR_Temporary *value = {};
    IR_Statement_Store_To_Var()
    : IR_Statement(IRS_STORE_TO_VAR)
    {}
};

struct IR_Statement_Load_From_Var : public IR_Statement {
    IR_Var *var = {};
    IR_Temporary *result = {};
    IR_Statement_Load_From_Var()
    : IR_Statement(IRS_LOAD_FROM_VAR)
    {}
};

IR_Proc *generate_ir_proc(IR *ir, Ast_Proc *procedure);
void printf_ir_statement(IR_Statement *statement);