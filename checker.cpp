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

    type_i8  = new Type_Primitive("i8", 1); type_i8->flags  = TF_NUMBER | TF_INTEGER | TF_SIGNED; all_types.append(type_i8);
    type_i16 = new Type_Primitive("i16", 2); type_i16->flags = TF_NUMBER | TF_INTEGER | TF_SIGNED; all_types.append(type_i16);
    type_i32 = new Type_Primitive("i32", 4); type_i32->flags = TF_NUMBER | TF_INTEGER | TF_SIGNED; all_types.append(type_i32);
    type_i64 = new Type_Primitive("i64", 8); type_i64->flags = TF_NUMBER | TF_INTEGER | TF_SIGNED; all_types.append(type_i64);

    type_u8  = new Type_Primitive("u8", 1); type_u8->flags  = TF_NUMBER | TF_INTEGER | TF_UNSIGNED; all_types.append(type_u8);
    type_u16 = new Type_Primitive("u16", 2); type_u16->flags = TF_NUMBER | TF_INTEGER | TF_UNSIGNED; all_types.append(type_u16);
    type_u32 = new Type_Primitive("u32", 4); type_u32->flags = TF_NUMBER | TF_INTEGER | TF_UNSIGNED; all_types.append(type_u32);
    type_u64 = new Type_Primitive("u64", 8); type_u64->flags = TF_NUMBER | TF_INTEGER | TF_UNSIGNED; all_types.append(type_u64);

    type_f32 = new Type_Primitive("f32", 4); type_f32->flags = TF_NUMBER | TF_FLOAT | TF_SIGNED; all_types.append(type_f32);
    type_f64 = new Type_Primitive("f64", 8); type_f64->flags = TF_NUMBER | TF_FLOAT | TF_SIGNED; all_types.append(type_f64);

    type_bool = new Type_Primitive("bool", 1); all_types.append(type_bool);

    type_typeid = new Type_Primitive("typeid", 8);

    type_untyped_number = new Type_Primitive("untyped number", -1); type_untyped_number->flags = TF_NUMBER  | TF_UNTYPED | TF_INTEGER | TF_FLOAT; // todo(josh): having this be both integer and float is kinda goofy
    type_untyped_null   = new Type_Primitive("untyped null", -1); type_untyped_null->flags   = TF_POINTER | TF_UNTYPED;
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

bool is_type_pointer (Type *type) { return type->flags & TF_POINTER;  }
bool is_type_array   (Type *type) { return type->flags & TF_ARRAY;    }
bool is_type_number  (Type *type) { return type->flags & TF_NUMBER;   }
bool is_type_integer (Type *type) { return type->flags & TF_INTEGER;  }
bool is_type_float   (Type *type) { return type->flags & TF_FLOAT;    }
bool is_type_untyped (Type *type) { return type->flags & TF_UNTYPED;  }
bool is_type_unsigned(Type *type) { return type->flags & TF_UNSIGNED; }
bool is_type_signed  (Type *type) { return type->flags & TF_SIGNED;   }

#define UNIMPLEMENTED(val) assert(false && "Unimplemented case: " #val "\n");

char *type_to_string(Type *type) {
    char *buffer = (char *)alloc(default_allocator(), 64);
    switch (type->kind) {
        case TYPE_PRIMITIVE: {
            Type_Primitive *primitive = (Type_Primitive *)type;
            sprintf(buffer, primitive->name);
            break;
        }
        case TYPE_STRUCT: {
            Type_Struct *structure = (Type_Struct *)type;
            sprintf(buffer, structure->name);
            break;
        }
        case TYPE_POINTER: {
            Type_Pointer *pointer = (Type_Pointer *)type;
            sprintf(buffer, "^%s", type_to_string(pointer->pointer_to));
            break;
        }
        case TYPE_ARRAY: {
            Type_Array *array = (Type_Array *)type;
            sprintf(buffer, "[%d]%s", array->count, type_to_string(array->array_of));
            break;
        }
    }
    return buffer;
}

