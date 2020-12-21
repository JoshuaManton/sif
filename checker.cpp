#include "checker.h"

#define POINTER_SIZE 8

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

static Array<Declaration *> ordered_declarations;

char *type_to_string(Type *type);
void type_mismatch(Location location, Type *got, Type *expected);
bool match_types(Operand *operand, Type *expected_type);
Type *get_or_create_type_pointer_to(Type *type);
Type *get_or_create_type_array_of(Type *type, int count);
Operand typecheck_expr(Ast_Expr *expr);
void typecheck_var(Ast_Var *var);
void typecheck_block(Ast_Block *block);
void typecheck_procedure_header(Ast_Proc_Header *header);
void typecheck_procedure(Ast_Proc *procedure);

void init_checker() {
    all_types.allocator = default_allocator();
    ordered_declarations.allocator = default_allocator();

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

    register_declaration(block, new Type_Declaration("i8",  type_i8));
    register_declaration(block, new Type_Declaration("i16", type_i16));
    register_declaration(block, new Type_Declaration("i32", type_i32));
    register_declaration(block, new Type_Declaration("i64", type_i64));

    register_declaration(block, new Type_Declaration("u8",  type_u8));
    register_declaration(block, new Type_Declaration("u16", type_u16));
    register_declaration(block, new Type_Declaration("u32", type_u32));
    register_declaration(block, new Type_Declaration("u64", type_u64));

    register_declaration(block, new Type_Declaration("f32", type_f32));
    register_declaration(block, new Type_Declaration("f64", type_f64));

    register_declaration(block, new Type_Declaration("int" ,  type_i64));
    register_declaration(block, new Type_Declaration("uint",  type_u64));
    register_declaration(block, new Type_Declaration("float", type_f32));

    register_declaration(block, new Type_Declaration("bool", type_bool));

    register_declaration(block, new Type_Declaration("typeid", type_typeid));
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

Type *get_or_create_type_pointer_to(Type *pointer_to) {
    assert(!is_type_untyped(pointer_to));
    For (idx, all_types) { // todo(josh): @Speed maybe have an `all_pointer_types` array
        Type *other_type = all_types[idx];
        if (other_type->kind == TYPE_POINTER) {
            Type_Pointer *other_type_pointer = (Type_Pointer *)other_type;
            if (other_type_pointer->pointer_to == pointer_to) {
                return other_type;
            }
        }
    }
    Type *new_type = new Type_Pointer(pointer_to);
    new_type->flags = TF_POINTER;
    new_type->size = POINTER_SIZE;
    all_types.append(new_type);
    return new_type;
}

Type *get_or_create_type_array_of(Type *array_of, int count) {
    assert(!is_type_untyped(array_of));
    For (idx, all_types) { // todo(josh): @Speed maybe have an `all_array_types` array
        Type *other_type = all_types[idx];
        if (other_type->kind == TYPE_ARRAY) {
            Type_Array *other_type_array = (Type_Array *)other_type;
            if (other_type_array->array_of == array_of && other_type_array->count) {
                return other_type;
            }
        }
    }
    Type *new_type = new Type_Array(array_of, count);
    new_type->flags = TF_ARRAY | TF_INCOMPLETE;
    all_types.append(new_type);
    return new_type;
}

Type *get_or_create_type_procedure(Array<Type *> parameter_types, Type *return_type) {
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
    Type *new_type = new Type_Procedure(parameter_types, return_type);
    new_type->flags = TF_PROCEDURE;
    new_type->size = POINTER_SIZE;
    all_types.append(new_type);
    return new_type;
}

Operand typecheck_expr(Ast_Expr *expr) {
    switch (expr->expr_kind) {
        case EXPR_UNARY: {
            UNIMPLEMENTED(EXPR_UNARY);
            break;
        }
        case EXPR_BINARY: {
            Expr_Binary *binary = (Expr_Binary *)expr;
            Operand lhs_operand = typecheck_expr(binary->lhs);
            Operand rhs_operand = typecheck_expr(binary->rhs);
            assert(!is_type_incomplete(lhs_operand.type));
            assert(!is_type_incomplete(rhs_operand.type));
            assert(match_types(&rhs_operand, lhs_operand.type));
            Operand operand(binary->location);
            operand.flags |= OPERAND_RVALUE;
            if ((lhs_operand.flags & OPERAND_CONSTANT) && (rhs_operand.flags & OPERAND_CONSTANT)) {
                operand.flags |= OPERAND_CONSTANT;
                switch (binary->op) {
                    case TK_EQUAL_TO: {
                        assert(lhs_operand.type->flags & TF_INTEGER);
                        assert(rhs_operand.type->flags & TF_INTEGER);
                        operand.type = type_bool;
                        operand.bool_value = lhs_operand.int_value == rhs_operand.int_value;
                        break;
                    }
                    case TK_PLUS: {
                        assert(lhs_operand.type->flags & TF_NUMBER);
                        assert(rhs_operand.type->flags & TF_NUMBER);
                        operand.type = lhs_operand.type;
                        operand.int_value = lhs_operand.int_value + rhs_operand.int_value;
                        operand.float_value = lhs_operand.float_value + rhs_operand.float_value;
                        break;
                    }
                    case TK_MINUS: {
                        assert(lhs_operand.type->flags & TF_NUMBER);
                        assert(rhs_operand.type->flags & TF_NUMBER);
                        operand.type = lhs_operand.type;
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
            assert(rhs_operand.flags & OPERAND_LVALUE | OPERAND_RVALUE);
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
            complete_type(array_type);
            assert(array_type->size > 0);

            Operand index_operand = typecheck_expr(subscript->index);
            assert(is_type_number(index_operand.type));
            assert(is_type_integer(index_operand.type));
            Operand operand(subscript->location);
            operand.type = array_type->array_of;
            operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;
            return operand;
        }
        case EXPR_DEREFERENCE: {
            Expr_Dereference *dereference = (Expr_Dereference *)expr;
            Operand lhs_operand = typecheck_expr(dereference->lhs);
            assert(is_type_pointer(lhs_operand.type));
            Type_Pointer *pointer_type = (Type_Pointer *)lhs_operand.type;
            Operand operand(dereference->location);
            operand.type = pointer_type->pointer_to;
            operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;
            return operand;
        }
        case EXPR_PROCEDURE_CALL: {
            Expr_Procedure_Call *call = (Expr_Procedure_Call *)expr;
            Operand procedure_operand = typecheck_expr(call->lhs);
            assert(procedure_operand.type->kind == TYPE_PROCEDURE);
            Type_Procedure *target_procedure_type = (Type_Procedure *)procedure_operand.type;
            assert(target_procedure_type->parameter_types.count == call->parameters.count);
            Operand operand(call->location);
            operand.type = target_procedure_type->return_type;
            if (operand.type) {
                operand.flags = OPERAND_RVALUE;
            }
            else {
                operand.flags = OPERAND_NO_VALUE;
            }
            // todo(josh): constant stuff? procedures are a bit weird in that way
            return operand;
        }
        case EXPR_SELECTOR: {
            Expr_Selector *selector = (Expr_Selector *)expr;
            Operand lhs_operand = typecheck_expr(selector->lhs);
            complete_type(lhs_operand.type);
            Type_Struct *struct_type = nullptr;
            if (lhs_operand.type->kind == TYPE_POINTER) {
                Type_Pointer *pointer_type = (Type_Pointer *)lhs_operand.type;
                assert(is_type_struct(pointer_type->pointer_to));
                complete_type(pointer_type->pointer_to);
                struct_type = (Type_Struct *)pointer_type->pointer_to;
            }
            else if (lhs_operand.type->kind == TYPE_STRUCT) {
                struct_type = (Type_Struct *)lhs_operand.type;
            }
            else {
                assert(false && "can only use . on struct and pointer types");
            }
            assert(struct_type != nullptr);
            assert(!is_type_incomplete(struct_type));
            Operand operand(selector->location);
            bool found_field = false;
            For (field_idx, struct_type->fields) {
                Struct_Field field = struct_type->fields[field_idx];
                if (strcmp(field.name, selector->field_name) == 0) {
                    // todo(josh): constants
                    found_field = true;
                    operand.type = field.type;
                    operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;
                    break;
                }
            }
            assert(found_field);
            return operand;
        }
        case EXPR_IDENTIFIER: {
            Expr_Identifier *ident = (Expr_Identifier *)expr;
            assert(ident->resolved_declaration != nullptr);
            if (ident->resolved_declaration->check_state == DCS_CHECKED) {
                return ident->resolved_declaration->operand;
            }

            if (ident->resolved_declaration->check_state == DCS_CHECKING) {
                assert(false && "cyclic dependency");
            }

            assert(ident->resolved_declaration->check_state == DCS_UNCHECKED);
            ident->resolved_declaration->check_state = DCS_CHECKING;
            defer(ident->resolved_declaration->check_state = DCS_CHECKED);

            Operand operand(ident->location);
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
                    operand.type = type_typeid;
                    operand.type_value = decl->structure->type;
                    operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
                    break;
                }
                case DECL_VAR: {
                    Var_Declaration *decl = (Var_Declaration *)ident->resolved_declaration;
                    // todo(josh): check for use-before-declaration
                    typecheck_var(decl->var);
                    assert(decl->var->type != nullptr);
                    operand.type = decl->var->type;
                    operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;
                    // todo(josh): constant propagation
                    break;
                }
                case DECL_PROC: {
                    Proc_Declaration *decl = (Proc_Declaration *)ident->resolved_declaration;
                    // todo(josh): separate procedure header and procedure body so we can handle recursion
                    typecheck_procedure_header(decl->procedure->header);
                    assert(decl->procedure->header->type != nullptr);
                    operand.type = decl->procedure->header->type;
                    operand.flags = OPERAND_RVALUE;
                    break;
                }
            }
            ident->resolved_declaration->operand = operand;
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
            break;
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
        case EXPR_SIZEOF: {
            Expr_Sizeof *expr_sizeof = (Expr_Sizeof *)expr;
            Operand expr_operand = typecheck_expr(expr_sizeof->expr);
            assert(expr_operand.type == type_typeid);
            assert(expr_operand.type_value != nullptr);
            complete_type(expr_operand.type_value);
            assert(!is_type_incomplete(expr_operand.type_value));
            assert(expr_operand.type_value->size > 0);
            Operand operand(expr_sizeof->location);
            operand.type = type_untyped_number;
            operand.int_value = expr_operand.type_value->size;
            operand.flags = OPERAND_CONSTANT;
            return operand;
        }
        case EXPR_TYPEOF: {
            Expr_Typeof *expr_typeof = (Expr_Typeof *)expr;
            Operand expr_operand = typecheck_expr(expr_typeof->expr);
            assert(expr_operand.type != nullptr);
            Operand operand(expr_typeof->location);
            operand.type = type_typeid;
            operand.type_value = expr_operand.type;
            operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
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
    return Operand();
}

void typecheck_var(Ast_Var *var) {
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
        Operand return_type_operand = typecheck_expr(header->return_type_expr);
        assert(return_type_operand.flags & OPERAND_CONSTANT);
        assert(return_type_operand.type == type_typeid);
        return_type = return_type_operand.type_value;
    }
    header->type = get_or_create_type_procedure(parameter_types, return_type);
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

            case AST_STRUCT: {
                break;
            }

            case AST_PROC: {
                Ast_Proc *procedure = (Ast_Proc *)node;
                typecheck_procedure_header(procedure->header);
                typecheck_block(procedure->body);
                break;
            }

            case AST_STATEMENT_EXPR: {
                Ast_Statement_Expr *stmt = (Ast_Statement_Expr *)node;
                typecheck_expr(stmt->expr);
                // todo(josh): maybe give an error for statements with no side-effects i.e.
                //             foo == bar;
                break;
            }

            case AST_DIRECTIVE_ASSERT: {
                Ast_Directive_Assert *directive_assert = (Ast_Directive_Assert *)node;
                Operand expr_operand = typecheck_expr(directive_assert->expr);
                assert(expr_operand.type == type_bool);
                assert(expr_operand.flags & OPERAND_CONSTANT);
                if (!expr_operand.bool_value) {
                    assert(false && "#assert failed");
                }
                break;
            }

            case AST_DIRECTIVE_PRINT: {
                Ast_Directive_Print *directive_print = (Ast_Directive_Print *)node;
                Operand expr_operand = typecheck_expr(directive_print->expr);
                assert(expr_operand.flags & OPERAND_CONSTANT);
                assert(expr_operand.type->flags & TF_INTEGER);
                printf("%s(%d:%d) #print directive: %lld\n", directive_print->location.filepath, directive_print->location.line, directive_print->location.character, expr_operand.int_value);
                break;
            }

            default: {
                UNIMPLEMENTED(node->ast_kind);
                break;
            }
        }
    }
}