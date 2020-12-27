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

Type *type_untyped_integer;
Type *type_untyped_float;
Type *type_untyped_null;

Type *type_typeid;

Type *type_string;
Type *type_rawptr;

Type *type_int;
Type *type_float;

Type *type_polymorphic;

Array<Declaration *> ordered_declarations;

char *type_to_string(Type *type);
bool complete_type(Type *type);
void type_mismatch(Location location, Type *got, Type *expected);
bool match_types(Operand *operand, Type *expected_type, bool do_report_error = true);
Operand *typecheck_expr(Ast_Expr *expr, Type *expected_type = nullptr);
bool typecheck_block(Ast_Block *block);
bool typecheck_procedure_header(Ast_Proc_Header *header);
bool do_assert_directives();
bool do_print_directives();

void add_variable_type_field(Type *type, const char *name, Type *variable_type, int offset) {
    assert(variable_type != nullptr);
    assert(!is_type_untyped(variable_type));
    Struct_Field field = {};
    field.name = name;
    field.offset = offset;
    field.operand.type = variable_type;
    field.operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;
    type->fields.append(field);
    type->num_variable_fields += 1;
}

void add_constant_type_field(Type *type, const char *name, Operand operand) {
    assert(operand.type != nullptr);
    assert(!is_type_untyped(operand.type));
    Struct_Field field = {};
    field.name = name;
    field.offset = -1;
    field.operand = operand;
    type->fields.append(field);
    type->num_constant_fields += 1;
}

void init_checker() {
    all_types.allocator = default_allocator();
    ordered_declarations.allocator = default_allocator();

    type_i8  = new Type_Primitive("i8", 1, 1);  type_i8->flags  = TF_NUMBER | TF_INTEGER | TF_SIGNED; all_types.append(type_i8);
    type_i16 = new Type_Primitive("i16", 2, 2); type_i16->flags = TF_NUMBER | TF_INTEGER | TF_SIGNED; all_types.append(type_i16);
    type_i32 = new Type_Primitive("i32", 4, 4); type_i32->flags = TF_NUMBER | TF_INTEGER | TF_SIGNED; all_types.append(type_i32);
    type_i64 = new Type_Primitive("i64", 8, 8); type_i64->flags = TF_NUMBER | TF_INTEGER | TF_SIGNED; all_types.append(type_i64);

    type_u8  = new Type_Primitive("u8", 1, 1);  type_u8->flags  = TF_NUMBER | TF_INTEGER | TF_UNSIGNED; all_types.append(type_u8);
    type_u16 = new Type_Primitive("u16", 2, 2); type_u16->flags = TF_NUMBER | TF_INTEGER | TF_UNSIGNED; all_types.append(type_u16);
    type_u32 = new Type_Primitive("u32", 4, 4); type_u32->flags = TF_NUMBER | TF_INTEGER | TF_UNSIGNED; all_types.append(type_u32);
    type_u64 = new Type_Primitive("u64", 8, 8); type_u64->flags = TF_NUMBER | TF_INTEGER | TF_UNSIGNED; all_types.append(type_u64);

    type_f32 = new Type_Primitive("f32", 4, 4); type_f32->flags = TF_NUMBER | TF_FLOAT | TF_SIGNED; all_types.append(type_f32);
    type_f64 = new Type_Primitive("f64", 8, 8); type_f64->flags = TF_NUMBER | TF_FLOAT | TF_SIGNED; all_types.append(type_f64);

    type_bool = new Type_Primitive("bool", 1, 1); all_types.append(type_bool);

    type_typeid = new Type_Primitive("typeid", 8, 8); all_types.append(type_typeid);
    type_string = new Type_Primitive("string", 16, 8); all_types.append(type_string);
    type_rawptr = new Type_Primitive("rawptr", 8, 8); all_types.append(type_rawptr); type_rawptr->flags = TF_POINTER;

    type_untyped_integer = new Type_Primitive("untyped integer", -1, -1); type_untyped_integer->flags = TF_NUMBER  | TF_UNTYPED | TF_INTEGER;
    type_untyped_float   = new Type_Primitive("untyped float", -1, -1);   type_untyped_float->flags   = TF_NUMBER  | TF_UNTYPED | TF_FLOAT;
    type_untyped_null    = new Type_Primitive("untyped null", -1, -1);    type_untyped_null->flags    = TF_POINTER | TF_UNTYPED;

    type_int = type_i64;
    type_float = type_f32;

    type_polymorphic = new Type_Primitive("type polymorphic", -1, -1); type_polymorphic->flags = TF_POLYMORPHIC;



    add_variable_type_field(type_string, "data", get_or_create_type_pointer_to(type_u8), 0);
    add_variable_type_field(type_string, "count", type_int, 8);
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
    register_declaration(new Type_Declaration("string", type_string, block));
    register_declaration(new Type_Declaration("rawptr", type_rawptr, block));
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

bool is_type_pointer    (Type *type) { return type->flags & TF_POINTER;     }
bool is_type_polymorphic(Type *type) { return type->flags & TF_POLYMORPHIC; }
bool is_type_reference  (Type *type) { return type->flags & TF_REFERENCE;   }
bool is_type_procedure  (Type *type) { return type->flags & TF_PROCEDURE;   }
bool is_type_array      (Type *type) { return type->flags & TF_ARRAY;       }
bool is_type_slice      (Type *type) { return type->flags & TF_SLICE;       }
bool is_type_number     (Type *type) { return type->flags & TF_NUMBER;      }
bool is_type_integer    (Type *type) { return type->flags & TF_INTEGER;     }
bool is_type_float      (Type *type) { return type->flags & TF_FLOAT;       }
bool is_type_bool       (Type *type) { return type == type_bool;            }
bool is_type_untyped    (Type *type) { return type->flags & TF_UNTYPED;     }
bool is_type_unsigned   (Type *type) { return type->flags & TF_UNSIGNED;    }
bool is_type_signed     (Type *type) { return type->flags & TF_SIGNED;      }
bool is_type_struct     (Type *type) { return type->flags & TF_STRUCT;      }
bool is_type_incomplete (Type *type) { return type->flags & TF_INCOMPLETE;  }
bool is_type_typeid     (Type *type) { return type == type_typeid;          }
bool is_type_string     (Type *type) { return type == type_string;          }

Type *get_most_concrete_type(Type *a, Type *b) {
    if (a->flags & TF_UNTYPED) {
        if (b->flags & TF_UNTYPED) {
            if ((a->flags & TF_FLOAT) && (b->flags & TF_INTEGER)) {
                return a;
            }
            else if ((b->flags & TF_FLOAT) && (a->flags & TF_INTEGER)) {
                return b;
            }
            else {
                assert(a == b);
                return a;
            }
        }
        else {
            return b;
        }
    }
    else {
        return a;
    }
}

Type *try_concretize_type_without_context(Type *type) {
    if (!(type->flags & TF_UNTYPED)) return type;
    if (type == type_untyped_integer) {
        return type_int;
    }
    if (type == type_untyped_float) {
        return type_float;
    }
    if (type == type_untyped_null) {
        return nullptr; // note(josh): nothing we can really do here, up to the caller to handle it
    }
    assert(false);
    return nullptr;
}

bool check_declaration(Declaration *decl, Location usage_location, Operand *out_operand = nullptr) {
    assert(!decl->is_polymorphic);
    if (decl->check_state == DCS_CHECKED) {
        if (out_operand) {
            *out_operand = decl->operand;
        }
        return true;
    }

    if (decl->check_state == DCS_CHECKING) {
        report_error(decl->location, "Cyclic dependency.");
        return false;
    }

    assert(decl->check_state == DCS_UNCHECKED);
    decl->check_state = DCS_CHECKING;

    Operand decl_operand;
    switch (decl->kind) {
        case DECL_TYPE: {
            Type_Declaration *type_decl = (Type_Declaration *)decl;
            decl_operand.type = type_typeid;
            decl_operand.type_value = type_decl->type;
            decl_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
            break;
        }
        case DECL_STRUCT: {
            Struct_Declaration *struct_decl = (Struct_Declaration *)decl;
            decl_operand.type = type_typeid;
            decl_operand.type_value = struct_decl->structure->type;
            decl_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
            break;
        }
        case DECL_POLYMORPHIC: {
            Polymorphic_Declaration *poly = (Polymorphic_Declaration *)decl;
            if (!poly->declaration) {
                report_error(usage_location, "Internal compiler error: polymorphic declaration has not yet been filled. todo(josh): do iterative solver.");
                return false;
            }
            assert(poly->declaration);
            if (!check_declaration(poly->declaration, usage_location)) {
                return false;
            }
            decl_operand = poly->declaration->operand;
            break;
        }
        case DECL_ENUM: {
            Enum_Declaration *enum_decl = (Enum_Declaration *)decl;
            Type_Enum *enum_type = new Type_Enum(enum_decl->name);
            enum_decl->ast_enum->type = enum_type;
            int enum_field_value = 0;
            For (idx, enum_decl->ast_enum->fields) {
                Enum_Field field = enum_decl->ast_enum->fields[idx];
                Operand *expr_operand = nullptr;
                if (field.expr) {
                    report_error(field.expr->location, "Enum field expressions are not yet supported.");
                    return false;
                    expr_operand = typecheck_expr(field.expr);
                    if (!expr_operand) {
                        return false;
                    }
                }
                Operand field_operand; // todo(josh): location info?
                field_operand.flags = OPERAND_CONSTANT | OPERAND_RVALUE;
                field_operand.type = enum_type;
                if (expr_operand) {
                    if (!(expr_operand->flags & OPERAND_CONSTANT)) {
                        report_error(expr_operand->location, "Enum fields must be constant.");
                        return false;
                    }
                    if (!is_type_integer(expr_operand->type)) {
                        report_error(expr_operand->location, "Enum fields must be integers.");
                        return false;
                    }
                    field_operand.int_value = expr_operand->int_value;
                }
                else {
                    field_operand.int_value = enum_field_value;
                    enum_field_value += 1;
                }
                add_constant_type_field(enum_type, field.name, field_operand);
            }
            decl_operand.type = type_typeid;
            decl_operand.type_value = enum_decl->ast_enum->type;
            decl_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
            break;
        }
        case DECL_VAR: {
            Var_Declaration *var_decl = (Var_Declaration *)decl;
            // todo(josh): check for use-before-declaration

            Ast_Var *var = var_decl->var;
            Type *declared_type = nullptr;
            if (var->type_expr) {
                Operand *type_operand = typecheck_expr(var->type_expr, type_typeid);
                if (!type_operand) {
                    return false;
                }
                assert(type_operand->flags & OPERAND_TYPE);
                assert(type_operand->type_value);
                declared_type = type_operand->type_value;
            }

            if (var->expr) {
                Operand *expr_operand = typecheck_expr(var->expr, declared_type);
                if (!expr_operand) {
                    return false;
                }
                if (declared_type) {
                    if (!match_types(expr_operand, declared_type)) {
                        assert(false);
                    }
                }
                else {
                    declared_type = try_concretize_type_without_context(expr_operand->type);
                    expr_operand->type = declared_type;
                    if (declared_type == nullptr) {
                        assert(expr_operand->type == type_untyped_null);
                        report_error(var->location, "Cannot infer type from expression `null`.");
                    }
                }

                if (var->is_constant) {
                    if (!(expr_operand->flags & OPERAND_CONSTANT)) {
                        report_error(var->expr->location, "Expression must be constant.");
                        return false;
                    }
                    else {
                        decl_operand = *expr_operand;
                        var->constant_operand = decl_operand;
                    }
                }
            }
            else {
                if (var->is_constant) {
                    report_error(var->location, "Constant must have an expression.");
                    return false;
                }
            }

            assert(declared_type != nullptr);

            var->type = declared_type;
            if (!complete_type(var->type)) {
                return false;
            }
            assert(var->type != nullptr);

            if (!var->is_constant) {
                decl_operand.type = var->type;
                decl_operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;
            }
            else {
                assert(decl_operand.flags != 0 && "decl_operand should have been filled in above");
                assert(decl_operand.type != nullptr && "decl_operand should have been filled in above");
            }
            break;
        }
        case DECL_PROC: {
            Proc_Declaration *proc_decl = (Proc_Declaration *)decl;
            if (!typecheck_procedure_header(proc_decl->procedure->header)) {
                return false;
            }
            assert(proc_decl->procedure->header->type != nullptr);
            decl_operand = proc_decl->procedure->header->operand;
            decl_operand.referenced_procedure = proc_decl->procedure;
            break;
        }
        case DECL_CONSTANT_VALUE: {
            Constant_Declaration *constant = (Constant_Declaration *)decl;
            decl_operand = constant->operand;
            break;
        }
        default: {
            printf("unhandled case: %d\n", decl->kind);
            assert(false);
        }
    }
    decl->operand = decl_operand;
    decl->check_state = DCS_CHECKED;

    if (decl->kind == DECL_PROC) {
        Proc_Declaration *proc_decl = (Proc_Declaration *)decl;
        if (!proc_decl->procedure->header->is_foreign) {
            assert(proc_decl->procedure->body != nullptr);
            if (!is_type_polymorphic(proc_decl->procedure->header->type)) {
                if (!typecheck_block(proc_decl->procedure->body)) {
                    return false;
                }
            }
            else {
                proc_decl->is_polymorphic = true;
            }
        }
        else {
            assert(proc_decl->procedure->body == nullptr);
        }
    }

    if (decl->kind == DECL_STRUCT) {
        Struct_Declaration *struct_decl = (Struct_Declaration *)decl;
        For (idx, struct_decl->structure->operator_overloads) {
            // todo(josh): this should just go through the normal check_declaration() path. having it copy pasted here is dumb
            Ast_Proc *proc = struct_decl->structure->operator_overloads[idx];
            assert(!proc->header->is_foreign);
            assert(proc->body != nullptr);
            if (!typecheck_procedure_header(proc->header)) {
                return false;
            }
            if (!typecheck_block(proc->body)) {
                return false;
            }
        }
    }
    else {
        if (decl->parent_block->flags & BF_IS_GLOBAL_SCOPE) {
            if (!decl->is_polymorphic) {
                ordered_declarations.append(decl);
            }
        }
    }

    if (out_operand) {
        *out_operand = decl_operand;
    }
    return true;
}

bool typecheck_global_scope(Ast_Block *block) {
    assert(block->flags & BF_IS_GLOBAL_SCOPE);
    make_incomplete_types_for_all_structs(); // todo(josh): this is kinda goofy. should be able to just do this as we traverse the program
    For (idx, block->declarations) {
        Declaration *decl = block->declarations[idx];
        if (decl->is_polymorphic) {
            continue;
        }
        if (!check_declaration(decl, decl->location)) {
            return false;
        }
    }
    if (!do_assert_directives()) {
        return false;
    }
    if (!do_print_directives()) {
        return false;
    }

    // note(josh): complete any types that haven't been completed because they haven't been used.
    //             we have to do this because using a struct type by pointer will NOT trigger
    //             a call to complete_type(), only actually using the type and it's members will.
    For (idx, all_types) {
        Type *type = all_types[idx];
        if (!complete_type(type)) {
            return false;
        }
    }
    return true;
}

char *type_to_string(Type *type) {
    // todo(josh): janky allocation
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
        case TYPE_PROCEDURE: {
            assert(false && "unimplemented");
            break;
        }
        case TYPE_POINTER: {
            Type_Pointer *pointer = (Type_Pointer *)type;
            sprintf(buffer, "^%s", type_to_string(pointer->pointer_to));
            break;
        }
        case TYPE_REFERENCE: {
            Type_Reference *reference = (Type_Reference *)type;
            sprintf(buffer, ">%s", type_to_string(reference->reference_to));
            break;
        }
        case TYPE_ARRAY: {
            Type_Array *array = (Type_Array *)type;
            sprintf(buffer, "[%d]%s", array->count, type_to_string(array->array_of));
            break;
        }
        case TYPE_SLICE: {
            Type_Slice *slice = (Type_Slice *)type;
            sprintf(buffer, "[]%s", type_to_string(slice->slice_of));
            break;
        }
        case TYPE_ENUM: {
            Type_Enum *enum_type = (Type_Enum *)type;
            sprintf(buffer, "%s", enum_type->name);
            break;
        }
        default: {
            assert(false);
        }
    }
    return buffer;
}

