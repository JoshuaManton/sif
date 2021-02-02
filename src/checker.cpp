#include "checker.h"

#define POINTER_SIZE 8

Array<Type *> all_types;

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
Type *type_untyped_string;

Type *type_typeid;

Type *type_string;
Type *type_cstring;
Type *type_rawptr;

Type *type_byte;
Type *type_int;
Type *type_uint;
Type *type_float;

Type *type_polymorphic;

Type *type_any;

Array<Declaration *> ordered_declarations;

Ast_Proc *g_main_proc;

Array<Ast_Directive_Assert *> g_all_assert_directives;
Array<Ast_Directive_Print *>  g_all_print_directives;

Declaration *sif_runtime_bounds_check_proc;
Declaration *sif_runtime_null_check_proc;
Declaration *sif_runtime_string_eq_proc;
Declaration *sif_runtime_zero_pointer_proc;
Declaration *sif_runtime_source_code_location;
Declaration *sif_runtime_make_source_code_location;
Struct_Declaration *sif_runtime_union_all_type_infos;
Struct_Declaration *sif_runtime_type_info;
Struct_Declaration *sif_runtime_type_info_integer;
Struct_Declaration *sif_runtime_type_info_float;
Struct_Declaration *sif_runtime_type_info_bool;
Struct_Declaration *sif_runtime_type_info_string;
Struct_Declaration *sif_runtime_type_info_struct_field;
Struct_Declaration *sif_runtime_type_info_struct;
Struct_Declaration *sif_runtime_type_info_union;
Struct_Declaration *sif_runtime_type_info_enum_field;
Struct_Declaration *sif_runtime_type_info_enum;
Struct_Declaration *sif_runtime_type_info_pointer;
Struct_Declaration *sif_runtime_type_info_slice;
Struct_Declaration *sif_runtime_type_info_array;
Struct_Declaration *sif_runtime_type_info_reference;
Struct_Declaration *sif_runtime_type_info_procedure;
Struct_Declaration *sif_runtime_type_info_typeid;

bool g_silence_errors; // todo(josh): this is pretty janky. would be better to pass some kind of Context struct around

bool complete_type(Type *type, Location usage_location, u64 completion_flags = 0);
bool check_declaration(Declaration *decl, Location usage_location, Operand *out_operand = nullptr);
void type_mismatch(Location location, Type *got, Type *expected);
bool match_types(Operand operand, Type *expected_type, Operand *out_operand, bool do_report_error = true);
Operand *typecheck_expr(Ast_Expr *expr, Type *expected_type = nullptr, bool require_value = true);
bool typecheck_block(Ast_Block *block);
Operand *typecheck_procedure_header(Ast_Proc_Header *header);
char *type_to_string_plain(Type *type);

template<typename T>
T *NEW_TYPE(T init, bool add_to_all_types_list = true, Ast_Block *declarations_block_override = nullptr) {
    T *t_ptr = SIF_NEW_CLONE(init, g_global_linear_allocator);
    if (add_to_all_types_list) {
        all_types.append(t_ptr);
        t_ptr->id = all_types.count;
    }

    if (declarations_block_override) {
        t_ptr->declarations_block = declarations_block_override;
    }
    else {
        t_ptr->declarations_block = SIF_NEW_CLONE(Ast_Block(g_global_linear_allocator, {}, {}), g_global_linear_allocator);
    }
    return t_ptr;
}

Struct_Member_Declaration *try_add_variable_type_field(Type *type, const char *name, Type *variable_type, int offset, Location location, Array<char *> notes = {}) {
    assert(variable_type != nullptr);
    assert(!is_type_untyped(variable_type));
    Struct_Field field = {};
    field.name = name;
    field.offset = offset;
    field.operand.type = variable_type;
    field.operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;
    field.notes = notes;
    type->all_fields.append(field);
    type->variable_fields.append(field);
    Struct_Member_Declaration *decl = SIF_NEW_CLONE(Struct_Member_Declaration(name, field.operand, offset, type->declarations_block, location), g_global_linear_allocator);
    if (!register_declaration(type->declarations_block, decl)) {
        return nullptr;
    }
    return decl;
}

Struct_Member_Declaration *try_add_constant_type_field(Type *type, const char *name, Operand operand, Location location, Array<char *> notes = {}) {
    assert(operand.type != nullptr);
    Struct_Field field = {};
    field.name = name;
    field.offset = -1;
    field.operand = operand;
    field.notes = notes;
    type->all_fields.append(field);
    type->constant_fields.append(field);
    Struct_Member_Declaration *decl = SIF_NEW_CLONE(Struct_Member_Declaration(name, operand, -1, type->declarations_block, location), g_global_linear_allocator);
    if (!register_declaration(type->declarations_block, decl)) {
        return nullptr;
    }
    return decl;
}

void init_checker() {
    g_all_assert_directives.allocator = g_global_linear_allocator;
    g_all_print_directives.allocator = g_global_linear_allocator;
    all_types.allocator = g_global_linear_allocator;
    ordered_declarations.allocator = g_global_linear_allocator;

    type_i8  = NEW_TYPE(Type_Primitive(intern_string("i8"),  1, 1)); type_i8->flags  = TF_NUMBER | TF_INTEGER | TF_SIGNED;
    type_i16 = NEW_TYPE(Type_Primitive(intern_string("i16"), 2, 2)); type_i16->flags = TF_NUMBER | TF_INTEGER | TF_SIGNED;
    type_i32 = NEW_TYPE(Type_Primitive(intern_string("i32"), 4, 4)); type_i32->flags = TF_NUMBER | TF_INTEGER | TF_SIGNED;
    type_i64 = NEW_TYPE(Type_Primitive(intern_string("i64"), 8, 8)); type_i64->flags = TF_NUMBER | TF_INTEGER | TF_SIGNED;

    type_u8  = NEW_TYPE(Type_Primitive(intern_string("u8"),  1, 1)); type_u8->flags  = TF_NUMBER | TF_INTEGER | TF_UNSIGNED;
    type_u16 = NEW_TYPE(Type_Primitive(intern_string("u16"), 2, 2)); type_u16->flags = TF_NUMBER | TF_INTEGER | TF_UNSIGNED;
    type_u32 = NEW_TYPE(Type_Primitive(intern_string("u32"), 4, 4)); type_u32->flags = TF_NUMBER | TF_INTEGER | TF_UNSIGNED;
    type_u64 = NEW_TYPE(Type_Primitive(intern_string("u64"), 8, 8)); type_u64->flags = TF_NUMBER | TF_INTEGER | TF_UNSIGNED;

    type_f32 = NEW_TYPE(Type_Primitive(intern_string("f32"), 4, 4)); type_f32->flags = TF_NUMBER | TF_FLOAT | TF_SIGNED;
    type_f64 = NEW_TYPE(Type_Primitive(intern_string("f64"), 8, 8)); type_f64->flags = TF_NUMBER | TF_FLOAT | TF_SIGNED;

    type_bool = NEW_TYPE(Type_Primitive(intern_string("bool"), 1, 1));

    type_typeid  = NEW_TYPE(Type_Primitive(intern_string("typeid"),  8, 8));
    type_cstring = NEW_TYPE(Type_Primitive(intern_string("cstring"), 8, 8)); type_cstring->flags = TF_POINTER;
    type_string  = NEW_TYPE(Type_Primitive(intern_string("string"), 16, 8)); type_string->flags  = TF_STRING;
    type_rawptr  = NEW_TYPE(Type_Primitive(intern_string("rawptr"),  8, 8)); type_rawptr->flags  = TF_POINTER;

    type_any = NEW_TYPE(Type_Primitive(intern_string("any"), 16, 8));

    type_untyped_integer = NEW_TYPE(Type_Primitive("untyped integer", -1, -1), false); type_untyped_integer->flags = TF_NUMBER  | TF_UNTYPED | TF_INTEGER;
    type_untyped_float   = NEW_TYPE(Type_Primitive("untyped float",   -1, -1), false); type_untyped_float->flags   = TF_NUMBER  | TF_UNTYPED | TF_FLOAT;
    type_untyped_null    = NEW_TYPE(Type_Primitive("untyped null",    -1, -1), false); type_untyped_null->flags    = TF_POINTER | TF_UNTYPED;
    type_untyped_string  = NEW_TYPE(Type_Primitive("untyped string",  -1, -1), false); type_untyped_string->flags  = TF_STRING  | TF_UNTYPED;

    type_byte  = type_u8;
    type_int   = type_i64;
    type_uint  = type_u64;
    type_float = type_f32;

    type_polymorphic = NEW_TYPE(Type_Primitive("type polymorphic", -1, -1), false); type_polymorphic->flags = TF_POLYMORPHIC;

    bool ok = false;
    ok = try_add_variable_type_field(type_string, intern_string(g_interned_data_string), get_or_create_type_pointer_to(type_u8), 0, {}); assert(ok);
    ok = try_add_variable_type_field(type_string, intern_string(g_interned_count_string), type_int, 8, {});                              assert(ok);

    ok = try_add_variable_type_field(type_any, intern_string(g_interned_data_string), type_rawptr, 0, {});                               assert(ok);
    ok = try_add_variable_type_field(type_any, intern_string(g_interned_type_string), type_typeid, 8, {});                               assert(ok);
}

void add_ordered_declaration(Declaration *declaration) {
    ordered_declarations.append(declaration);
}

void add_global_declarations(Ast_Block *block) {
    assert(type_i8 != nullptr);

    bool ok = false;
    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("i8"),  type_i8, block),  g_global_linear_allocator));         assert(ok);
    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("i16"), type_i16, block), g_global_linear_allocator));         assert(ok);
    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("i32"), type_i32, block), g_global_linear_allocator));         assert(ok);
    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("i64"), type_i64, block), g_global_linear_allocator));         assert(ok);

    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("u8"),  type_u8, block),  g_global_linear_allocator));         assert(ok);
    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("u16"), type_u16, block), g_global_linear_allocator));         assert(ok);
    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("u32"), type_u32, block), g_global_linear_allocator));         assert(ok);
    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("u64"), type_u64, block), g_global_linear_allocator));         assert(ok);

    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("f32"), type_f32, block), g_global_linear_allocator));         assert(ok);
    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("f64"), type_f64, block), g_global_linear_allocator));         assert(ok);

    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("byte"),  type_u8, block),  g_global_linear_allocator));       assert(ok);
    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("int"),   type_i64, block), g_global_linear_allocator));       assert(ok);
    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("uint"),  type_u64, block), g_global_linear_allocator));       assert(ok);
    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("float"), type_f32, block), g_global_linear_allocator));       assert(ok);

    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("bool"), type_bool, block), g_global_linear_allocator));       assert(ok);

    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("typeid"),  type_typeid,  block), g_global_linear_allocator)); assert(ok);
    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("string"),  type_string,  block), g_global_linear_allocator)); assert(ok);
    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("cstring"), type_cstring, block), g_global_linear_allocator)); assert(ok);
    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("rawptr"),  type_rawptr,  block), g_global_linear_allocator)); assert(ok);

    ok = register_declaration(block, SIF_NEW_CLONE(Type_Declaration(intern_string("any"), type_any, block), g_global_linear_allocator));         assert(ok);
}

Type_Struct *make_incomplete_type_for_struct(Ast_Struct *structure) {
    Type_Struct *incomplete_type = NEW_TYPE(Type_Struct(structure), true, structure->struct_block);
    incomplete_type->is_union = structure->is_union;
    structure->type = incomplete_type;
    return incomplete_type;
}

bool is_type_pointer    (Type *type) { return type->kind == TYPE_POINTER;       }
bool is_type_polymorphic(Type *type) { return type->flags & TF_POLYMORPHIC;     }
bool is_type_reference  (Type *type) { return type->kind == TYPE_REFERENCE;     }
bool is_type_procedure  (Type *type) { return type->kind == TYPE_PROCEDURE;     }
bool is_type_array      (Type *type) { return type->kind == TYPE_ARRAY;         }
bool is_type_slice      (Type *type) { return type->kind == TYPE_SLICE;         }
bool is_type_number     (Type *type) { return type->flags & TF_NUMBER;          }
bool is_type_integer    (Type *type) { return type->flags & TF_INTEGER;         }
bool is_type_float      (Type *type) { return type->flags & TF_FLOAT;           }
bool is_type_bool       (Type *type) { return type == type_bool;                }
bool is_type_untyped    (Type *type) { return type->flags & TF_UNTYPED;         }
bool is_type_unsigned   (Type *type) { return type->flags & TF_UNSIGNED;        }
bool is_type_signed     (Type *type) { return type->flags & TF_SIGNED;          }
bool is_type_struct     (Type *type) { return type->kind == TYPE_STRUCT;        }
bool is_type_incomplete (Type *type) { return type->completed_flags != TCF_ALL; }
bool is_type_typeid     (Type *type) { return type == type_typeid;              }
bool is_type_string     (Type *type) { return type->flags & TF_STRING;          }
bool is_type_varargs    (Type *type) { return type->kind == TYPE_VARARGS;       }
bool is_type_enum       (Type *type) { return type->kind == TYPE_ENUM;          }

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
    if (type == type_untyped_string) {
        return type_string;
    }
    if (type == type_untyped_null) {
        return nullptr; // note(josh): nothing we can really do here, up to the caller to handle it
    }
    assert(false);
    return nullptr;
}

bool broadcast_declarations_for_using(Ast_Block *block_to_broadcast_into, Ast_Block *block_with_declarations, Declaration *from_using, Ast_Node *node_to_link_back_to) {
    For (idx, block_with_declarations->declarations) {
        Declaration *decl_to_broadcast = block_with_declarations->declarations[idx];
        Using_Declaration *new_declaration = SIF_NEW_CLONE(Using_Declaration(node_to_link_back_to, decl_to_broadcast, block_to_broadcast_into, node_to_link_back_to->location), g_global_linear_allocator);
        new_declaration->from_using = from_using;
        if (!register_declaration(block_to_broadcast_into, new_declaration)) {
            return false;
        }
    }
    return true;
}

bool check_basic_block_for_return(Ast_Basic_Block *block) {
    if (block->to.count == 0) {
        if (!block->has_return) {
            bool all_froms_have_return = block->from.count > 0; // if we don't have any froms, default to false
            For (idx, block->from) {
                if (block->from[idx]->has_return == false) {
                    all_froms_have_return = false;
                    break;
                }
            }
            if (!all_froms_have_return) {
                report_error(block->location, "Basic block starting here doesn't have a return statement.");
                return false;
            }
        }
        return true;
    }
    For (idx, block->to) {
        if (!check_basic_block_for_return(block->to[idx])) {
            return false;
        }
    }
    return true;
}

bool broadcast_var_using(Ast_Var *var, Ast_Block *broadcast_into) {
    if (var->is_using) {
        if (var->is_constant) {
            report_error(var->location, "Cannot apply 'using' to a constant.");
            return false;
        }
        // todo(josh): what about references here?
        //             should calling complete_type() with a reference complete the thing
        //             it's a reference to? probably. hmmmm
        if (is_type_pointer(var->type)) {
            Type_Pointer *pointer_type = (Type_Pointer *)var->type;
            assert(pointer_type->pointer_to);
            if (!complete_type(pointer_type->pointer_to, var->location)) {
                return false;
            }
        }
        return broadcast_declarations_for_using(broadcast_into, var->type->declarations_block, var->declaration, var);
    }
    return true;
}

