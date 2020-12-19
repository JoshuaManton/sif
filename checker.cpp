#include "checker.h"

static Array<Type *> all_types;

static Type *type_i8;
static Type *type_i16;
static Type *type_i32;
static Type *type_i64;

static Type *type_u8;
static Type *type_u16;
static Type *type_u32;
static Type *type_u64;

static Type *type_f32;
static Type *type_f64;

static Type *type_bool;

static Type *type_untyped_number;
static Type *type_untyped_null;

static Type *type_typeid;

void init_checker() {
    all_types.allocator = default_allocator();

    type_i8  = new Type_Primitive(1); type_i8->flags  = TF_NUMBER | TF_INTEGER | TF_SIGNED; all_types.append(type_i8);
    type_i16 = new Type_Primitive(2); type_i16->flags = TF_NUMBER | TF_INTEGER | TF_SIGNED; all_types.append(type_i16);
    type_i32 = new Type_Primitive(4); type_i32->flags = TF_NUMBER | TF_INTEGER | TF_SIGNED; all_types.append(type_i32);
    type_i64 = new Type_Primitive(8); type_i64->flags = TF_NUMBER | TF_INTEGER | TF_SIGNED; all_types.append(type_i64);

    type_u8  = new Type_Primitive(1); type_u8->flags  = TF_NUMBER | TF_INTEGER | TF_UNSIGNED; all_types.append(type_u8);
    type_u16 = new Type_Primitive(2); type_u16->flags = TF_NUMBER | TF_INTEGER | TF_UNSIGNED; all_types.append(type_u16);
    type_u32 = new Type_Primitive(4); type_u32->flags = TF_NUMBER | TF_INTEGER | TF_UNSIGNED; all_types.append(type_u32);
    type_u64 = new Type_Primitive(8); type_u64->flags = TF_NUMBER | TF_INTEGER | TF_UNSIGNED; all_types.append(type_u64);

    type_f32 = new Type_Primitive(4); type_u32->flags = TF_NUMBER | TF_FLOAT | TF_SIGNED; all_types.append(type_f32);
    type_f64 = new Type_Primitive(8); type_u64->flags = TF_NUMBER | TF_FLOAT | TF_SIGNED; all_types.append(type_f64);

    type_bool = new Type_Primitive(1); all_types.append(type_bool);

    type_typeid = new Type_Primitive(8);

    type_untyped_number = new Type_Primitive(-1); type_untyped_number->flags = TF_NUMBER  | TF_UNTYPED;
    type_untyped_null   = new Type_Primitive(-1); type_untyped_null->flags   = TF_POINTER | TF_UNTYPED;
}

void add_global_declarations(Ast_Block *block) {
    assert(type_i8 != nullptr);

    block->declarations.append(new Type_Declaration("i8",  type_i8));
    block->declarations.append(new Type_Declaration("i16", type_i16));
    block->declarations.append(new Type_Declaration("i32", type_i32));
    block->declarations.append(new Type_Declaration("i64", type_i64));

    block->declarations.append(new Type_Declaration("u8",  type_u8));
    block->declarations.append(new Type_Declaration("u16", type_u16));
    block->declarations.append(new Type_Declaration("u32", type_u32));
    block->declarations.append(new Type_Declaration("u64", type_u64));

    block->declarations.append(new Type_Declaration("f32", type_f32));
    block->declarations.append(new Type_Declaration("f64", type_f64));

    block->declarations.append(new Type_Declaration("int" ,  type_i64));
    block->declarations.append(new Type_Declaration("uint",  type_u64));
    block->declarations.append(new Type_Declaration("float", type_f32));

    block->declarations.append(new Type_Declaration("typeid", type_typeid));
}

bool types_match(Type *a, Type *b) {
    if (a == b) {
        return true;
    }
    return false;
}