bool complete_type(Type *type) {
    if (type->check_state == CS_CHECKED) {
        return true;
    }
    if (type->check_state == CS_CHECKING) {
        report_error({}, "Circular dependency."); // todo(josh): better error message
        return false;
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
                int size = 0;
                int largest_alignment = 1;
                if (structure->fields.count == 0) {
                    size = 1;
                }
                else {
                    For (idx, structure->fields) {
                        Ast_Var *var = structure->fields[idx];
                        assert(check_declaration(var->declaration, var->location));
                        if (!complete_type(var->type)) {
                            return false;
                        }
                    }

                    For (idx, structure->fields) {
                        Ast_Var *var = structure->fields[idx];
                        if (var->is_constant) {
                            assert(var->expr != nullptr);
                            assert(var->constant_operand.type != nullptr);
                            assert(var->constant_operand.flags & OPERAND_CONSTANT);
                            add_constant_type_field(struct_type, var->name, var->constant_operand);
                        }
                        else {
                            assert(var->type->size > 0);
                            // todo(josh): alignment
                            int next_alignment = -1;
                            for (int i = idx; i < (structure->fields.count-1); i++) {
                                Ast_Var *next_var = structure->fields[i+1];
                                if (!var->is_constant) {
                                    next_alignment = next_var->type->align;
                                    assert(next_alignment > 0);
                                    break;
                                }
                            }
                            add_variable_type_field(struct_type, var->name, var->type, size);

                            int size_delta = var->type->size;
                            if (next_alignment != -1) {
                                size_delta += modulo((next_alignment - var->type->size), next_alignment);
                            }
                            size += size_delta;
                            largest_alignment = max(largest_alignment, var->type->align);
                        }
                    }
                }

                assert(size > 0);
                struct_type->size = (int)align_forward((uintptr_t)size, (uintptr_t)largest_alignment);
                struct_type->align = largest_alignment;
                struct_type->flags &= ~(TF_INCOMPLETE);

                assert(structure->parent_block->flags & BF_IS_GLOBAL_SCOPE); // todo(josh): locally scoped structs and procs
                ordered_declarations.append(structure->declaration);
                break;
            }
            case TYPE_ARRAY: {
                Type_Array *array_type = (Type_Array *)type;
                if (!complete_type(array_type->array_of)) {
                    return false; // todo(josh): error message
                }
                assert(array_type->count > 0);
                assert(array_type->array_of->size > 0);
                assert(array_type->array_of->align > 0);
                array_type->size = array_type->array_of->size * array_type->count;
                array_type->align = array_type->array_of->align;
                assert(array_type->size > 0);
                array_type->flags &= ~(TF_INCOMPLETE);
                break;
            }
        }
    }
    return true;
}

void type_mismatch(Location location, Type *got, Type *expected) {
    report_error(location, "Type mismatch. Expected '%s', got '%s'.", type_to_string(expected), type_to_string(got));
}

bool match_types(Operand *operand, Type *expected_type, bool do_report_error) {
    assert(operand->type != nullptr);
    assert(expected_type != nullptr);

    if (operand->type == expected_type) {
        return true;
    }

    if (is_type_reference(expected_type)) {
        if ((operand->flags & OPERAND_LVALUE)) {
            return true;
        }
        Type_Reference *reference = (Type_Reference *)expected_type;
        return match_types(operand, reference->reference_to, do_report_error);
    }

    if (is_type_reference(operand->type)) {
        Type_Reference *reference_type = (Type_Reference *)operand->type;
        if (reference_type->reference_to == expected_type) {
            return true;
        }
    }

    if (operand->type->flags & TF_UNTYPED) {
        if (is_type_number(operand->type) && is_type_number(expected_type)) {
            if (is_type_integer(expected_type) && is_type_float(operand->type)) {
                report_error(operand->location, "Expected an integer type, got a float.");
                return false;
            }
            operand->type = expected_type;
            return true;
        }

        if (is_type_pointer(operand->type) && is_type_pointer(expected_type)) {
            assert(operand->type == type_untyped_null);
            operand->type = expected_type;
            return true;
        }
    }

    if ((expected_type == type_rawptr) && (is_type_pointer(operand->type))) {
        return true;
    }

    if (do_report_error) {
        type_mismatch(operand->location, operand->type, expected_type);
    }
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
    new_type->align = POINTER_SIZE;
    all_types.append(new_type);
    return new_type;
}