bool complete_type(Type *type, Location usage_location, u64 completion_flags) {
    completion_flags |= TCF_GENERATE_FIELDS;
    completion_flags |= TCF_ADD_ORDERED_DECLARATION;
    completion_flags &= ~type->completed_flags; // unset the ones we've completed already
    if (completion_flags == 0) {
        return true;
    }
    if ((type->currently_completing_flags & completion_flags) > 0) {
        report_error(usage_location, "Circular dependency."); // todo(josh): better @ErrorMessage
        return false;
    }
    assert(completion_flags > 0);
    type->currently_completing_flags |= completion_flags;
    assert((type->completed_flags & completion_flags) == 0);
    defer(type->currently_completing_flags = 0);

    if (completion_flags & TCF_GENERATE_FIELDS) {
        defer(type->completed_flags |= TCF_GENERATE_FIELDS);
        defer(type->currently_completing_flags &= ~TCF_GENERATE_FIELDS);

        switch (type->kind) {
            case TYPE_STRUCT: {
                Type_Struct *struct_type = (Type_Struct *)type;
                assert(struct_type->ast_struct != nullptr);
                Ast_Struct *structure = struct_type->ast_struct;
                For (idx, structure->fields) {
                    Ast_Var *var = structure->fields[idx];
                    if (!check_declaration(var->declaration, var->location)) {
                        return false;
                    }
                    if (!complete_type(var->type, var->location)) {
                        return false;
                    }
                    if (var->is_constant) {
                        assert(var->expr != nullptr);
                        assert(var->constant_operand.type != nullptr);
                        assert(var->constant_operand.flags & OPERAND_CONSTANT);
                        var->struct_member = try_add_constant_type_field(struct_type, var->name, var->constant_operand, var->location, var->declaration->notes);
                        if (!var->struct_member) {
                            return false;
                        }
                    }
                    else {
                        var->struct_member = try_add_variable_type_field(struct_type, var->name, var->type, -1, var->location, var->declaration->notes);
                        assert(var->struct_member);
                    }
                }
                break;
            }
        }
    }

    if (completion_flags & TCF_USINGS) {
        defer(type->completed_flags |= TCF_USINGS);
        defer(type->currently_completing_flags &= ~TCF_USINGS);

        switch (type->kind) {
            case TYPE_STRUCT: {
                Type_Struct *struct_type = (Type_Struct *)type;
                Ast_Struct *structure = struct_type->ast_struct;
                For (idx, structure->fields) {
                    Ast_Var *var = structure->fields[idx];
                    if (!broadcast_var_using(var, struct_type->declarations_block)) {
                        return false;
                    }
                }
                break;
            }
            case TYPE_POINTER: {
                Type_Pointer *pointer_type = (Type_Pointer *)type;
                assert(pointer_type->pointer_to);
                if (!complete_type(pointer_type->pointer_to, usage_location, TCF_USINGS)) {
                    return false;
                }
                break;
            }
        }
    }

    if (completion_flags & TCF_ADD_ORDERED_DECLARATION) {
        defer(type->completed_flags |= TCF_ADD_ORDERED_DECLARATION);
        defer(type->currently_completing_flags &= ~TCF_ADD_ORDERED_DECLARATION);
        switch (type->kind) {
            case TYPE_STRUCT: {
                Type_Struct *struct_type = (Type_Struct *)type;
                Ast_Struct *structure = struct_type->ast_struct;
                if (!check_declaration(structure->declaration, {})) { // note(josh): just for making sure the link_name is set
                    assert(false);
                }
                assert(structure->declaration->link_name);
                add_ordered_declaration(structure->declaration);
                break;
            }
            case TYPE_ARRAY: {
                Type_Array *array_type = (Type_Array *)type;
                if (!complete_type(array_type->array_of, usage_location)) {
                    return false;
                }
                add_ordered_declaration(SIF_NEW_CLONE(Type_Declaration("", array_type, nullptr), g_global_linear_allocator));
                break;
            }
        }
    }

    if (completion_flags & TCF_SIZE) {
        defer(type->completed_flags |= TCF_SIZE);
        defer(type->currently_completing_flags &= ~TCF_SIZE);
        switch (type->kind) {
            case TYPE_STRUCT: {
                Type_Struct *struct_type = (Type_Struct *)type;
                assert(struct_type->ast_struct != nullptr);
                Ast_Struct *structure = struct_type->ast_struct;

                For (idx, structure->fields) {
                    Ast_Var *var = structure->fields[idx];
                    if (!check_declaration(var->declaration, var->location)) {
                        return false;
                    }
                    if (!complete_type(var->type, var->location, TCF_SIZE)) {
                        return false;
                    }
                }

                int size = 0;
                int largest_alignment = 1;
                For (idx, struct_type->variable_fields) {
                    Struct_Field *field = &struct_type->variable_fields[idx];
                    if (structure->is_union) {
                        field->offset = 0;
                        size = max(size, field->operand.type->size);
                    }
                    else {
                        size = align_forward(size, field->operand.type->align);
                        field->offset = size;
                        size += field->operand.type->size;
                    }
                    largest_alignment = max(largest_alignment, field->operand.type->align);
                }

                if (size == 0) {
                    size = 1;
                }

                assert(is_power_of_two(largest_alignment));
                assert(size > 0);
                struct_type->size = (int)align_forward((uintptr_t)size, (uintptr_t)largest_alignment);
                struct_type->align = largest_alignment;
                break;
            }
            case TYPE_ARRAY: {
                Type_Array *array_type = (Type_Array *)type;
                if (!complete_type(array_type->array_of, usage_location, TCF_SIZE)) {
                    return false;
                }
                assert(array_type->count > 0);
                assert(array_type->array_of->size > 0);
                assert(array_type->array_of->align > 0);
                array_type->size = array_type->array_of->size * array_type->count;
                array_type->align = array_type->array_of->align;
                assert(array_type->size > 0);
                break;
            }
            case TYPE_REFERENCE: {
                Type_Reference *reference_type = (Type_Reference *)type;
                assert(reference_type->reference_to);
                if (!complete_type(reference_type->reference_to, usage_location, TCF_SIZE)) {
                    return false;
                }
                break;
            }
        }
    }
    return true;
}

