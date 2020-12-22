#include "checker.h"

#define POINTER_SIZE 8

static Array<Type *> all_types;

Type *type_i8;
Type *type_i16;
Type *type_i32;
Type *type_i64;

Type *type_u8;
Type *type_u16;
Type *type_u32;
Type *type_u64;

Type *type_f32;
Type *type_f64;

Type *type_bool;

Type *type_untyped_number;
Type *type_untyped_null;

Type *type_typeid;

Type *type_int;
Type *type_float;

Array<Declaration *> ordered_declarations;

char *type_to_string(Type *type);
void complete_type(Type *type);
void type_mismatch(Location location, Type *got, Type *expected);
bool match_types(Operand *operand, Type *expected_type);
Type_Pointer *get_or_create_type_pointer_to(Type *type);
Type_Array *get_or_create_type_array_of(Type *type, int count);
Operand *typecheck_expr(Ast_Expr *expr, Type *expected_type = nullptr);
void typecheck_var(Ast_Var *var);
void typecheck_block(Ast_Block *block);
void typecheck_procedure_header(Ast_Proc_Header *header);
void typecheck_procedure(Ast_Proc *procedure);
bool do_assert_directives();
void do_print_directives();

void init_checker() {
    all_types.allocator = default_allocator();
    ordered_declarations.allocator = default_allocator();

    type_i8  = new Type_Primitive("i8", 1);  type_i8->flags  = TF_NUMBER | TF_INTEGER | TF_SIGNED; all_types.append(type_i8);
    type_i16 = new Type_Primitive("i16", 2); type_i16->flags = TF_NUMBER | TF_INTEGER | TF_SIGNED; all_types.append(type_i16);
    type_i32 = new Type_Primitive("i32", 4); type_i32->flags = TF_NUMBER | TF_INTEGER | TF_SIGNED; all_types.append(type_i32);
    type_i64 = new Type_Primitive("i64", 8); type_i64->flags = TF_NUMBER | TF_INTEGER | TF_SIGNED; all_types.append(type_i64);

    type_u8  = new Type_Primitive("u8", 1);  type_u8->flags  = TF_NUMBER | TF_INTEGER | TF_UNSIGNED; all_types.append(type_u8);
    type_u16 = new Type_Primitive("u16", 2); type_u16->flags = TF_NUMBER | TF_INTEGER | TF_UNSIGNED; all_types.append(type_u16);
    type_u32 = new Type_Primitive("u32", 4); type_u32->flags = TF_NUMBER | TF_INTEGER | TF_UNSIGNED; all_types.append(type_u32);
    type_u64 = new Type_Primitive("u64", 8); type_u64->flags = TF_NUMBER | TF_INTEGER | TF_UNSIGNED; all_types.append(type_u64);

    type_f32 = new Type_Primitive("f32", 4); type_f32->flags = TF_NUMBER | TF_FLOAT | TF_SIGNED; all_types.append(type_f32);
    type_f64 = new Type_Primitive("f64", 8); type_f64->flags = TF_NUMBER | TF_FLOAT | TF_SIGNED; all_types.append(type_f64);

    type_bool = new Type_Primitive("bool", 1); all_types.append(type_bool);

    type_typeid = new Type_Primitive("typeid", 8);

    type_untyped_number = new Type_Primitive("untyped number", -1); type_untyped_number->flags = TF_NUMBER | TF_UNTYPED | TF_INTEGER | TF_FLOAT; // todo(josh): having this be both integer and float is kinda goofy
    type_untyped_null   = new Type_Primitive("untyped null", -1); type_untyped_null->flags   = TF_POINTER | TF_UNTYPED;

    type_int = type_i64;
    type_float = type_f32;
}

