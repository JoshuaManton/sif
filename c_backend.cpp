#include "c_backend.h"

void c_print_type(String_Builder *sb, Type *type, char *var_name);
void c_print_expr(String_Builder *sb, Ast_Expr *expr);

bool has_a_nested_type_array_before_another_pointer(Type *type) {
    switch (type->kind) {
        case TYPE_PRIMITIVE: {
            return false;
        }
        case TYPE_STRUCT: {
            return false;
        }
        case TYPE_ENUM: {
            return false;
        }
        case TYPE_POINTER: {
            return false;
        }
        case TYPE_ARRAY: {
            return true;
        }
        case TYPE_SLICE: {
            return false;
        }
        case TYPE_PROCEDURE: {
            return false;
        }
        default: {
            assert(false);
            return false;
        }
    }
}

void c_print_type_prefix(String_Builder *sb, Type *type) {
    if (type == nullptr) {
        sb->printf("void ");
        return;
    }

    // todo(josh): handle procedure types
    switch (type->kind) {
        case TYPE_PRIMITIVE: {
            Type_Primitive *type_primitive = (Type_Primitive *)type;
            if (strcmp(type_primitive->name, "string") == 0) {
                sb->print("String ");
            }
            else if (strcmp(type_primitive->name, "rawptr") == 0) {
                sb->print("void *");
            }
            else {
                sb->printf("%s ", type_primitive->name);
            }
            break;
        }
        case TYPE_STRUCT: {
            Type_Struct *type_struct = (Type_Struct *)type;
            sb->printf("%s ", type_struct->name);
            break;
        }
        case TYPE_ENUM: {
            sb->printf("i64 ");
            break;
        }
        case TYPE_POINTER: {
            Type_Pointer *type_pointer = (Type_Pointer *)type;
            c_print_type_prefix(sb, type_pointer->pointer_to);
            if (has_a_nested_type_array_before_another_pointer(type_pointer->pointer_to)) {
                sb->print("(");
            }
            sb->print("*");
            break;
        }
        case TYPE_ARRAY: {
            Type_Array *type_array = (Type_Array *)type;
            c_print_type_prefix(sb, type_array->array_of);
            break;
        }
        case TYPE_SLICE: {
            sb->print("Slice ");
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
        case TYPE_ENUM: {
            break;
        }
        case TYPE_POINTER: {
            Type_Pointer *type_pointer = (Type_Pointer *)type;
            if (has_a_nested_type_array_before_another_pointer(type_pointer->pointer_to)) {
                sb->print(")");
            }
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
        case TYPE_SLICE: {
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
    assert(!var->is_constant);
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
        assert(!parameter->is_constant);
        c_print_var(sb, parameter);
        if (idx != (header->parameters.count-1)) {
            sb->print(", ");
        }
    }
    sb->print(")");
}

void c_print_expr(String_Builder *sb, Ast_Expr *expr) {
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
        else if (expr->operand.type == type_string) {
            sb->printf("MAKE_STRING(\"%s\", %d)", expr->operand.scanned_string_value, expr->operand.escaped_string_length);
        }
        else {
            assert(false);
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
                case TK_PLUS:                     { sb->print(" + ");  break; }
                case TK_MINUS:                    { sb->print(" - ");  break; }
                case TK_MULTIPLY:                 { sb->print(" * ");  break; }
                case TK_DIVIDE:                   { sb->print(" / ");  break; }
                case TK_LESS_THAN:                { sb->print(" < ");  break; }
                case TK_LESS_THAN_OR_EQUAL:       { sb->print(" <= "); break; }
                case TK_GREATER_THAN:             { sb->print(" > ");  break; }
                case TK_GREATER_THAN_OR_EQUAL:    { sb->print(" >= "); break; }
                case TK_EQUAL_TO:                 { sb->print(" == "); break; }
                case TK_NOT_EQUAL_TO:             { sb->print(" != "); break; }
                case TK_AMPERSAND:                { sb->print(" & ");  break; }
                case TK_BIT_OR:                   { sb->print(" | ");  break; }
                case TK_BOOLEAN_AND:              { sb->print(" && "); break; }
                case TK_BOOLEAN_OR:               { sb->print(" || "); break; }
                case TK_LEFT_SHIFT:               { sb->print(" << "); break; }
                case TK_RIGHT_SHIFT:              { sb->print(" >> "); break; }
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
            Expr_Address_Of *address_of = (Expr_Address_Of *)expr;
            sb->print("&");
            c_print_expr(sb, address_of->rhs);
            break;
        }
        case EXPR_SUBSCRIPT: {
            Expr_Subscript *subscript = (Expr_Subscript *)expr;
            if (is_type_slice(subscript->lhs->operand.type)) {
                Type_Slice *slice_type = (Type_Slice *)subscript->lhs->operand.type;
                sb->print("((");
                c_print_type(sb, slice_type->data_pointer_type, "");
                sb->print(")");
                c_print_expr(sb, subscript->lhs);
                sb->print(".data)[");
                c_print_expr(sb, subscript->index);
                sb->print("]");
            }
            else {
                assert(is_type_array(subscript->lhs->operand.type));
                c_print_expr(sb, subscript->lhs);
                sb->print("[");
                c_print_expr(sb, subscript->index);
                sb->print("]");
            }
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
            bool is_accessing_slice_data_field = false;
            if ((selector->type_with_field->kind == TYPE_SLICE) && (strcmp(selector->field_name, "data") == 0)) {
                is_accessing_slice_data_field = true;
                Type_Slice *slice_type = (Type_Slice *)selector->type_with_field;
                sb->print("*((");
                c_print_type(sb, get_or_create_type_pointer_to(slice_type->data_pointer_type), ""); // todo(josh): @Speed we should be able to cache a pointer-to-pointer-to-element-type
                sb->print(")&");
            }
            c_print_expr(sb, selector->lhs);
            if (is_type_pointer(selector->lhs->operand.type)) {
                sb->print("->");
            }
            else {
                sb->print(".");
            }
            sb->print(selector->field_name);
            if (is_accessing_slice_data_field) {
                sb->print(")");
            }
            break;
        }
        case EXPR_CAST: {
            Expr_Cast *expr_cast = (Expr_Cast *)expr;
            sb->print("(");
            sb->print("(");
            c_print_type(sb, expr_cast->type_expr->operand.type_value, "");
            sb->print(")");
            c_print_expr(sb, expr_cast->rhs);
            sb->print(")");
            break;
        }
        case EXPR_NUMBER_LITERAL: {
            assert(false && "shouldn't ever get in here with a number literal because of constant handling above");
            break;
        }
        case EXPR_STRING_LITERAL: {
            assert(false && "shouldn't ever get in here with a string literal because of constant handling above");
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
            break;
        }
        case EXPR_NULL: {
            // todo(josh): should null be a constant? probably
            sb->print("nullptr");
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

void print_indents(String_Builder *sb, int indent_level) {
    for (int i = 0; i < indent_level; i++) {
        sb->print("    ");
    }
}

void c_print_block(String_Builder *sb, Ast_Block *block, int indent_level);

void c_print_statement(String_Builder *sb, Ast_Node *node, int indent_level, bool print_semicolon) {
    switch (node->ast_kind) {
        case AST_VAR: {
            Ast_Var *var = (Ast_Var *)node;
            if (!var->is_constant) {
                c_print_var(sb, var);
                if (var->expr == nullptr) {
                    sb->printf(" = {}");
                }
                if (print_semicolon) {
                    sb->print(";\n");
                }
            }
            else {
                sb->printf("// constant declaration omitted: %s\n", var->name);
            }
            break;
        }

        case AST_ASSIGN: {
            Ast_Assign *assign = (Ast_Assign *)node;
            c_print_expr(sb, assign->lhs);
            sb->print(" = ");
            c_print_expr(sb, assign->rhs);
            if (print_semicolon) {
                sb->print(";\n");
            }
            break;
        }

        case AST_STATEMENT_EXPR: {
            Ast_Statement_Expr *statement = (Ast_Statement_Expr *)node;
            c_print_expr(sb, statement->expr);
            if (print_semicolon) {
                sb->print(";\n");
            }
            break;
        }

        case AST_IF: {
            Ast_If *ast_if = (Ast_If *)node;
            assert(ast_if->condition != nullptr);
            assert(ast_if->body != nullptr);
            sb->print("if (");
            c_print_expr(sb, ast_if->condition);
            sb->print(") {\n");
            c_print_block(sb, ast_if->body, indent_level + 1);
            print_indents(sb, indent_level);
            sb->print("}\n");
            if (ast_if->else_body) {
                sb->print("else ");
                c_print_block(sb, ast_if->else_body, indent_level + 1);
            }
            break;
        }

        case AST_FOR_LOOP: {
            Ast_For_Loop *for_loop = (Ast_For_Loop *)node;
            sb->print("for (");
            c_print_statement(sb, for_loop->pre, indent_level, false);
            sb->print("; ");
            c_print_expr(sb, for_loop->condition);
            sb->print("; ");
            c_print_statement(sb, for_loop->post, indent_level, false);
            sb->print(") {\n");
            c_print_block(sb, for_loop->body, indent_level + 1);
            print_indents(sb, indent_level);
            sb->print("}\n");
            break;
        }

        case AST_WHILE_LOOP: {
            Ast_While_Loop *while_loop = (Ast_While_Loop *)node;
            sb->print("while (");
            c_print_expr(sb, while_loop->condition);
            sb->print(") {\n");
            c_print_block(sb, while_loop->body, indent_level + 1);
            print_indents(sb, indent_level);
            sb->print("}\n");
            break;
        }

        case AST_RETURN: {
            Ast_Return *ast_return = (Ast_Return *)node;
            sb->print("return");
            if (ast_return->expr) {
                sb->print(" ");
                c_print_expr(sb, ast_return->expr);
            }
            sb->print(";\n");
            break;
        }

        default: {
            assert(false);
            break;
        }
    }
}

void c_print_block(String_Builder *sb, Ast_Block *block, int indent_level) {
    For (idx, block->nodes) {
        print_indents(sb, indent_level);
        Ast_Node *node = block->nodes[idx];
        c_print_statement(sb, node, indent_level, true);
    }
}

String_Builder generate_c_main_file(Ast_Block *global_scope) {
    // todo(josh): I think there's a bug in my String_Buffer implementation
    //             as this crashes on resize sometimes
    String_Builder sb = make_string_builder(default_allocator(), 10 * 1024);

    sb.print("#include <stdint.h>\n");
    sb.print("#include <stdbool.h>\n");
    sb.print("#include <stdio.h>\n");
    sb.print("#include <cstdlib>\n");

    sb.print("typedef int8_t i8;\n");
    sb.print("typedef int16_t i16;\n");
    sb.print("typedef int32_t i32;\n");
    sb.print("typedef int64_t i64;\n");

    sb.print("typedef uint8_t u8;\n");
    sb.print("typedef uint16_t u16;\n");
    sb.print("typedef uint32_t u32;\n");
    sb.print("typedef uint64_t u64;\n");

    sb.print("typedef float f32;\n");
    sb.print("typedef double f64;\n");

    sb.print("struct String {\n");
    sb.print("    char *data;\n");
    sb.print("    i64 count;\n");
    sb.print("};\n");
    sb.print("String MAKE_STRING(char *data, i64 count) {\n");
    sb.print("    String string;\n");
    sb.print("    string.data = data;\n");
    sb.print("    string.count = count;\n");
    sb.print("    return string;\n");
    sb.print("};\n");

    sb.print("struct Slice {\n");
    sb.print("    void *data;\n");
    sb.print("    i64 count;\n");
    sb.print("};\n");

    For (idx, g_all_c_code_directives) {
        Ast_Directive_C_Code *directive = g_all_c_code_directives[idx];
        sb.print(directive->text);
    }

    // todo(josh): we could clean this up a bunch by introducing some kind of
    // Incomplete_Declaration and only outputting the ones we need to, rather
    // than predeclaring literally everything in the program
    sb.print("\n// Forward declarations\n");
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

    sb.print("\n// Actual declarations\n");
    For (idx, ordered_declarations) {
        Declaration *decl = ordered_declarations[idx];
        switch (decl->kind) {
            case DECL_STRUCT: {
                Struct_Declaration *structure = (Struct_Declaration *)decl;
                sb.printf("struct %s {\n", decl->name);
                For (idx, structure->structure->fields) {
                    Ast_Var *var = structure->structure->fields[idx];
                    if (var->is_constant) {
                        continue;
                    }
                    sb.print("    ");
                    c_print_var(&sb, var);
                    sb.print(";\n");
                }
                sb.print("};\n");
                break;
            }
            case DECL_VAR: {
                Var_Declaration *var = (Var_Declaration *)decl;
                if (!var->var->is_constant) {
                    c_print_var(&sb, var->var);
                    if (var->var->expr == nullptr) {
                        sb.printf(" = {}");
                    }
                    sb.print(";\n");
                }
                break;
            }
            case DECL_PROC: {
                Proc_Declaration *procedure = (Proc_Declaration *)decl;
                if (!procedure->procedure->header->is_foreign) {
                    assert(procedure->procedure->body != nullptr);
                    c_print_procedure_header(&sb, procedure->procedure->header);
                    sb.print(" {\n");
                    c_print_block(&sb, procedure->procedure->body, 1);
                    sb.print("}\n");
                }
                break;
            }
        }
    }
    return sb;
}