Type_Reference *get_or_create_type_reference_to(Type *reference_to) {
    assert(!is_type_untyped(reference_to));
    For (idx, all_types) { // todo(josh): @Speed maybe have an `all_reference_types` array
        Type *other_type = all_types[idx];
        if (other_type->kind == TYPE_REFERENCE) {
            Type_Reference *other_type_reference = (Type_Reference *)other_type;
            if (other_type_reference->reference_to == reference_to) {
                return other_type_reference;
            }
        }
    }
    Type_Reference *new_type = new Type_Reference(reference_to);
    new_type->flags = TF_REFERENCE;
    new_type->size = POINTER_SIZE;
    new_type->align = POINTER_SIZE;
    all_types.append(new_type);
    return new_type;
}

Type_Array *get_or_create_type_array_of(Type *array_of, Operand *count_operand) {
    assert(array_of != nullptr);
    assert(!is_type_untyped(array_of));
    assert(count_operand->flags & OPERAND_CONSTANT);
    assert(is_type_integer(count_operand->type));
    For (idx, all_types) { // todo(josh): @Speed maybe have an `all_array_types` array
        Type *other_type = all_types[idx];
        if (other_type->kind == TYPE_ARRAY) {
            Type_Array *other_type_array = (Type_Array *)other_type;
            if (other_type_array->array_of == array_of && other_type_array->count == count_operand->int_value) {
                return other_type_array;
            }
        }
    }
    Type_Array *new_type = new Type_Array(array_of, count_operand->int_value);
    new_type->flags = TF_ARRAY | TF_INCOMPLETE;
    add_variable_type_field(new_type, "data", type_rawptr, 0);
    add_constant_type_field(new_type, "count", *count_operand);
    all_types.append(new_type);
    return new_type;
}