void add_global_declarations(Ast_Block *block) {
    assert(type_i8 != nullptr);

    register_declaration(new Type_Declaration("i8",  type_i8, block));
    register_declaration(new Type_Declaration("i16", type_i16, block));
    register_declaration(new Type_Declaration("i32", type_i32, block));
    register_declaration(new Type_Declaration("i64", type_i64, block));

    register_declaration(new Type_Declaration("u8",  type_u8, block));
    register_declaration(new Type_Declaration("u16", type_u16, block));
    register_declaration(new Type_Declaration("u32", type_u32, block));
    register_declaration(new Type_Declaration("u64", type_u64, block));

    register_declaration(new Type_Declaration("f32", type_f32, block));
    register_declaration(new Type_Declaration("f64", type_f64, block));

    register_declaration(new Type_Declaration("int" ,  type_i64, block));
    register_declaration(new Type_Declaration("uint",  type_u64, block));
    register_declaration(new Type_Declaration("float", type_f32, block));

    register_declaration(new Type_Declaration("bool", type_bool, block));

    register_declaration(new Type_Declaration("typeid", type_typeid, block));
}

void make_incomplete_types_for_all_structs() {
    For (idx, g_all_declarations) {
        Declaration *decl = g_all_declarations[idx];
        if (decl->kind != DECL_STRUCT) { // todo(josh): @Speed maybe make a g_all_structs?
            continue;
        }

        Struct_Declaration *struct_decl = (Struct_Declaration *)decl;
        Type *incomplete_type = new Type_Struct(struct_decl->structure);
        incomplete_type->flags |= (TF_STRUCT | TF_INCOMPLETE);
        all_types.append(incomplete_type);
        struct_decl->structure->type = incomplete_type;
    }
}

bool is_type_pointer   (Type *type) { return type->flags & TF_POINTER;    }
bool is_type_array     (Type *type) { return type->flags & TF_ARRAY;      }
bool is_type_number    (Type *type) { return type->flags & TF_NUMBER;     }
bool is_type_integer   (Type *type) { return type->flags & TF_INTEGER;    }
bool is_type_float     (Type *type) { return type->flags & TF_FLOAT;      }
bool is_type_untyped   (Type *type) { return type->flags & TF_UNTYPED;    }
bool is_type_unsigned  (Type *type) { return type->flags & TF_UNSIGNED;   }
bool is_type_signed    (Type *type) { return type->flags & TF_SIGNED;     }
bool is_type_struct    (Type *type) { return type->flags & TF_STRUCT;     }
bool is_type_incomplete(Type *type) { return type->flags & TF_INCOMPLETE; }

Operand check_declaration(Declaration *decl) {
    if (decl->check_state == DCS_CHECKED) {
        return decl->operand;
    }

    if (decl->check_state == DCS_CHECKING) {
        assert(false && "cyclic dependency");
    }

    assert(decl->check_state == DCS_UNCHECKED);
    decl->check_state = DCS_CHECKING;

    Operand operand;
    switch (decl->kind) {
        case DECL_TYPE: {
            Type_Declaration *type_decl = (Type_Declaration *)decl;
            operand.type = type_typeid;
            operand.type_value = type_decl->type;
            operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
            break;
        }
        case DECL_STRUCT: {
            Struct_Declaration *struct_decl = (Struct_Declaration *)decl;
            operand.type = type_typeid;
            operand.type_value = struct_decl->structure->type;
            operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
            break;
        }
        case DECL_VAR: {
            Var_Declaration *var_decl = (Var_Declaration *)decl;
            // todo(josh): check for use-before-declaration
            typecheck_var(var_decl->var);
            assert(var_decl->var->type != nullptr);
            operand.type = var_decl->var->type;
            operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;
            // todo(josh): constant propagation
            break;
        }
        case DECL_PROC: {
            Proc_Declaration *proc_decl = (Proc_Declaration *)decl;
            typecheck_procedure_header(proc_decl->procedure->header);
            assert(proc_decl->procedure->header->type != nullptr);
            operand.type = proc_decl->procedure->header->type;
            operand.flags = OPERAND_RVALUE;
            break;
        }
    }
    decl->operand = operand;
    decl->check_state = DCS_CHECKED;

    if (decl->kind == DECL_PROC) {
        Proc_Declaration *proc_decl = (Proc_Declaration *)decl;
        typecheck_block(proc_decl->procedure->body);
    }

    if (decl->kind != DECL_STRUCT) {
        if (decl->parent_block->flags & BF_IS_GLOBAL_SCOPE) {
            ordered_declarations.append(decl);
        }
    }

    return operand;
}