void type_mismatch(Location location, Type *got, Type *expected) {
    // todo(josh): create a unified report_error() procedure. just copy pasted this from unexpected_token() for now
    printf("%s(%d:%d) Type mismatch. Expected %s, got %s.\n", location.filepath, location.line, location.character, type_to_string(expected), type_to_string(got));
    g_reported_error = true;
}

bool match_types(Operand *operand, Type *expected_type) {
    if (operand->type == expected_type) {
        return true;
    }

    if (operand->type->flags & TF_UNTYPED) {
        if (is_type_number(operand->type) && is_type_number(expected_type)) {
            // todo(josh): this will truncate floats in the case of:
            //     var x: int = 1.3;
            operand->type = expected_type;
            return true;
        }

        if (is_type_pointer(operand->type) && is_type_pointer(expected_type)) {
            assert(operand->type == type_untyped_null);
            operand->type = expected_type;
            return true;
        }
    }

    type_mismatch(operand->location, operand->type, expected_type);
    return false;
}

Type *get_or_create_type_pointer_to(Type *type) {
    assert(!is_type_untyped(type));
    For (idx, all_types) {
        Type *other_type = all_types[idx];
        if (other_type->kind == TYPE_POINTER) {
            Type_Pointer *other_type_pointer = (Type_Pointer *)other_type;
            if (other_type_pointer->pointer_to == type) {
                return other_type;
            }
        }
    }
    Type *new_type = new Type_Pointer(type);
    new_type->flags = TF_POINTER;
    all_types.append(new_type);
    return new_type;
}

Type *get_or_create_type_array_of(Type *type, int count) {
    assert(!is_type_untyped(type));
    For (idx, all_types) {
        Type *other_type = all_types[idx];
        if (other_type->kind == TYPE_ARRAY) {
            Type_Array *other_type_array = (Type_Array *)other_type;
            if (other_type_array->array_of == type && other_type_array->count) {
                return other_type;
            }
        }
    }
    Type *new_type = new Type_Array(type, count);
    new_type->flags = TF_ARRAY;
    all_types.append(new_type);
    return new_type;
}