Type_Slice *get_or_create_type_slice_of(Type *slice_of) {
    assert(!is_type_untyped(slice_of));
    For (idx, all_types) { // todo(josh): @Speed maybe have an `all_slice_types` array
        Type *other_type = all_types[idx];
        if (other_type->kind == TYPE_SLICE) {
            Type_Slice *other_type_slice = (Type_Slice *)other_type;
            if (other_type_slice->slice_of == slice_of) {
                return other_type_slice;
            }
        }
    }
    Type_Pointer *pointer_to_element_type = get_or_create_type_pointer_to(slice_of);
    Type_Slice *new_type = new Type_Slice(slice_of, pointer_to_element_type);
    new_type->flags = TF_SLICE;
    new_type->size  = 16;
    new_type->align = 8;
    add_variable_type_field(new_type, "data", pointer_to_element_type, 0);
    add_variable_type_field(new_type, "count", type_int, 8);
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
    For (idx, parameter_types) {
        if (is_type_polymorphic(parameter_types[idx])) {
            new_type->flags |= TF_POLYMORPHIC;
        }
    }
    new_type->size = POINTER_SIZE;
    new_type->align = POINTER_SIZE;
    all_types.append(new_type);
    return new_type;
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

bool operator_is_defined(Type *lhs, Type *rhs, Token_Kind op) {
    switch (op) {
        case TK_EQUAL_TO: {
                 if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            else if (is_type_float(lhs)   && is_type_float(rhs))    return true;
            else if (is_type_bool(lhs)    && is_type_bool(rhs))     return true;
            else if (is_type_typeid(lhs)  && is_type_typeid(rhs))   return true;
            else if (is_type_string(lhs)  && is_type_string(rhs))   return true;
            else if (is_type_pointer(lhs) && is_type_pointer(rhs))  return true;
            break;
        }
        case TK_NOT_EQUAL_TO: {
                 if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            else if (is_type_float(lhs)   && is_type_float(rhs))    return true;
            else if (is_type_bool(lhs)    && is_type_bool(rhs))     return true;
            else if (is_type_typeid(lhs)  && is_type_typeid(rhs))   return true;
            else if (is_type_string(lhs)  && is_type_string(rhs))   return true;
            else if (is_type_pointer(lhs) && is_type_pointer(rhs))  return true;
            break;
        }
        case TK_PLUS: {
                 if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            else if (is_type_float(lhs)   && is_type_float(rhs))    return true;
            else if (is_type_string(lhs)  && is_type_string(rhs))   return true;
            break;
        }
        case TK_MINUS: {
                 if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            else if (is_type_float(lhs)   && is_type_float(rhs))    return true;
            break;
        }
        case TK_MULTIPLY: {
                 if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            else if (is_type_float(lhs)   && is_type_float(rhs))    return true;
            break;
        }
        case TK_DIVIDE: {
                 if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            else if (is_type_float(lhs)   && is_type_float(rhs))    return true;
            break;
        }
        case TK_AMPERSAND: { // note(josh): BIT_AND
                 if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            break;
        }
        case TK_BIT_OR: {
                 if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            break;
        }
        case TK_LESS_THAN: {
                 if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            else if (is_type_float(lhs)   && is_type_float(rhs))    return true;
            break;
        }
        case TK_LESS_THAN_OR_EQUAL: {
                 if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            else if (is_type_float(lhs)   && is_type_float(rhs))    return true;
            break;
        }
        case TK_GREATER_THAN: {
                 if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            else if (is_type_float(lhs)   && is_type_float(rhs))    return true;
            break;
        }
        case TK_GREATER_THAN_OR_EQUAL: {
                 if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            else if (is_type_float(lhs)   && is_type_float(rhs))    return true;
            break;
        }
        case TK_BOOLEAN_AND: {
                 if (is_type_bool(lhs) && is_type_bool(rhs))        return true;
            break;
        }
        case TK_BOOLEAN_OR: {
                 if (is_type_bool(lhs) && is_type_bool(rhs))        return true;
            break;
        }
        case TK_LEFT_SHIFT: {
            UNIMPLEMENTED(TK_LEFT_SHIFT);
        }
        case TK_RIGHT_SHIFT: {
            UNIMPLEMENTED(TK_RIGHT_SHIFT);
        }
        default: {
            printf("Unhandled operator: %s\n", token_string(op));
            assert(false);
        }
    }
    return false;
}

bool type_declares_polymorphic_variable(Ast_Expr *expr) {
    // todo(josh): handle more cases
    switch (expr->expr_kind) {
        case EXPR_POLYMORPHIC_VARIABLE: {
            return true;
        }
    }
    return false;
}

int total_num_polymorphs = 0;

Ast_Proc *polymorph_procedure(Ast_Proc *proc, Location polymorph_location, Array<Declaration *> replacements) {
    // re-parse the procedure to be polymorphed
    Ast_Proc *original_procedure = proc;
    Lexer lexer(proc->location.filepath, proc->location.text);
    lexer.location = proc->location;
    lexer.location.index = 0;
    Ast_Block *old_block = push_ast_block(proc->parent_block);
    defer(pop_ast_block(old_block));
    String_Builder sb = make_string_builder(default_allocator());
    sb.print(proc->header->name);
    sb.printf("__polymorph_%d", total_num_polymorphs);
    total_num_polymorphs += 1;
    Ast_Proc *procedure_polymorph = parse_proc(&lexer, sb.string());
    assert(procedure_polymorph != nullptr);
    original_procedure->header->polymorphs.append(procedure_polymorph);

    // insert the declaration replacements
    For (idx, replacements) {
        Declaration *replacement = replacements[idx];
        assert(replacement->parent_block == nullptr);
        bool inserted_replacement = false;
        For (decl_idx, procedure_polymorph->header->procedure_block->declarations) {
            Declaration *decl = procedure_polymorph->header->procedure_block->declarations[decl_idx];
            if (decl->kind == DECL_POLYMORPHIC && (strcmp(decl->name, replacement->name) == 0)) {
                Polymorphic_Declaration *poly_decl = (Polymorphic_Declaration *)decl;
                assert(poly_decl->declaration == nullptr);
                replacement->parent_block = procedure_polymorph->header->procedure_block;
                poly_decl->declaration = replacement;
                inserted_replacement = true;
            }
        }
        assert(inserted_replacement);
    }

    for (int idx = procedure_polymorph->header->parameters.count-1; idx >= 0; idx--) {
        Ast_Var *parameter = procedure_polymorph->header->parameters[idx];
        if (parameter->is_polymorphic_value) {
            procedure_polymorph->header->parameters.ordered_remove(idx);
        }
    }

    procedure_polymorph->declaration->is_polymorphic = false;
    if (!check_declaration(procedure_polymorph->declaration, polymorph_location)) {
        return nullptr;
    }

    return procedure_polymorph;
}

bool maybe_create_polymorph_type_declaration(Ast_Var *var, Ast_Expr *parameter, Declaration **out_declaration) {
    switch (var->type_expr->expr_kind) {
        case EXPR_IDENTIFIER: {
            break;
        }
        case EXPR_POLYMORPHIC_VARIABLE: {
            // name: $T
            Expr_Polymorphic_Variable *poly = (Expr_Polymorphic_Variable *)var->type_expr;
            assert(poly->poly_decl->declaration == nullptr);
            Operand *parameter_operand = typecheck_expr(parameter);
            assert(parameter_operand->type != nullptr);
            Type *concrete_type = try_concretize_type_without_context(parameter_operand->type);
            if (!concrete_type) {
                assert(false && "todo(josh): error message");
            }
            *out_declaration = new Type_Declaration(poly->ident->name, concrete_type, nullptr);
            return true;
        }
        default: {
            assert(false);
        }
    }
    return true;
}

bool maybe_create_polymorph_value_declaration(Ast_Var *var, Ast_Expr *parameter, Declaration **out_declaration) {
    switch (var->name_expr->expr_kind) {
        case EXPR_IDENTIFIER: {
            break;
        }
        case EXPR_POLYMORPHIC_VARIABLE: {
            // $name: T
            Expr_Polymorphic_Variable *poly = (Expr_Polymorphic_Variable *)var->name_expr;
            assert(poly->poly_decl->declaration == nullptr);
            Operand *type_operand = typecheck_expr(var->type_expr);
            Operand *parameter_operand = nullptr;
            if (!is_type_polymorphic(type_operand->type)) {
                parameter_operand = typecheck_expr(parameter, type_operand->type_value);
                if (!parameter_operand) {
                    return false;
                }
            }
            else {
                parameter_operand = typecheck_expr(parameter);
            }
            assert(parameter_operand->type != nullptr);
            if (!(parameter_operand->flags & OPERAND_CONSTANT)) {
                report_error(parameter->location, "Parameter must be constant for polymorphic variable.");
                return false;
            }
            *out_declaration = new Constant_Declaration(poly->ident->name, *parameter_operand, nullptr, parameter->location);
            return true;
        }
        default: {
            assert(false);
        }
    }
    return true;
}

bool typecheck_procedure_call(Ast_Expr *expr, Operand procedure_operand, Array<Ast_Expr *> parameters, Operand *out_operand, Array<Ast_Expr *> *out_params_to_emit) {
    out_params_to_emit->allocator = default_allocator();

    // copy all the exprs into out_params_to_emit
    For (idx, parameters) {
        out_params_to_emit->append(parameters[idx]);
    }

    // todo(josh): deduplicate polymorphs

    Ast_Proc *procedure_polymorph = nullptr; // todo(josh): it would be nice if we could remove this to keep the poly stuff localized

    Type_Procedure *target_procedure_type = nullptr;
    if (is_type_polymorphic(procedure_operand.type)) {
        assert(procedure_operand.referenced_procedure != nullptr);

        Array<Declaration *> replacements = {};
        replacements.allocator = default_allocator();

        // go through the parameters and fill in polymorphic declarations
        For (idx, procedure_operand.referenced_procedure->header->parameters) {
            Ast_Var *param_decl = procedure_operand.referenced_procedure->header->parameters[idx];
            Operand *parameter_operand = nullptr;
            Ast_Expr *parameter = parameters[idx];
            Declaration *poly_type_decl = nullptr;
            Declaration *poly_value_decl = nullptr;
            bool ok = false;
            ok = maybe_create_polymorph_type_declaration (param_decl, parameter, &poly_type_decl);
            ok &= maybe_create_polymorph_value_declaration(param_decl, parameter, &poly_value_decl);
            if (!ok) {
                return false;
            }

            if (poly_type_decl) {
                replacements.append(poly_type_decl);
            }

            if (poly_value_decl) {
                replacements.append(poly_value_decl);
                out_params_to_emit->ordered_remove(idx);
            }
        }

        procedure_polymorph = polymorph_procedure(procedure_operand.referenced_procedure, expr->location, replacements);
        if (procedure_polymorph == nullptr) {
            report_info(expr->location, "This polymorph was generated here.");
            return false;
        }

        assert(!is_type_polymorphic(procedure_polymorph->declaration->operand.type));
        procedure_operand = procedure_polymorph->declaration->operand;
        assert(is_type_procedure(procedure_operand.type));
        target_procedure_type = (Type_Procedure *)procedure_operand.type;
    }
    else {
        assert(procedure_operand.type->kind == TYPE_PROCEDURE);
        target_procedure_type = (Type_Procedure *)procedure_operand.type;
        assert(target_procedure_type->parameter_types.count == parameters.count);
    }

    // typecheck the other parameters
    For (idx, *out_params_to_emit) {
        Type *parameter_type = target_procedure_type->parameter_types[idx];
        Ast_Expr *parameter = (*out_params_to_emit)[idx];
        if (parameter->operand.type == nullptr) {
            Operand *parameter_operand = typecheck_expr(parameter, target_procedure_type->parameter_types[idx]);
            if (!parameter_operand) {
                return false;
            }
        }
    }

    Operand result_operand(expr->location);
    result_operand.type = target_procedure_type->return_type;
    if (result_operand.type) {
        result_operand.flags = OPERAND_RVALUE;
    }
    else {
        result_operand.flags = OPERAND_NO_VALUE;
    }
    *out_operand = result_operand;
    if (procedure_polymorph != nullptr) {
        assert(procedure_operand.referenced_procedure != nullptr);
        out_operand->referenced_procedure = procedure_polymorph;
    }
    return true;
}

Ast_Proc *find_operator_overload(Ast_Struct *structure, Array<Ast_Expr *> exprs, Token_Kind op) {
    For (overload_idx, structure->operator_overloads) {
        Ast_Proc *proc = structure->operator_overloads[overload_idx];
        assert(proc->header->operator_to_overload != TK_INVALID);
        if (proc->header->operator_to_overload != op) {
            continue;
        }
        if (proc->header->parameters.count != exprs.count) {
            continue;
        }
        bool all_matched = true;
        For (expr_idx, exprs) {
            Ast_Expr *expr = exprs[expr_idx];
            assert(expr->operand.type != nullptr);
            assert(proc->header->parameters[expr_idx]->type != nullptr);
            // speculatively try to match the types
            Operand operand_copy_for_types_match = expr->operand;
            if (!match_types(&operand_copy_for_types_match, proc->header->parameters[expr_idx]->type, false)) {
                all_matched = false;
                break;
            }
        }
        if (all_matched) {
            // actually match the types
            For (expr_idx, exprs) {
                Ast_Expr *expr = exprs[expr_idx];
                assert(match_types(&expr->operand, proc->header->parameters[expr_idx]->type, false));
            }
            return proc;
        }
    }
    return nullptr;
}

Type *unwrap_type(Type *type) {
    assert(type != nullptr);
    bool did_something = true;
    do {
        did_something = false;
        while (type->kind == TYPE_REFERENCE) {
            did_something = true;
            Type_Reference *reference = (Type_Reference *)type;
            assert(reference->reference_to != nullptr);
            type = reference->reference_to;
        }
    } while (did_something);

    return type;
}

Ast_Expr *unparen_expr(Ast_Expr *expr) {
    assert(expr != nullptr);
    while (expr->expr_kind == EXPR_PAREN) {
        Expr_Paren *paren = (Expr_Paren *)expr;
        expr = paren->nested;
    }
    return expr;
}

Operand *typecheck_expr(Ast_Expr *expr, Type *expected_type) {
    assert(expr != nullptr);
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
            if (is_cmp_op(binary->op)) {
                expected_type = nullptr;
            }
            Operand *lhs_operand = typecheck_expr(binary->lhs);
            if (!lhs_operand) {
                return nullptr;
            }
            Operand *rhs_operand = typecheck_expr(binary->rhs);
            if (!rhs_operand) {
                return nullptr;
            }
            assert(!is_type_incomplete(lhs_operand->type));
            assert(!is_type_incomplete(rhs_operand->type));
            result_operand.flags |= OPERAND_RVALUE;
            Type *most_concrete = get_most_concrete_type(lhs_operand->type, rhs_operand->type);
            if (expected_type != nullptr) {
                most_concrete = get_most_concrete_type(most_concrete, expected_type);
            }

            // speculatively try to match the types to see if we should go into the
            // normal binop path or the operator overload path
            Operand lhs_operand_copy = *lhs_operand;
            Operand rhs_operand_copy = *rhs_operand;
            bool types_matched = match_types(&lhs_operand_copy, most_concrete, false);
            types_matched = types_matched && match_types(&rhs_operand_copy, most_concrete, false);

            if (types_matched && operator_is_defined(lhs_operand_copy.type, rhs_operand_copy.type, binary->op)) {
                Operand *lhs_operand = typecheck_expr(binary->lhs, expected_type);
                if (!lhs_operand) {
                    return nullptr;
                }
                Operand *rhs_operand = typecheck_expr(binary->rhs, expected_type);
                if (!rhs_operand) {
                    return nullptr;
                }

                assert(match_types(lhs_operand, most_concrete));
                assert(match_types(rhs_operand, most_concrete));
                switch (binary->op) {
                    case TK_EQUAL_TO: {
                        result_operand.type = type_bool;
                        if ((lhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_CONSTANT)) {
                            result_operand.flags |= OPERAND_CONSTANT;
                                 if (is_type_integer(lhs_operand->type) && is_type_integer(rhs_operand->type))  result_operand.bool_value = lhs_operand->int_value    == rhs_operand->int_value;
                            else if (is_type_float(lhs_operand->type)   && is_type_float(rhs_operand->type))    result_operand.bool_value = lhs_operand->float_value  == rhs_operand->float_value;
                            else if (is_type_bool(lhs_operand->type)    && is_type_bool(rhs_operand->type))     result_operand.bool_value = lhs_operand->bool_value   == rhs_operand->bool_value;
                            else if (is_type_typeid(lhs_operand->type)  && is_type_typeid(rhs_operand->type))   result_operand.bool_value = lhs_operand->type_value   == rhs_operand->type_value;
                            else if (is_type_string(lhs_operand->type)  && is_type_string(rhs_operand->type))   result_operand.bool_value = (lhs_operand->escaped_string_length == rhs_operand->escaped_string_length) && (strcmp(lhs_operand->escaped_string_value, rhs_operand->escaped_string_value) == 0);
                            else {
                                report_error(binary->location, "Operator == is unsupported for types '%s' and '%s'.", type_to_string(lhs_operand->type), type_to_string(rhs_operand->type));
                                return nullptr;
                            }
                        }
                        break;
                    }
                    case TK_NOT_EQUAL_TO: {
                        result_operand.type = type_bool;
                        if ((lhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_CONSTANT)) {
                            result_operand.flags |= OPERAND_CONSTANT;
                                 if (is_type_integer(lhs_operand->type) && is_type_integer(rhs_operand->type))  result_operand.bool_value = lhs_operand->int_value   != rhs_operand->int_value;
                            else if (is_type_float(lhs_operand->type)   && is_type_float(rhs_operand->type))    result_operand.bool_value = lhs_operand->float_value != rhs_operand->float_value;
                            else if (is_type_bool(lhs_operand->type)    && is_type_bool(rhs_operand->type))     result_operand.bool_value = lhs_operand->bool_value  != rhs_operand->bool_value;
                            else if (is_type_typeid(lhs_operand->type)  && is_type_typeid(rhs_operand->type))   result_operand.bool_value = lhs_operand->type_value  != rhs_operand->type_value;
                            else if (is_type_string(lhs_operand->type)  && is_type_string(rhs_operand->type))   result_operand.bool_value = (lhs_operand->escaped_string_length != rhs_operand->escaped_string_length) || (strcmp(lhs_operand->escaped_string_value, rhs_operand->escaped_string_value) != 0);
                            else {
                                report_error(binary->location, "Operator != is unsupported for types '%s' and '%s'.", type_to_string(lhs_operand->type), type_to_string(rhs_operand->type));
                                return nullptr;
                            }
                        }
                        break;
                    }
                    case TK_PLUS: {
                        result_operand.type = most_concrete;
                        if ((lhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_CONSTANT)) {
                            result_operand.flags |= OPERAND_CONSTANT;
                                 if (is_type_integer(lhs_operand->type) && is_type_integer(rhs_operand->type))  result_operand.int_value   = lhs_operand->int_value   + rhs_operand->int_value;
                            else if (is_type_float(lhs_operand->type)   && is_type_float(rhs_operand->type))    result_operand.float_value = lhs_operand->float_value + rhs_operand->float_value;
                            else if (is_type_string(lhs_operand->type)  && is_type_string(rhs_operand->type)) {
                                int total_length_scanned = (lhs_operand->scanned_string_length + rhs_operand->scanned_string_length);
                                int total_length_escaped = (lhs_operand->escaped_string_length + rhs_operand->escaped_string_length);
                                char *new_scanned_string = (char *)alloc(default_allocator(), total_length_scanned+1);
                                memcpy(new_scanned_string, lhs_operand->scanned_string_value, lhs_operand->scanned_string_length);
                                memcpy(new_scanned_string+lhs_operand->scanned_string_length, rhs_operand->scanned_string_value, rhs_operand->scanned_string_length);
                                char *new_escaped_string = (char *)alloc(default_allocator(), total_length_escaped+1);
                                memcpy(new_escaped_string, lhs_operand->escaped_string_value, lhs_operand->escaped_string_length);
                                memcpy(new_escaped_string+lhs_operand->escaped_string_length, rhs_operand->escaped_string_value, rhs_operand->escaped_string_length);
                                new_scanned_string[total_length_scanned] = '\0';
                                new_escaped_string[total_length_escaped] = '\0';
                                result_operand.scanned_string_value = new_scanned_string;
                                result_operand.scanned_string_length = total_length_scanned;
                                result_operand.escaped_string_value = new_escaped_string;
                                result_operand.escaped_string_length = total_length_escaped;
                            }
                            else {
                                report_error(binary->location, "Operator + is unsupported for types '%s' and '%s'.", type_to_string(lhs_operand->type), type_to_string(rhs_operand->type));
                                return nullptr;
                            }
                        }
                        break;
                    }
                    case TK_MINUS: {
                        result_operand.type = most_concrete;
                        if ((lhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_CONSTANT)) {
                            result_operand.flags |= OPERAND_CONSTANT;
                                 if (is_type_integer(lhs_operand->type) && is_type_integer(rhs_operand->type))  result_operand.int_value   = lhs_operand->int_value   - rhs_operand->int_value;
                            else if (is_type_float(lhs_operand->type)   && is_type_float(rhs_operand->type))    result_operand.float_value = lhs_operand->float_value - rhs_operand->float_value;
                            else {
                                report_error(binary->location, "Operator - is unsupported for types '%s' and '%s'.", type_to_string(lhs_operand->type), type_to_string(rhs_operand->type));
                                return nullptr;
                            }
                        }
                        break;
                    }
                    case TK_MULTIPLY: {
                        result_operand.type = most_concrete;
                        if ((lhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_CONSTANT)) {
                            result_operand.flags |= OPERAND_CONSTANT;
                                 if (is_type_integer(lhs_operand->type) && is_type_integer(rhs_operand->type))  result_operand.int_value   = lhs_operand->int_value   * rhs_operand->int_value;
                            else if (is_type_float(lhs_operand->type)   && is_type_float(rhs_operand->type))    result_operand.float_value = lhs_operand->float_value * rhs_operand->float_value;
                            else {
                                report_error(binary->location, "Operator * is unsupported for types '%s' and '%s'.", type_to_string(lhs_operand->type), type_to_string(rhs_operand->type));
                                return nullptr;
                            }
                        }
                        break;
                    }
                    case TK_DIVIDE: {
                        result_operand.type = most_concrete;
                        if ((lhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_CONSTANT)) {
                            result_operand.flags |= OPERAND_CONSTANT;
                                 if (is_type_integer(lhs_operand->type) && is_type_integer(rhs_operand->type))  result_operand.int_value   = lhs_operand->int_value   / rhs_operand->int_value;
                            else if (is_type_float(lhs_operand->type)   && is_type_float(rhs_operand->type))    result_operand.float_value = lhs_operand->float_value / rhs_operand->float_value;
                            else {
                                report_error(binary->location, "Operator / is unsupported for types '%s' and '%s'.", type_to_string(lhs_operand->type), type_to_string(rhs_operand->type));
                                return nullptr;
                            }
                        }
                        break;
                    }
                    case TK_AMPERSAND: { // note(josh): BIT_AND
                        result_operand.type = most_concrete;
                        if ((lhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_CONSTANT)) {
                            result_operand.flags |= OPERAND_CONSTANT;
                                 if (is_type_integer(lhs_operand->type) && is_type_integer(rhs_operand->type))  result_operand.int_value = lhs_operand->int_value & rhs_operand->int_value;
                            else {
                                report_error(binary->location, "Operator / is unsupported for types '%s' and '%s'.", type_to_string(lhs_operand->type), type_to_string(rhs_operand->type));
                                return nullptr;
                            }
                        }
                        break;
                    }
                    case TK_BIT_OR: {
                        result_operand.type = most_concrete;
                        if ((lhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_CONSTANT)) {
                            result_operand.flags |= OPERAND_CONSTANT;
                                 if (is_type_integer(lhs_operand->type) && is_type_integer(rhs_operand->type))  result_operand.int_value = lhs_operand->int_value | rhs_operand->int_value;
                            else {
                                report_error(binary->location, "Operator / is unsupported for types '%s' and '%s'.", type_to_string(lhs_operand->type), type_to_string(rhs_operand->type));
                                return nullptr;
                            }
                        }
                        break;
                    }
                    case TK_LESS_THAN: {
                        result_operand.type = type_bool;
                        if ((lhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_CONSTANT)) {
                            result_operand.flags |= OPERAND_CONSTANT;
                                 if (is_type_integer(lhs_operand->type) && is_type_integer(rhs_operand->type))  result_operand.int_value   = lhs_operand->int_value   < rhs_operand->int_value;
                            else if (is_type_float(lhs_operand->type)   && is_type_float(rhs_operand->type))    result_operand.float_value = lhs_operand->float_value < rhs_operand->float_value;
                            else {
                                report_error(binary->location, "Operator < is unsupported for types '%s' and '%s'.", type_to_string(lhs_operand->type), type_to_string(rhs_operand->type));
                                return nullptr;
                            }
                        }
                        break;
                    }
                    case TK_LESS_THAN_OR_EQUAL: {
                        result_operand.type = type_bool;
                        if ((lhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_CONSTANT)) {
                            result_operand.flags |= OPERAND_CONSTANT;
                                 if (is_type_integer(lhs_operand->type) && is_type_integer(rhs_operand->type))  result_operand.int_value   = lhs_operand->int_value   <= rhs_operand->int_value;
                            else if (is_type_float(lhs_operand->type)   && is_type_float(rhs_operand->type))    result_operand.float_value = lhs_operand->float_value <= rhs_operand->float_value;
                            else {
                                report_error(binary->location, "Operator <= is unsupported for types '%s' and '%s'.", type_to_string(lhs_operand->type), type_to_string(rhs_operand->type));
                                return nullptr;
                            }
                        }
                        break;
                    }
                    case TK_GREATER_THAN: {
                        result_operand.type = type_bool;
                        if ((lhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_CONSTANT)) {
                            result_operand.flags |= OPERAND_CONSTANT;
                                 if (is_type_integer(lhs_operand->type) && is_type_integer(rhs_operand->type))  result_operand.int_value   = lhs_operand->int_value   > rhs_operand->int_value;
                            else if (is_type_float(lhs_operand->type)   && is_type_float(rhs_operand->type))    result_operand.float_value = lhs_operand->float_value > rhs_operand->float_value;
                            else {
                                report_error(binary->location, "Operator > is unsupported for types '%s' and '%s'.", type_to_string(lhs_operand->type), type_to_string(rhs_operand->type));
                                return nullptr;
                            }
                        }
                        break;
                    }
                    case TK_GREATER_THAN_OR_EQUAL: {
                        result_operand.type = type_bool;
                        if ((lhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_CONSTANT)) {
                            result_operand.flags |= OPERAND_CONSTANT;
                                 if (is_type_integer(lhs_operand->type) && is_type_integer(rhs_operand->type))  result_operand.int_value   = lhs_operand->int_value   >= rhs_operand->int_value;
                            else if (is_type_float(lhs_operand->type)   && is_type_float(rhs_operand->type))    result_operand.float_value = lhs_operand->float_value >= rhs_operand->float_value;
                            else {
                                report_error(binary->location, "Operator >= is unsupported for types '%s' and '%s'.", type_to_string(lhs_operand->type), type_to_string(rhs_operand->type));
                                return nullptr;
                            }
                        }
                        break;
                    }
                    case TK_BOOLEAN_AND: {
                        result_operand.type = type_bool;
                        if ((lhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_CONSTANT)) {
                            result_operand.flags |= OPERAND_CONSTANT;
                                 if (is_type_bool(lhs_operand->type) && is_type_bool(rhs_operand->type))  result_operand.bool_value = lhs_operand->bool_value && rhs_operand->bool_value;
                            else {
                                report_error(binary->location, "Operator && is unsupported for types '%s' and '%s'.", type_to_string(lhs_operand->type), type_to_string(rhs_operand->type));
                                return nullptr;
                            }
                        }
                        break;
                    }
                    case TK_BOOLEAN_OR: {
                        result_operand.type = type_bool;
                        if ((lhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_CONSTANT)) {
                            result_operand.flags |= OPERAND_CONSTANT;
                                 if (is_type_bool(lhs_operand->type) && is_type_bool(rhs_operand->type))  result_operand.bool_value = lhs_operand->bool_value || rhs_operand->bool_value;
                            else {
                                report_error(binary->location, "Operator || is unsupported for types '%s' and '%s'.", type_to_string(lhs_operand->type), type_to_string(rhs_operand->type));
                                return nullptr;
                            }
                        }
                        break;
                    }
                    case TK_LEFT_SHIFT: {
                        UNIMPLEMENTED(TK_LEFT_SHIFT);
                    }
                    case TK_RIGHT_SHIFT: {
                        UNIMPLEMENTED(TK_RIGHT_SHIFT);
                    }
                    default: {
                        printf("Unhandled operator: %s\n", token_string(binary->op));
                        assert(false);
                    }
                }
            }
            else {
                // check for overloads!
                // todo(josh): this is pretty crude but maybe fine. means that for operator overloads
                // the struct type has to be the first argument. again, maybe fine
                if (is_type_struct(lhs_operand->type)) {
                    Type_Struct *struct_type = (Type_Struct *)lhs_operand->type;
                    Array<Ast_Expr *> parameters = {};
                    parameters.allocator = default_allocator();
                    parameters.append(binary->lhs);
                    parameters.append(binary->rhs);
                    Ast_Proc *overload_proc = find_operator_overload(struct_type->ast_struct, parameters, binary->op);
                    if (!overload_proc) {
                        report_error(binary->location, "Operator '%s' is unsupported for types '%s' and '%s'.", token_string(binary->op), type_to_string(lhs_operand->type), type_to_string(rhs_operand->type));
                        return nullptr;
                    }
                    else {
                        assert(overload_proc->header->operand.type != nullptr);
                        Array<Ast_Expr *> params_to_emit;
                        if (!typecheck_procedure_call(binary, overload_proc->header->operand, parameters, &result_operand, &params_to_emit)) {
                            return nullptr;
                        }
                        binary->resolved_operator_overload = overload_proc;
                        binary->operator_overload_parameters = params_to_emit;
                    }
                }
                else {
                    report_error(binary->location, "Operator '%s' is unsupported for types '%s' and '%s'.", token_string(binary->op), type_to_string(lhs_operand->type), type_to_string(rhs_operand->type));
                    return nullptr;
                }
            }

            assert(result_operand.type != nullptr);
            break;
        }
        case EXPR_CAST: {
            Expr_Cast *expr_cast = (Expr_Cast *)expr;
            Operand *type_operand = typecheck_expr(expr_cast->type_expr);
            if (!type_operand) {
                return nullptr;
            }
            assert(expr_cast->type_expr->operand.type == type_typeid);
            Operand *rhs_operand = typecheck_expr(expr_cast->rhs);
            if (!rhs_operand) {
                return nullptr;
            }
            assert(can_cast(expr_cast->rhs, expr_cast->type_expr->operand.type_value));
            result_operand.type = type_operand->type_value;
            result_operand.flags = OPERAND_RVALUE;
            // todo(josh): constant propagation
            break;
        }
        case EXPR_ADDRESS_OF: {
            Expr_Address_Of *address_of = (Expr_Address_Of *)expr;
            Operand *rhs_operand = typecheck_expr(address_of->rhs);
            if (!rhs_operand) {
                return nullptr;
            }
            assert(rhs_operand->flags & OPERAND_LVALUE | OPERAND_RVALUE);
            result_operand.type = get_or_create_type_pointer_to(rhs_operand->type);
            result_operand.flags = OPERAND_RVALUE;
            break;
        }
        case EXPR_SUBSCRIPT: {
            Expr_Subscript *subscript = (Expr_Subscript *)expr;
            Operand *lhs_operand = typecheck_expr(subscript->lhs);
            if (!lhs_operand) {
                return nullptr;
            }
            if (!complete_type(lhs_operand->type)) {
                return nullptr;
            }

            Operand *index_operand = typecheck_expr(subscript->index, type_int);
            if (!index_operand) {
                return nullptr;
            }
            assert(is_type_number(index_operand->type));
            assert(is_type_integer(index_operand->type));

            Type *elem_type = nullptr;
            if (is_type_array(lhs_operand->type)) {
                Type_Array *array_type = (Type_Array *)lhs_operand->type;
                assert(array_type->size > 0);
                elem_type = array_type->array_of;
                result_operand.type = elem_type;
                result_operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;
            }
            else if (is_type_slice(lhs_operand->type)) {
                Type_Slice *slice_type = (Type_Slice *)lhs_operand->type;
                elem_type = slice_type->slice_of;
                result_operand.type = elem_type;
                result_operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;
            }
            else if (is_type_struct(lhs_operand->type)) {
                Type_Struct *struct_type = (Type_Struct *)lhs_operand->type;
                Array<Ast_Expr *> parameters = {};
                parameters.allocator = default_allocator();
                parameters.append(subscript->lhs);
                parameters.append(subscript->index);
                Ast_Proc *overload_proc = find_operator_overload(struct_type->ast_struct, parameters, TK_LEFT_SQUARE);
                if (overload_proc == nullptr) {
                    report_error(subscript->location, "Cannot subscript type '%s'.", type_to_string(lhs_operand->type));
                    return nullptr;
                }
                else {
                    assert(overload_proc->header->operand.type != nullptr);
                    Array<Ast_Expr *> params_to_emit;
                    if (!typecheck_procedure_call(subscript, overload_proc->header->operand, parameters, &result_operand, &params_to_emit)) {
                        return nullptr;
                    }
                    subscript->resolved_operator_overload = overload_proc;
                    subscript->operator_overload_parameters = parameters;
                }
            }
            else {
                report_error(subscript->location, "Cannot subscript type '%s'.", type_to_string(lhs_operand->type));
                return nullptr;
            }
            assert(result_operand.flags != 0);
            break;
        }
        case EXPR_SELECTOR: {
            Expr_Selector *selector = (Expr_Selector *)expr;
            Operand *lhs_operand = typecheck_expr(selector->lhs);
            if (!lhs_operand) {
                return nullptr;
            }
            if (!complete_type(lhs_operand->type)) {
                return nullptr;
            }

            bool is_instance_of_type_rather_that_a_type_itself = true;
            assert(lhs_operand->type != nullptr);
            Type *type_with_fields = lhs_operand->type;
            if (is_type_pointer(lhs_operand->type)) {
                Type_Pointer *pointer_type = (Type_Pointer *)lhs_operand->type;
                assert(pointer_type->pointer_to != nullptr);
                type_with_fields = pointer_type->pointer_to;
            }
            else if (is_type_reference(lhs_operand->type)) {
                Type_Reference *reference_type = (Type_Reference *)lhs_operand->type;
                assert(reference_type->reference_to != nullptr);
                type_with_fields = reference_type->reference_to;
            }
            else if (is_type_typeid(lhs_operand->type)) {
                assert(lhs_operand->type_value);
                type_with_fields = lhs_operand->type_value;
                is_instance_of_type_rather_that_a_type_itself = false;
            }
            assert(type_with_fields != nullptr);
            if (!complete_type(type_with_fields)) {
                return nullptr;
            }
            bool found_field = false;
            For (idx, type_with_fields->fields) {
                Struct_Field field = type_with_fields->fields[idx];
                if (strcmp(field.name, selector->field_name) == 0) {
                    if (!is_instance_of_type_rather_that_a_type_itself) {
                        if (!(field.operand.flags & OPERAND_CONSTANT)) {
                            // todo(josh): there's no real reason why we _can't_ support this, it's just weird
                            report_error(selector->location, "Cannot access instance fields from type.");
                            return nullptr;
                        }
                    }
                    found_field = true;
                    result_operand = field.operand;
                    result_operand.location = selector->location;
                    break;
                }
            }
            if (!found_field) {
                report_error(selector->location, "Type '%s' doesn't have field '%s'.", type_to_string(type_with_fields), selector->field_name);
                return nullptr;
            }

            selector->type_with_field = type_with_fields;
            break;
        }
        case EXPR_DEREFERENCE: {
            Expr_Dereference *dereference = (Expr_Dereference *)expr;
            Operand *lhs_operand = typecheck_expr(dereference->lhs);
            if (!lhs_operand) {
                return nullptr;
            }
            assert(is_type_pointer(lhs_operand->type));
            Type_Pointer *pointer_type = (Type_Pointer *)lhs_operand->type;
            result_operand.type = pointer_type->pointer_to;
            result_operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;
            break;
        }
        case EXPR_PROCEDURE_CALL: {
            Expr_Procedure_Call *call = (Expr_Procedure_Call *)expr;
            Operand *procedure_operand = typecheck_expr(call->lhs);
            if (!procedure_operand) {
                return nullptr;
            }
            if (!is_type_polymorphic(procedure_operand->type)) {
                if (!is_type_procedure(procedure_operand->type)) {
                    report_error(call->location, "Attempted to call a non-procedure type.");
                    return nullptr;
                }
            }
            Array<Ast_Expr *> params_to_emit;
            if (!typecheck_procedure_call(call, *procedure_operand, call->parameters, &result_operand, &params_to_emit)) {
                return nullptr;
            }
            call->parameters_to_emit = params_to_emit;
            call->target_procedure_type = (Type_Procedure *)procedure_operand->type;
            if (result_operand.referenced_procedure) {
                // @PolymorphCleanup
                assert(is_type_procedure(result_operand.referenced_procedure->declaration->operand.type));
                call->target_procedure_type = (Type_Procedure *)result_operand.referenced_procedure->declaration->operand.type;
            }
            assert(!is_type_polymorphic(call->target_procedure_type));
            assert(is_type_procedure(call->target_procedure_type));
            // todo(josh): constant stuff? procedures are a bit weird in that way in that the name is constant but the value isn't
            break;
        }
        case EXPR_IDENTIFIER: {
            Expr_Identifier *ident = (Expr_Identifier *)expr;
            Ast_Block *block = ident->parent_block;
            while (block != nullptr) {
                For (decl_idx, block->declarations) {
                    Declaration *decl = block->declarations[decl_idx];
                    if (strcmp(decl->name, ident->name) == 0) {
                        ident->resolved_declaration = decl;
                        break;
                    }
                }
                if (ident->resolved_declaration) {
                    break;
                }
                block = block->parent_block;
            }
            if (!ident->resolved_declaration) {
                report_error(ident->location, "Unresolved identifier '%s'.", ident->name);
                return nullptr;
            }
            assert(ident->resolved_declaration != nullptr);
            if (ident->resolved_declaration->is_polymorphic) {
                result_operand.type = type_polymorphic;
                assert(ident->resolved_declaration->kind == DECL_PROC);
                result_operand.referenced_procedure = ((Proc_Declaration *)ident->resolved_declaration)->procedure;
            }
            else {
                if (!check_declaration(ident->resolved_declaration, ident->location, &result_operand)) {
                    return nullptr;
                }
            }
            result_operand.location = ident->location;
            break;
        }
        case EXPR_COMPOUND_LITERAL: {
            Expr_Compound_Literal *compound_literal = (Expr_Compound_Literal *)expr;
            Operand *type_operand = typecheck_expr(compound_literal->type_expr);
            if (!type_operand) {
                return nullptr;
            }
            if (!is_type_typeid(type_operand->type)) {
                report_error(compound_literal->type_expr->location, "Compound literals require a constant type.");
                return nullptr;
            }
            Type *target_type = type_operand->type_value;
            if (!complete_type(target_type)) {
                return nullptr;
            }

            if (is_type_array(target_type)) {
                Type_Array *array_type = (Type_Array *)target_type;
                assert(array_type->count > 0);
                if (compound_literal->exprs.count != 0) {
                    if (compound_literal->exprs.count != array_type->count) {
                        report_error(compound_literal->location, "Expression count for compound literal doesn't match the type. Expected %d, got %d.", array_type->count, compound_literal->exprs.count);
                        return nullptr;
                    }

                    Type *element_type = array_type->array_of;
                    For (idx, compound_literal->exprs) {
                        Ast_Expr *expr = compound_literal->exprs[idx];
                        Operand *expr_operand = typecheck_expr(expr);
                        if (!expr_operand) {
                            return nullptr;
                        }
                        if (!match_types(expr_operand, element_type, false)) {
                            report_error(expr->location, "Expression within compound literal doesn't match the required type for the compound literal.");
                            report_info(expr->location, "Expected '%s', got '%s'.", type_to_string(element_type), type_to_string(expr_operand->type));
                            return nullptr;
                        }
                    }
                }
            }
            else {
                if (compound_literal->exprs.count != 0) {
                    if (compound_literal->exprs.count != target_type->num_variable_fields) {
                        report_error(compound_literal->location, "Expression count for compound literal doesn't match the type. Expected %d, got %d.", target_type->fields.count, compound_literal->exprs.count);
                        return nullptr;
                    }
                    int variable_field_index = -1;
                    For (idx, compound_literal->exprs) {
                        variable_field_index += 1;
                        while (target_type->fields[variable_field_index].operand.flags & OPERAND_CONSTANT) {
                            variable_field_index += 1;
                        }

                        Type *element_type = target_type->fields[variable_field_index].operand.type;

                        Ast_Expr *expr = compound_literal->exprs[idx];
                        Operand *expr_operand = typecheck_expr(expr);
                        if (!expr_operand) {
                            return nullptr;
                        }
                        if (!match_types(expr_operand, element_type, false)) {
                            report_error(expr->location, "Expression within compound literal doesn't match the required type for the compound literal.");
                            report_info(expr->location, "Expected '%s', got '%s'.", type_to_string(element_type), type_to_string(expr_operand->type));
                            return nullptr;
                        }
                    }
                }
            }

            result_operand.type = type_operand->type_value;
            result_operand.flags = OPERAND_RVALUE;

            break;
        }
        case EXPR_NUMBER_LITERAL: {
            Expr_Number_Literal *number = (Expr_Number_Literal *)expr;
            if (number->has_a_dot) {
                result_operand.type = type_untyped_float;
            }
            else {
                result_operand.type = type_untyped_integer;
            }
            result_operand.flags = OPERAND_CONSTANT | OPERAND_RVALUE;
            result_operand.int_value   = atoi(number->number_string);
            result_operand.float_value = atof(number->number_string);
            break;
        }
        case EXPR_STRING_LITERAL: {
            Expr_String_Literal *string_literal = (Expr_String_Literal *)expr;
            result_operand.flags = OPERAND_CONSTANT | OPERAND_RVALUE;
            result_operand.type = type_string;
            result_operand.scanned_string_value = string_literal->text;
            result_operand.escaped_string_value = string_literal->escaped_text;
            result_operand.scanned_string_length = string_literal->scanner_length;
            result_operand.escaped_string_length = string_literal->escaped_length;
            break;
        }
        case EXPR_NULL: {
            result_operand.flags = OPERAND_RVALUE;
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
            if (!expr_operand) {
                return nullptr;
            }
            assert(expr_operand->type == type_typeid);
            assert(expr_operand->type_value != nullptr);
            if (!complete_type(expr_operand->type_value)) {
                return nullptr;
            }
            assert(!is_type_incomplete(expr_operand->type_value));
            assert(expr_operand->type_value->size > 0);
            result_operand.type = type_untyped_integer;
            result_operand.int_value = expr_operand->type_value->size;
            result_operand.flags = OPERAND_CONSTANT | OPERAND_RVALUE;
            break;
        }
        case EXPR_TYPEOF: {
            Expr_Typeof *expr_typeof = (Expr_Typeof *)expr;
            Operand *expr_operand = typecheck_expr(expr_typeof->expr);
            if (!expr_operand) {
                return nullptr;
            }
            assert(expr_operand->type != nullptr);
            result_operand.type = type_typeid;
            result_operand.type_value = expr_operand->type;
            result_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
            break;
        }
        case EXPR_POINTER_TYPE: {
            Expr_Pointer_Type *expr_pointer = (Expr_Pointer_Type *)expr;
            assert(expr_pointer->pointer_to != nullptr);
            Operand *pointer_to_operand = typecheck_expr(expr_pointer->pointer_to, type_typeid);
            if (!pointer_to_operand) {
                return nullptr;
            }
            assert((pointer_to_operand->flags & OPERAND_CONSTANT) && (pointer_to_operand->flags & OPERAND_TYPE));
            result_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
            result_operand.type = type_typeid;
            result_operand.type_value = get_or_create_type_pointer_to(pointer_to_operand->type_value);
            break;
        }
        case EXPR_REFERENCE_TYPE: {
            Expr_Reference_Type *expr_reference = (Expr_Reference_Type *)expr;
            assert(expr_reference->reference_to != nullptr);
            Operand *reference_to_operand = typecheck_expr(expr_reference->reference_to, type_typeid);
            if (!reference_to_operand) {
                return nullptr;
            }
            assert((reference_to_operand->flags & OPERAND_CONSTANT) && (reference_to_operand->flags & OPERAND_TYPE));
            result_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
            result_operand.type = type_typeid;
            result_operand.type_value = get_or_create_type_reference_to(reference_to_operand->type_value);
            break;
        }
        case EXPR_ARRAY_TYPE: {
            Expr_Array_Type *expr_array = (Expr_Array_Type *)expr;
            assert(expr_array->array_of != nullptr);
            Operand *array_of_operand = typecheck_expr(expr_array->array_of, type_typeid);
            if (!array_of_operand) {
                return nullptr;
            }
            assert((array_of_operand->flags & OPERAND_CONSTANT) && (array_of_operand->flags & OPERAND_TYPE));
            Operand *count_operand = typecheck_expr(expr_array->count_expr, type_int);
            if (!count_operand) {
                return nullptr;
            }
            assert((count_operand->flags & OPERAND_CONSTANT) && is_type_integer(count_operand->type));
            if (count_operand->int_value <= 0) {
                report_error(expr_array->count_expr->location, "Array size must be greater than 0.");
                return nullptr;
            }
            result_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
            result_operand.type = type_typeid;
            result_operand.type_value = get_or_create_type_array_of(array_of_operand->type_value, count_operand);
            break;
        }
        case EXPR_SLICE_TYPE: {
            Expr_Slice_Type *expr_slice = (Expr_Slice_Type *)expr;
            assert(expr_slice->slice_of != nullptr);
            Operand *slice_of_operand = typecheck_expr(expr_slice->slice_of, type_typeid);
            if (!slice_of_operand) {
                return nullptr;
            }
            assert((slice_of_operand->flags & OPERAND_CONSTANT) && (slice_of_operand->flags & OPERAND_TYPE));
            result_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE;
            result_operand.type = type_typeid;
            result_operand.type_value = get_or_create_type_slice_of(slice_of_operand->type_value);
            break;
        }
        case EXPR_POLYMORPHIC_VARIABLE: {
            Expr_Polymorphic_Variable *poly = (Expr_Polymorphic_Variable *)expr;
            assert(poly->poly_decl != nullptr);
            if (poly->poly_decl->declaration) {
                if (!check_declaration(poly->poly_decl->declaration, poly->location, &result_operand)) {
                    return nullptr;
                }
            }
            else {
                result_operand.type = type_polymorphic;
            }
            break;
        }
        case EXPR_PAREN: {
            Expr_Paren *paren = (Expr_Paren *)expr;
            Operand *expr_operand = typecheck_expr(paren->nested, expected_type);
            if (!expr_operand) {
                return nullptr;
            }
            result_operand = *expr_operand;
            result_operand.location = paren->location;
            break;
        }
        default: {
            assert(false);
        }
    }

    if ((!(result_operand.flags & OPERAND_NO_VALUE))) {
        if (is_type_reference(result_operand.type)) {
            result_operand.flags |= OPERAND_LVALUE;
            result_operand.reference_type = result_operand.type;
        }
        result_operand.type = unwrap_type(result_operand.type);
    }

    if (expected_type) {
        if (!match_types(&result_operand, expected_type)) {
            return nullptr;
        }
        assert(!(result_operand.type->flags & TF_UNTYPED));
    }

    expr->operand = result_operand;
    return &expr->operand;
}