void typecheck_global_scope(Ast_Block *block) {
    assert(block->flags & BF_IS_GLOBAL_SCOPE);
    make_incomplete_types_for_all_structs(); // todo(josh): this is kinda goofy. should be able to just do this as we traverse the program
    For (idx, block->declarations) {
        Declaration *decl = block->declarations[idx];
        check_declaration(decl);
    }
    bool all_assert_directives_passed = do_assert_directives();
    if (!all_assert_directives_passed) {
        g_reported_error = true;
    }
    do_print_directives();

    // note(josh): complete any types that haven't been completed because they haven't been used.
    //             we have to do this because using a struct type by pointer will NOT trigger
    //             a call to complete_type(), only actually using the type and it's members will.
    For (idx, all_types) {
        Type *type = all_types[idx];
        complete_type(type);
    }
}

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

void complete_type(Type *type) {
    if (type->check_state == CS_CHECKED) {
        return;
    }
    if (type->check_state == CS_CHECKING) {
        assert(false && "circular dependency");
    }
    assert(type->check_state == CS_NOT_CHECKED);
    type->check_state = CS_CHECKING;
    defer(type->check_state = CS_CHECKED);
    if (is_type_incomplete(type)) {
        switch (type->kind) {
            case TYPE_STRUCT: {
                Type_Struct *struct_type = (Type_Struct *)type;
                assert(struct_type->ast_struct != nullptr);
                Ast_Struct *structure = struct_type->ast_struct;
                typecheck_block(structure->body);
                int size = 0;
                Array<Struct_Field> fields = {};
                fields.allocator = default_allocator();
                if (structure->fields.count == 0) {
                    size = 1;
                }
                else {
                    For (idx, structure->fields) {
                        Ast_Var *var = structure->fields[idx];
                        complete_type(var->type);
                        assert(var->type->size > 0);
                        Struct_Field field = {};
                        field.name = var->name;
                        field.type = var->type;
                        fields.append(field);
                        size += field.type->size;
                    }
                }

                assert(size > 0);
                struct_type->size = size;
                struct_type->fields = fields;
                struct_type->flags &= ~(TF_INCOMPLETE);

                assert(structure->parent_block->flags & BF_IS_GLOBAL_SCOPE); // todo(josh): locally scoped structs and procs
                ordered_declarations.append(structure->declaration);
                break;
            }
            case TYPE_ARRAY: {
                Type_Array *array_type = (Type_Array *)type;
                complete_type(array_type->array_of);
                assert(array_type->array_of->size > 0);
                array_type->size = array_type->array_of->size * array_type->count;
                assert(array_type->size > 0);
                array_type->flags &= ~(TF_INCOMPLETE);
                break;
            }
        }
    }
}

void type_mismatch(Location location, Type *got, Type *expected) {
    report_error(location, "Type mismatch. Expected %s, got %s.\n", type_to_string(expected), type_to_string(got));
}

