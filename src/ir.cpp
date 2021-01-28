#include "ir.h"
#include "basic.h"
#include "common.h"

static int g_ir_temporaries;

IR_Temporary *ir_temporary(IR *ir) {
    IR_Temporary *t = SIF_NEW(IR_Temporary, g_global_linear_allocator);
    t->reg = g_ir_temporaries;
    g_ir_temporaries += 1;
    return t;
}

IR_Register_Storage *ir_register_storage(IR *ir, Type *type) {
    IR_Temporary *t = ir_temporary(ir);
    return SIF_NEW_CLONE(IR_Register_Storage(t, type), g_global_linear_allocator);
}

void ir_add_statement(IR *ir, IR_Statement *statement) {
    ir->current_proc->statements.append(statement);
}

void ir_store(IR *ir, IR_Storage *dst, IR_Storage *src) {
    IR_Statement_Store *store = SIF_NEW_CLONE(IR_Statement_Store(), g_global_linear_allocator);
    store->dst = dst;
    store->src = src;
    ir_add_statement(ir, store);
}

IR_Register_Storage *ir_load(IR *ir, IR_Storage *src) {
    IR_Register_Storage *dst = ir_register_storage(ir, src->type);
    IR_Statement_Load *load = SIF_NEW_CLONE(IR_Statement_Load(), g_global_linear_allocator);
    load->dst = dst;
    load->src = src;
    ir_add_statement(ir, load);
    return dst;
}

IR_Storage *ir_emit_expr(IR *ir, Ast_Expr *expr) {
    if (expr->operand.flags & OPERAND_CONSTANT) {
        assert(!is_type_untyped(expr->operand.type));
        IR_Statement_Move_Constant *move = SIF_NEW_CLONE(IR_Statement_Move_Constant(), g_global_linear_allocator);
        move->operand = expr->operand;
        move->result = ir_register_storage(ir, expr->operand.type);
        ir_add_statement(ir, move);
        return move->result;
    }
    else {
        switch (expr->expr_kind) {
            case EXPR_BINARY: {
                Expr_Binary *binary = (Expr_Binary *)expr;
                IR_Storage *lhs = ir_emit_expr(ir, binary->lhs);
                IR_Storage *rhs = ir_emit_expr(ir, binary->rhs);
                IR_Statement_Binary *statement = SIF_NEW_CLONE(IR_Statement_Binary(), g_global_linear_allocator);
                statement->kind = IRS_BINARY;
                statement->lhs = lhs;
                statement->rhs = rhs;
                statement->result = ir_register_storage(ir, binary->operand.type);
                switch (binary->op) {
                    case TK_PLUS:     statement->op = IRO_ADD; break;
                    case TK_MINUS:    statement->op = IRO_SUB; break;
                    case TK_MULTIPLY: statement->op = IRO_MUL; break;
                    case TK_DIVIDE:   statement->op = IRO_DIV; break;
                    default: {
                        printf("Unhandled op %d\n", binary->op);
                        assert(false);
                    }
                }
                ir_add_statement(ir, statement);
                return statement->result;
            }
            case EXPR_IDENTIFIER: {
                Expr_Identifier *ident = (Expr_Identifier *)expr;
                if (is_type_integer(ident->operand.type)) {
                    assert(ident->operand.referenced_declaration->kind == DECL_VAR);
                    Ast_Var *var = ((Var_Declaration *)ident->operand.referenced_declaration)->var;
                    return var->ir_var->storage;
                }
                else {
                    assert(false);
                }
            }
            case EXPR_ADDRESS_OF: {
                Expr_Address_Of *address = (Expr_Address_Of *)expr;
                assert(address->rhs->operand.flags & OPERAND_LVALUE);
                IR_Register_Storage *result = ir_register_storage(ir, address->operand.type);
                return result;
            }
            default: {
                printf("Unhandled expr type in ir_emit_expr(): %d\n", expr->expr_kind);
                assert(false);
            }
        }
    }
    printf("unreachable\n");
    assert(false);
    return nullptr;
}