bool typecheck_procedure_header(Ast_Proc_Header *header) {
    assert(header->type == nullptr);
    Array<Type *> parameter_types = {};
    parameter_types.allocator = default_allocator();
    bool is_polymorphic = false;
    For (idx, header->parameters) {
        Ast_Var *parameter = header->parameters[idx];
        if (parameter->is_constant) {
            report_error(parameter->location, "Constant parameters are not allowed.");
            return false;
        }
        if (parameter->expr != nullptr) {
            report_error(parameter->expr->location, "Default values for procedure parameters are not yet supported.");
            return false;
        }
        if (!check_declaration(parameter->declaration, parameter->location)) {
            return false;
        }
        assert(parameter->type != nullptr);
        parameter_types.append(parameter->type);
    }
    Type *return_type = {};
    if (header->return_type_expr) {
        Operand *return_type_operand = typecheck_expr(header->return_type_expr);
        if (!return_type_operand) {
            return false;
        }
        assert(return_type_operand->flags & OPERAND_CONSTANT);
        assert(return_type_operand->type == type_typeid);
        return_type = return_type_operand->type_value;
    }
    header->type = get_or_create_type_procedure(parameter_types, return_type);
    if (header->name == nullptr) {
        assert(header->operator_to_overload != TK_INVALID);
        char *name = (char *)alloc(default_allocator(), 64);
        // todo(josh): put the parameter types in the name too probably?
        assert(header->struct_to_operator_overload != nullptr);
        sprintf(name, "__operator_overload_%s_%s", header->struct_to_operator_overload->name, token_name(header->operator_to_overload));
        header->name = name;
    }
    else {
        assert(header->operator_to_overload == TK_INVALID);
    }
    Operand operand = {};
    operand.type = header->type;
    operand.flags = OPERAND_RVALUE;
    header->operand = operand;
    return true;
}