bool check_declaration(Declaration *decl, Location usage_location, Operand *out_operand) {
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
    decl->link_name = decl->name;
    assert(decl->link_name);

    Operand decl_operand;
    switch (decl->kind) {
        case DECL_TYPE: {
            Type_Declaration *type_decl = (Type_Declaration *)decl;
            decl_operand.type = type_typeid;
            decl_operand.type_value = type_decl->type;
            decl_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE | OPERAND_RVALUE;
            break;
        }
        case DECL_STRUCT: {
            Struct_Declaration *struct_decl = (Struct_Declaration *)decl;
            decl_operand.type = type_typeid;
            if (!struct_decl->structure->type) {
                struct_decl->structure->type = make_incomplete_type_for_struct(struct_decl->structure);
            }
            decl_operand.type_value = struct_decl->structure->type;
            decl_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE | OPERAND_RVALUE;

            String_Builder link_name_sb = make_string_builder(g_global_linear_allocator, 128);
            if (struct_decl->structure->parent_declaration != nullptr) {
                // todo(josh): this should be the link name
                link_name_sb.print(struct_decl->structure->parent_declaration->name);
            }
            link_name_sb.printf("__SIF__%s", struct_decl->name);
            struct_decl->link_name = intern_string(link_name_sb.string());
            break;
        }
        case DECL_ENUM: {
            Enum_Declaration *enum_decl = (Enum_Declaration *)decl;
            Type_Enum *enum_type = NEW_TYPE(Type_Enum(enum_decl->name, enum_decl->notes));
            Type *enum_base_type = type_int;
            if (enum_decl->ast_enum->base_type_expr) {
                Operand *type_operand = typecheck_expr(enum_decl->ast_enum->base_type_expr, type_typeid);
                if (!type_operand) {
                    return false;
                }
                assert(is_type_typeid(type_operand->type));
                if (!is_type_integer(type_operand->type_value)) {
                    report_error(enum_decl->ast_enum->base_type_expr->location, "Enum base type must be an integer type.");
                    return false;
                }
                enum_base_type = type_operand->type_value;
            }
            assert(is_type_integer(enum_base_type));
            enum_type->flags = enum_base_type->flags;
            enum_type->size = enum_base_type->size;
            enum_type->align = enum_base_type->align;
            enum_type->base_type = enum_base_type;
            enum_decl->ast_enum->type = enum_type;

            assert(!g_silence_errors);
            g_silence_errors = true;

            int count_left_to_resolve = enum_decl->ast_enum->fields.count;
            int enum_field_value = 0;
            bool made_progress = true;
            while ((made_progress && count_left_to_resolve > 0) || !g_silence_errors) {
                made_progress = false;
                For (idx, enum_decl->ast_enum->fields) {
                    Enum_Field *field = &enum_decl->ast_enum->fields[idx];
                    if (field->resolved) {
                        continue;
                    }

                    if (field->expr) {
                        Operand *expr_operand = typecheck_expr(field->expr);
                        if (!expr_operand) {
                            if (g_silence_errors) {
                                continue;
                            }
                            return false;
                        }
                        if (!(expr_operand->flags & OPERAND_CONSTANT)) {
                            report_error(expr_operand->location, "Enum fields must be constant.");
                            return false;
                        }
                        if (!is_type_integer(expr_operand->type)) {
                            report_error(expr_operand->location, "Enum fields must be integers.");
                            return false;
                        }
                        enum_field_value = expr_operand->int_value;
                    }
                    Operand field_operand(field->location);
                    field_operand.flags = OPERAND_CONSTANT | OPERAND_RVALUE;
                    field_operand.type = enum_type;
                    field_operand.int_value  = enum_field_value;
                    field_operand.uint_value = enum_field_value;
                    enum_field_value += 1;
                    if (!try_add_constant_type_field(enum_type, field->name, field_operand, field->location)) {
                        assert(false);
                    }
                    if (!register_declaration(enum_decl->ast_enum->enum_block, SIF_NEW_CLONE(Constant_Declaration(field->name, field_operand, enum_decl->ast_enum->enum_block, field->location), g_global_linear_allocator))) {
                        return false;
                    }
                    field->resolved = true;
                    made_progress = true;
                    count_left_to_resolve -= 1;
                }

                if (!made_progress) {
                    g_silence_errors = false;
                }
            }

            assert(count_left_to_resolve == 0);

            assert(g_silence_errors);
            g_silence_errors = false;
            decl_operand.type = type_typeid;
            decl_operand.type_value = enum_decl->ast_enum->type;
            decl_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE | OPERAND_RVALUE;
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
                if ((!(type_operand->flags & OPERAND_CONSTANT)) || (!is_type_typeid(type_operand->type)) || (type_operand->type_value == nullptr)) {
                    report_error(var->type_expr->location, "Variable type expression must be a constant typeid.");
                    return false;
                }
                assert(type_operand->type_value);
                declared_type = type_operand->type_value;
            }

            if (var->expr) {
                Operand *expr_operand = typecheck_expr(var->expr, declared_type);
                if (!expr_operand) {
                    return false;
                }
                if (declared_type) {
                    if (!match_types(*expr_operand, declared_type, expr_operand)) {
                        report_error(var->expr->location, "Variable was declared with type '%s' but was given an expression of type '%s'.", type_to_string(declared_type), type_to_string(expr_operand->type));
                        return false;
                    }
                }
                else {
                    if (!var->is_constant) {
                        expr_operand->type = try_concretize_type_without_context(expr_operand->type);
                        if (expr_operand->type == nullptr) {
                            assert(expr_operand->type == type_untyped_null);
                            report_error(var->location, "Cannot infer type from expression `null`.");
                            return false;
                        }
                        assert(!is_type_untyped(expr_operand->type));
                    }
                    assert(expr_operand->type != nullptr);
                    declared_type = expr_operand->type;
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
            if (!complete_type(var->type, var->location)) {
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
                if (is_type_typeid(decl_operand.type)) {
                    assert(decl_operand.type_value != nullptr);
                }
            }

            if (is_type_varargs(var->type)) {
                if (var->is_parameter_for_procedure == nullptr) {
                    report_error(var->location, "Vararg types are only allowed as procedure parameters.");
                    return false;
                }
                Type_Varargs *varargs_type = (Type_Varargs *)var->type;
                if (varargs_type->is_c_varargs && !var->is_parameter_for_procedure->is_foreign) {
                    report_error(var->location, "#c_varargs is only allowed on #foreign procedures.");
                    return false;
                }
            }

            if (var->parent_block->flags & BF_IS_FILE_SCOPE) {
                if (var->expr != nullptr && (!(var->expr->operand.flags & OPERAND_CONSTANT))) {
                    report_error(var->expr->location, "Initialization expression for global variable must be constant.");
                    return false;
                }
            }

            break;
        }
        case DECL_PROC: {
            Proc_Declaration *proc_decl = (Proc_Declaration *)decl;
            if (!typecheck_procedure_header(proc_decl->header)) {
                return false;
            }
            assert(proc_decl->header->type != nullptr);
            decl_operand = proc_decl->header->operand;

            if (proc_decl->name == g_interned_main_string) {
                assert(g_main_proc == nullptr);
                g_main_proc = proc_decl->header->procedure;
            }
            else if (!proc_decl->header->is_foreign) {
                assert(proc_decl->header->name != nullptr);
                String_Builder link_name_sb = make_string_builder(g_global_linear_allocator, 128);
                if (proc_decl->header->parent_declaration != nullptr) {
                    link_name_sb.print(proc_decl->header->parent_declaration->link_name);
                }
                link_name_sb.printf("__SIF__%s", proc_decl->name);
                For (idx, proc_decl->header->type->parameter_types) {
                    assert(proc_decl->header->type->parameter_types[idx] != nullptr);
                    link_name_sb.printf("_%s", type_to_string_plain(proc_decl->header->parameters[idx]->type));
                }
                if (proc_decl->header->type->return_type != nullptr) {
                    link_name_sb.printf("_%s", type_to_string_plain(proc_decl->header->type->return_type));
                }
                decl->link_name = intern_string(link_name_sb.string());
            }
            break;
        }
        case DECL_CONSTANT_VALUE: {
            Constant_Declaration *constant = (Constant_Declaration *)decl;
            decl_operand = constant->operand;
            break;
        }
        case DECL_STRUCT_MEMBER: {
            Struct_Member_Declaration *struct_member_decl = (Struct_Member_Declaration *)decl;
            decl_operand = struct_member_decl->operand;
            break;
        }
        case DECL_USING: {
            Using_Declaration *using_decl = (Using_Declaration *)decl;
            if (!check_declaration(using_decl->declaration, using_decl->location, &decl_operand)) {
                return false;
            }
            break;
        }
        default: {
            printf("unhandled case: %d\n", decl->kind);
            assert(false);
        }
    }
    decl_operand.referenced_declaration = decl;
    decl->operand = decl_operand;
    decl->check_state = DCS_CHECKED;



    if (decl->kind == DECL_PROC) {
        Proc_Declaration *proc_decl = (Proc_Declaration *)decl;
        if (!proc_decl->header->is_foreign) {
            assert(proc_decl->header->procedure->body != nullptr);
            if (!is_type_polymorphic(proc_decl->header->type)) {
                if (!typecheck_block(proc_decl->header->procedure->body)) {
                    return false;
                }
                if (proc_decl->header->type->return_type) {
                    if (!check_basic_block_for_return(proc_decl->header->root_basic_block)) {
                        report_error(proc_decl->header->location, "Procedure doesn't return on all code paths.");
                        return false;
                    }
                }
            }
            else {
                proc_decl->is_polymorphic = true;
            }
        }
        else {
            assert(proc_decl->header->procedure->body == nullptr);
        }
    }

    if (decl->kind == DECL_STRUCT) {
        Struct_Declaration *struct_decl = (Struct_Declaration *)decl;
        For (idx, struct_decl->structure->operator_overloads) {
            Ast_Proc *procedure = struct_decl->structure->operator_overloads[idx];

            assert(!procedure->header->is_foreign);
            assert(procedure->body != nullptr);
            assert(procedure->header->name == nullptr);
            if (procedure->header->operator_to_overload != TK_INVALID) {
                assert(procedure->header->name == nullptr);
                String_Builder op_overload_name_sb = make_string_builder(g_global_linear_allocator, 128);
                op_overload_name_sb.printf("__SIF__operator_overload_%s", token_name(procedure->header->operator_to_overload));
                assert(procedure->header->parameters.count == 2);
                procedure->header->name = intern_string(op_overload_name_sb.string());
                assert(procedure->header->declaration != nullptr);
                assert(procedure->header->declaration->name == nullptr);
                procedure->header->declaration->name = procedure->header->name;
            }

            if (!typecheck_procedure_header(procedure->header)) {
                return false;
            }
            assert(procedure->header->name != nullptr);
            assert(procedure->header->declaration != nullptr);
            if (!check_declaration(procedure->header->declaration, usage_location)) {
                return false;
            }
        }
        // todo(josh): this feels bad
        For (idx, struct_decl->structure->procedures) {
            Ast_Proc *procedure = struct_decl->structure->procedures[idx];
            if (!check_declaration(procedure->header->declaration, usage_location)) {
                return false;
            }
            if (!register_declaration(struct_decl->structure->type->declarations_block, procedure->header->declaration)) {
                return false;
            }
        }
        For (idx, struct_decl->structure->local_structs) {
            Ast_Struct *local_struct = struct_decl->structure->local_structs[idx];
            if (!check_declaration(local_struct->declaration, usage_location)) {
                return false;
            }
            if (!register_declaration(struct_decl->structure->type->declarations_block, local_struct->declaration)) {
                return false;
            }
        }
        For (idx, struct_decl->structure->local_enums) {
            Ast_Enum *local_enum = struct_decl->structure->local_enums[idx];
            if (!check_declaration(local_enum->declaration, usage_location)) {
                return false;
            }
            if (!register_declaration(struct_decl->structure->type->declarations_block, local_enum->declaration)) {
                return false;
            }
        }
    }
    else {
        assert(decl->parent_block);
        if (decl->parent_block->flags & BF_IS_FILE_SCOPE || decl->kind == DECL_PROC) {
            if (!decl->is_polymorphic) {
                add_ordered_declaration(decl);
            }
        }
    }

    if (out_operand) {
        *out_operand = decl_operand;
    }
    return true;
}

bool typecheck_global_scope(Ast_Block *global_scope) {
    assert(global_scope->flags & BF_IS_GLOBAL_SCOPE);

    For (idx, global_scope->declarations) {
        Declaration *decl = global_scope->declarations[idx];
        if (decl->is_polymorphic) {
            continue;
        }
        For (note_idx, decl->notes) {
            char *note = decl->notes[note_idx];
            if (note == g_interned_sif_runtime_string) {
                     if (decl->name == g_interned_sif_runtime_bounds_check_proc_string)            { sif_runtime_bounds_check_proc             = decl; }
                else if (decl->name == g_interned_sif_runtime_null_check_proc_string)              { sif_runtime_null_check_proc               = decl; }
                else if (decl->name == g_interned_sif_runtime_string_eq_proc_string)               { sif_runtime_string_eq_proc                = decl; }
                else if (decl->name == g_interned_sif_runtime_zero_pointer_proc_string)            { sif_runtime_zero_pointer_proc             = decl; }
                else if (decl->name == g_interned_sif_runtime_source_code_location_string)         { sif_runtime_source_code_location          = decl; }
                else if (decl->name == g_interned_sif_runtime_make_source_code_location_string)    { sif_runtime_make_source_code_location     = decl; }
                else if (decl->name == g_interned_sif_runtime_union_all_type_infos_string)         { assert(decl->kind == DECL_STRUCT); sif_runtime_union_all_type_infos   = (Struct_Declaration *)decl; }
                else if (decl->name == g_interned_sif_runtime_type_info_string)                    { assert(decl->kind == DECL_STRUCT); sif_runtime_type_info              = (Struct_Declaration *)decl; }
                else if (decl->name == g_interned_sif_runtime_type_info_integer_string)            { assert(decl->kind == DECL_STRUCT); sif_runtime_type_info_integer      = (Struct_Declaration *)decl; }
                else if (decl->name == g_interned_sif_runtime_type_info_float_string)              { assert(decl->kind == DECL_STRUCT); sif_runtime_type_info_float        = (Struct_Declaration *)decl; }
                else if (decl->name == g_interned_sif_runtime_type_info_bool_string)               { assert(decl->kind == DECL_STRUCT); sif_runtime_type_info_bool         = (Struct_Declaration *)decl; }
                else if (decl->name == g_interned_sif_runtime_type_info_string_string)             { assert(decl->kind == DECL_STRUCT); sif_runtime_type_info_string       = (Struct_Declaration *)decl; }
                else if (decl->name == g_interned_sif_runtime_type_info_struct_field_string)       { assert(decl->kind == DECL_STRUCT); sif_runtime_type_info_struct_field = (Struct_Declaration *)decl; }
                else if (decl->name == g_interned_sif_runtime_type_info_struct_string)             { assert(decl->kind == DECL_STRUCT); sif_runtime_type_info_struct       = (Struct_Declaration *)decl; }
                else if (decl->name == g_interned_sif_runtime_type_info_union_string)              { assert(decl->kind == DECL_STRUCT); sif_runtime_type_info_union        = (Struct_Declaration *)decl; }
                else if (decl->name == g_interned_sif_runtime_type_info_enum_field_string)         { assert(decl->kind == DECL_STRUCT); sif_runtime_type_info_enum_field   = (Struct_Declaration *)decl; }
                else if (decl->name == g_interned_sif_runtime_type_info_enum_string)               { assert(decl->kind == DECL_STRUCT); sif_runtime_type_info_enum         = (Struct_Declaration *)decl; }
                else if (decl->name == g_interned_sif_runtime_type_info_pointer_string)            { assert(decl->kind == DECL_STRUCT); sif_runtime_type_info_pointer      = (Struct_Declaration *)decl; }
                else if (decl->name == g_interned_sif_runtime_type_info_slice_string)              { assert(decl->kind == DECL_STRUCT); sif_runtime_type_info_slice        = (Struct_Declaration *)decl; }
                else if (decl->name == g_interned_sif_runtime_type_info_array_string)              { assert(decl->kind == DECL_STRUCT); sif_runtime_type_info_array        = (Struct_Declaration *)decl; }
                else if (decl->name == g_interned_sif_runtime_type_info_reference_string)          { assert(decl->kind == DECL_STRUCT); sif_runtime_type_info_reference    = (Struct_Declaration *)decl; }
                else if (decl->name == g_interned_sif_runtime_type_info_procedure_string)          { assert(decl->kind == DECL_STRUCT); sif_runtime_type_info_procedure    = (Struct_Declaration *)decl; }
                else if (decl->name == g_interned_sif_runtime_type_info_typeid_string)             { assert(decl->kind == DECL_STRUCT); sif_runtime_type_info_typeid       = (Struct_Declaration *)decl; }

                if (!check_declaration(decl, decl->location)) {
                    report_error(decl->location, "Internal compiler error. check_declaration() failed.");
                    return false;
                }
                if (!complete_type(decl->operand.type, decl->location, TCF_ALL)) {
                    report_error(decl->location, "Internal compiler error. complete_type() failed.");
                    return false;
                }
                break;
            }
        }
    }

    For (idx, global_scope->declarations) {
        Declaration *decl = global_scope->declarations[idx];
        if (decl->is_polymorphic) {
            continue;
        }
        if (!check_declaration(decl, decl->location)) {
            return false;
        }
    }

    // do assert directives
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

    // do print directives
    For (idx, g_all_print_directives) {
        Ast_Directive_Print *print_directive = g_all_print_directives[idx];
        Operand *expr_operand = typecheck_expr(print_directive->expr);
        if (!expr_operand) {
            return false;
        }
        if (!(expr_operand->flags & OPERAND_CONSTANT)) {
            report_error(print_directive->expr->location, "#print directive require a constant.");
            return false;
        }

        if (is_type_integer(expr_operand->type)) {
            report_info(print_directive->location, "%lld", expr_operand->int_value);
        }
        else if (is_type_float(expr_operand->type)) {
            report_info(print_directive->location, "%f", expr_operand->f64_value);
        }
        else if (expr_operand->type == type_bool) {
            report_info(print_directive->location, (expr_operand->bool_value ? "true" : "false"));
        }
        else if (is_type_string(expr_operand->type)) {
            report_info(print_directive->location, "%s", expr_operand->escaped_string_value);
        }
        else if (is_type_typeid(expr_operand->type)) {
            report_info(print_directive->location, "%s", type_to_string(expr_operand->type_value));
        }
        else {
            assert(false);
        }
    }

    // note(josh): complete any types that haven't been completed because they haven't been used.
    //             we have to do this because using a struct type by pointer will NOT trigger
    //             a call to complete_type(), only actually using the type and it's members will.
    For (idx, all_types) {
        Type *type = all_types[idx];
        if (!complete_type(type, {}, TCF_ALL)) {
            return false;
        }
    }

    // check the entrypoint
    if (g_main_proc == nullptr) {
        report_error({}, "main() must be defined.");
        return false;
    }
    if (g_main_proc->header->type->return_type && g_main_proc->header->type->return_type != type_i32) {
        report_error(g_main_proc->header->location, "main() must return nothing or i32.");
        return false;
    }
    if (g_main_proc->header->type->parameter_types.count != 0) {
        report_error(g_main_proc->header->location, "main() cannot have parameters.");
        return false;
    }
    if (g_main_proc->header->procedure == nullptr) {
        report_error(g_main_proc->header->location, "main() must have a body.");
        return false;
    }
    if (g_main_proc->header->is_foreign) {
        report_error(g_main_proc->header->location, "main() cannot be foreign.");
        return false;
    }
    assert(g_main_proc->parent_block != nullptr);
    if (!(g_main_proc->parent_block->flags & BF_IS_FILE_SCOPE)) {
        report_error(g_main_proc->header->location, "main() must be located at file scope.");
        return false;
    }
    assert(g_main_proc->parent_block->parent_block == global_scope);
    return true;
}

void sbprint_constant_operand(String_Builder *sb, Operand operand) {
    assert(operand.flags & OPERAND_CONSTANT);
    // todo(josh): :IntUintCorrectness
    if (is_type_integer(operand.type)) {
        sb->printf("%lld", operand.int_value);
    }
    else if (is_type_float(operand.type)) {
        sb->printf("%f", operand.f64_value);
    }
    else if (is_type_string(operand.type)) {
        sb->printf("%s", operand.escaped_string_value);
    }
    else if (is_type_bool(operand.type)) {
        sb->print(operand.bool_value ? "true" : "false");
    }
    else if (is_type_typeid(operand.type)) {
        assert(operand.type_value != nullptr);
        sb->printf("%s", type_to_string(operand.type_value));
    }
    else {
        assert(false);
    }
}

char *type_to_string(Type *type, int *out_length) {
    if (type->printable_name) {
        if (out_length) {
            *out_length = type->printable_name_length;
        }
        return type->printable_name;
    }
    String_Builder sb = make_string_builder(g_global_linear_allocator, 128);
    switch (type->kind) {
        case TYPE_PRIMITIVE: {
            Type_Primitive *primitive = (Type_Primitive *)type;
            sb.print(primitive->name);
            break;
        }
        case TYPE_STRUCT: {
            Type_Struct *structure = (Type_Struct *)type;
            if (structure->is_polymorph_of) {
                sb.print(structure->is_polymorph_of->name);
                sb.print("!(");
                For (idx, structure->polymorphic_parameter_values) {
                    if (idx != 0) {
                        sb.print(", ");
                    }
                    sbprint_constant_operand(&sb, structure->polymorphic_parameter_values[idx]);
                }
                sb.print(")");
            }
            else {
                sb.print(structure->name);
            }
            break;
        }
        case TYPE_PROCEDURE: {
            Type_Procedure *type_proc = (Type_Procedure *)type;
            sb.printf("proc(");
            For (idx, type_proc->parameter_types) {
                if (idx != 0) {
                    sb.printf(", ");
                }
                sb.printf("%s", type_to_string(type_proc->parameter_types[idx]));
            }
            sb.printf(")");
            if (type_proc->return_type) {
                sb.printf(" : %s", type_to_string(type_proc->return_type));
            }
            break;
        }
        case TYPE_POINTER: {
            Type_Pointer *pointer = (Type_Pointer *)type;
            assert(pointer->pointer_to);
            sb.printf("^%s", type_to_string(pointer->pointer_to));
            break;
        }
        case TYPE_REFERENCE: {
            Type_Reference *reference = (Type_Reference *)type;
            sb.printf(">%s", type_to_string(reference->reference_to));
            break;
        }
        case TYPE_ARRAY: {
            Type_Array *array = (Type_Array *)type;
            sb.printf("[%d]%s", array->count, type_to_string(array->array_of));
            break;
        }
        case TYPE_SLICE: {
            Type_Slice *slice = (Type_Slice *)type;
            sb.printf("[]%s", type_to_string(slice->slice_of));
            break;
        }
        case TYPE_ENUM: {
            Type_Enum *enum_type = (Type_Enum *)type;
            sb.printf("%s", enum_type->name);
            break;
        }
        case TYPE_VARARGS: {
            Type_Varargs *varargs_type = (Type_Varargs *)type;
            sb.printf("..%s", type_to_string(varargs_type->varargs_of));
            break;
        }
        default: {
            assert(false);
        }
    }
    type->printable_name = intern_string(sb.string());
    type->printable_name_length = sb.buf.count;
    if (out_length) {
        *out_length = type->printable_name_length;
    }
    return type->printable_name;
}

char *type_to_string_plain(Type *type) {
    String_Builder sb = make_string_builder(g_global_linear_allocator, 128);
    switch (type->kind) {
        case TYPE_PRIMITIVE: {
            Type_Primitive *primitive = (Type_Primitive *)type;
            sb.print(primitive->name);
            break;
        }
        case TYPE_STRUCT: {
            Type_Struct *structure = (Type_Struct *)type;
            if (structure->is_polymorph_of) {
                sb.print(structure->is_polymorph_of->name);
                sb.print("_poly_");
                For (idx, structure->polymorphic_parameter_values) {
                    if (idx != 0) {
                        sb.print("_");
                    }
                    // todo(josh): this might add non-identifier characters. need to do something special here.
                    Operand value = structure->polymorphic_parameter_values[idx];
                    if (is_type_typeid(value.type)) {
                        sb.print(type_to_string_plain(value.type_value));
                    }
                    else {
                        sbprint_constant_operand(&sb, value);
                    }
                }
            }
            else {
                sb.print(structure->name);
            }
            break;
        }
        case TYPE_PROCEDURE: {
            Type_Procedure *procedure = (Type_Procedure *)type;
            sb.printf("procedure");
            For (idx, procedure->parameter_types) {
                sb.printf("_param_%s", type_to_string_plain(procedure->parameter_types[idx]));
            }
            if (procedure->return_type) {
                sb.printf("_return_%s", type_to_string_plain(procedure->return_type));
            }
            break;
        }
        case TYPE_POINTER: {
            Type_Pointer *pointer = (Type_Pointer *)type;
            assert(pointer->pointer_to);
            sb.printf("pointer_%s", type_to_string_plain(pointer->pointer_to));
            break;
        }
        case TYPE_REFERENCE: {
            Type_Reference *reference = (Type_Reference *)type;
            sb.printf("reference_%s", type_to_string_plain(reference->reference_to));
            break;
        }
        case TYPE_ARRAY: {
            Type_Array *array = (Type_Array *)type;
            sb.printf("array_%d_%s", array->count, type_to_string_plain(array->array_of));
            break;
        }
        case TYPE_SLICE: {
            Type_Slice *slice = (Type_Slice *)type;
            sb.printf("slice_%s", type_to_string_plain(slice->slice_of));
            break;
        }
        case TYPE_ENUM: {
            Type_Enum *enum_type = (Type_Enum *)type;
            sb.printf("enum_%s", enum_type->name);
            break;
        }
        case TYPE_VARARGS: {
            Type_Varargs *varargs_type = (Type_Varargs *)type;
            sb.printf("varargs_%s", type_to_string_plain(varargs_type->varargs_of));
            break;
        }
        default: {
            assert(false);
        }
    }
    return sb.string();
}

void type_mismatch(Location location, Type *got, Type *expected) {
    report_error(location, "Type mismatch. Expected '%s', got '%s'.", type_to_string(expected), type_to_string(got));
}

bool match_types(Operand operand, Type *expected_type, Operand *out_operand, bool do_report_error) {
    assert(operand.type != nullptr);
    assert(expected_type != nullptr);

    *out_operand = operand;

    if (out_operand->type == expected_type) {
        return true;
    }

    if (expected_type == type_any) {
        out_operand->type = try_concretize_type_without_context(out_operand->type);
        if (!out_operand->type) {
            assert(out_operand->type == type_untyped_null);
            if (do_report_error) {
                report_error(out_operand->location, "todo(josh): allow assigning null to `any` to clear the value.");
            }
            return false;
        }
        return true;
    }

    if (is_type_reference(expected_type)) {
        if ((out_operand->flags & OPERAND_LVALUE)) {
            return true;
        }
        Type_Reference *reference = (Type_Reference *)expected_type;
        return match_types(operand, reference->reference_to, out_operand, do_report_error);
    }

    if (is_type_reference(out_operand->type)) {
        Type_Reference *reference_type = (Type_Reference *)out_operand->type;
        if (reference_type->reference_to == expected_type) {
            return true;
        }
    }

    if (out_operand->type->flags & TF_UNTYPED) {
        if (is_type_number(out_operand->type) && is_type_number(expected_type)) {
            if (is_type_integer(expected_type) && is_type_float(out_operand->type)) {
                if (do_report_error) {
                    report_error(out_operand->location, "Expected an integer type, got a float.");
                }
                return false;
            }
            out_operand->type = expected_type;
            return true;
        }

        if ((out_operand->type->flags & TF_POINTER) && (expected_type->flags & TF_POINTER)) {
            assert(out_operand->type == type_untyped_null);
            out_operand->type = expected_type;
            return true;
        }

        if (is_type_string(out_operand->type)) {
            if (is_type_string(expected_type)) {
                out_operand->type = expected_type;
                return true;
            }
            if (expected_type == type_cstring) {
                out_operand->type = type_cstring;
                return true;
            }
        }
    }

    if (expected_type == type_rawptr && (out_operand->type->flags & TF_POINTER)) {
        return true;
    }

    if (do_report_error) {
        type_mismatch(out_operand->location, out_operand->type, expected_type);
    }
    return false;
}

Type_Pointer *get_or_create_type_pointer_to(Type *pointer_to) {
    assert(!is_type_untyped(pointer_to));
    if (pointer_to->pointer_to_this_type) {
        return pointer_to->pointer_to_this_type;
    }
    Type_Pointer *new_type = NEW_TYPE(Type_Pointer(pointer_to));
    new_type->flags = TF_POINTER;
    new_type->size = POINTER_SIZE;
    new_type->align = POINTER_SIZE;
    new_type->declarations_block = pointer_to->declarations_block;
    pointer_to->pointer_to_this_type = new_type;
    return new_type;
}

Type_Reference *get_or_create_type_reference_to(Type *reference_to) {
    assert(!is_type_untyped(reference_to));
    if (reference_to->reference_to_this_type) {
        return reference_to->reference_to_this_type;
    }
    Type_Reference *new_type = NEW_TYPE(Type_Reference(reference_to));
    new_type->size = POINTER_SIZE;
    new_type->align = POINTER_SIZE;
    reference_to->reference_to_this_type = new_type;
    return new_type;
}

Type_Array *get_or_create_type_array_of(Type *array_of, int count) {
    assert(array_of != nullptr);
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
    Type_Array *new_type = NEW_TYPE(Type_Array(array_of, count));
    if (!try_add_variable_type_field(new_type, intern_string(g_interned_data_string), type_rawptr, 0, {})) {
        assert(false);
    }
    Operand operand = {};
    operand.type = type_int;
    operand.flags = OPERAND_RVALUE | OPERAND_CONSTANT;
    operand.int_value  = count;
    operand.uint_value = count;
    if (!try_add_constant_type_field(new_type, intern_string(g_interned_count_string), operand, {})) {
        assert(false);
    }
    return new_type;
}

Type_Slice *get_or_create_type_slice_of(Type *slice_of) {
    assert(!is_type_untyped(slice_of));
    if (slice_of->slice_of_this_type) {
        return slice_of->slice_of_this_type;
    }
    Type_Pointer *pointer_to_element_type = get_or_create_type_pointer_to(slice_of);
    Type_Slice *new_type = NEW_TYPE(Type_Slice(slice_of, pointer_to_element_type));
    new_type->size  = 16;
    new_type->align = 8;
    if (!try_add_variable_type_field(new_type, intern_string(g_interned_data_string), pointer_to_element_type, 0, {})) {
        assert(false);
    }
    if (!try_add_variable_type_field(new_type, intern_string(g_interned_count_string), type_int, 8, {})) {
        assert(false);
    }
    slice_of->slice_of_this_type = new_type;
    return new_type;
}

Type_Varargs *get_or_create_type_varargs_of(Type *varargs_of, bool is_c_varargs) {
    assert(!is_type_untyped(varargs_of));
    if (is_c_varargs) {
        if (varargs_of->c_varargs_of_this_type) {
            return varargs_of->c_varargs_of_this_type;
        }
    }
    else {
        if (varargs_of->varargs_of_this_type) {
            return varargs_of->varargs_of_this_type;
        }
    }
    Type_Pointer *pointer_to_element_type = get_or_create_type_pointer_to(varargs_of);
    Type_Slice *slice_type = get_or_create_type_slice_of(varargs_of);
    Type_Varargs *new_type = NEW_TYPE(Type_Varargs(varargs_of, pointer_to_element_type, slice_type, is_c_varargs));
    new_type->size  = 16;
    new_type->align = 8;
    if (!try_add_variable_type_field(new_type, intern_string(g_interned_data_string), pointer_to_element_type, 0, {})) {
        assert(false);
    }
    if (!try_add_variable_type_field(new_type, intern_string(g_interned_count_string), type_int, 8, {})) {
        assert(false);
    }
    if (is_c_varargs) {
        varargs_of->c_varargs_of_this_type = new_type;
    }
    else {
        varargs_of->varargs_of_this_type = new_type;
    }
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
    Type_Procedure *new_type = NEW_TYPE(Type_Procedure(parameter_types, return_type));
    new_type->flags = TF_POINTER;
    For (idx, parameter_types) {
        if (is_type_polymorphic(parameter_types[idx])) {
            new_type->flags |= TF_POLYMORPHIC;
        }
    }
    new_type->size = POINTER_SIZE;
    new_type->align = POINTER_SIZE;
    return new_type;
}

bool can_cast(Ast_Expr *expr, Type *target_type) {
    assert(expr != nullptr);
    assert(target_type != nullptr);
    if (expr->operand.type == target_type) {
        // casting to the same type
        // todo(josh): should we disallow casting for struct types?
        return true;
    }
    if (is_type_number(expr->operand.type)) {
        if (is_type_enum(target_type)) {
            return true;
        }
        if (is_type_number(target_type)) {
            return true;
        }
        if (target_type->flags & TF_POINTER) {
            return true;
        }
    }
    if (expr->operand.type == type_untyped_string) {
        if (target_type == type_cstring || target_type == type_string) {
            return true;
        }
    }
    if ((expr->operand.type->flags & TF_POINTER) && (target_type->flags & TF_POINTER)) {
        return true;
    }
    return false;
}

bool operator_is_defined(Type *lhs, Type *rhs, Token_Kind op, bool both_are_constant) {
    switch (op) {
        case TK_EQUAL_TO: {
                 if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            else if (is_type_float(lhs)   && is_type_float(rhs))    return true;
            else if (is_type_bool(lhs)    && is_type_bool(rhs))     return true;
            else if (is_type_typeid(lhs)  && is_type_typeid(rhs))   return true;
            else if (is_type_string(lhs)  && is_type_string(rhs))   return true;
            else if ((lhs->flags & TF_POINTER) && (rhs->flags & TF_POINTER))  return true;
            break;
        }
        case TK_NOT_EQUAL_TO: {
                 if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            else if (is_type_float(lhs)   && is_type_float(rhs))    return true;
            else if (is_type_bool(lhs)    && is_type_bool(rhs))     return true;
            else if (is_type_typeid(lhs)  && is_type_typeid(rhs))   return true;
            else if (is_type_string(lhs)  && is_type_string(rhs))   return true;
            else if ((lhs->flags & TF_POINTER) && (rhs->flags & TF_POINTER))  return true;
            break;
        }
        case TK_PLUS: {
                 if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            else if (is_type_float(lhs)   && is_type_float(rhs))    return true;
            if (both_are_constant) {
                if (is_type_string(lhs)  && is_type_string(rhs))   return true;
            }
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
        case TK_MOD: {
                 if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            // todo(josh): fmod?
            // else if (is_type_float(lhs)   && is_type_float(rhs))    return true;
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
        case TK_AMPERSAND: { // note(josh): BIT_AND
            if (both_are_constant) {
                if (is_type_number(lhs) && is_type_number(rhs))  return true;
            }
            else {
                if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            }
            break;
        }
        case TK_BIT_OR: {
            if (both_are_constant) {
                if (is_type_number(lhs) && is_type_number(rhs))  return true;
            }
            else {
                if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            }
            break;
        }
        case TK_TILDE: { // note(josh): bit XOR
            if (both_are_constant) {
                if (is_type_number(lhs) && is_type_number(rhs))  return true;
            }
            else {
                if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            }
            break;
        }
        case TK_LEFT_SHIFT: {
            if (both_are_constant) {
                if (is_type_number(lhs) && is_type_number(rhs))  return true;
            }
            else {
                if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            }
            break;
        }
        case TK_RIGHT_SHIFT: {
            if (both_are_constant) {
                if (is_type_number(lhs) && is_type_number(rhs))  return true;
            }
            else {
                if (is_type_integer(lhs) && is_type_integer(rhs))  return true;
            }
            break;
        }
        default: {
            printf("Unhandled operator: %s\n", token_string(op));
            assert(false);
        }
    }
    return false;
}

bool binary_eval(Operand lhs, Operand rhs, Token_Kind op, Location location, Operand *out_operand) {
    Type *most_concrete = get_most_concrete_type(lhs.type, rhs.type);

    Operand result_operand(location);
    result_operand.flags |= OPERAND_RVALUE;
    switch (op) {
        case TK_EQUAL_TO: {
            result_operand.type = type_bool;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                     if (is_type_integer(lhs.type)   && is_type_integer(rhs.type))   result_operand.bool_value = lhs.int_value    == rhs.int_value;
                else if (is_type_float(lhs.type)     && is_type_float(rhs.type))     result_operand.bool_value = lhs.f64_value    == rhs.f64_value;
                else if (is_type_bool(lhs.type)      && is_type_bool(rhs.type))      result_operand.bool_value = lhs.bool_value   == rhs.bool_value;
                else if (is_type_typeid(lhs.type)    && is_type_typeid(rhs.type))    result_operand.bool_value = lhs.type_value   == rhs.type_value;
                else if (is_type_procedure(lhs.type) && is_type_procedure(rhs.type)) result_operand.bool_value = lhs.proc_value   == rhs.proc_value;
                else if (is_type_string(lhs.type)    && is_type_string(rhs.type))    result_operand.bool_value = (lhs.escaped_string_length == rhs.escaped_string_length) && (strcmp(lhs.escaped_string_value, rhs.escaped_string_value) == 0);
                else {
                    report_error(location, "Operator == is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            break;
        }
        case TK_NOT_EQUAL_TO: {
            result_operand.type = type_bool;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                     if (is_type_integer(lhs.type)   && is_type_integer(rhs.type))   result_operand.bool_value = lhs.int_value   != rhs.int_value;
                else if (is_type_float(lhs.type)     && is_type_float(rhs.type))     result_operand.bool_value = lhs.f64_value   != rhs.f64_value;
                else if (is_type_bool(lhs.type)      && is_type_bool(rhs.type))      result_operand.bool_value = lhs.bool_value  != rhs.bool_value;
                else if (is_type_typeid(lhs.type)    && is_type_typeid(rhs.type))    result_operand.bool_value = lhs.type_value  != rhs.type_value;
                else if (is_type_procedure(lhs.type) && is_type_procedure(rhs.type)) result_operand.bool_value = lhs.proc_value  != rhs.proc_value;
                else if (is_type_string(lhs.type)    && is_type_string(rhs.type))    result_operand.bool_value = (lhs.escaped_string_length != rhs.escaped_string_length) || (strcmp(lhs.escaped_string_value, rhs.escaped_string_value) != 0);
                else {
                    report_error(location, "Operator != is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            break;
        }
        case TK_PLUS: {
            result_operand.type = most_concrete;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                if (is_type_integer(lhs.type) && is_type_integer(rhs.type)) {
                    result_operand.int_value   = lhs.int_value   + rhs.int_value;
                    result_operand.uint_value  = lhs.uint_value  + rhs.uint_value;
                    result_operand.f32_value = lhs.f32_value + rhs.f32_value;
                    result_operand.f64_value = lhs.f64_value + rhs.f64_value;
                }
                else if (is_type_float(lhs.type)   && is_type_float(rhs.type)) {
                    result_operand.f32_value = lhs.f32_value + rhs.f32_value;
                    result_operand.f64_value = lhs.f64_value + rhs.f64_value;
                }
                else if (is_type_string(lhs.type)  && is_type_string(rhs.type)) {
                    int total_length_scanned = (lhs.scanned_string_length + rhs.scanned_string_length);
                    int total_length_escaped = (lhs.escaped_string_length + rhs.escaped_string_length);
                    char *new_scanned_string = (char *)alloc(g_global_linear_allocator, total_length_scanned+1);
                    memcpy(new_scanned_string, lhs.scanned_string_value, lhs.scanned_string_length);
                    memcpy(new_scanned_string+lhs.scanned_string_length, rhs.scanned_string_value, rhs.scanned_string_length);
                    char *new_escaped_string = (char *)alloc(g_global_linear_allocator, total_length_escaped+1);
                    memcpy(new_escaped_string, lhs.escaped_string_value, lhs.escaped_string_length);
                    memcpy(new_escaped_string+lhs.escaped_string_length, rhs.escaped_string_value, rhs.escaped_string_length);
                    new_scanned_string[total_length_scanned] = '\0';
                    new_escaped_string[total_length_escaped] = '\0';
                    result_operand.scanned_string_value = new_scanned_string;
                    result_operand.scanned_string_length = total_length_scanned;
                    result_operand.escaped_string_value = new_escaped_string;
                    result_operand.escaped_string_length = total_length_escaped;
                }
                else {
                    report_error(location, "Operator + is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            else {
                if (is_type_string(lhs.type)) {
                    assert(is_type_string(rhs.type));
                    report_error(location, "Operand + only works for constant strings, not runtime string values.");
                    return false;
                }
            }
            break;
        }
        case TK_MINUS: {
            result_operand.type = most_concrete;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                if (is_type_integer(lhs.type) && is_type_integer(rhs.type)) {
                    result_operand.int_value   = lhs.int_value   - rhs.int_value;
                    result_operand.uint_value  = lhs.uint_value  - rhs.uint_value;
                    result_operand.f32_value   = lhs.f32_value   - rhs.f32_value;
                    result_operand.f64_value   = lhs.f64_value   - rhs.f64_value;
                }
                else if (is_type_float(lhs.type)   && is_type_float(rhs.type)) {
                    result_operand.f32_value = lhs.f32_value - rhs.f32_value;
                    result_operand.f64_value = lhs.f64_value - rhs.f64_value;
                }
                else {
                    report_error(location, "Operator - is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            break;
        }
        case TK_MULTIPLY: {
            result_operand.type = most_concrete;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                if (is_type_integer(lhs.type) && is_type_integer(rhs.type)) {
                    result_operand.int_value   = lhs.int_value   * rhs.int_value;
                    result_operand.uint_value  = lhs.uint_value  * rhs.uint_value;
                    result_operand.f32_value = lhs.f32_value * rhs.f32_value;
                    result_operand.f64_value = lhs.f64_value * rhs.f64_value;
                }
                else if (is_type_float(lhs.type)   && is_type_float(rhs.type)) {
                    result_operand.f32_value = lhs.f32_value * rhs.f32_value;
                    result_operand.f64_value = lhs.f64_value * rhs.f64_value;
                }
                else {
                    report_error(location, "Operator * is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            break;
        }
        case TK_DIVIDE: {
            result_operand.type = most_concrete;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                if (is_type_integer(lhs.type) && is_type_integer(rhs.type)) {
                    result_operand.int_value   = lhs.int_value   / rhs.int_value;
                    result_operand.uint_value  = lhs.uint_value  / rhs.uint_value;
                    result_operand.f32_value = lhs.f32_value / rhs.f32_value;
                    result_operand.f64_value = lhs.f64_value / rhs.f64_value;
                }
                else if (is_type_float(lhs.type) && is_type_float(rhs.type)) {
                    result_operand.f32_value = lhs.f32_value / rhs.f32_value;
                    result_operand.f64_value = lhs.f64_value / rhs.f64_value;
                }
                else {
                    report_error(location, "Operator / is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            break;
        }
        case TK_MOD: {
            result_operand.type = most_concrete;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                if (is_type_integer(lhs.type) && is_type_integer(rhs.type)) {
                    result_operand.int_value   = lhs.int_value   % rhs.int_value;
                    result_operand.uint_value  = lhs.uint_value  % rhs.uint_value;
                    result_operand.f32_value = (f32)result_operand.int_value;
                    result_operand.f64_value = (f64)result_operand.int_value;
                    // todo(josh): fmod?
                }
                // todo(josh): fmod?
                // else if (is_type_float(lhs.type)   && is_type_float(rhs.type))    result_operand.float_value = lhs.float_value % rhs.float_value;
                else {
                    report_error(location, "Operator %% is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            break;
        }
        case TK_LESS_THAN: {
            result_operand.type = type_bool;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                // todo(josh): :IntUintCorrectness
                if (is_type_integer(lhs.type) && is_type_integer(rhs.type)) {
                    result_operand.bool_value  = lhs.int_value   < rhs.int_value;
                }
                else if (is_type_float(lhs.type)   && is_type_float(rhs.type))    result_operand.bool_value = lhs.f64_value < rhs.f64_value;
                else {
                    report_error(location, "Operator < is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            break;
        }
        case TK_LESS_THAN_OR_EQUAL: {
            result_operand.type = type_bool;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                    // todo(josh): :IntUintCorrectness
                     if (is_type_integer(lhs.type) && is_type_integer(rhs.type))  result_operand.bool_value = lhs.int_value <= rhs.int_value;
                else if (is_type_float(lhs.type)   && is_type_float(rhs.type))    result_operand.bool_value = lhs.f64_value <= rhs.f64_value;
                else {
                    report_error(location, "Operator <= is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            break;
        }
        case TK_GREATER_THAN: {
            result_operand.type = type_bool;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                    // todo(josh): :IntUintCorrectness
                     if (is_type_integer(lhs.type) && is_type_integer(rhs.type))  result_operand.bool_value = lhs.int_value > rhs.int_value;
                else if (is_type_float(lhs.type)   && is_type_float(rhs.type))    result_operand.bool_value = lhs.f64_value > rhs.f64_value;
                else {
                    report_error(location, "Operator > is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            break;
        }
        case TK_GREATER_THAN_OR_EQUAL: {
            result_operand.type = type_bool;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                    // todo(josh): :IntUintCorrectness
                     if (is_type_integer(lhs.type) && is_type_integer(rhs.type))  result_operand.bool_value = lhs.int_value  = rhs.int_value;
                else if (is_type_float(lhs.type)   && is_type_float(rhs.type))    result_operand.bool_value = lhs.f64_value >= rhs.f64_value;
                else {
                    report_error(location, "Operator >= is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            break;
        }
        case TK_BOOLEAN_AND: {
            result_operand.type = type_bool;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                if (is_type_bool(lhs.type) && is_type_bool(rhs.type))  result_operand.bool_value = lhs.bool_value && rhs.bool_value;
                else {
                    report_error(location, "Operator && is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            break;
        }
        case TK_BOOLEAN_OR: {
            result_operand.type = type_bool;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                if (is_type_bool(lhs.type) && is_type_bool(rhs.type))  result_operand.bool_value = lhs.bool_value || rhs.bool_value;
                else {
                    report_error(location, "Operator || is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            break;
        }
        case TK_AMPERSAND: { // note(josh): BIT_AND
            result_operand.type = most_concrete;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                if (is_type_number(lhs.type) && is_type_number(rhs.type)) {
                    result_operand.int_value  = lhs.int_value  & rhs.int_value;
                    result_operand.uint_value = lhs.uint_value & rhs.uint_value;
                    result_operand.f32_value = (f32)result_operand.int_value;
                    result_operand.f64_value = (f64)result_operand.int_value;
                }
                else {
                    report_error(location, "Operator & is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            break;
        }
        case TK_BIT_OR: {
            result_operand.type = most_concrete;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                if (is_type_number(lhs.type) && is_type_number(rhs.type)) {
                    result_operand.int_value  = lhs.int_value  | rhs.int_value;
                    result_operand.uint_value = lhs.uint_value | rhs.uint_value;
                    result_operand.f32_value = (f32)result_operand.int_value;
                    result_operand.f64_value = (f64)result_operand.int_value;
                }
                else {
                    report_error(location, "Operator | is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            break;
        }
        case TK_TILDE: { // XOR
            result_operand.type = most_concrete;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                if (is_type_number(lhs.type) && is_type_number(rhs.type)) {
                    result_operand.int_value = lhs.int_value ^ rhs.int_value;
                    result_operand.uint_value = lhs.uint_value ^ rhs.uint_value;
                    result_operand.f32_value = (f32)result_operand.int_value;
                    result_operand.f64_value = (f64)result_operand.int_value;
                }
                else {
                    report_error(location, "Operator ^ is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            break;
        }
        case TK_LEFT_SHIFT: {
            result_operand.type = most_concrete;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                if (is_type_number(lhs.type) && is_type_number(rhs.type)) {
                    result_operand.int_value  = lhs.int_value  << rhs.int_value;
                    result_operand.uint_value = lhs.uint_value << rhs.uint_value;
                    result_operand.f32_value = (f32)result_operand.int_value;
                    result_operand.f64_value = (f64)result_operand.int_value;
                }
                else {
                    report_error(location, "Operator << is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            break;
        }
        case TK_RIGHT_SHIFT: {
            result_operand.type = most_concrete;
            if ((lhs.flags & OPERAND_CONSTANT) && (rhs.flags & OPERAND_CONSTANT)) {
                result_operand.flags |= OPERAND_CONSTANT;
                if (is_type_number(lhs.type) && is_type_number(rhs.type)) {
                    result_operand.int_value  = lhs.int_value  >> rhs.int_value;
                    result_operand.uint_value = lhs.uint_value >> rhs.uint_value;
                    result_operand.f32_value = (f32)result_operand.int_value;
                    result_operand.f64_value = (f64)result_operand.int_value;
                }
                else {
                    report_error(location, "Operator >> is unsupported for types '%s' and '%s'.", type_to_string(lhs.type), type_to_string(rhs.type));
                    return false;
                }
            }
            break;
        }
        default: {
            printf("Unhandled operator: %s\n", token_string(op));
            assert(false);
        }
    }

    *out_operand = result_operand;
    return true;
}



int total_num_polymorphs = 0;

bool try_create_polymorph_value_declaration(Ast_Expr *value_expr, Operand parameter_operand, Ast_Block *block_to_insert_into, Array<Declaration *> *out_polymorphic_declarations) {
    switch (value_expr->expr_kind) {
        case EXPR_IDENTIFIER: {
            break;
        }
        case EXPR_POLYMORPHIC_VARIABLE: {
            // $name: T
            Expr_Polymorphic_Variable *poly = (Expr_Polymorphic_Variable *)value_expr;
            assert(poly->inserted_declaration == nullptr);
            assert(parameter_operand.type != nullptr);
            if (!(parameter_operand.flags & OPERAND_CONSTANT)) {
                report_error(parameter_operand.location, "Parameter must be constant for polymorphic variable.");
                return false;
            }
            assert(parameter_operand.type != nullptr);
            Declaration *declaration = SIF_NEW_CLONE(Constant_Declaration(poly->ident->name, parameter_operand, block_to_insert_into, value_expr->location), g_global_linear_allocator);
            poly->inserted_declaration = declaration;
            if (!register_declaration(block_to_insert_into, declaration)) {
                assert(false);
            }
            out_polymorphic_declarations->append(declaration);
            break;
        }
        default: {
            Operand *value_operand = typecheck_expr(value_expr);
            Operand result = {};
            if (!binary_eval(*value_operand, parameter_operand, TK_EQUAL_TO, parameter_operand.location, &result)) {
                assert(false);
                return false;
            }
            assert(result.type == type_bool);
            if (!result.bool_value) {
                // note(josh): we don't log an error message here so the caller better do it!
                return false;
            }
            break;
        }
    }
    return true;
}

bool try_create_polymorph_type_declarations(Ast_Expr *type_expr, Type *parameter_type, Location parameter_location, Ast_Block *block_to_insert_into, Array<Declaration *> *out_polymorphic_declarations) {
    switch (type_expr->expr_kind) {
        case EXPR_POLYMORPHIC_VARIABLE: {
            // name: $T
            Expr_Polymorphic_Variable *poly = (Expr_Polymorphic_Variable *)type_expr;
            assert(poly->inserted_declaration == nullptr);
            Type *concrete_type = try_concretize_type_without_context(parameter_type);
            if (!concrete_type) {
                assert(parameter_type == type_untyped_null);
                report_error(parameter_location, "Cannot infer type for polymorphic value '%s' from untyped null.", poly->ident->name);
                return false;
            }
            Operand type_operand(parameter_location);
            type_operand.flags = OPERAND_TYPE | OPERAND_RVALUE | OPERAND_CONSTANT;
            type_operand.type = type_typeid;
            type_operand.type_value = concrete_type;
            Declaration *declaration = SIF_NEW_CLONE(Constant_Declaration(poly->ident->name, type_operand, block_to_insert_into, type_expr->location), g_global_linear_allocator);
            poly->inserted_declaration = declaration;
            if (!register_declaration(block_to_insert_into, declaration)) {
                assert(false);
            }
            out_polymorphic_declarations->append(declaration);
            break;
        }
        case EXPR_POLYMORPHIC_TYPE: {
            assert(parameter_type != nullptr);
            Expr_Polymorphic_Type *poly_type = (Expr_Polymorphic_Type *)type_expr;
            Operand *type_expr_operand = typecheck_expr(poly_type->type_expr);
            if (!is_type_polymorphic(type_expr_operand->type)) {
                report_error(poly_type->type_expr->location, "Expected a polymorphic struct type, got '%s'.", type_to_string(parameter_type));
                return false;
            }
            assert(type_expr_operand->referenced_declaration != nullptr);
            assert(type_expr_operand->referenced_declaration->kind == DECL_STRUCT);
            Ast_Struct *referenced_struct = ((Struct_Declaration *)type_expr_operand->referenced_declaration)->structure;
            assert(referenced_struct != nullptr);
            assert(poly_type->type_expr->operand.type != nullptr);
            if (!is_type_struct(parameter_type)) {
                report_error(parameter_location, "Expected an instance of polymorphic struct '%s', got '%s'.", referenced_struct->name, type_to_string(parameter_type));
                return false;
            }
            Type_Struct *struct_type = (Type_Struct *)parameter_type;
            if (struct_type->is_polymorph_of == nullptr) {
                report_error(parameter_location, "Expected an instance of polymorphic struct '%s', got '%s'.", referenced_struct->name, type_to_string(parameter_type));
                return false;
            }
            if (struct_type->is_polymorph_of != referenced_struct) {
                report_error(parameter_location, "Expected an instance of '%s', got '%s'.", referenced_struct->name, struct_type->is_polymorph_of->name);
                return false;
            }
            assert(struct_type->polymorphic_parameter_values.count == poly_type->parameters.count);
            For (idx, poly_type->parameters) {
                Ast_Expr *poly_expr = poly_type->parameters[idx];
                bool ok = try_create_polymorph_value_declaration(poly_expr, struct_type->polymorphic_parameter_values[idx], block_to_insert_into, out_polymorphic_declarations);
                if (!ok) {
                    return false;
                }
            }
            break;
        }
        case EXPR_POINTER_TYPE: {
            Expr_Pointer_Type *pointer = (Expr_Pointer_Type *)type_expr;
            if (!is_type_pointer(parameter_type)) {
                report_error(parameter_location, "Expected a pointer type, got '%s'.", type_to_string(parameter_type));
                return false;
            }
            Type_Pointer *parameter_pointer = (Type_Pointer *)parameter_type;
            assert(pointer->pointer_to);
            assert(parameter_pointer->pointer_to);
            return try_create_polymorph_type_declarations(pointer->pointer_to, parameter_pointer->pointer_to, parameter_location, block_to_insert_into, out_polymorphic_declarations);
        }
        case EXPR_SLICE_TYPE: {
            Expr_Slice_Type *slice_type = (Expr_Slice_Type *)type_expr;
            if (!is_type_slice(parameter_type)) {
                report_error(parameter_location, "Expected a slice type, got '%s'.", type_to_string(parameter_type));
                return false;
            }
            Type_Slice *parameter_slice = (Type_Slice *)parameter_type;
            return try_create_polymorph_type_declarations(slice_type->slice_of, parameter_slice->slice_of, parameter_location, block_to_insert_into, out_polymorphic_declarations);
        }
        case EXPR_ARRAY_TYPE: {
            Expr_Array_Type *array_type = (Expr_Array_Type *)type_expr;
            if (!is_type_array(parameter_type)) {
                report_error(parameter_location, "Expected an array type, got '%s'.", type_to_string(parameter_type));
                return false;
            }
            Type_Array *parameter_array = (Type_Array *)parameter_type;
            Operand number_operand(parameter_location);
            number_operand.type = type_int;
            number_operand.flags = OPERAND_CONSTANT | OPERAND_RVALUE;
            number_operand.int_value = parameter_array->count;
            if (!try_create_polymorph_value_declaration(array_type->count_expr, number_operand, block_to_insert_into, out_polymorphic_declarations)) {
                assert(array_type->count_expr->operand.type != nullptr);
                report_error(parameter_location, "Expected an array of size %d, got %d.", array_type->count_expr->operand.int_value, parameter_array->count);
                return false;
            }
            return try_create_polymorph_type_declarations(array_type->array_of, parameter_array->array_of, parameter_location, block_to_insert_into, out_polymorphic_declarations);
        }
        case EXPR_SPREAD: {
            Expr_Spread *spread = (Expr_Spread *)type_expr;
            return try_create_polymorph_type_declarations(spread->rhs, parameter_type, parameter_location, block_to_insert_into, out_polymorphic_declarations);
        }
        case EXPR_PROCEDURE_TYPE: {
            Expr_Procedure_Type *proc_type = (Expr_Procedure_Type *)type_expr;
            if (!is_type_procedure(parameter_type)) {
                report_error(parameter_location, "Expected a procedure type, got '%s'.", type_to_string(parameter_type));
                return false;
            }
            Type_Procedure *parameter_type_proc = (Type_Procedure *)parameter_type;
            if (proc_type->header->parameters.count != parameter_type_proc->parameter_types.count) {
                report_error(parameter_location, "Expected procedure type with %d parameters, got %d.", proc_type->header->parameters.count, parameter_type_proc->parameter_types.count);
                return false;
            }
            For (idx, parameter_type_proc->parameter_types) {
                Type *proc_param_type = parameter_type_proc->parameter_types[idx];
                Ast_Var *proc_param = proc_type->header->parameters[idx];
                if (!try_create_polymorph_type_declarations(proc_param->type_expr, proc_param_type, parameter_location, block_to_insert_into, out_polymorphic_declarations)) {
                    return false;
                }
            }
            if (proc_type->header->return_type_expr) {
                if (parameter_type_proc->return_type == nullptr) {
                    report_error(parameter_location, "Expected a procedure type with a return value.");
                    return false;
                }
                if (!try_create_polymorph_type_declarations(proc_type->header->return_type_expr, parameter_type_proc->return_type, parameter_location, block_to_insert_into, out_polymorphic_declarations)) {
                    return false;
                }
            }
            else {
                if (parameter_type_proc->return_type != nullptr) {
                    report_error(parameter_location, "Expected a procedure type without a return value.");
                    return false;
                }
            }
            break;
        }
    }
    return true;
}

bool try_create_polymorph_declarations(Ast_Var *var, Operand parameter_operand, Ast_Block *block_to_insert_into, Array<Declaration *> *out_polymorphic_declarations) {
    assert(parameter_operand.type != nullptr);
    if (!try_create_polymorph_type_declarations(var->type_expr, parameter_operand.type, parameter_operand.location, block_to_insert_into, out_polymorphic_declarations)) {
        return false;
    }
    if (!try_create_polymorph_value_declaration(var->name_expr, parameter_operand, block_to_insert_into, out_polymorphic_declarations)) {
        return false;
    }
    return true;
}

Ast_Node *polymorph_node(Ast_Node *node_to_polymorph, char *original_name, Array<Operand> parameters, Location polymorph_location, Array<int> *polymorphic_constants) {
    // deduplicate polymorphs
    {
        Array<Ast_Var *> *root_vars_to_polymorph = {};
        Array<Node_Polymorph> *polymorphs_list = {};
        switch (node_to_polymorph->ast_kind) {
            case AST_PROC: {
                Ast_Proc *proc = (Ast_Proc *)node_to_polymorph;
                root_vars_to_polymorph = &proc->header->parameters;
                polymorphs_list = &proc->polymorphs;
                break;
            }
            case AST_STRUCT: {
                Ast_Struct *structure = (Ast_Struct *)node_to_polymorph;
                root_vars_to_polymorph = &structure->polymorphic_parameters;
                polymorphs_list = &structure->polymorphs;
                break;
            }
            default: {
                assert(false);
            }
        }

        assert(root_vars_to_polymorph->count > 0);
        int num_mandatory_parameters = root_vars_to_polymorph->count;
        Ast_Var *last_parameter = (*root_vars_to_polymorph)[root_vars_to_polymorph->count-1];
        bool is_varargs = false;
        bool is_polymorphic_varargs = false;
        if (unparen_expr(last_parameter->type_expr)->expr_kind == EXPR_SPREAD) {
            is_varargs = true;
            if (last_parameter->is_polymorphic_type) {
                is_polymorphic_varargs = true;
            }
            else {
                num_mandatory_parameters -= 1;
            }
        }

        if (parameters.count != num_mandatory_parameters) {
            if (is_varargs) {
                if (parameters.count < num_mandatory_parameters) {
                    if (is_polymorphic_varargs) {
                        report_error(polymorph_location, "Expected at least %d parameter(s), got %d.", root_vars_to_polymorph->count, parameters.count);
                        report_info(polymorph_location, "The last parameter is polymorphic varargs which means you have to provide at least one vararg parameter for the compiler to figure out the type of the varargs.");
                        return false;
                    }
                    else {
                        report_error(polymorph_location, "Expected at least %d parameter(s), got %d.", root_vars_to_polymorph->count-1, parameters.count);
                        return false;
                    }
                }
            }
            else {
                report_error(polymorph_location, "Expected %d parameter(s), got %d.", root_vars_to_polymorph->count, parameters.count);
                return false;
            }
        }

        // deduplicate
        For (idx, *polymorphs_list) {
            Node_Polymorph polymorph = (*polymorphs_list)[idx];
            bool all_matched = true;
            For (value_idx, polymorph.polymorph_values) {
                Node_Polymorph_Parameter poly_parameter_value = polymorph.polymorph_values[value_idx];
                Operand param_operand = parameters[value_idx];
                Operand param_matched;
                assert(poly_parameter_value.type);
                if (!match_types(param_operand, poly_parameter_value.type, &param_matched, false)) {
                    all_matched = false;
                    break;
                }
                if (poly_parameter_value.is_polymorphic_value) {
                    polymorphic_constants->append(value_idx);
                    assert(param_matched.flags & OPERAND_CONSTANT);
                    assert(param_matched.type != nullptr);
                    Operand result;
                    bool ok = binary_eval(poly_parameter_value.value, param_matched, TK_EQUAL_TO, param_matched.location, &result);
                    assert(ok);
                    assert(result.type == type_bool);
                    assert(result.flags & OPERAND_CONSTANT);
                    if (result.bool_value == false) {
                        all_matched = false;
                        break;
                    }
                }
                else {
                    if (node_to_polymorph->ast_kind == AST_STRUCT) {
                        assert(poly_parameter_value.is_polymorphic_value);
                    }
                }
                if (poly_parameter_value.is_polymorphic_type) {
                    assert(param_matched.type != nullptr);
                    if (poly_parameter_value.type != param_matched.type) {
                        all_matched = false;
                        break;
                    }
                }
            }

            if (all_matched) {
                return polymorph.polymorphed_node;
            }

            polymorphic_constants->clear();
        }
    }

    // re-parse the node to be polymorphed
    Lexer lexer(node_to_polymorph->location.filepath, node_to_polymorph->location.text);
    lexer.location = node_to_polymorph->location;
    lexer.location.index = 0;
    lexer.current_block = node_to_polymorph->parent_block;
    lexer.allocator = g_global_linear_allocator;
    String_Builder sb = make_string_builder(g_global_linear_allocator, 128);
    sb.print(original_name);
    sb.printf("__polymorph_%d", total_num_polymorphs);
    total_num_polymorphs += 1;
    Ast_Node *new_parse = parse_single_statement(&lexer, true, sb.string());

    Ast_Block *block_to_insert_declarations_into = {};
    Array<Ast_Var *> *polymorph_vars = {};
    switch (new_parse->ast_kind) {
        case AST_PROC: {
            Ast_Proc *proc = (Ast_Proc *)new_parse;
            block_to_insert_declarations_into = proc->header->procedure_block;
            polymorph_vars = &proc->header->parameters;
            break;
        }
        case AST_STRUCT: {
            Ast_Struct *structure = (Ast_Struct *)new_parse;
            make_incomplete_type_for_struct(structure);
            block_to_insert_declarations_into = structure->type->declarations_block;
            polymorph_vars = &structure->polymorphic_parameters;
            break;
        }
        default: {
            assert(false);
        }
    }

    assert(block_to_insert_declarations_into != nullptr);
    assert(polymorph_vars != nullptr);
    // assert(parameters.count == polymorph_vars->count);

    // go through the parameters and create polymorphic declarations
    Array<Declaration *> polymorphic_declarations;
    polymorphic_declarations.allocator = g_global_linear_allocator;
    For (idx, *polymorph_vars) {
        Ast_Var *param_decl = (*polymorph_vars)[idx];
        Operand parameter_operand = parameters[idx];
        if (!try_create_polymorph_declarations(param_decl, parameter_operand, block_to_insert_declarations_into, &polymorphic_declarations)) {
            report_info(polymorph_location, "Error during polymorph triggered here.");
            return nullptr;
        }
    }

    // check the declarations
    For (idx, polymorphic_declarations) {
        Declaration *inserted_declaration = polymorphic_declarations[idx];
        if (!check_declaration(inserted_declaration, inserted_declaration->location)) {
            report_info(polymorph_location, "Error during polymorph triggered here.");
            return nullptr;
        }
        assert(inserted_declaration->check_state == DCS_CHECKED);
        assert(inserted_declaration->operand.type != nullptr);
    }

    // build the list of polymorph parameters for future deduplication
    Array<Node_Polymorph_Parameter> polymorph_values;
    polymorph_values.allocator = g_global_linear_allocator;
    For (idx, *polymorph_vars) {
        Ast_Var *var = (*polymorph_vars)[idx];
        Operand param = parameters[idx];

        if (var->is_polymorphic_value) {
            polymorphic_constants->append(idx);
        }

        Node_Polymorph_Parameter poly_value = {};
        poly_value.type = param.type;

        if (var->is_polymorphic_value) {
            assert(param.flags & OPERAND_CONSTANT);
            poly_value.is_polymorphic_value = true;
            poly_value.value = param;
        }
        if (var->is_polymorphic_type) {
            poly_value.is_polymorphic_type = true;
        }

        polymorph_values.append(poly_value);
    }

    // remove constant-value-polys from parameter list
    for (int i = polymorphic_constants->count-1; i >= 0; i--) {
        polymorph_vars->ordered_remove((*polymorphic_constants)[i]);
    }

    Node_Polymorph new_poly_node = {};
    new_poly_node.polymorphed_node = new_parse;
    new_poly_node.polymorph_values = polymorph_values;
    node_to_polymorph->polymorphs.append(new_poly_node);

    switch (new_parse->ast_kind) {
        case AST_STRUCT: {
            Ast_Struct *structure_polymorph = (Ast_Struct *)new_parse;
            structure_polymorph->declaration->is_polymorphic = false;
            assert(structure_polymorph->type != nullptr);
            structure_polymorph->type->polymorphic_parameter_values = parameters;
            structure_polymorph->type->is_polymorph_of = (Ast_Struct *)node_to_polymorph;
            break;
        }
        case AST_PROC: {
            Ast_Proc *procedure_polymorph = (Ast_Proc *)new_parse;
            procedure_polymorph->header->declaration->is_polymorphic = false;
            if (!check_declaration(procedure_polymorph->header->declaration, polymorph_location)) {
                report_info(polymorph_location, "Error during polymorph triggered here.");
                return nullptr;
            }
            break;
        }
        default: {
            assert(false);
        }
    }

    return new_parse;
}

Ast_Proc *polymorph_procedure(Ast_Proc *proc_to_polymorph, Location polymorph_location, Array<Ast_Expr *> parameters, Array<int> *polymorphic_constants) {
    // todo(josh): compress this out with the one in polymorph_struct()
    Array<Operand> parameter_operands = {};
    parameter_operands.allocator = g_global_linear_allocator;
    For (idx, parameters) {
        Ast_Expr *parameter = parameters[idx];
        Operand *param_operand = typecheck_expr(parameter);
        if (!param_operand) {
            return nullptr;
        }
        parameter_operands.append(*param_operand);
    }

    Ast_Node *procedure_polymorph_node = polymorph_node(proc_to_polymorph, proc_to_polymorph->header->name, parameter_operands, polymorph_location, polymorphic_constants);
    if (!procedure_polymorph_node) {
        return nullptr;
    }
    assert(procedure_polymorph_node->ast_kind == AST_PROC);
    Ast_Proc *procedure_polymorph = (Ast_Proc *)procedure_polymorph_node;
    return procedure_polymorph;
}

Ast_Struct *polymorph_struct(Ast_Struct *structure, Location polymorph_location, Array<Ast_Expr *> parameters) {
    // todo(josh): compress this out with the one in polymorph_procedure()
    Array<Operand> parameter_operands = {};
    parameter_operands.allocator = g_global_linear_allocator;
    For (idx, parameters) {
        Ast_Expr *parameter = parameters[idx];
        Operand *param_operand = typecheck_expr(parameter);
        if (!(param_operand->flags & OPERAND_CONSTANT)) {
            report_error(parameter->location, "Value must be constant for struct polymorph.");
            return nullptr;
        }
        parameter_operands.append(*param_operand);
    }

    if (parameters.count != structure->polymorphic_parameters.count) {
        report_error(polymorph_location, "Expected %d parmeter(s) for polymorphic struct '%s', got %d.", structure->polymorphic_parameters.count, structure->name, parameters.count);
        return nullptr;
    }

    Array<int> parameter_indices_to_remove;
    parameter_indices_to_remove.allocator = g_global_linear_allocator;
    Ast_Node *struct_polymorph_node = polymorph_node(structure, structure->name, parameter_operands, polymorph_location, &parameter_indices_to_remove);
    if (!struct_polymorph_node) {
        return nullptr;
    }
    assert(struct_polymorph_node->ast_kind == AST_STRUCT);
    Ast_Struct *structure_polymorph = (Ast_Struct *)struct_polymorph_node;
    return structure_polymorph;
}

bool typecheck_procedure_call(Ast_Expr *expr, Operand procedure_operand, Array<Ast_Expr *> parameters, Operand *out_operand) {
    // copy all the exprs into params_to_emit
    Array<Ast_Expr *> params_to_emit = {};
    params_to_emit.allocator = g_global_linear_allocator;
    For (idx, parameters) {
        params_to_emit.append(parameters[idx]);
    }

    Type_Procedure *target_procedure_type = nullptr;
    if (is_type_polymorphic(procedure_operand.type)) {
        assert(procedure_operand.referenced_declaration != nullptr);
        assert(procedure_operand.referenced_declaration->kind == DECL_PROC);
        Ast_Proc *referenced_procedure = ((Proc_Declaration *)procedure_operand.referenced_declaration)->header->procedure;

        Array<int> parameter_indices_to_remove;
        parameter_indices_to_remove.allocator = g_global_linear_allocator;
        Ast_Proc *procedure_polymorph = polymorph_procedure(referenced_procedure, expr->location, parameters, &parameter_indices_to_remove);
        if (procedure_polymorph == nullptr) {
            return false;
        }
        for (int i = parameter_indices_to_remove.count-1; i >= 0; i--) {
            params_to_emit.ordered_remove(parameter_indices_to_remove[i]);
        }

        assert(!is_type_polymorphic(procedure_polymorph->header->declaration->operand.type));
        procedure_operand = procedure_polymorph->header->declaration->operand;
        assert(is_type_procedure(procedure_operand.type));
    }

    target_procedure_type = (Type_Procedure *)procedure_operand.type;
    assert(is_type_procedure(target_procedure_type));
    assert(!is_type_polymorphic(target_procedure_type));

    bool proc_has_varargs = false;
    if (target_procedure_type->parameter_types.count > 0 && is_type_varargs(target_procedure_type->parameter_types[target_procedure_type->parameter_types.count-1])) {
        proc_has_varargs = true;
    }

    if (!proc_has_varargs) {
        if (params_to_emit.count < target_procedure_type->parameter_types.count) {
            report_error(expr->location, "Not enough parameters in procedure call. Expected %d, got %d.", target_procedure_type->parameter_types.count, params_to_emit.count);
            return false;
        }

        if (params_to_emit.count > target_procedure_type->parameter_types.count) {
            report_error(expr->location, "Too many parameters in procedure call. Expected %d, got %d.", target_procedure_type->parameter_types.count, params_to_emit.count);
            return false;
        }
    }
    else {
        if (params_to_emit.count < (target_procedure_type->parameter_types.count-1)) {
            report_error(expr->location, "Not enough parameters in procedure call. Expected at least %d, got %d.", target_procedure_type->parameter_types.count-1, params_to_emit.count);
            return false;
        }
    }

    // typecheck parameters now that we have concrete types for the procedure parameters
    Array<Procedure_Call_Parameter> processed_procedure_call_parameters = {};
    processed_procedure_call_parameters.allocator = g_global_linear_allocator;
    int type_idx = 0;
    int param_idx = 0;
    for (; param_idx < params_to_emit.count; param_idx += 1) {
        Type *target_type = target_procedure_type->parameter_types[type_idx];
        if (is_type_varargs(target_type)) {
            assert(type_idx == target_procedure_type->parameter_types.count-1);
            break;
        }
        Ast_Expr *parameter = params_to_emit[param_idx];
        Operand *parameter_operand = typecheck_expr(parameter, target_type);
        if (!parameter_operand) {
            return false;
        }
        Procedure_Call_Parameter call_parameter = {};
        call_parameter.exprs.allocator = g_global_linear_allocator;
        call_parameter.exprs.append(parameter);
        processed_procedure_call_parameters.append(call_parameter);
        type_idx += 1;
    }

    if (proc_has_varargs) {
        assert(type_idx < target_procedure_type->parameter_types.count);
        assert(is_type_varargs(target_procedure_type->parameter_types[type_idx]));
        Type_Varargs *vararg_type = (Type_Varargs *)target_procedure_type->parameter_types[type_idx];
        Procedure_Call_Parameter vararg_parameter = {};
        vararg_parameter.exprs.allocator = g_global_linear_allocator;
        for (; param_idx < params_to_emit.count; param_idx += 1) {
            Ast_Expr *parameter = params_to_emit[param_idx];
            Type *target_type = vararg_type->varargs_of;
            bool parameter_is_spread = unparen_expr(parameter)->expr_kind == EXPR_SPREAD;
            if (parameter_is_spread) {
                if (vararg_parameter.exprs.count > 0) {
                    report_error(parameter->location, "Cannot mix normal varargs with a spread operation.");
                    return false;
                }
                if (vararg_type->is_c_varargs) {
                    report_error(parameter->location, "Cannot spread into #c_varargs.");
                    return false;
                }
                target_type = vararg_type;
            }
            Operand *parameter_operand = typecheck_expr(parameter, target_type);
            if (!parameter_operand) {
                return false;
            }
            vararg_parameter.exprs.append(parameter);
        }
        processed_procedure_call_parameters.append(vararg_parameter);
        type_idx += 1;
    }

    assert(type_idx == target_procedure_type->parameter_types.count);
    assert(param_idx == params_to_emit.count);
    assert(processed_procedure_call_parameters.count == target_procedure_type->parameter_types.count);

    if (procedure_operand.referenced_declaration != nullptr) {
        if (procedure_operand.referenced_declaration->kind == DECL_PROC) { // check that it's not a function pointer
            Ast_Proc *referenced_procedure = ((Proc_Declaration *)procedure_operand.referenced_declaration)->header->procedure;
            expr->desugared_procedure_to_call = referenced_procedure;
        }
    }
    expr->processed_procedure_call_parameters = processed_procedure_call_parameters;

    if (expr->expr_kind == EXPR_PROCEDURE_CALL) {
        Expr_Procedure_Call *call = (Expr_Procedure_Call *)expr;
        call->target_procedure_type = target_procedure_type;
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
            if (!match_types(expr->operand, proc->header->parameters[expr_idx]->type, &operand_copy_for_types_match, false)) {
                all_matched = false;
                break;
            }
        }
        if (all_matched) {
            // actually match the types
            For (expr_idx, exprs) {
                Ast_Expr *expr = exprs[expr_idx];
                if (!match_types(expr->operand, proc->header->parameters[expr_idx]->type, &expr->operand, false)) {
                    assert(false);
                }
            }
            return proc;
        }
    }
    return nullptr;
}

bool try_resolve_operator_overload(Ast_Struct *structure, Ast_Expr *root_expr, Ast_Expr *lhs, Ast_Expr *rhs, Token_Kind op, Operand *out_result_operand) {
    assert(lhs->operand.type != nullptr);
    assert(rhs->operand.type != nullptr);
    Array<Ast_Expr *> parameters = {};
    parameters.allocator = g_global_linear_allocator;
    parameters.append(lhs);
    parameters.append(rhs);
    Ast_Proc *overload_proc = find_operator_overload(structure, parameters, op);
    if (overload_proc == nullptr) {
        if (op == TK_LEFT_SQUARE) {
            report_error(root_expr->location, "Cannot subscript type '%s'.", type_to_string(lhs->operand.type));
        }
        else {
            report_error(root_expr->location, "Operator '%s' is unsupported for types '%s' and '%s'.", token_string(op), type_to_string(lhs->operand.type), type_to_string(rhs->operand.type));
        }
        return false;
    }
    else {
        assert(overload_proc->header->operand.type != nullptr);
        assert(overload_proc->header->operand.referenced_declaration != nullptr);
        if (!typecheck_procedure_call(root_expr, overload_proc->header->operand, parameters, out_result_operand)) {
            return false;
        }
    }
    return true;
}

// return value is false if there was a compile error, true otherwise.
// this means that this procedure can return true even if it doesn't resolve the declaration, so be sure to check out_resolved_declaration for null
bool try_resolve_identifier(const char *name, Ast_Block *start_block, Operand *out_result_operand, Location usage_location, bool climb_up_blocks, Declaration **out_resolved_declaration) {
    Declaration *resolved_declaration = nullptr;
    while (start_block != nullptr) {
        Declaration **existing_declaration = start_block->declarations_lookup.get(name);
        if (existing_declaration) {
            resolved_declaration = *existing_declaration;
            break;
        }
        if (climb_up_blocks) {
            start_block = start_block->parent_block;
        }
        else {
            break;
        }
    }
    if (!resolved_declaration) {
        return true;
    }
    assert(resolved_declaration != nullptr);
    if (resolved_declaration->is_polymorphic) {
        out_result_operand->type = type_polymorphic;
    }
    else {
        if (!check_declaration(resolved_declaration, usage_location, out_result_operand)) {
            return false;
        }
    }
    out_result_operand->referenced_declaration = resolved_declaration;
    out_result_operand->location = usage_location;
    *out_resolved_declaration = resolved_declaration;
    return true;
}

bool do_selector_lookup(Ast_Expr *lhs, char *field_name, Selector_Expression_Lookup_Result *out_result, Location field_location) {
    Operand *lhs_operand = typecheck_expr(lhs);
    if (!lhs_operand) {
        return false;
    }
    if (!complete_type(lhs_operand->type, lhs->location, TCF_USINGS)) {
        return false;
    }

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
    }
    assert(type_with_fields != nullptr);
    if (!complete_type(type_with_fields, lhs->location, TCF_USINGS)) {
        return false;
    }

    out_result->type_with_field = type_with_fields;
    Declaration *resolved_declaration = nullptr;
    if (!try_resolve_identifier(field_name, type_with_fields->declarations_block, &out_result->operand, field_location, false, &resolved_declaration)) {
        return false;
    }
    if (!resolved_declaration) {
        report_error(field_location, "Type '%s' doesn't have field '%s'.", type_to_string(type_with_fields), field_name);
        return false;
    }
    assert(resolved_declaration);
    out_result->declaration = resolved_declaration;
    return true;
}

Type_Tuple *typecheck_expr_list(Ast_Expr_List *list, Array<Operand *> *out_operands) {
    assert(list->exprs.count > 0);
    Array<Operand *> operands = {};
    operands.allocator = g_global_linear_allocator;
    Array<Type *> types = {};
    types.allocator = g_global_linear_allocator;
    For (idx, list->exprs) {
        Operand *operand = typecheck_expr(list->exprs[idx]);
        if (operand == nullptr) {
            return nullptr;
        }
        operands.append(operand);
        types.append(operand->type);
    }
    *out_operands = operands;
    return SIF_NEW_CLONE(Type_Tuple(types), g_global_linear_allocator);
}

bool typecheck_expr_list_against_type_tuple(Ast_Expr_List *list, Type_Tuple *tuple_type, Array<Operand *> *out_operands, Token_Kind op = TK_ASSIGN) {
    assert(list->exprs.count > 0);
    Array<Operand *> operands = {};
    operands.allocator = g_global_linear_allocator;
    assert(list->exprs.count == tuple_type->types.count);
    For (idx, list->exprs) {
        Type *expected_lhs_type = tuple_type->types[idx];
        Ast_Expr *expr = list->exprs[idx];
        // if (is_type_reference(expected_lhs_type)) {
        //     Type_Reference *type_reference = (Type_Reference *)expected_lhs_type;
        //     expected_lhs_type = type_reference->reference_to;
        // }
        assert(expected_lhs_type != nullptr);
        Operand *operand = typecheck_expr(expr, expected_lhs_type);
        if (operand == nullptr) {
            return false;
        }
        operands.append(operand);

        assert(operand->flags & OPERAND_RVALUE);
        if (op != TK_ASSIGN) {
            if (!operator_is_defined(expected_lhs_type, operand->type, op, false)) {
                report_error(expr->location, "Operator '%s' is not defined for types '%s and '%s'.", token_string(op), type_to_string(expected_lhs_type), type_to_string(operand->type));
                return false;
            }
        }
    }
    *out_operands = operands;
    return true;
}

bool typecheck_expr_array_type_with_known_count(Expr_Array_Type *expr_array, int count, Operand *out_operand) {
    assert(expr_array->array_of != nullptr);
    Operand *array_of_operand = typecheck_expr(expr_array->array_of, type_typeid);
    if (!array_of_operand) {
        return false;
    }
    assert((array_of_operand->flags & OPERAND_CONSTANT) && (array_of_operand->flags & OPERAND_TYPE));
    out_operand->flags = OPERAND_CONSTANT | OPERAND_TYPE | OPERAND_RVALUE;
    out_operand->type = type_typeid;
    out_operand->type_value = get_or_create_type_array_of(array_of_operand->type_value, count);
    return true;
}

Operand *typecheck_expr(Ast_Expr *expr, Type *expected_type, bool require_value) {
    assert(expr != nullptr);

    if (expr->operand.type == nullptr) {
        Operand result_operand(expr->location);
        switch (expr->expr_kind) {
            case EXPR_UNARY: {
                // todo(josh): @ErrorMessage
                Expr_Unary *unary = (Expr_Unary *)expr;
                Operand *rhs_operand = typecheck_expr(unary->rhs, expected_type);
                if (rhs_operand == nullptr) {
                    return nullptr;
                }
                switch (unary->op) {
                    case TK_PLUS: {
                        if (!is_type_number(rhs_operand->type)) {
                            report_error(rhs_operand->location, "Unary plus requires a numeric type.");
                            return nullptr;
                        }
                    }
                    case TK_MINUS: {
                        if (!is_type_number(rhs_operand->type)) {
                            report_error(rhs_operand->location, "Unary minus requires a numeric type.");
                            return nullptr;
                        }
                        if (rhs_operand->flags & OPERAND_CONSTANT) {
                            result_operand.int_value = -rhs_operand->int_value;
                            result_operand.f32_value = -rhs_operand->f32_value;
                            result_operand.f64_value = -rhs_operand->f64_value;
                        }
                        break;
                    }
                    case TK_TILDE: {
                        if (!is_type_integer(rhs_operand->type)) {
                            report_error(rhs_operand->location, "Unary bitwise NOT requires an integer type.");
                            return nullptr;
                        }
                        if (rhs_operand->flags & OPERAND_CONSTANT) {
                            result_operand.uint_value = ~rhs_operand->int_value;
                            result_operand.int_value = ~rhs_operand->int_value;
                        }
                        break;
                    }
                    case TK_NOT: {
                        if (!is_type_bool(rhs_operand->type)) {
                            report_error(rhs_operand->location, "Unary ! requires a bool.");
                            return nullptr;
                        }
                        if (rhs_operand->flags & OPERAND_CONSTANT) {
                            result_operand.bool_value = !rhs_operand->bool_value;
                        }
                        break;
                    }
                    default: {
                        assert(false);
                    }
                }

                result_operand.flags = rhs_operand->flags;
                result_operand.flags &= ~OPERAND_LVALUE;
                result_operand.type = rhs_operand->type;
                break;
            }
            case EXPR_BINARY: {
                Expr_Binary *binary = (Expr_Binary *)expr;
                if (is_cmp_op(binary->op)) {
                    expected_type = nullptr; // todo(josh): this looks sketchy because it will affect the typechecking at the bottom of this procedure, maybe pull expected_type into a local variable?
                }
                Operand *lhs_operand = nullptr;
                Operand *rhs_operand = nullptr;
                Type *implicit_enum_selector_type = nullptr;
                if (unparen_expr(binary->lhs)->expr_kind == EXPR_IMPLICIT_ENUM_SELECTOR) {
                    rhs_operand = typecheck_expr(binary->rhs);
                    if (!rhs_operand) {
                        return nullptr;
                    }
                    if (!is_type_enum(rhs_operand->type)) {
                        report_error(binary->rhs->location, "Expected enum type, got '%s'.", type_to_string(rhs_operand->type));
                        return nullptr;
                    }
                    implicit_enum_selector_type = rhs_operand->type;
                }
                else if (unparen_expr(binary->rhs)->expr_kind == EXPR_IMPLICIT_ENUM_SELECTOR) {
                    lhs_operand = typecheck_expr(binary->lhs);
                    if (!lhs_operand) {
                        return nullptr;
                    }
                    if (!is_type_enum(lhs_operand->type)) {
                        report_error(binary->lhs->location, "Expected enum type, got '%s'.", type_to_string(lhs_operand->type));
                        return nullptr;
                    }
                    implicit_enum_selector_type = lhs_operand->type;
                }
                if (lhs_operand == nullptr) {
                    lhs_operand = typecheck_expr(binary->lhs, implicit_enum_selector_type);
                    if (!lhs_operand) {
                        return nullptr;
                    }
                }
                if (rhs_operand == nullptr) {
                    rhs_operand = typecheck_expr(binary->rhs, implicit_enum_selector_type);
                    if (!rhs_operand) {
                        return nullptr;
                    }
                }
                Type *most_concrete = get_most_concrete_type(lhs_operand->type, rhs_operand->type);
                if (expected_type != nullptr) {
                    most_concrete = get_most_concrete_type(most_concrete, expected_type);
                }

                // speculatively try to match the types to see if we should go into the
                // normal binop path or the operator overload path
                Operand lhs_operand_speculative = {};
                Operand rhs_operand_speculative = {};
                bool types_matched = match_types(*lhs_operand, most_concrete, &lhs_operand_speculative, false);
                types_matched = types_matched && match_types(*rhs_operand, most_concrete, &rhs_operand_speculative, false);
                if (types_matched && operator_is_defined(lhs_operand_speculative.type, rhs_operand_speculative.type, binary->op, (lhs_operand_speculative.flags & OPERAND_CONSTANT) && rhs_operand_speculative.flags & OPERAND_CONSTANT)) {
                    Operand *lhs_operand = typecheck_expr(binary->lhs, most_concrete);
                    if (!lhs_operand) {
                        return nullptr;
                    }
                    Operand *rhs_operand = typecheck_expr(binary->rhs, most_concrete);
                    if (!rhs_operand) {
                        return nullptr;
                    }

                    if (!match_types(*lhs_operand, most_concrete, lhs_operand)) { assert(false); }
                    if (!match_types(*rhs_operand, most_concrete, rhs_operand)) { assert(false); }
                    bool ok = binary_eval(*lhs_operand, *rhs_operand, binary->op, binary->location, &result_operand);
                    if (!ok) {
                        return nullptr;
                    }
                }
                else {
                    if (is_type_struct(lhs_operand->type)) {
                        Type_Struct *struct_type = (Type_Struct *)lhs_operand->type;
                        if (!try_resolve_operator_overload(struct_type->ast_struct, binary, binary->lhs, binary->rhs, binary->op, &result_operand)) {
                            return nullptr;
                        }
                    }
                    else {
                        report_error(binary->location, "Operator '%s' is unsupported for types '%s' and '%s'.", token_string(binary->op), type_to_string(lhs_operand_speculative.type), type_to_string(rhs_operand_speculative.type));
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
                if (!is_type_typeid(expr_cast->type_expr->operand.type)) {
                    report_error(expr_cast->type_expr->location, "Cast target must be a type.");
                    return nullptr;
                }
                Operand *rhs_operand = typecheck_expr(expr_cast->rhs);
                if (!rhs_operand) {
                    return nullptr;
                }
                if (!can_cast(expr_cast->rhs, expr_cast->type_expr->operand.type_value)) {
                    report_error(expr_cast->location, "Cannot cast from %s to %s.", type_to_string(rhs_operand->type), type_to_string(type_operand->type_value));
                    return nullptr;
                }
                result_operand.type = type_operand->type_value;
                result_operand.flags = OPERAND_RVALUE;
                if (rhs_operand->flags & OPERAND_CONSTANT) {
                    result_operand.flags |= OPERAND_CONSTANT;
                    result_operand.uint_value            = rhs_operand->uint_value;
                    result_operand.int_value             = rhs_operand->int_value;
                    result_operand.f32_value             = rhs_operand->f32_value;
                    result_operand.f64_value             = rhs_operand->f64_value;
                    result_operand.bool_value            = rhs_operand->bool_value;
                    result_operand.type_value            = rhs_operand->type_value;
                    result_operand.scanned_string_value  = rhs_operand->scanned_string_value;
                    result_operand.scanned_string_length = rhs_operand->scanned_string_length;
                    result_operand.escaped_string_value  = rhs_operand->escaped_string_value;
                    result_operand.escaped_string_length = rhs_operand->escaped_string_length;
                }
                break;
            }
            case EXPR_TRANSMUTE: {
                Expr_Transmute *transmute = (Expr_Transmute *)expr;
                Operand *type_operand = typecheck_expr(transmute->type_expr);
                if (!type_operand) {
                    return nullptr;
                }
                if (!is_type_typeid(transmute->type_expr->operand.type)) {
                    report_error(transmute->type_expr->location, "Cast target must be a type.");
                    return nullptr;
                }
                Operand *rhs_operand = typecheck_expr(transmute->rhs);
                if (!rhs_operand) {
                    return nullptr;
                }
                if (!complete_type(type_operand->type_value, transmute->location, TCF_SIZE)) {
                    return nullptr;
                }
                rhs_operand->type = try_concretize_type_without_context(rhs_operand->type);
                if (rhs_operand->type == nullptr) {
                    assert(rhs_operand->type == type_untyped_null);
                    report_error(transmute->rhs->location, "Cannot transmute untyped null.");
                    return nullptr;
                }
                if (!complete_type(rhs_operand->type, rhs_operand->location, TCF_SIZE)) {
                    return nullptr;
                }
                assert(type_operand->type_value->size > 0);
                assert(rhs_operand->type->size > 0);
                if (rhs_operand->type->size != type_operand->type_value->size) {
                    report_error(transmute->location, "Transmute requires both types to be the same size.");
                    report_info(transmute->location, "Size of target type '%s': %d, size of given expression type '%s': %d.", type_to_string(type_operand->type_value), type_operand->type_value->size, type_to_string(rhs_operand->type), rhs_operand->type->size);
                    return nullptr;
                }
                result_operand.type = type_operand->type_value;
                result_operand.flags = OPERAND_RVALUE;
                break;
            }
            case EXPR_ADDRESS_OF: {
                Expr_Address_Of *address_of = (Expr_Address_Of *)expr;
                Operand *rhs_operand = typecheck_expr(address_of->rhs);
                if (!rhs_operand) {
                    return nullptr;
                }
                if (!(rhs_operand->flags & OPERAND_LVALUE)) {
                    report_error(address_of->location, "Cannot take the address of a non-lvalue.");
                    return nullptr;
                }
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
                if (!complete_type(lhs_operand->type, expr->location)) {
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
                    elem_type = array_type->array_of;
                    result_operand.type = elem_type;
                    result_operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;

                    if (index_operand->flags & OPERAND_CONSTANT) {
                        if ((index_operand->int_value < 0) || (index_operand->int_value >= array_type->count)) {
                            report_error(subscript->index->location, "Index %d out of bounds 0..<%d.", index_operand->int_value, array_type->count);
                            return nullptr;
                        }
                    }
                }
                else if (is_type_slice(lhs_operand->type)) {
                    Type_Slice *slice_type = (Type_Slice *)lhs_operand->type;
                    elem_type = slice_type->slice_of;
                    result_operand.type = elem_type;
                    result_operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;
                }
                else if (is_type_string(lhs_operand->type)) {
                    elem_type = type_u8;
                    result_operand.type = elem_type;
                    result_operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;
                }
                else if (is_type_varargs(lhs_operand->type)) {
                    Type_Varargs *varargs_type = (Type_Varargs *)lhs_operand->type;
                    elem_type = varargs_type->varargs_of;
                    result_operand.type = elem_type;
                    result_operand.flags = OPERAND_LVALUE | OPERAND_RVALUE;
                }
                else if (is_type_struct(lhs_operand->type)) {
                    Type_Struct *struct_type = (Type_Struct *)lhs_operand->type;
                    if (!try_resolve_operator_overload(struct_type->ast_struct, subscript, subscript->lhs, subscript->index, TK_LEFT_SQUARE, &result_operand)) {
                        return nullptr;
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
                Selector_Expression_Lookup_Result lookup;
                if (!do_selector_lookup(selector->lhs, selector->field_name, &lookup, selector->field_location)) {
                    return nullptr;
                }
                selector->lookup = lookup;
                result_operand = lookup.operand;
                break;
            }
            case EXPR_DEREFERENCE: {
                Expr_Dereference *dereference = (Expr_Dereference *)expr;
                Operand *lhs_operand = typecheck_expr(dereference->lhs);
                if (!lhs_operand) {
                    return nullptr;
                }
                if (!is_type_pointer(lhs_operand->type)) {
                    report_error(dereference->location, "You cannot dereference a non-pointer type.");
                    return nullptr;
                }
                Type_Pointer *pointer_type = (Type_Pointer *)lhs_operand->type;
                assert(pointer_type->pointer_to);
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

                if (!typecheck_procedure_call(call, *procedure_operand, call->parameters, &result_operand)) {
                    return nullptr;
                }
                assert(!is_type_polymorphic(call->target_procedure_type));
                assert(is_type_procedure(call->target_procedure_type));
                break;
            }
            case EXPR_IDENTIFIER: {
                Expr_Identifier *ident = (Expr_Identifier *)expr;
                Ast_Block *block = ident->parent_block;
                if (!try_resolve_identifier(ident->name, block, &result_operand, ident->location, true, &ident->resolved_declaration)) {
                    return nullptr;
                }
                if (!ident->resolved_declaration) {
                    report_error(ident->location, "Unresolved identifier '%s'.", ident->name);
                    return nullptr;
                }
                break;
            }
            case EXPR_IMPLICIT_ENUM_SELECTOR: {
                Expr_Implicit_Enum_Selector *selector = (Expr_Implicit_Enum_Selector *)expr;
                if (expected_type == nullptr) {
                    report_error(selector->location, "Unable to infer type for implicit enum selector. Please specify the enum explicitly.");
                    return nullptr;
                }
                if (!is_type_enum(expected_type)) {
                    report_error(selector->location, "Implicit enum selector expression provided for a non-enum type '%s'.", type_to_string(expected_type));
                    return nullptr;
                }
                Type_Enum *enum_type = (Type_Enum *)expected_type;
                Declaration *declaration = {};
                if (!try_resolve_identifier(selector->field, enum_type->declarations_block, &result_operand, selector->location, false, &declaration)) {
                    return nullptr;
                }
                if (declaration == nullptr) {
                    report_error(selector->location, "Unable to find enum value '%s' in enum '%s'.", selector->field, type_to_string(expected_type));
                    return nullptr;
                }
                assert(result_operand.type != nullptr);
                assert(is_type_enum(result_operand.type));
                break;
            }
            case EXPR_COMPOUND_LITERAL: {
                Expr_Compound_Literal *compound_literal = (Expr_Compound_Literal *)expr;
                Type *target_type = expected_type;
                if (compound_literal->type_expr == nullptr) {
                    if (expected_type == nullptr) {
                        report_error(compound_literal->location, "Unable to infer compound literal type from context. Please provide an explicit type.");
                        return nullptr;
                    }
                }
                else {
                    Ast_Expr *type_expr = unparen_expr(compound_literal->type_expr);
                    Operand *type_operand = nullptr;
                    if (type_expr->expr_kind == EXPR_ARRAY_TYPE) {
                        Expr_Array_Type *array_type_expr = (Expr_Array_Type *)type_expr;
                        if (unparen_expr(array_type_expr->count_expr)->expr_kind == EXPR_UNKNOWN_ARRAY_COUNT) {
                            Operand type_operand_value = {};
                            if (!typecheck_expr_array_type_with_known_count(array_type_expr, compound_literal->exprs.count, &type_operand_value)) {
                                return false;
                            }
                            assert(is_type_typeid(type_operand_value.type));
                            assert(type_operand_value.type_value != nullptr);
                            target_type = type_operand_value.type_value;
                            type_operand = &type_operand_value;
                        }
                    }
                    if (type_operand == nullptr) {
                        type_operand = typecheck_expr(compound_literal->type_expr, type_typeid);
                        if (!type_operand) {
                            return nullptr;
                        }
                    }
                    assert(is_type_typeid(type_operand->type));
                    assert(type_operand->type_value != nullptr);
                    target_type = type_operand->type_value;
                }
                assert(target_type != nullptr);
                if (!complete_type(target_type, expr->location)) {
                    return nullptr;
                }

                if (is_type_array(target_type)) {
                    // note(josh): copy paste from below
                    Type_Array *array_type = (Type_Array *)target_type;
                    assert(array_type->count > 0);
                    if (compound_literal->exprs.count != 0) {
                        if ((compound_literal->exprs.count != array_type->count) && !compound_literal->is_partial) {
                            report_error(compound_literal->location, "Expected %d expressions for compound literal, got %d.", array_type->count, compound_literal->exprs.count);
                            if (compound_literal->exprs.count < array_type->count) {
                                report_info(compound_literal->location, "Use #partial if you don't want to specify all values.");
                            }
                            return nullptr;
                        }
                        Type *element_type = array_type->array_of;
                        For (idx, compound_literal->exprs) {
                            Ast_Expr *expr = compound_literal->exprs[idx];
                            Operand *expr_operand = typecheck_expr(expr, element_type);
                            if (!expr_operand) {
                                return nullptr;
                            }
                        }
                    }
                }
                else {
                    // note(josh): copy paste from above
                    if (compound_literal->exprs.count != 0) {
                        if ((compound_literal->exprs.count != target_type->variable_fields.count) && !compound_literal->is_partial) {
                            report_error(compound_literal->location, "Expected %d expressions for compound literal, got %d.", target_type->variable_fields.count, compound_literal->exprs.count);
                            if (compound_literal->exprs.count < target_type->variable_fields.count) {
                                report_info(compound_literal->location, "Use #partial if you don't want to specify all values.");
                            }
                            return nullptr;
                        }
                        For (idx, compound_literal->exprs) {
                            Ast_Expr *expr = compound_literal->exprs[idx];
                            Type *element_type = target_type->variable_fields[idx].operand.type;
                            Operand *expr_operand = typecheck_expr(expr, element_type);
                            if (!expr_operand) {
                                return nullptr;
                            }
                        }
                    }
                }

                result_operand.type = target_type;
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
                result_operand.uint_value  = number->uint_value;
                result_operand.int_value   = number->int_value;
                result_operand.f32_value = number->f32_value;
                result_operand.f64_value = number->f64_value;
                break;
            }
            case EXPR_STRING_LITERAL: {
                Expr_String_Literal *string_literal = (Expr_String_Literal *)expr;
                result_operand.flags = OPERAND_CONSTANT | OPERAND_RVALUE;
                result_operand.type = type_untyped_string;
                result_operand.scanned_string_value = string_literal->text;
                result_operand.escaped_string_value = string_literal->escaped_text;
                result_operand.scanned_string_length = string_literal->scanner_length;
                result_operand.escaped_string_length = string_literal->escaped_length;
                break;
            }
            case EXPR_CHAR_LITERAL: {
                Expr_Char_Literal *char_literal = (Expr_Char_Literal *)expr;
                result_operand.flags = OPERAND_CONSTANT | OPERAND_RVALUE;
                result_operand.type = type_untyped_integer;
                result_operand.uint_value = char_literal->c;
                result_operand.int_value  = char_literal->c;
                result_operand.f32_value  = char_literal->c;
                result_operand.f64_value  = char_literal->c;
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
                if (!is_type_typeid(expr_operand->type)) {
                    report_error(expr_operand->location, "Expected a type for sizeof(). Maybe try sizeof(typeof(var))?");
                    return nullptr;
                }
                assert(expr_operand->type_value != nullptr);
                if (!complete_type(expr_operand->type_value, expr->location, TCF_SIZE)) {
                    return nullptr;
                }
                assert(expr_operand->type_value->size > 0);
                result_operand.type = type_untyped_integer;
                result_operand.uint_value = expr_operand->type_value->size;
                result_operand.int_value  = expr_operand->type_value->size;
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
                if (is_type_untyped(expr_operand->type)) {
                    report_error(expr->location, "Cannot use 'typeof' on an untyped declaration.");
                    return nullptr;
                }
                result_operand.type = type_typeid;
                result_operand.type_value = expr_operand->type;
                result_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE | OPERAND_RVALUE;
                break;
            }
            case EXPR_TYPEOFELEMENT: {
                Expr_Typeofelement *typeof_element = (Expr_Typeofelement *)expr;
                Operand *expr_operand = typecheck_expr(typeof_element->expr);
                if (!expr_operand) {
                    return nullptr;
                }
                assert(expr_operand->type != nullptr);
                if (is_type_untyped(expr_operand->type)) {
                    report_error(expr->location, "Cannot use 'typeofelement' on an untyped declaration.");
                    return nullptr;
                }
                if (!is_type_typeid(expr_operand->type)) {
                    report_error(expr_operand->location, "Expected a type for typeofelement(). Maybe try typeofelement(typeof(var))?");
                    return nullptr;
                }
                Type *element_type = nullptr;
                if (is_type_array(expr_operand->type_value)) {
                    Type_Array *array = (Type_Array *)expr_operand->type_value;
                    element_type = array->array_of;
                }
                else if (is_type_slice(expr_operand->type_value)) {
                    Type_Slice *slice = (Type_Slice *)expr_operand->type_value;
                    element_type = slice->slice_of;
                }
                else if (expr_operand->type_value == type_string) {
                    element_type = type_byte;
                }
                else {
                    report_error(expr_operand->location, "typeofelement() requires an array type, slice type, or string. Cannot use typeofelement() on type '%s'.", type_to_string(expr_operand->type_value));
                    return nullptr;
                }
                assert(element_type != nullptr);
                result_operand.type = type_typeid;
                result_operand.type_value = element_type;
                result_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE | OPERAND_RVALUE;
                break;
            }
            case EXPR_STRUCT_TYPE: {
                Expr_Struct_Type *anonymous_struct = (Expr_Struct_Type *)expr;
                assert(anonymous_struct->structure != nullptr);
                assert(anonymous_struct->structure->type == nullptr);
                anonymous_struct->structure->type = make_incomplete_type_for_struct(anonymous_struct->structure);
                result_operand.type = type_typeid;
                result_operand.type_value = anonymous_struct->structure->type;
                result_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE | OPERAND_RVALUE;
                // todo(josh): I don't think this complete_type needs to be here?
                if (!complete_type(anonymous_struct->structure->type, expr->location, TCF_ALL)) {
                    return nullptr;
                }
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
                result_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE | OPERAND_RVALUE;
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
                result_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE | OPERAND_RVALUE;
                result_operand.type = type_typeid;
                result_operand.type_value = get_or_create_type_reference_to(reference_to_operand->type_value);
                break;
            }
            case EXPR_ARRAY_TYPE: {
                Expr_Array_Type *expr_array = (Expr_Array_Type *)expr;
                Operand *count_operand = typecheck_expr(expr_array->count_expr, type_int);
                if (!count_operand) {
                    return nullptr;
                }
                assert((count_operand->flags & OPERAND_CONSTANT) && is_type_integer(count_operand->type));
                if (count_operand->int_value <= 0) {
                    report_error(expr_array->count_expr->location, "Array size must be greater than 0.");
                    return nullptr;
                }
                if (!typecheck_expr_array_type_with_known_count(expr_array, count_operand->int_value, &result_operand)) {
                    return nullptr;
                }
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
                result_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE | OPERAND_RVALUE;
                result_operand.type = type_typeid;
                result_operand.type_value = get_or_create_type_slice_of(slice_of_operand->type_value);
                break;
            }
            case EXPR_PROCEDURE_TYPE: {
                Expr_Procedure_Type *proc_type_expr = (Expr_Procedure_Type *)expr;
                assert(proc_type_expr->header->operator_to_overload == TK_INVALID); // todo(josh): handle this case
                Operand *proc_operand = typecheck_procedure_header(proc_type_expr->header);
                if (!proc_operand) {
                    return nullptr;
                }
                assert(is_type_procedure(proc_operand->type));
                Type_Procedure *proc_type = (Type_Procedure *)proc_operand->type;
                result_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE | OPERAND_RVALUE;
                result_operand.type = type_typeid;
                result_operand.type_value = proc_operand->type;
                break;
            }
            case EXPR_SPREAD: {
                Expr_Spread *spread = (Expr_Spread *)expr;
                assert(spread->rhs != nullptr);
                Operand *rhs_operand = typecheck_expr(spread->rhs);
                if (!rhs_operand) {
                    return nullptr;
                }
                if (is_type_typeid(rhs_operand->type)) {
                    assert((rhs_operand->flags & OPERAND_CONSTANT) && (rhs_operand->flags & OPERAND_TYPE));
                    result_operand.flags = OPERAND_CONSTANT | OPERAND_TYPE | OPERAND_RVALUE;
                    result_operand.type = type_typeid;
                    result_operand.type_value = get_or_create_type_varargs_of(rhs_operand->type_value, spread->is_c_varargs);
                }
                else {
                    Type *elem_type = nullptr;
                    if (is_type_array(rhs_operand->type)) {
                        elem_type = ((Type_Array *)rhs_operand->type)->array_of;
                    }
                    else if (is_type_slice(rhs_operand->type)) {
                        elem_type = ((Type_Slice *)rhs_operand->type)->slice_of;
                    }
                    else if (is_type_varargs(rhs_operand->type)) {
                        elem_type = ((Type_Varargs *)rhs_operand->type)->varargs_of;
                    }
                    else {
                        report_error(spread->location, "Cannot spread expression of type '%s'.", type_to_string(rhs_operand->type));
                        return nullptr;
                    }
                    assert(elem_type != nullptr);
                    result_operand.flags = rhs_operand->flags;
                    result_operand.type = get_or_create_type_varargs_of(elem_type, false);
                }
                break;
            }
            case EXPR_POLYMORPHIC_TYPE: {
                // foo: Some_Struct!(123, float);
                //      ^^^^^^^^^^^^^^^^^^^^^^^^

                Expr_Polymorphic_Type *poly_type = (Expr_Polymorphic_Type *)expr;
                Operand *type_operand = typecheck_expr(poly_type->type_expr);
                if (!type_operand) {
                    return nullptr;
                }
                assert(is_type_polymorphic(type_operand->type));
                assert(type_operand->referenced_declaration != nullptr);
                assert(type_operand->referenced_declaration->kind == DECL_STRUCT);
                Ast_Struct *referenced_struct = ((Struct_Declaration *)type_operand->referenced_declaration)->structure;

                Ast_Struct *polymorphed_struct = polymorph_struct(referenced_struct, poly_type->location, poly_type->parameters);
                if (!polymorphed_struct) {
                    return nullptr;
                }
                if (!check_declaration(polymorphed_struct->declaration, poly_type->location)) {
                    return nullptr;
                }
                assert(polymorphed_struct->declaration != nullptr);
                assert(polymorphed_struct->declaration->operand.type != nullptr);
                result_operand = polymorphed_struct->declaration->operand;
                break;
            }
            case EXPR_POLYMORPHIC_VARIABLE: {
                Expr_Polymorphic_Variable *poly = (Expr_Polymorphic_Variable *)expr;
                if (poly->inserted_declaration) {
                    if (!check_declaration(poly->inserted_declaration, poly->location, &result_operand)) {
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
            case EXPR_UNKNOWN_ARRAY_COUNT: {
                report_error(expr->location, "? is only allowed in array compound literals e.g. '[?]int.{1, 2, 3, 4}'.");
                return nullptr;
            }
            default: {
                assert(false);
            }
        }

        if ((!(result_operand.flags & OPERAND_NO_VALUE))) {
            if (!complete_type(result_operand.type, expr->location)) { // if an expressions yields a type, that type better be complete! I think.
                return nullptr;
            }

            if (is_type_reference(result_operand.type)) {
                result_operand.flags |= OPERAND_LVALUE;
                result_operand.reference_type = result_operand.type;

                while (is_type_reference(result_operand.type)) {
                    Type_Reference *reference = (Type_Reference *)result_operand.type;
                    assert(reference->reference_to != nullptr);
                    result_operand.type = reference->reference_to;
                }
            }
        }
        else {
            if (require_value) {
                report_error(expr->location, "Expression does not yield a value.");
                return nullptr;
            }
        }

        expr->operand = result_operand;
    }

    if (expected_type) {
        if (expr->operand.flags & OPERAND_NO_VALUE) {
            report_error(expr->location, "Expected '%s', got no value.", type_to_string(expected_type));
            return nullptr;
        }
        if (!match_types(expr->operand, expected_type, &expr->operand)) {
            return nullptr;
        }
    }
    return &expr->operand;
}

Operand *typecheck_procedure_header(Ast_Proc_Header *header) {
    if (header->type) {
        assert(header->operand.type != nullptr);
        return &header->operand;
    }
    Array<Type *> parameter_types = {};
    parameter_types.allocator = g_global_linear_allocator;
    For (idx, header->parameters) {
        Ast_Var *parameter = header->parameters[idx];
        if (parameter->is_constant) {
            report_error(parameter->location, "Constant parameters are not allowed.");
            return nullptr;
        }
        if (parameter->expr != nullptr) {
            report_error(parameter->expr->location, "Default values for procedure parameters are not yet supported.");
            return nullptr;
        }
        if (!check_declaration(parameter->declaration, parameter->location)) {
            return nullptr;
        }
        if (parameter->is_using) {
            if (!complete_type(parameter->declaration->operand.type, parameter->location, TCF_USINGS)) {
                return nullptr;
            }
            if (!broadcast_var_using(parameter, parameter->parent_block)) {
                return nullptr;
            }
        }

        assert(parameter->type != nullptr);
        if (is_type_varargs(parameter->type)) {
            if (idx != header->parameters.count-1) {
                report_error(parameter->location, "Varargs must be the last parameter for a procedure.");
                return nullptr;
            }
            Type_Varargs *varargs_type = (Type_Varargs *)parameter->type;
            if (varargs_type->is_c_varargs) {
                if (idx == 0) {
                    report_error(parameter->location, "#c_varargs cannot be the first parameter for a procedure.");
                    return nullptr;
                }
            }
        }
        parameter_types.append(parameter->type);
    }
    Type *return_type = {};
    if (header->return_type_expr) {
        Operand *return_type_operand = typecheck_expr(header->return_type_expr);
        if (!return_type_operand) {
            return nullptr;
        }
        assert(return_type_operand->flags & OPERAND_CONSTANT);
        assert(return_type_operand->type == type_typeid);
        return_type = return_type_operand->type_value;
    }

    header->type = get_or_create_type_procedure(parameter_types, return_type);

    assert(header->declaration);
    Operand operand = {};
    operand.referenced_declaration = header->declaration;
    operand.type = header->type;
    operand.flags = OPERAND_RVALUE | OPERAND_CONSTANT;
    operand.proc_value = header;
    header->operand = operand;
    return &header->operand;
}

bool typecheck_node(Ast_Node *node) {
    switch (node->ast_kind) {
        case AST_VAR: {
            Ast_Var *var = (Ast_Var *)node;
            if (!check_declaration(var->declaration, var->location)) {
                return false;
            }
            if (!typecheck_expr(var->name_expr)) {
                assert(false);
            }
            if (var->type_expr) {
                if (!typecheck_expr(var->type_expr)) {
                    assert(false);
                }
            }
            if (var->is_using) {
                if (!complete_type(var->type, var->location, TCF_USINGS)) {
                    return false;
                }
                if (!broadcast_var_using(var, var->parent_block)) {
                    return false;
                }
            }
            if (var->parent_proc) {
                var->parent_proc->local_variables.append(var);
            }
            break;
        }

        case AST_ASSIGN: {
            Ast_Assign *assign = (Ast_Assign *)node;

            if (assign->lhs->exprs.count > 1 && assign->op != TK_ASSIGN) {
                report_error(assign->location, "Cannot use assignment operator %s with multiple expressions. Only simple '=' assignments are allowed.", token_string(assign->op));
                return false;
            }

            if (assign->lhs->exprs.count != assign->rhs->exprs.count) {
                report_error(assign->location, "Expected %d expressions on right-hand-side, got %d.", assign->lhs->exprs.count, assign->rhs->exprs.count);
                return false;
            }

            Token_Kind binary_operation = TK_INVALID;
            switch (assign->op) {
                case TK_ASSIGN:             binary_operation = TK_ASSIGN;      break;
                case TK_PLUS_ASSIGN:        binary_operation = TK_PLUS;        break;
                case TK_MINUS_ASSIGN:       binary_operation = TK_MINUS;       break;
                case TK_MULTIPLY_ASSIGN:    binary_operation = TK_MULTIPLY;    break;
                case TK_DIVIDE_ASSIGN:      binary_operation = TK_DIVIDE;      break;
                case TK_MOD_ASSIGN:         binary_operation = TK_MOD;         break;
                case TK_LEFT_SHIFT_ASSIGN:  binary_operation = TK_LEFT_SHIFT;  break;
                case TK_RIGHT_SHIFT_ASSIGN: binary_operation = TK_RIGHT_SHIFT; break;
                case TK_BIT_AND_ASSIGN:     binary_operation = TK_AMPERSAND;   break;
                case TK_BIT_OR_ASSIGN:      binary_operation = TK_BIT_OR;      break;
                case TK_BIT_XOR_ASSIGN:     binary_operation = TK_TILDE;       break;
                case TK_BOOLEAN_AND_ASSIGN: binary_operation = TK_BOOLEAN_AND; break;
                case TK_BOOLEAN_OR_ASSIGN:  binary_operation = TK_BOOLEAN_OR;  break;
                default: {
                    assert(false);
                }
            }
            Array<Operand *> lhs_operands;
            Type_Tuple *tuple = typecheck_expr_list(assign->lhs, &lhs_operands);
            if (tuple == nullptr) {
                return false;
            }
            For (idx, lhs_operands) {
                Operand *lhs_operand = lhs_operands[idx];
                if (!(lhs_operand->flags & OPERAND_LVALUE)) {
                    report_error(assign->location, "Cannot assign to non-lvalue.");
                    return false;
                }
            }
            Array<Operand *> rhs_operands;
            if (!typecheck_expr_list_against_type_tuple(assign->rhs, tuple, &rhs_operands, binary_operation)) {
                return false;
            }
            break;
        }

        case AST_DEFER: {
            Ast_Defer *ast_defer = (Ast_Defer *)node;
            if (!typecheck_node(ast_defer->node_to_defer)) {
                return false;
            }
            break;
        }

        case AST_USING: {
            Ast_Using *ast_using = (Ast_Using *)node;
            if (unparen_expr(ast_using->expr)->expr_kind != EXPR_IDENTIFIER) {
                report_error(ast_using->location, "'using' can only be applied to identifiers.");
                return false;
            }
            // todo(josh): instead of broadcasting each declaration, we could just make a single Using_Declaration that points at the block it's using.
            //             this would mean identifier resolving would have to be recursive. hmmmm
            Operand *using_operand = typecheck_expr(ast_using->expr);
            if (!using_operand) {
                return false;
            }
            assert(using_operand->referenced_declaration != nullptr);

            Type *type_to_broadcast = using_operand->type;
            if (is_type_typeid(using_operand->type)) {
                type_to_broadcast = using_operand->type_value;
            }
            assert(type_to_broadcast != nullptr);
            Ast_Block *block_to_broadcast_declarations_of = type_to_broadcast->declarations_block;
            assert(block_to_broadcast_declarations_of != nullptr);
            if (!complete_type(type_to_broadcast, node->location)) {
                return false;
            }
            assert(block_to_broadcast_declarations_of != nullptr);
            if (!broadcast_declarations_for_using(ast_using->parent_block, block_to_broadcast_declarations_of, using_operand->referenced_declaration, ast_using->expr)) {
                return false;
            }
            break;
        }

        case AST_STATEMENT_EXPR: {
            Ast_Statement_Expr *stmt = (Ast_Statement_Expr *)node;
            Operand *statement_operand = typecheck_expr(stmt->expr, nullptr, false);
            if (!statement_operand) {
                return false;
            }
            if (unparen_expr(stmt->expr)->expr_kind != EXPR_PROCEDURE_CALL) {
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
                    return false;
                }
                Operand *return_operand = typecheck_expr(ast_return->expr, ast_return->matching_procedure->type->return_type);
                if (!return_operand) {
                    return false;
                }
                if (!match_types(*return_operand, ast_return->matching_procedure->type->return_type, return_operand, false)) {
                    report_error(ast_return->expr->location, "Return expression didn't match procedure return type. Expected '%s', got '%s'.", type_to_string(ast_return->matching_procedure->type->return_type), type_to_string(return_operand->type));
                    return false;
                }
            }
            else {
                if (ast_return->expr) {
                    assert(ast_return->matching_procedure->name != nullptr); // todo(josh): handle operator overloads since they don't have names
                    report_error(ast_return->expr->location, "Procedure '%s' doesn't have a return value.", ast_return->matching_procedure->name);
                    return false;
                }
            }
            break;
        }

        case AST_BREAK: {
            Ast_Break *break_stmt = (Ast_Break *)node;
            assert(break_stmt->matching_loop != nullptr);
            break;
        }

        case AST_CONTINUE: {
            Ast_Continue *continue_stmt = (Ast_Continue *)node;
            assert(continue_stmt->matching_loop != nullptr);
            break;
        }

        case AST_EMPTY_STATEMENT: {
            break;
        }

        case AST_DIRECTIVE_ASSERT: {
            g_all_assert_directives.append((Ast_Directive_Assert *)node);
            break;
        }

        case AST_DIRECTIVE_PRINT: {
            g_all_print_directives.append((Ast_Directive_Print *)node);
            break;
        }

        case AST_PROC: {
            break;
        }

        case AST_STRUCT: {
            break;
        }

        case AST_ENUM: {
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