Operand typecheck_expr(Ast_Expr *expr) {
    switch (expr->expr_kind) {
        case EXPR_UNARY: {
            break;
        }
        case EXPR_BINARY: {
            Expr_Binary *binary = (Expr_Binary *)expr;
            Operand lhs_operand = typecheck_expr(binary->lhs);
            Operand rhs_operand = typecheck_expr(binary->rhs);
            assert(types_match(lhs_operand.type, rhs_operand.type));
            Operand operand = {};
            operand.type = lhs_operand.type;
            if ((lhs_operand.flags & OPERAND_CONSTANT) && (rhs_operand.flags & OPERAND_CONSTANT)) {
                operand.flags |= OPERAND_CONSTANT;
                switch (binary->op) {
                    case TK_PLUS: {
                        assert(lhs_operand.type->flags & TF_NUMBER);
                        assert(rhs_operand.type->flags & TF_NUMBER);
                        operand.int_value = lhs_operand.int_value + rhs_operand.int_value;
                        operand.float_value = lhs_operand.float_value + rhs_operand.float_value;
                        break;
                    }
                    case TK_MINUS: {
                        assert(lhs_operand.type->flags & TF_NUMBER);
                        assert(rhs_operand.type->flags & TF_NUMBER);
                        operand.int_value = lhs_operand.int_value - rhs_operand.int_value;
                        operand.float_value = lhs_operand.float_value - rhs_operand.float_value;
                        break;
                    }
                    default: {
                        printf("Unhandled operator: %s\n", token_string(binary->op));
                        assert(false);
                    }
                }
            }
            return operand;
        }
        case EXPR_ADDRESS_OF: {
            break;
        }
        case EXPR_SUBSCRIPT: {
            break;
        }
        case EXPR_DEREFERENCE: {
            break;
        }
        case EXPR_PROCEDURE_CALL: {
            break;
        }
        case EXPR_SELECTOR: {
            break;
        }
        case EXPR_IDENTIFIER: {
            Expr_Identifier *ident = (Expr_Identifier *)expr;
            Operand operand = {};
            assert(ident->resolved_declaration != nullptr);
            switch (ident->resolved_declaration->kind) {
                case DECL_TYPE: {
                    Type_Declaration *decl = (Type_Declaration *)ident->resolved_declaration;
                    operand.type = type_typeid;
                    operand.type_value = decl->type;
                    operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
                    break;
                }
                case DECL_STRUCT: {
                    Struct_Declaration *decl = (Struct_Declaration *)ident->resolved_declaration;
                    assert(decl->structure->type != nullptr && "todo(josh): order independence");
                    operand.type = type_typeid;
                    operand.type_value = decl->structure->type;
                    operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
                    break;
                }
                case DECL_VAR: {
                    Var_Declaration *decl = (Var_Declaration *)ident->resolved_declaration;
                    assert(decl->var->type != nullptr && "todo(josh): order independence");
                    operand.type = decl->var->type;
                    // todo(josh): constant propagation
                    break;
                }
                case DECL_PROC: {
                    Proc_Declaration *decl = (Proc_Declaration *)ident->resolved_declaration;
                    assert(decl->procedure->type != nullptr && "todo(josh): order independence");
                    operand.type = decl->procedure->type;
                    break;
                }
            }
            return operand;
        }
        case EXPR_NUMBER_LITERAL: {
            Expr_Number_Literal *number = (Expr_Number_Literal *)expr;
            Operand op = {};
            op.flags = OPERAND_CONSTANT;
            op.type = type_i64; // todo(josh): untyped numbers
            op.int_value   = atoi(number->number_string);
            op.float_value = atof(number->number_string);
            return op;
        }
        case EXPR_STRING_LITERAL: {
            break;
        }
        case EXPR_NULL: {
            Operand op = {};
            op.flags = OPERAND_CONSTANT;
            op.type = type_untyped_null;
            return op;
        }
        case EXPR_TRUE: {
            Operand op = {};
            op.flags = OPERAND_CONSTANT;
            op.type = type_bool;
            op.bool_value = true;
            return op;
        }
        case EXPR_FALSE: {
            Operand op = {};
            op.flags = OPERAND_CONSTANT;
            op.type = type_bool;
            op.bool_value = false;
            return op;
        }
        case EXPR_POINTER_TYPE: {
            break;
        }
        case EXPR_ARRAY_TYPE: {
            break;
        }
        case EXPR_PAREN: {
            Expr_Paren *paren = (Expr_Paren *)expr;
            return typecheck_expr(paren->nested);
        }
        default: {
            assert(false);
        }
    }
    assert(false && "unreachable");
    return {};
}

void typecheck_block(Ast_Block *block) {
    For (idx, block->nodes) {
        Ast_Node *node = block->nodes[idx];
        switch (node->ast_kind) {
            case AST_VAR: {
                Ast_Var *var = (Ast_Var *)node;
                Operand type_operand = typecheck_expr(var->type_expr);
                assert(type_operand.flags & OPERAND_TYPE);
                assert(type_operand.type_value);
                var->type = type_operand.type_value;

                assert(var->expr != nullptr);
                Operand expr_operand = typecheck_expr(var->expr);
                assert(expr_operand.type == var->type);
                break;
            }

            case AST_STRUCT: {
                break;
            }

            case AST_PROC: {
                break;
            }
        }
    }
}