bool do_assert_directives() {
    For (idx, g_all_assert_directives) {
        Ast_Directive_Assert *assert_directive = g_all_assert_directives[idx];
        Operand *expr_operand = typecheck_expr(assert_directive->expr, type_bool);
        if (!expr_operand) {
            return nullptr;
        }
        assert(expr_operand->type == type_bool);
        if (!(expr_operand->flags & OPERAND_CONSTANT)) {
            report_error(assert_directive->expr->location, "#assert directive parameter must be constant.");
            return false;
        }
        if (!expr_operand->bool_value) {
            report_error(assert_directive->location, "#assert directive failed.");
            return false;
        }
    }
    return true;
}

bool do_print_directives() {
    For (idx, g_all_print_directives) {
        Ast_Directive_Print *print_directive = g_all_print_directives[idx];
        Operand *expr_operand = typecheck_expr(print_directive->expr);
        if (!expr_operand) {
            return false;
        }
        assert(expr_operand->flags & OPERAND_CONSTANT);
        assert(expr_operand->type->flags & TF_INTEGER);
        printf("%s(%d:%d) #print directive: %lld\n", print_directive->location.filepath, print_directive->location.line, print_directive->location.character, expr_operand->int_value);
    }
    return true;
}

bool typecheck_node(Ast_Node *node) {
    switch (node->ast_kind) {
        case AST_VAR: {
            Ast_Var *var = (Ast_Var *)node;
            if (!check_declaration(var->declaration, var->location)) {
                return false;
            }
            break;
        }

        case AST_ASSIGN: {
            Ast_Assign *assign = (Ast_Assign *)node;
            Operand *lhs_operand = typecheck_expr(assign->lhs);
            if (!lhs_operand) {
                return false;
            }
            if (!(lhs_operand->flags & OPERAND_LVALUE)) {
                report_error(assign->location, "Cannot assign to non-lvalue.");
                return false;
            }
            Type *expected_lhs_type = lhs_operand->type;
            if (is_type_reference(expected_lhs_type)) {
                Type_Reference *type_reference = (Type_Reference *)expected_lhs_type;
                expected_lhs_type = type_reference->reference_to;
            }
            assert(expected_lhs_type != nullptr);
            Operand *rhs_operand = typecheck_expr(assign->rhs, expected_lhs_type);
            if (!rhs_operand) {
                return false;
            }

            Token_Kind binary_operation = TK_INVALID;
            switch (assign->op) {
                case TK_ASSIGN:             binary_operation = TK_INVALID;     break;
                case TK_PLUS_ASSIGN:        binary_operation = TK_PLUS;        break;
                case TK_MINUS_ASSIGN:       binary_operation = TK_MINUS;       break;
                case TK_MULTIPLY_ASSIGN:    binary_operation = TK_MULTIPLY;    break;
                case TK_DIVIDE_ASSIGN:      binary_operation = TK_DIVIDE;      break;
                case TK_LEFT_SHIFT_ASSIGN:  binary_operation = TK_LEFT_SHIFT;  break;
                case TK_RIGHT_SHIFT_ASSIGN: binary_operation = TK_RIGHT_SHIFT; break;
                case TK_BIT_AND_ASSIGN:     binary_operation = TK_AMPERSAND;   break;
                case TK_BIT_OR_ASSIGN:      binary_operation = TK_BIT_OR;      break;
                case TK_BOOLEAN_AND_ASSIGN: binary_operation = TK_BOOLEAN_AND; break;
                case TK_BOOLEAN_OR_ASSIGN:  binary_operation = TK_BOOLEAN_OR;  break;
                default: {
                    assert(false);
                }
            }

            if (binary_operation == TK_INVALID) {
                assert(assign->op = TK_ASSIGN);
            }
            else {
                if (!operator_is_defined(expected_lhs_type, rhs_operand->type, binary_operation)) {
                    report_error(assign->location, "Operator '%s' is not defined for types '%s and '%s'.", token_string(assign->op), type_to_string(expected_lhs_type), type_to_string(rhs_operand->type));
                    return false;
                }
            }

            assert(rhs_operand->flags & OPERAND_RVALUE);
            if (!match_types(rhs_operand, expected_lhs_type)) {
                return false;
            }
            break;
        }

        case AST_STATEMENT_EXPR: {
            Ast_Statement_Expr *stmt = (Ast_Statement_Expr *)node;
            Operand *statement_operand = typecheck_expr(stmt->expr);
            if (!statement_operand) {
                return false;
            }
            if (unparen_expr(stmt->expr)->expr_kind != EXPR_PROCEDURE_CALL) { // todo(josh): this check won't work if the procedure call is nested like `(foo());`
                report_error(stmt->expr->location, "Statement doesn't have side-effects.");
                return false;
            }
            break;
        }

        case AST_BLOCK_STATEMENT: {
            Ast_Block_Statement *block_statement = (Ast_Block_Statement *)node;
            if (!typecheck_block(block_statement->block)) {
                return false;
            }
            break;
        }

        case AST_IF: {
            Ast_If *ast_if = (Ast_If *)node;
            Operand *condition_operand = typecheck_expr(ast_if->condition);
            if (!condition_operand) {
                return false;
            }
            if (condition_operand->type != type_bool) {
                report_error(ast_if->condition->location, "Expression in if-statement must have type bool.");
                return false;
            }
            if (!typecheck_block(ast_if->body)) {
                return false;
            }
            if (ast_if->else_body) {
                if (!typecheck_block(ast_if->else_body)) {
                    return false;
                }
            }
            break;
        }

        case AST_FOR_LOOP: {
            Ast_For_Loop *for_loop = (Ast_For_Loop *)node;
            assert(for_loop->pre);
            assert(for_loop->condition);
            assert(for_loop->post);
            if (!typecheck_node(for_loop->pre)) {
                return false;
            }
            Operand *condition_operand = typecheck_expr(for_loop->condition);
            if (!condition_operand) {
                return false;
            }
            if (condition_operand->type != type_bool) {
                report_error(for_loop->condition->location, "For loop condition must be of type bool.");
                return false;
            }
            if (!typecheck_node(for_loop->post)) {
                return false;
            }
            if (!typecheck_block(for_loop->body)) {
                return false;
            }
            break;
        }

        case AST_WHILE_LOOP: {
            Ast_While_Loop *while_loop = (Ast_While_Loop *)node;
            Operand *condition_operand = typecheck_expr(while_loop->condition);
            if (!condition_operand) {
                return false;
            }
            if (condition_operand->type != type_bool) {
                report_error(while_loop->condition->location, "While loop condition must be of type bool.");
                return false;
            }
            if (!typecheck_block(while_loop->body)) {
                return false;
            }
            break;
        }

        case AST_RETURN: {
            Ast_Return *ast_return = (Ast_Return *)node;
            assert(ast_return->matching_procedure != nullptr);
            assert(ast_return->matching_procedure->type != nullptr);
            if (ast_return->matching_procedure->type->return_type != nullptr) {
                if (ast_return->expr == nullptr) {
                    report_error(ast_return->location, "Return statement missing expression. Expected expression with type '%s'.", type_to_string(ast_return->matching_procedure->type->return_type));
                    return nullptr;
                }
                Operand *return_operand = typecheck_expr(ast_return->expr);
                if (!return_operand) {
                    return nullptr;
                }
                if (!match_types(return_operand, ast_return->matching_procedure->type->return_type, false)) {
                    report_error(ast_return->expr->location, "Return expression didn't match procedure return type. Expected '%s', got '%s'.", type_to_string(ast_return->matching_procedure->type->return_type), type_to_string(return_operand->type));
                    return nullptr;
                }
            }
            else {
                if (ast_return->expr) {
                    assert(ast_return->matching_procedure->name != nullptr); // todo(josh): handle operator overloads since they don't have names
                    report_error(ast_return->expr->location, "Procedure '%s' doesn't have a return value.", ast_return->matching_procedure->name);
                    return nullptr;
                }
            }
            break;
        }

        case AST_EMPTY_STATEMENT: {
            break;
        }

        default: {
            UNIMPLEMENTED(node->ast_kind);
            break;
        }
    }
    return true;
}

bool typecheck_block(Ast_Block *block) {
    For (idx, block->nodes) {
        Ast_Node *node = block->nodes[idx];
        if (!typecheck_node(node)) {
            return false;
        }
    }
    return true;
}