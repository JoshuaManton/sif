#include "c_backend.h"

void c_print_type(String_Builder *sb, Type *type, char *var_name);

void c_print_type_prefix(String_Builder *sb, Type *type) {
    if (type == nullptr) {
        sb->printf("void ");
        return;
    }

    // todo(josh): handle procedure types
    switch (type->kind) {
        case TYPE_PRIMITIVE: {
            Type_Primitive *type_primitive = (Type_Primitive *)type;
            sb->printf("%s ", type_primitive->name);
            break;
        }
        case TYPE_STRUCT: {
            Type_Struct *type_struct = (Type_Struct *)type;
            sb->printf("%s ", type_struct->name);
            break;
        }
        case TYPE_POINTER: {
            Type_Pointer *type_pointer = (Type_Pointer *)type;
            c_print_type_prefix(sb, type_pointer->pointer_to);
            sb->print("(");
            sb->print("*");
            break;
        }
        case TYPE_ARRAY: {
            Type_Array *type_array = (Type_Array *)type;
            c_print_type_prefix(sb, type_array->array_of);
            // sb->print("Static_Array<");
            // c_print_type(sb, type_array->array_of, var_name);
            // sb->print(", ");
            // sb->printf("%lld", type_array->count);
            // sb->print(">");
            break;
        }
        case TYPE_PROCEDURE: {
            Type_Procedure *type_procedure = (Type_Procedure *)type;
            c_print_type_prefix(sb, type_procedure->return_type);
            sb->print("(*");
            break;
        }
        default: {
            assert(false);
        }
    }
}

void c_print_type_postfix(String_Builder *sb, Type *type) {
    // todo(josh): handle procedure types
    switch (type->kind) {
        case TYPE_PRIMITIVE: {
            break;
        }
        case TYPE_STRUCT: {
            break;
        }
        case TYPE_POINTER: {
            Type_Pointer *type_pointer = (Type_Pointer *)type;
            sb->print(")");
            c_print_type_postfix(sb, type_pointer->pointer_to);
            break;
        }
        case TYPE_ARRAY: {
            Type_Array *type_array = (Type_Array *)type;
            sb->printf("[%lld]", type_array->count);
            c_print_type_postfix(sb, type_array->array_of);
            // sb->print("Static_Array<");
            // c_print_type(sb, type_array->array_of, var_name);
            // sb->print(", ");
            // sb->printf("%lld", type_array->count);
            // sb->print(">");
            break;
        }
        case TYPE_PROCEDURE: {
            Type_Procedure *type_procedure = (Type_Procedure *)type;
            sb->print(")(");
            For (idx, type_procedure->parameter_types) {
                c_print_type(sb, type_procedure->parameter_types[idx], "");
                if (idx != (type_procedure->parameter_types.count-1)) {
                    sb->print(", ");
                }
            }
            sb->print(")");
            break;
        }
        default: {
            assert(false);
        }
    }
}

void c_print_type(String_Builder *sb, Type *type, char *var_name) {
    if (type == nullptr) {
        sb->printf("void %s", var_name);
        return;
    }

    assert(!(type->flags & TF_UNTYPED));
    assert(!(type->flags & TF_INCOMPLETE));
    c_print_type_prefix(sb, type);
    sb->print(var_name);
    c_print_type_postfix(sb, type);
    // switch (type->kind) {
    //     case TYPE_PRIMITIVE: {
    //         Type_Primitive *type_primitive = (Type_Primitive *)type;
    //         sb->printf("%s %s", type_primitive->name, var_name);
    //         break;
    //     }
    //     case TYPE_STRUCT: {
    //         Type_Struct *type_struct = (Type_Struct *)type;
    //         sb->printf("%s", type_struct->name);
    //         break;
    //     }
    //     case TYPE_POINTER: {
    //         Type_Pointer *type_pointer = (Type_Pointer *)type;
    //         c_print_type(sb, type_pointer->pointer_to, var_name);
    //         sb->print(" *");
    //         sb->print(var_name);
    //         break;
    //     }
    //     case TYPE_ARRAY: {
    //         Type_Array *type_array = (Type_Array *)type;
    //         sb->print("Static_Array<");
    //         c_print_type(sb, type_array->array_of, var_name);
    //         sb->print(", ");
    //         sb->printf("%lld", type_array->count);
    //         sb->print(">");
    //         break;
    //     }
    //     default: {
    //         assert(false);
    //     }
    // }
}

void c_print_var(String_Builder *sb, Ast_Var *var) {
    c_print_type(sb, var->type, var->name);
}

void c_print_procedure_header(String_Builder *sb, Ast_Proc_Header *header) {
    c_print_type(sb, header->type->return_type, header->name);
    sb->print("(");
    For (idx, header->parameters) {
        Ast_Var *parameter = header->parameters[idx];
        c_print_var(sb, parameter);
        if (idx != (header->parameters.count-1)) {
            sb->print(", ");
        }
    }
    sb->print(")");
}

String_Builder generate_c_main_file(Ast_Block *global_scope) {
    // todo(josh): I think there's a bug in my String_Buffer implementation
    //             as this crashes on resize sometimes
    String_Builder sb = make_string_builder(default_allocator(), 10 * 1024);
    // predeclare everything
    // todo(josh): we could clean this up a bunch by introducing some kind of
    // Incomplete_Declaration and only outputting the ones we need to, rather
    // than predeclaring literally everything in the program
    sb.print("// Forward declarations\n");
    For (idx, ordered_declarations) {
        Declaration *decl = ordered_declarations[idx];
        switch (decl->kind) {
            case DECL_STRUCT: {
                Struct_Declaration *structure = (Struct_Declaration *)decl;
                sb.printf("struct %s", decl->name);
                sb.print(";\n");
                break;
            }
            case DECL_VAR: {
                Var_Declaration *var = (Var_Declaration *)decl;
                sb.print("extern ");
                c_print_var(&sb, var->var);
                sb.print(";\n");
                break;
            }
            case DECL_PROC: {
                Proc_Declaration *procedure = (Proc_Declaration *)decl;
                c_print_procedure_header(&sb, procedure->procedure->header);
                sb.print(";\n");
                break;
            }
        }
    }

    // actually generate the declarations
    sb.print("\n// Actual declarations\n");
    For (idx, ordered_declarations) {
        Declaration *decl = ordered_declarations[idx];
        switch (decl->kind) {
            case DECL_STRUCT: {
                Struct_Declaration *structure = (Struct_Declaration *)decl;
                sb.printf("struct %s {\n", decl->name);
                For (idx, structure->structure->fields) {
                    Ast_Var *var = structure->structure->fields[idx];
                    sb.print("    ");
                    c_print_var(&sb, var);
                    sb.print(";\n");
                }
                sb.print("};\n\n");
                break;
            }
            case DECL_VAR: {
                Var_Declaration *var = (Var_Declaration *)decl;
                c_print_var(&sb, var->var);
                sb.print(";\n\n");
                break;
            }
            case DECL_PROC: {
                Proc_Declaration *procedure = (Proc_Declaration *)decl;
                c_print_procedure_header(&sb, procedure->procedure->header);
                sb.print(" {\n");
                For (idx, procedure->procedure->body->nodes) {
                    Ast_Node *node = procedure->procedure->body->nodes[idx];
                    // sb.printf("    %d\n", node->ast_kind);
                }
                sb.print("}\n\n");
                break;
            }
        }
    }
    return sb;
}