Operand typecheck_expr(Ast_Expr *expr) {
    switch (expr->expr_kind) {
        case EXPR_UNARY: {
            UNIMPLEMENTED(EXPR_UNARY);
        }
        case EXPR_BINARY: {
            Expr_Binary *binary = (Expr_Binary *)expr;
            Operand lhs_operand = typecheck_expr(binary->lhs);
            Operand rhs_operand = typecheck_expr(binary->rhs);
            assert(match_types(&rhs_operand, lhs_operand.type));
            Operand operand(binary->location);
            operand.type = lhs_operand.type;
            operand.flags |= OPERAND_RVALUE;
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
            Expr_Address_Of *address_of = (Expr_Address_Of *)expr;
            Operand rhs_operand = typecheck_expr(address_of->rhs);
            assert(rhs_operand.flags & OPERAND_LVALUE);
            Operand operand(address_of->location);
            operand.type = get_or_create_type_pointer_to(rhs_operand.type);
            operand.flags = OPERAND_RVALUE;
            return operand;
        }
        case EXPR_SUBSCRIPT: {
            Expr_Subscript *subscript = (Expr_Subscript *)expr;
            Operand lhs_operand = typecheck_expr(subscript->lhs);
            assert(is_type_array(lhs_operand.type));
            assert(lhs_operand.type->kind == TYPE_ARRAY);
            Type_Array *array_type = (Type_Array *)lhs_operand.type;

            Operand index_operand = typecheck_expr(subscript->index);
            assert(is_type_number(index_operand.type));
            assert(is_type_integer(index_operand.type));
            Operand operand(subscript->location);
            operand.type = array_type->array_of;
            operand.flags = OPERAND_LVALUE;
            return operand;
        }
        case EXPR_DEREFERENCE: {
            Expr_Dereference *dereference = (Expr_Dereference *)expr;
            Operand lhs_operand = typecheck_expr(dereference->lhs);
            assert(is_type_pointer(lhs_operand.type));
            Type_Pointer *pointer_type = (Type_Pointer *)lhs_operand.type;
            Operand operand(dereference->location);
            operand.type = pointer_type->pointer_to;
            operand.flags = OPERAND_LVALUE;
            return operand;
        }
        case EXPR_PROCEDURE_CALL: {
            UNIMPLEMENTED(EXPR_PROCEDURE_CALL);
        }
        case EXPR_SELECTOR: {
            UNIMPLEMENTED(EXPR_SELECTOR);
        }
        case EXPR_IDENTIFIER: {
            Expr_Identifier *ident = (Expr_Identifier *)expr;
            Operand operand(ident->location);
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
                    operand.flags = OPERAND_LVALUE;
                    // todo(josh): constant propagation
                    break;
                }
                case DECL_PROC: {
                    Proc_Declaration *decl = (Proc_Declaration *)ident->resolved_declaration;
                    assert(decl->procedure->type != nullptr && "todo(josh): order independence");
                    operand.type = decl->procedure->type;
                    operand.flags = OPERAND_RVALUE;
                    break;
                }
            }
            return operand;
        }
        case EXPR_NUMBER_LITERAL: {
            Expr_Number_Literal *number = (Expr_Number_Literal *)expr;
            Operand operand(number->location);
            operand.flags = OPERAND_CONSTANT;
            operand.type = type_untyped_number;
            operand.int_value   = atoi(number->number_string);
            operand.float_value = atof(number->number_string);
            return operand;
        }
        case EXPR_STRING_LITERAL: {
            UNIMPLEMENTED(EXPR_STRING_LITERAL);
        }
        case EXPR_NULL: {
            Operand operand(expr->location);
            operand.flags = OPERAND_CONSTANT;
            operand.type = type_untyped_null;
            return operand;
        }
        case EXPR_TRUE: {
            Operand operand(expr->location);
            operand.flags = OPERAND_CONSTANT;
            operand.type = type_bool;
            operand.bool_value = true;
            return operand;
        }
        case EXPR_FALSE: {
            Operand operand(expr->location);
            operand.flags = OPERAND_CONSTANT;
            operand.type = type_bool;
            operand.bool_value = false;
            return operand;
        }
        case EXPR_POINTER_TYPE: {
            Expr_Pointer_Type *expr_pointer = (Expr_Pointer_Type *)expr;
            assert(expr_pointer->pointer_to != nullptr);
            Operand pointer_to_operand = typecheck_expr(expr_pointer->pointer_to);
            assert((pointer_to_operand.flags & OPERAND_CONSTANT) && (pointer_to_operand.flags & OPERAND_TYPE));
            Operand operand(expr_pointer->location);
            operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
            operand.type = type_typeid;
            operand.type_value = get_or_create_type_pointer_to(pointer_to_operand.type_value);
            return operand;
        }
        case EXPR_ARRAY_TYPE: {
            Expr_Array_Type *expr_array = (Expr_Array_Type *)expr;
            assert(expr_array->array_of != nullptr);
            Operand array_of_operand = typecheck_expr(expr_array->array_of);
            assert((array_of_operand.flags & OPERAND_CONSTANT) && (array_of_operand.flags & OPERAND_TYPE));
            Operand count_operand = typecheck_expr(expr_array->count_expr);
            assert((count_operand.flags & OPERAND_CONSTANT) && is_type_integer(count_operand.type));
            assert(count_operand.int_value > 0);
            Operand operand(expr_array->location);
            operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
            operand.type = type_typeid;
            operand.type_value = get_or_create_type_array_of(array_of_operand.type_value, count_operand.int_value);
            return operand;
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
    return Operand({});
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

                if (var->expr) {
                    Operand expr_operand = typecheck_expr(var->expr);
                    if (!match_types(&expr_operand, var->type)) {
                        assert(false);
                    }
                    assert(var->type == expr_operand.type);
                }

                break;
            }

            case AST_STRUCT: {
                Ast_Struct *structure = (Ast_Struct *)node;
                typecheck_block(structure->body);
                break;
            }

            case AST_PROC: {
                break;
            }
        }
    }
}