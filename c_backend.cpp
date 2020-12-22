#include "c_backend.h"

void c_print_type(String_Builder *sb, Type *type, char *var_name);
void c_print_expr(String_Builder *sb, Ast_Expr *expr);

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
    if (var->expr != nullptr) {
        sb->print(" = ");
        c_print_expr(sb, var->expr);
    }
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

void c_print_expr(String_Builder *sb, Ast_Expr *expr) {
    if (expr->expr_kind == EXPR_NUMBER_LITERAL) {
        int a = 123;
    }
    if (expr->operand.flags & OPERAND_CONSTANT) {
        if (is_type_float(expr->operand.type)) {
            sb->printf("%f", expr->operand.float_value);
        }
        else if (is_type_integer(expr->operand.type)) {
            sb->printf("%d", expr->operand.int_value);
        }
        else if (expr->operand.type == type_bool) {
            sb->print(expr->operand.bool_value ? "true" : "false");
        }
        return;
    }

    if (!(expr->operand.flags & OPERAND_NO_VALUE)) {
        assert(expr->operand.type != nullptr);
    }
    assert(expr->expr_kind != EXPR_NUMBER_LITERAL);

    switch (expr->expr_kind) {
        case EXPR_IDENTIFIER: {
            Expr_Identifier *identifier = (Expr_Identifier *)expr;
            sb->print(identifier->name);
            break;
        }
        case EXPR_UNARY: {
            UNIMPLEMENTED(EXPR_UNARY);
            Expr_Unary *unary = (Expr_Unary *)expr;
            break;
        }
        case EXPR_BINARY: {
            Expr_Binary *binary = (Expr_Binary *)expr;
            c_print_expr(sb, binary->lhs);
            switch (binary->op) {
                case TK_PLUS: {
                    sb->print(" + ");
                    break;
                }
                case TK_MINUS: {
                    sb->print(" - ");
                    break;
                }
                case TK_MULTIPLY: {
                    sb->print(" * ");
                    break;
                }
                case TK_DIVIDE: {
                    sb->print(" / ");
                    break;
                }
                default: {
                    assert(false);
                }
            }
            c_print_expr(sb, binary->rhs);
            break;
        }
        case EXPR_PROCEDURE_CALL: {
            Expr_Procedure_Call *call = (Expr_Procedure_Call *)expr;
            c_print_expr(sb, call->lhs);
            sb->print("(");
            For (idx, call->parameters) {
                Ast_Expr *parameter = call->parameters[idx];
                c_print_expr(sb, parameter);
                if (idx != (call->parameters.count-1)) {
                    sb->print(", ");
                }
            }
            sb->print(")");
            break;
        }
        case EXPR_ADDRESS_OF: {
            UNIMPLEMENTED(EXPR_ADDRESS_OF);
            Expr_Address_Of *address_of = (Expr_Address_Of *)expr;
            break;
        }
        case EXPR_SUBSCRIPT: {
            Expr_Subscript *subscript = (Expr_Subscript *)expr;
            c_print_expr(sb, subscript->lhs);
            sb->print("[");
            c_print_expr(sb, subscript->index);
            sb->print("]");
            break;
        }
        case EXPR_DEREFERENCE: {
            Expr_Dereference *dereference = (Expr_Dereference *)expr;
            sb->print("*");
            c_print_expr(sb, dereference->lhs);
            break;
        }
        case EXPR_SELECTOR: {
            Expr_Selector *selector = (Expr_Selector *)expr;
            c_print_expr(sb, selector->lhs);
            if (is_type_pointer(selector->lhs->operand.type)) {
                sb->print("->");
            }
            else {
                assert(is_type_struct(selector->lhs->operand.type));
                sb->print(".");
            }
            break;
        }
        case EXPR_NUMBER_LITERAL: {
            assert(false && "shouldn't ever get in here with a number literal because of constant handling above");
            break;
        }
        case EXPR_STRING_LITERAL: {
            UNIMPLEMENTED(EXPR_STRING_LITERAL);
            break;
        }
        case EXPR_POINTER_TYPE: {
            Expr_Pointer_Type *expr_pointer = (Expr_Pointer_Type *)expr;
            UNIMPLEMENTED(EXPR_STRING_LITERAL);
            break;
        }
        case EXPR_ARRAY_TYPE: {
            Expr_Array_Type *expr_array = (Expr_Array_Type *)expr;
            UNIMPLEMENTED(EXPR_STRING_LITERAL);
            break;
        }
        case EXPR_PAREN: {
            Expr_Paren *paren = (Expr_Paren *)expr;
            sb->print("(");
            c_print_expr(sb, paren->nested);
            sb->print(")");
            UNIMPLEMENTED(EXPR_STRING_LITERAL);
            break;
        }
        case EXPR_NULL: {
            assert(false && "shouldn't ever get in here with a null because of constant handling above");
            break;
        }
        case EXPR_TRUE: {
            assert(false && "shouldn't ever get in here with a true because of constant handling above");
            break;
        }
        case EXPR_FALSE: {
            assert(false && "shouldn't ever get in here with a false because of constant handling above");
            break;
        }
        case EXPR_SIZEOF: {
            assert(false && "shouldn't ever get in here with a sizeof because of constant handling above");
            break;
        }
        case EXPR_TYPEOF: {
            assert(false && "shouldn't ever get in here with a typeof because of constant handling above");
            break;
        }
        default: {
            assert(false);
        }
    }
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
                sb.print("};\n");
                break;
            }
            case DECL_VAR: {
                Var_Declaration *var = (Var_Declaration *)decl;
                c_print_var(&sb, var->var);
                sb.print(";\n");
                break;
            }
            case DECL_PROC: {
                Proc_Declaration *procedure = (Proc_Declaration *)decl;
                c_print_procedure_header(&sb, procedure->procedure->header);
                sb.print(" {\n");
                For (idx, procedure->procedure->body->nodes) {
                    Ast_Node *node = procedure->procedure->body->nodes[idx];
                    sb.printf("    ", node->ast_kind);
                    switch (node->ast_kind) {
                        case AST_VAR: {
                            Ast_Var *var = (Ast_Var *)node;
                            c_print_var(&sb, var);
                            sb.print(";\n");
                            break;
                        }

                        case AST_ASSIGN: {
                            Ast_Assign *assign = (Ast_Assign *)node;
                            c_print_expr(&sb, assign->lhs);
                            sb.print(" = ");
                            c_print_expr(&sb, assign->rhs);
                            sb.print(";\n");
                            break;
                        }

                        case AST_STATEMENT_EXPR: {
                            Ast_Statement_Expr *statement = (Ast_Statement_Expr *)node;
                            c_print_expr(&sb, statement->expr);
                            sb.print(";\n");
                            break;
                        }

                        default: {
                            assert(false);
                            break;
                        }
                    }
                }
                sb.print("}\n");
                break;
            }
        }
    }
    return sb;
}