bool match_types(Operand *operand, Type *expected_type) {
    assert(operand->type != nullptr);
    assert(expected_type != nullptr);

    if (operand->type == expected_type) {
        return true;
    }

    assert(!(expected_type->flags & TF_UNTYPED));

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

Type_Pointer *get_or_create_type_pointer_to(Type *pointer_to) {
    assert(!is_type_untyped(pointer_to));
    For (idx, all_types) { // todo(josh): @Speed maybe have an `all_pointer_types` array
        Type *other_type = all_types[idx];
        if (other_type->kind == TYPE_POINTER) {
            Type_Pointer *other_type_pointer = (Type_Pointer *)other_type;
            if (other_type_pointer->pointer_to == pointer_to) {
                return other_type_pointer;
            }
        }
    }
    Type_Pointer *new_type = new Type_Pointer(pointer_to);
    new_type->flags = TF_POINTER;
    new_type->size = POINTER_SIZE;
    all_types.append(new_type);
    return new_type;
}

Type_Array *get_or_create_type_array_of(Type *array_of, int count) {
    assert(!is_type_untyped(array_of));
    For (idx, all_types) { // todo(josh): @Speed maybe have an `all_array_types` array
        Type *other_type = all_types[idx];
        if (other_type->kind == TYPE_ARRAY) {
            Type_Array *other_type_array = (Type_Array *)other_type;
            if (other_type_array->array_of == array_of && other_type_array->count == count) {
                return other_type_array;
            }
        }
    }
    Type_Array *new_type = new Type_Array(array_of, count);
    new_type->flags = TF_ARRAY | TF_INCOMPLETE;
    all_types.append(new_type);
    return new_type;
}

Type_Procedure *get_or_create_type_procedure(Array<Type *> parameter_types, Type *return_type) {
    For (idx, all_types) { // todo(josh): @Speed maybe have an `all_procedure_types` array
        Type *other_type = all_types[idx];
        if (other_type->kind == TYPE_PROCEDURE) {
            Type_Procedure *other_type_procedure = (Type_Procedure *)other_type;
            if (other_type_procedure->parameter_types.count != parameter_types.count) {
                continue;
            }
            bool all_parameters_match = true;
            For (parameter_idx, other_type_procedure->parameter_types) {
                if (other_type_procedure->parameter_types[parameter_idx] != parameter_types[parameter_idx]) {
                    all_parameters_match = false;
                    break;
                }
            }
            if (!all_parameters_match) {
                continue;
            }

            if (other_type_procedure->return_type == return_type) {
                return other_type_procedure;
            }
        }
    }
    Type_Procedure *new_type = new Type_Procedure(parameter_types, return_type);
    new_type->flags = TF_PROCEDURE;
    new_type->size = POINTER_SIZE;
    all_types.append(new_type);
    return new_type;
}

Type *get_most_concrete_type(Type *a, Type *b) {
    if (a->flags & TF_UNTYPED) {
        if (b->flags & TF_UNTYPED) {
            return a;
        }
        else {
            return b;
        }
    }
    else {
        return a;
    }
}

bool can_cast(Ast_Expr *expr, Type *type) {
    assert(expr != nullptr);
    assert(type != nullptr);
    if ((expr->operand.type->flags & TF_NUMBER) && (type->flags & TF_NUMBER)) {
        return true;
    }
    if ((expr->operand.type->flags & TF_POINTER) && (type->flags & TF_POINTER)) {
        return true;
    }
    return false;
}

Operand *typecheck_expr(Ast_Expr *expr, Type *expected_type) {
    if (expected_type != nullptr) {
        assert(!(expected_type->flags & TF_UNTYPED)); // note(josh): an expected_type should never be untyped
    }
    Operand result_operand(expr->location);
    switch (expr->expr_kind) {
        case EXPR_UNARY: {
            UNIMPLEMENTED(EXPR_UNARY);
            break;
        }
        case EXPR_BINARY: {
            Expr_Binary *binary = (Expr_Binary *)expr;
            Operand *lhs_operand = typecheck_expr(binary->lhs, expected_type);
            Operand *rhs_operand = typecheck_expr(binary->rhs, expected_type);
            assert(!is_type_incomplete(lhs_operand->type));
            assert(!is_type_incomplete(rhs_operand->type));
            result_operand.flags |= OPERAND_RVALUE;
            Type *most_concrete = get_most_concrete_type(lhs_operand->type, rhs_operand->type);
            if (expected_type != nullptr) {
                most_concrete = get_most_concrete_type(most_concrete, expected_type);
            }
            if (most_concrete->flags & TF_UNTYPED) {
                // note(josh): if the most concrete type was untyped, that should mean that both were
                //             untyped. in which case make sure they are both the same
                assert(lhs_operand->type == rhs_operand->type);
            }
            else {
                assert(match_types(lhs_operand, most_concrete));
                assert(match_types(rhs_operand, most_concrete));
            }

            switch (binary->op) {
                case TK_EQUAL_TO: {
                    assert(lhs_operand->type->flags & TF_INTEGER);
                    assert(rhs_operand->type->flags & TF_INTEGER);
                    result_operand.type = type_bool;
                    if ((lhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_CONSTANT)) {
                        result_operand.flags |= OPERAND_CONSTANT;
                        result_operand.bool_value = lhs_operand->int_value == rhs_operand->int_value;
                    }
                    break;
                }
                case TK_PLUS: {
                    assert(lhs_operand->type->flags & TF_NUMBER);
                    assert(rhs_operand->type->flags & TF_NUMBER);
                    result_operand.type = most_concrete;
                    if ((lhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_CONSTANT)) {
                        result_operand.flags |= OPERAND_CONSTANT;
                        result_operand.int_value = lhs_operand->int_value + rhs_operand->int_value;
                        result_operand.float_value = lhs_operand->float_value + rhs_operand->float_value;
                    }
                    break;
                }
                case TK_MINUS: {
                    assert(lhs_operand->type->flags & TF_NUMBER);
                    assert(rhs_operand->type->flags & TF_NUMBER);
                    result_operand.type = most_concrete;
                    if ((lhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_CONSTANT)) {
                        result_operand.flags |= OPERAND_CONSTANT;
                        result_operand.int_value = lhs_operand->int_value - rhs_operand->int_value;
                        result_operand.float_value = lhs_operand->float_value - rhs_operand->float_value;
                    }
                    break;
                }
                default: {
                    printf("Unhandled operator: %s\n", token_string(binary->op));
                    assert(false);
                }
            }
            assert(result_operand.type != nullptr);
            break;
        }
        case EXPR_CAST: {
            Expr_Cast *expr_cast = (Expr_Cast *)expr;
            Operand *type_operand = typecheck_expr(expr_cast->type_expr);
            assert(expr_cast->type_expr->operand.type == type_typeid);
            Operand *rhs_operand = typecheck_expr(expr_cast->rhs);
            assert(can_cast(expr_cast->rhs, expr_cast->type_expr->operand.type_value));
            result_operand.type = type_operand->type_value;
            result_operand.flags = OPERAND_RVALUE;
            // todo(josh): constant propagation
            break;
        }
        case EXPR_ADDRESS_OF: {
            Expr_Address_Of *address_of = (Expr_Address_Of *)expr;
            Operand *rhs_operand = typecheck_expr(address_of->rhs);
            assert(rhs_operand->flags & OPERAND_LVALUE | OPERAND_RVALUE);
            result_operand.type = get_or_create_type_pointer_to(rhs_operand->type);
            result_operand.flags = OPERAND_RVALUE;
            break;
        }
        case EXPR_SUBSCRIPT: {
            Expr_Subscript *subscript = (Expr_Subscript *)expr;
            Operand *lhs_operand = typecheck_expr(subscript->lhs); // todo(josh): should we pass expected_type down here?
            assert(is_type_array(lhs_operand->type));
            assert(lhs_operand->type->kind == TYPE_ARRAY);
            Type_Array *array_type = (Type_Array *)lhs_operand->type;
            complete_type(array_type);
            assert(array_type->size > 0);

            Operand *index_operand = typecheck_expr(subscript->index, type_int); // todo(josh): should we pass expected_type down here?
            assert(is_type_number(index_operand->type));
            assert(is_type_integer(index_operand->type));
            result_operand.type = array_type->array_of;
            result_operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;
            break;
        }
        case EXPR_DEREFERENCE: {
            Expr_Dereference *dereference = (Expr_Dereference *)expr;
            Operand *lhs_operand = typecheck_expr(dereference->lhs); // todo(josh): should we pass expected_type down here?
            assert(is_type_pointer(lhs_operand->type));
            Type_Pointer *pointer_type = (Type_Pointer *)lhs_operand->type;
            result_operand.type = pointer_type->pointer_to;
            result_operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;
            break;
        }
        case EXPR_PROCEDURE_CALL: {
            Expr_Procedure_Call *call = (Expr_Procedure_Call *)expr;
            Operand *procedure_operand = typecheck_expr(call->lhs);
            assert(procedure_operand->type->kind == TYPE_PROCEDURE);
            Type_Procedure *target_procedure_type = (Type_Procedure *)procedure_operand->type;
            assert(target_procedure_type->parameter_types.count == call->parameters.count);
            For (idx, call->parameters) {
                Ast_Expr *parameter = call->parameters[idx];
                assert(target_procedure_type->parameter_types[idx] != nullptr);
                typecheck_expr(parameter, target_procedure_type->parameter_types[idx]);
                assert(parameter->operand.type != nullptr);
            }
            result_operand.type = target_procedure_type->return_type;
            if (result_operand.type) {
                result_operand.flags = OPERAND_RVALUE;
            }
            else {
                result_operand.flags = OPERAND_NO_VALUE;
            }
            // todo(josh): constant stuff? procedures are a bit weird in that way in that the name is constant but the value isn't
            break;
        }
        case EXPR_SELECTOR: {
            Expr_Selector *selector = (Expr_Selector *)expr;
            Operand *lhs_operand = typecheck_expr(selector->lhs);
            complete_type(lhs_operand->type);
            Type_Struct *struct_type = nullptr;
            if (lhs_operand->type->kind == TYPE_POINTER) {
                Type_Pointer *pointer_type = (Type_Pointer *)lhs_operand->type;
                assert(is_type_struct(pointer_type->pointer_to));
                complete_type(pointer_type->pointer_to);
                struct_type = (Type_Struct *)pointer_type->pointer_to;
            }
            else if (lhs_operand->type->kind == TYPE_STRUCT) {
                struct_type = (Type_Struct *)lhs_operand->type;
            }
            else {
                assert(false && "can only use . on struct and pointer types");
            }
            assert(struct_type != nullptr);
            assert(!is_type_incomplete(struct_type));
            bool found_field = false;
            For (field_idx, struct_type->fields) {
                Struct_Field field = struct_type->fields[field_idx];
                if (strcmp(field.name, selector->field_name) == 0) {
                    // todo(josh): constants
                    found_field = true;
                    result_operand.type = field.type;
                    result_operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;
                    break;
                }
            }
            assert(found_field);
            break;
        }
        case EXPR_IDENTIFIER: {
            Expr_Identifier *ident = (Expr_Identifier *)expr;
            assert(ident->resolved_declaration != nullptr);
            result_operand = check_declaration(ident->resolved_declaration);
            result_operand.location = ident->location;
            break;
        }
        case EXPR_NUMBER_LITERAL: {
            Expr_Number_Literal *number = (Expr_Number_Literal *)expr;
            result_operand.flags = OPERAND_CONSTANT | OPERAND_RVALUE;
            result_operand.type = type_untyped_number;
            result_operand.int_value   = atoi(number->number_string);
            result_operand.float_value = atof(number->number_string);
            break;
        }
        case EXPR_STRING_LITERAL: {
            UNIMPLEMENTED(EXPR_STRING_LITERAL);
            break;
        }
        case EXPR_NULL: {
            result_operand.flags = OPERAND_CONSTANT | OPERAND_RVALUE;
            result_operand.type = type_untyped_null;
            break;
        }
        case EXPR_TRUE: {
            result_operand.flags = OPERAND_CONSTANT | OPERAND_RVALUE;
            result_operand.type = type_bool;
            result_operand.bool_value = true;
            break;
        }
        case EXPR_FALSE: {
            result_operand.flags = OPERAND_CONSTANT | OPERAND_RVALUE;
            result_operand.type = type_bool;
            result_operand.bool_value = false;
            break;
        }
        case EXPR_SIZEOF: {
            Expr_Sizeof *expr_sizeof = (Expr_Sizeof *)expr;
            Operand *expr_operand = typecheck_expr(expr_sizeof->expr);
            assert(expr_operand->type == type_typeid);
            assert(expr_operand->type_value != nullptr);
            complete_type(expr_operand->type_value);
            assert(!is_type_incomplete(expr_operand->type_value));
            assert(expr_operand->type_value->size > 0);
            result_operand.type = type_untyped_number;
            result_operand.int_value = expr_operand->type_value->size;
            result_operand.flags = OPERAND_CONSTANT | OPERAND_RVALUE;
            break;
        }
        case EXPR_TYPEOF: {
            Expr_Typeof *expr_typeof = (Expr_Typeof *)expr;
            Operand *expr_operand = typecheck_expr(expr_typeof->expr);
            assert(expr_operand->type != nullptr);
            result_operand.type = type_typeid;
            result_operand.type_value = expr_operand->type;
            result_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
            break;
        }
        case EXPR_POINTER_TYPE: {
            Expr_Pointer_Type *expr_pointer = (Expr_Pointer_Type *)expr;
            assert(expr_pointer->pointer_to != nullptr);
            Operand *pointer_to_operand = typecheck_expr(expr_pointer->pointer_to);
            assert((pointer_to_operand->flags & OPERAND_CONSTANT) && (pointer_to_operand->flags & OPERAND_TYPE));
            result_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
            result_operand.type = type_typeid;
            result_operand.type_value = get_or_create_type_pointer_to(pointer_to_operand->type_value);
            break;
        }
        case EXPR_ARRAY_TYPE: {
            Expr_Array_Type *expr_array = (Expr_Array_Type *)expr;
            assert(expr_array->array_of != nullptr);
            Operand *array_of_operand = typecheck_expr(expr_array->array_of);
            assert((array_of_operand->flags & OPERAND_CONSTANT) && (array_of_operand->flags & OPERAND_TYPE));
            Operand *count_operand = typecheck_expr(expr_array->count_expr);
            assert((count_operand->flags & OPERAND_CONSTANT) && is_type_integer(count_operand->type));
            assert(count_operand->int_value > 0);
            result_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
            result_operand.type = type_typeid;
            result_operand.type_value = get_or_create_type_array_of(array_of_operand->type_value, count_operand->int_value);
            break;
        }
        case EXPR_PAREN: {
            Expr_Paren *paren = (Expr_Paren *)expr;
            result_operand = *typecheck_expr(paren->nested);
            result_operand.location = paren->location;
            break;
        }
        default: {
            assert(false);
        }
    }
    if (expected_type) {
        if (!match_types(&result_operand, expected_type)) {
            assert(false);
        }
        assert(!(result_operand.type->flags & TF_UNTYPED));
    }

    expr->operand = result_operand;
    return &expr->operand;
}

void typecheck_var(Ast_Var *var) {
    Operand *type_operand = typecheck_expr(var->type_expr, type_typeid);
    assert(type_operand->flags & OPERAND_TYPE);
    assert(type_operand->type_value);
    var->type = type_operand->type_value;

    if (var->expr) {
        Operand *expr_operand = typecheck_expr(var->expr, var->type);
        if (!match_types(expr_operand, var->type)) {
            assert(false);
        }
        assert(var->type == expr_operand->type);
    }

    complete_type(var->type);
}

void typecheck_procedure_header(Ast_Proc_Header *header) {
    if (header->type != nullptr) {
        // note(josh): typecheck_procedure() calls this, but also
        // the normal declaration resolving does. hmmmmm.
        return;
    }
    assert(header->type == nullptr);
    Array<Type *> parameter_types = {};
    parameter_types.allocator = default_allocator();
    For (idx, header->parameters) {
        Ast_Var *parameter = header->parameters[idx];
        typecheck_var(parameter);
        assert(parameter->type != nullptr);
        parameter_types.append(parameter->type);
    }
    Type *return_type = {};
    if (header->return_type_expr) {
        Operand *return_type_operand = typecheck_expr(header->return_type_expr);
        assert(return_type_operand->flags & OPERAND_CONSTANT);
        assert(return_type_operand->type == type_typeid);
        return_type = return_type_operand->type_value;
    }
    header->type = get_or_create_type_procedure(parameter_types, return_type);
}

bool do_assert_directives() {
    bool all_pass = true;
    For (idx, g_all_assert_directives) {
        Ast_Directive_Assert *assert_directive = g_all_assert_directives[idx];
        Operand *expr_operand = typecheck_expr(assert_directive->expr);
        assert(expr_operand->type == type_bool);
        assert(expr_operand->flags & OPERAND_CONSTANT);
        if (!expr_operand->bool_value) {
            report_error(assert_directive->location, "#assert directive failed.\n");
            all_pass = false;
            return false;
        }
    }
    return all_pass;
}

void do_print_directives() {
    For (idx, g_all_print_directives) {
        Ast_Directive_Print *print_directive = g_all_print_directives[idx];
        Operand *expr_operand = typecheck_expr(print_directive->expr);
        assert(expr_operand->flags & OPERAND_CONSTANT);
        assert(expr_operand->type->flags & TF_INTEGER);
        printf("%s(%d:%d) #print directive: %lld\n", print_directive->location.filepath, print_directive->location.line, print_directive->location.character, expr_operand->int_value);
    }
}

void typecheck_block(Ast_Block *block) {
    For (idx, block->nodes) {
        Ast_Node *node = block->nodes[idx];
        switch (node->ast_kind) {
            case AST_VAR: {
                Ast_Var *var = (Ast_Var *)node;
                typecheck_var(var);
                break;
            }

            case AST_ASSIGN: {
                Ast_Assign *assign = (Ast_Assign *)node;
                Operand *lhs_operand = typecheck_expr(assign->lhs);
                assert(lhs_operand->flags & OPERAND_LVALUE);
                assert(lhs_operand->type != nullptr);
                Operand *rhs_operand = typecheck_expr(assign->rhs, lhs_operand->type);
                assert(rhs_operand->flags & OPERAND_RVALUE);
                if (!match_types(rhs_operand, lhs_operand->type)) {
                    assert(false);
                }
                break;
            }

            case AST_STATEMENT_EXPR: {
                Ast_Statement_Expr *stmt = (Ast_Statement_Expr *)node;
                typecheck_expr(stmt->expr);
                // todo(josh): maybe give an error for statements with no side-effects i.e.
                //             foo == bar;
                break;
            }

            case AST_IF: {
                Ast_If *ast_if = (Ast_If *)node;
                Operand *condition_operand = typecheck_expr(ast_if->condition);
                assert(condition_operand->type == type_bool);
                typecheck_block(ast_if->body);
                if (ast_if->else_body) {
                    typecheck_block(ast_if->else_body);
                }
                break;
            }

            case AST_RETURN: {
                Ast_Return *ast_return = (Ast_Return *)node;
                assert(ast_return->matching_procedure != nullptr);
                assert(ast_return->matching_procedure->type != nullptr);
                if (ast_return->matching_procedure->type->return_type != nullptr) {
                    assert(ast_return->expr != nullptr);
                    Operand *return_operand = typecheck_expr(ast_return->expr);
                    assert(match_types(return_operand, ast_return->matching_procedure->type->return_type));
                }
                break;
            }

            default: {
                UNIMPLEMENTED(node->ast_kind);
                break;
            }
        }
    }
}