void ir_emit_block(IR *ir, Ast_Block *block) {
    For (idx, block->nodes) {
        Ast_Node *node = block->nodes[idx];
        switch (node->ast_kind) {
            case AST_VAR: {
                Ast_Var *var = (Ast_Var *)node;
                if (var->expr) {
                    IR_Storage *result = ir_emit_expr(ir, var->expr);
                    ir_store(ir, var->ir_var->storage, result);
                }
                break;
            }
            case AST_ASSIGN: {
                Ast_Assign *assign = (Ast_Assign *)node;
                break;
            }
            case AST_STRUCT: break;
            case AST_PROC:   break;
            case AST_ENUM:   break;
            default: {
                printf("Unhandled node_kind in generate_ir_proc(): %d\n", node->ast_kind);
                assert(false);
            }
        }
    }
}

IR_Proc *generate_ir_proc(IR *ir, Ast_Proc *procedure) {
    IR_Proc *ir_proc = SIF_NEW_CLONE(IR_Proc(), g_global_linear_allocator);
    assert(ir->current_proc == nullptr);
    ir->current_proc = ir_proc;
    defer(ir->current_proc = nullptr);

    For (idx, procedure->header->parameters) {
        Ast_Var *ast_var = procedure->header->parameters[idx];
        IR_Var *ir_var = SIF_NEW_CLONE(IR_Var(), g_global_linear_allocator);
        ast_var->ir_var = ir_var;
        ir_var->type = ast_var->type;
        ir_var->storage = SIF_NEW_CLONE(IR_Stack_Storage(ir_proc, ir_var->type), g_global_linear_allocator);
        ir_proc->parameters.append(ir_var);
        ir_proc->all_variables.append(ir_var);
    }

    For (idx, procedure->header->local_variables) {
        Ast_Var *ast_var = procedure->header->local_variables[idx];
        IR_Var *ir_var = SIF_NEW_CLONE(IR_Var(), g_global_linear_allocator);
        ast_var->ir_var = ir_var;
        ir_var->type = ast_var->type;
        ir_var->storage = SIF_NEW_CLONE(IR_Stack_Storage(ir_proc, ir_var->type), g_global_linear_allocator);
        ir_proc->all_variables.append(ir_var);
    }

    ir_emit_block(ir, procedure->body);
    return ir_proc;
}

void printf_ir_statement(IR_Statement *statement) {
    switch (statement->kind) {
        case IRS_BINARY: {
            IR_Statement_Binary *stmt = (IR_Statement_Binary *)statement;
            printf("unimplemented printf IRS_BINARY");
            // printf("IRS_BINARY r%d ", stmt->result->reg);
            // switch (stmt->op) {
            //     case IRO_ADD: printf("ADD "); break;
            //     case IRO_SUB: printf("SUB "); break;
            //     case IRO_MUL: printf("MUL "); break;
            //     case IRO_DIV: printf("DIV "); break;
            //     default: {
            //         printf("<unhandled op %d>", stmt->op);
            //     }
            // }
            // printf("r%d r%d", stmt->lhs->reg, stmt->rhs->reg);
            break;
        }
        case IRS_MOVE_CONSTANT: {
            IR_Statement_Move_Constant *stmt = (IR_Statement_Move_Constant *)statement;
            printf("unimplemented printf IRS_MOVE_CONSTANT");
            // printf("IRS_MOVE_CONSTANT r%d ", stmt->result->reg);
            // if (is_type_integer(stmt->operand.type)) {
            //     if (is_type_signed(stmt->operand.type)) {
            //         printf("%lld", stmt->operand.int_value);
            //     }
            //     else {
            //         printf("%llu", stmt->operand.uint_value);
            //     }
            // }
            // else {
            //     assert(false);
            // }
            break;
        }
        case IRS_STORE: {
            IR_Statement_Store *store = (IR_Statement_Store *)statement;
            printf("unimplemented printf IRS_STORE");
            // printf("IRS_STORE 0x%p r%d", stmt->var, stmt->value->reg);
            break;
        }
        case IRS_LOAD: {
            IR_Statement_Load *load = (IR_Statement_Load *)statement;
            printf("unimplemented printf IRS_LOAD");
            // printf("IRS_LOAD_FROM_VAR r%d 0x%p", stmt->result->reg, stmt->var);
            break;
        }
        default: {
            printf("unhandled ir statement kind in print: %d\n", statement->kind);
            assert(false);
        }
    }
}