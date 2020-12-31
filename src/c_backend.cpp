#include "c_backend.h"

void c_print_type(String_Builder *sb, Type *type, const char *var_name);
void c_print_type_plain(String_Builder *sb, Type *type, const char *var_name);
void c_print_expr(String_Builder *sb, Ast_Expr *expr, Type *target_type = nullptr);
void print_indents(String_Builder *sb, int indent_level);

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
            sb->printf("struct %s ", type_struct->name);
            break;
        }
        case TYPE_ENUM: {
            sb->printf("i64 ");
            break;
        }
        case TYPE_REFERENCE: {
            // note(josh): copypasted from TYPE_POINTER
            Type_Reference *type_reference = (Type_Reference *)type;
            c_print_type_prefix(sb, type_reference->reference_to);
            sb->print("*");
            break;
        }
        case TYPE_POINTER: {
            // note(josh): identical to TYPE_REFERENCE
            // note(josh): identical to TYPE_REFERENCE
            // note(josh): identical to TYPE_REFERENCE
            Type_Pointer *type_pointer = (Type_Pointer *)type;
            c_print_type_prefix(sb, type_pointer->pointer_to);
            sb->print("*");
            break;
        }
        case TYPE_ARRAY: {
            Type_Array *type_array = (Type_Array *)type;
            sb->print("struct ");
            c_print_type_plain(sb, type_array, "");
            sb->print(" ");
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
        case TYPE_REFERENCE: {
            Type_Reference *type_reference = (Type_Reference *)type;
            c_print_type_postfix(sb, type_reference->reference_to);
            break;
        }
        case TYPE_POINTER: {
            Type_Pointer *type_pointer = (Type_Pointer *)type;
            c_print_type_postfix(sb, type_pointer->pointer_to);
            break;
        }
        case TYPE_ARRAY: {
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

void c_print_type(String_Builder *sb, Type *type, const char *var_name) {
    if (type == nullptr) {
        sb->printf("void %s", var_name);
        return;
    }

    assert(!(type->flags & TF_UNTYPED));
    assert(!(type->flags & TF_INCOMPLETE));

    c_print_type_prefix(sb, type);
    sb->print(var_name);
    c_print_type_postfix(sb, type);
}

void c_print_type_plain_prefix(String_Builder *sb, Type *type) {
    if (type == nullptr) {
        sb->printf("void_");
        return;
    }

    // todo(josh): handle procedure types
    switch (type->kind) {
        case TYPE_PRIMITIVE: {
            Type_Primitive *type_primitive = (Type_Primitive *)type;
            if (strcmp(type_primitive->name, "string") == 0) {
                sb->print("String");
            }
            else if (strcmp(type_primitive->name, "rawptr") == 0) {
                sb->print("voidpointer");
            }
            else {
                sb->printf("%s", type_primitive->name);
            }
            break;
        }
        case TYPE_STRUCT: {
            Type_Struct *type_struct = (Type_Struct *)type;
            sb->printf("%s_", type_struct->name);
            break;
        }
        case TYPE_ENUM: {
            sb->printf("i64_");
            break;
        }
        case TYPE_REFERENCE: {
            // note(josh): copypasted from TYPE_POINTER
            Type_Reference *type_reference = (Type_Reference *)type;
            sb->print("ptr_");
            c_print_type_plain_prefix(sb, type_reference->reference_to);
            break;
        }
        case TYPE_POINTER: {
            // note(josh): identical to TYPE_REFERENCE
            // note(josh): identical to TYPE_REFERENCE
            // note(josh): identical to TYPE_REFERENCE
            Type_Pointer *type_pointer = (Type_Pointer *)type;
            sb->print("ptr_");
            c_print_type_plain_prefix(sb, type_pointer->pointer_to);
            break;
        }
        case TYPE_ARRAY: {
            Type_Array *type_array = (Type_Array *)type;
            sb->print("Static_Array_");
            sb->printf("%d_", type_array->count);
            c_print_type_plain(sb, type_array->array_of, "");
            break;
        }
        case TYPE_SLICE: {
            sb->print("Slice_");
            break;
        }
        case TYPE_PROCEDURE: {
            Type_Procedure *type_procedure = (Type_Procedure *)type;
            c_print_type_plain_prefix(sb, type_procedure->return_type);
            sb->print("(*");
            break;
        }
        default: {
            assert(false);
        }
    }
}

void c_print_type_plain_postfix(String_Builder *sb, Type *type) {
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
        case TYPE_REFERENCE: {
            Type_Reference *type_reference = (Type_Reference *)type;
            c_print_type_plain_postfix(sb, type_reference->reference_to);
            break;
        }
        case TYPE_POINTER: {
            Type_Pointer *type_pointer = (Type_Pointer *)type;
            c_print_type_plain_postfix(sb, type_pointer->pointer_to);
            break;
        }
        case TYPE_ARRAY: {
            break;
        }
        case TYPE_PROCEDURE: {
            Type_Procedure *type_procedure = (Type_Procedure *)type;
            sb->print(")(");
            For (idx, type_procedure->parameter_types) {
                c_print_type_plain(sb, type_procedure->parameter_types[idx], "");
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

void c_print_type_plain(String_Builder *sb, Type *type, const char *var_name) {
    if (type == nullptr) {
        sb->printf("void_%s", var_name);
        return;
    }

    assert(!(type->flags & TF_UNTYPED));
    assert(!(type->flags & TF_INCOMPLETE));
    c_print_type_plain_prefix(sb, type);
    sb->print(var_name);
    c_print_type_plain_postfix(sb, type);
}

void c_print_var(String_Builder *sb, const char *var_name, Type *type, Ast_Expr *expr) {
    c_print_type(sb, type, var_name);
    if (expr != nullptr) {
        sb->print(" = ");
        c_print_expr(sb, expr, type);
    }
}

void c_print_var(String_Builder *sb, Ast_Var *var) {
    assert(!var->is_constant);
    c_print_var(sb, var->name, var->type, var->expr);
}

void c_print_procedure_header(String_Builder *sb, Ast_Proc_Header *header) {
    assert(header->name != nullptr);
    c_print_type(sb, header->type->return_type, header->name);
    sb->print("(");
    For (idx, header->parameters) {
        Ast_Var *parameter = header->parameters[idx];
        assert(!parameter->is_constant);
        assert(!parameter->is_polymorphic_value);
        if (idx != 0) {
            sb->print(", ");
        }
        c_print_var(sb, parameter);
    }
    sb->print(")");
}

static int num_compound_literal_temporaries_emitted = 0;

void c_emit_compound_literal_temporaries(String_Builder *sb, Ast_Expr *expr, int indent_level) {
    if (!expr) {
        return;
    }

    switch (expr->expr_kind) {
        case EXPR_COMPOUND_LITERAL: {
            Expr_Compound_Literal *compound_literal = (Expr_Compound_Literal *)expr;
            c_emit_compound_literal_temporaries(sb, compound_literal->type_expr, indent_level);
            For (idx, compound_literal->exprs) {
                Ast_Expr *nested = compound_literal->exprs[idx];
                c_emit_compound_literal_temporaries(sb, nested, indent_level);
            }
            char *var_name = (char *)alloc(default_allocator(), 64);
            sprintf(var_name, "__generated_compound_literal_%d\0", num_compound_literal_temporaries_emitted);
            assert(var_name[63] == '\0'); // todo(josh): we should properly handle this case and allocate a bigger string
            num_compound_literal_temporaries_emitted += 1;
            c_print_var(sb, var_name, compound_literal->operand.type, nullptr);
            sb->printf(" = {0};\n");
            print_indents(sb, indent_level);
            compound_literal->generated_temporary_variable_name = var_name;
            Type *compound_literal_type = compound_literal->operand.type;
            if (is_type_array(compound_literal_type)) {
                Type_Array *array_type = (Type_Array *)compound_literal_type;
                For (idx, compound_literal->exprs) {
                    Ast_Expr *nested = compound_literal->exprs[idx];
                    sb->printf("%s.elements[%d] = ", compound_literal->generated_temporary_variable_name, idx);
                    c_print_expr(sb, nested, array_type->array_of);
                    sb->printf(";\n");
                    print_indents(sb, indent_level);
                }
            }
            else {
                int variable_field_index = -1;
                For (idx, compound_literal->exprs) {
                    variable_field_index += 1;
                    while (compound_literal_type->fields[variable_field_index].operand.flags & OPERAND_CONSTANT) {
                        variable_field_index += 1;
                    }
                    Struct_Field target_field = compound_literal_type->fields[variable_field_index];
                    assert(!(target_field.operand.flags & OPERAND_CONSTANT));
                    assert(target_field.operand.flags & OPERAND_LVALUE);
                    sb->printf("%s.%s = ", compound_literal->generated_temporary_variable_name, target_field.name);
                    Ast_Expr *nested = compound_literal->exprs[idx];
                    c_print_expr(sb, nested, target_field.operand.type);
                    sb->printf(";\n");
                    print_indents(sb, indent_level);
                }
            }

            break;
        }
        case EXPR_IDENTIFIER: {
            break;
        }
        case EXPR_UNARY: {
            UNIMPLEMENTED(EXPR_UNARY);
            break;
        }
        case EXPR_BINARY: {
            Expr_Binary *binary = (Expr_Binary *)expr;
            c_emit_compound_literal_temporaries(sb, binary->lhs, indent_level);
            c_emit_compound_literal_temporaries(sb, binary->rhs, indent_level);
            break;
        }
        case EXPR_PROCEDURE_CALL: {
            Expr_Procedure_Call *call = (Expr_Procedure_Call *)expr;
            For (idx, call->parameters_to_emit) {
                Ast_Expr *parameter = call->parameters_to_emit[idx];
                c_emit_compound_literal_temporaries(sb, parameter, indent_level);
            }
        }
        case EXPR_ADDRESS_OF: {
            Expr_Address_Of *address_of = (Expr_Address_Of *)expr;
            c_emit_compound_literal_temporaries(sb, address_of->rhs, indent_level);
            break;
        }
        case EXPR_SUBSCRIPT: {
            Expr_Subscript *subscript = (Expr_Subscript *)expr;
            c_emit_compound_literal_temporaries(sb, subscript->lhs, indent_level);
            c_emit_compound_literal_temporaries(sb, subscript->index, indent_level);
            break;
        }
        case EXPR_DEREFERENCE: {
            Expr_Dereference *dereference = (Expr_Dereference *)expr;
            c_emit_compound_literal_temporaries(sb, dereference->lhs, indent_level);
            break;
        }
        case EXPR_SELECTOR: {
            Expr_Selector *selector = (Expr_Selector *)expr;
            c_emit_compound_literal_temporaries(sb, selector->lhs, indent_level);
            break;
        }
        case EXPR_CAST: {
            Expr_Cast *expr_cast = (Expr_Cast *)expr;
            c_emit_compound_literal_temporaries(sb, expr_cast->type_expr, indent_level);
            c_emit_compound_literal_temporaries(sb, expr_cast->rhs, indent_level);
            break;
        }
        case EXPR_NUMBER_LITERAL: {
            break;
        }
        case EXPR_STRING_LITERAL: {
            break;
        }
        case EXPR_POINTER_TYPE: {
            Expr_Pointer_Type *expr_pointer = (Expr_Pointer_Type *)expr;
            c_emit_compound_literal_temporaries(sb, expr_pointer->pointer_to, indent_level);
            break;
        }
        case EXPR_ARRAY_TYPE: {
            Expr_Array_Type *expr_array = (Expr_Array_Type *)expr;
            c_emit_compound_literal_temporaries(sb, expr_array->array_of, indent_level);
            break;
        }
        case EXPR_PAREN: {
            Expr_Paren *paren = (Expr_Paren *)expr;
            c_emit_compound_literal_temporaries(sb, paren->nested, indent_level);
            break;
        }
        case EXPR_NULL: {
            break;
        }
        case EXPR_TRUE: {
            break;
        }
        case EXPR_FALSE: {
            break;
        }
        case EXPR_SIZEOF: {
            Expr_Sizeof *expr_sizeof = (Expr_Sizeof *)expr;
            c_emit_compound_literal_temporaries(sb, expr_sizeof->expr, indent_level);
            break;
        }
        case EXPR_TYPEOF: {
            Expr_Typeof *expr_typeof = (Expr_Typeof *)expr;
            c_emit_compound_literal_temporaries(sb, expr_typeof->expr, indent_level);
            break;
        }
        default: {
            printf("unhandled case: %d\n", expr->expr_kind);
            assert(false);
        }
    }
}

void c_print_expr(String_Builder *sb, Ast_Expr *expr, Type *target_type) {
    if (expr->expr_kind == EXPR_PAREN) {
        Expr_Paren *paren = (Expr_Paren *)expr;
        sb->print("(");
        c_print_expr(sb, paren->nested, target_type);
        sb->print(")");
        return;
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

    if (expr->operand.reference_type != nullptr) {
        assert(is_type_reference(expr->operand.reference_type));
        sb->print("(");
        Type *type = expr->operand.reference_type;
        while (is_type_reference(type)) {
            sb->print("*");
            Type_Reference *reference = (Type_Reference *)type;
            type = reference->reference_to;
        }
    }

    if (expr->resolved_operator_overload != nullptr) {
        assert(expr->resolved_operator_overload != nullptr);
        assert(expr->resolved_operator_overload->header->name != nullptr);
        sb->printf("%s(", expr->resolved_operator_overload->header->name);
        For (idx, expr->operator_overload_parameters) {
            c_print_expr(sb, expr->operator_overload_parameters[idx], expr->resolved_operator_overload->header->type->parameter_types[idx]);
            if (idx != (expr->operator_overload_parameters.count-1)) {
                sb->print(", ");
            }
        }
        sb->print(")");
    }
    else {
        // todo(josh): should this be above the operator overload stuff?
        if (target_type) {
            if (is_type_reference(target_type)) {
                sb->print("&");
            }
        }

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
                if (binary->op == TK_EQUAL_TO) {
                    if (is_type_string(binary->lhs->operand.type) && is_type_string(binary->lhs->operand.type)) {
                        sb->print("string_eq(");
                        c_print_expr(sb, binary->lhs);
                        sb->print(", ");
                        c_print_expr(sb, binary->rhs);
                        sb->print(")");
                        break;
                    }
                }
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
                assert(call->target_procedure_type != nullptr);
                assert(!is_type_polymorphic(call->target_procedure_type));
                if (expr->operand.referenced_declaration != nullptr) {
                    assert(expr->operand.referenced_declaration->kind == DECL_PROC);
                    Ast_Proc *referenced_procedure = ((Proc_Declaration *)expr->operand.referenced_declaration)->procedure;
                    sb->print(referenced_procedure->header->name);
                }
                else {
                    c_print_expr(sb, call->lhs);
                }
                sb->print("(");
                assert(call->parameters_to_emit.count == call->target_procedure_type->parameter_types.count);
                For (idx, call->parameters_to_emit) {
                    Ast_Expr *parameter = call->parameters_to_emit[idx];
                    Type *parameter_type = call->target_procedure_type->parameter_types[idx];
                    if (idx != 0) {
                        sb->print(", ");
                    }
                    c_print_expr(sb, parameter, parameter_type);
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
                else if (is_type_array(subscript->lhs->operand.type)) {
                    c_print_expr(sb, subscript->lhs);
                    sb->print(".elements[");
                    c_print_expr(sb, subscript->index);
                    sb->print("]");
                }
                else if (is_type_string(subscript->lhs->operand.type)) {
                    c_print_expr(sb, subscript->lhs);
                    sb->print(".data[");
                    c_print_expr(sb, subscript->index);
                    sb->print("]");
                }
                else {
                    assert(false);
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
            case EXPR_COMPOUND_LITERAL: {
                Expr_Compound_Literal *compound_literal = (Expr_Compound_Literal *)expr;
                assert(compound_literal->generated_temporary_variable_name != nullptr);
                sb->print(compound_literal->generated_temporary_variable_name);
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
                assert(false && "handled above");
            }
            case EXPR_NULL: {
                // todo(josh): should null be a constant? probably
                sb->print("NULL");
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
                printf("unhandled case: %d\n", expr->expr_kind);
                assert(false);
            }
        }
    }

    if (expr->operand.reference_type != nullptr) {
        assert(is_type_reference(expr->operand.reference_type));
        sb->print(")");
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
            c_emit_compound_literal_temporaries(sb, var->expr, indent_level);
            if (!var->is_constant) {
                c_print_var(sb, var);
                if (var->expr == nullptr) {
                    sb->printf(" = {0}");
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
            c_emit_compound_literal_temporaries(sb, assign->lhs, indent_level);
            c_emit_compound_literal_temporaries(sb, assign->rhs, indent_level);
            c_print_expr(sb, assign->lhs);
            switch (assign->op) {
                case TK_ASSIGN:             sb->print(" = "); break;
                case TK_PLUS_ASSIGN:        sb->print(" += "); break;
                case TK_MINUS_ASSIGN:       sb->print(" -= "); break;
                case TK_MULTIPLY_ASSIGN:    sb->print(" *= "); break;
                case TK_DIVIDE_ASSIGN:      sb->print(" /= "); break;
                case TK_LEFT_SHIFT_ASSIGN:  sb->print(" <<= "); break;
                case TK_RIGHT_SHIFT_ASSIGN: sb->print(" >>= "); break;
                case TK_BIT_AND_ASSIGN:     sb->print(" &= "); break;
                case TK_BIT_OR_ASSIGN:      sb->print(" |= "); break;
                case TK_BOOLEAN_AND_ASSIGN: sb->print(" &&= "); break;
                case TK_BOOLEAN_OR_ASSIGN:  sb->print(" ||= "); break;
                default: {
                    assert(false);
                }
            }
            c_print_expr(sb, assign->rhs);
            if (print_semicolon) {
                sb->print(";\n");
            }
            break;
        }

        case AST_STATEMENT_EXPR: {
            Ast_Statement_Expr *statement = (Ast_Statement_Expr *)node;
            c_emit_compound_literal_temporaries(sb, statement->expr, indent_level);
            c_print_expr(sb, statement->expr);
            if (print_semicolon) {
                sb->print(";\n");
            }
            break;
        }

        case AST_BLOCK_STATEMENT: {
            Ast_Block_Statement *block_statement = (Ast_Block_Statement *)node;
            sb->print("{\n");
            c_print_block(sb, block_statement->block, indent_level + 1);
            print_indents(sb, indent_level);
            sb->print("}\n");
            break;
        }

        case AST_IF: {
            Ast_If *ast_if = (Ast_If *)node;
            c_emit_compound_literal_temporaries(sb, ast_if->condition, indent_level);
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
            c_emit_compound_literal_temporaries(sb, for_loop->condition, indent_level);
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
            c_emit_compound_literal_temporaries(sb, while_loop->condition, indent_level);
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
            c_emit_compound_literal_temporaries(sb, ast_return->expr, indent_level);
            sb->print("return");
            if (ast_return->expr) {
                sb->print(" ");
                c_print_expr(sb, ast_return->expr, ast_return->matching_procedure->type->return_type);
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

void c_print_procedure(String_Builder *sb, Ast_Proc *proc) {
    assert(proc->body != nullptr);
    assert(!proc->header->is_foreign);
    c_print_procedure_header(sb, proc->header);
    sb->print(" {\n");
    c_print_block(sb, proc->body, 1);
    sb->print("}\n");
}

void c_print_struct(String_Builder *sb, Ast_Struct *structure) {
    sb->printf("struct %s {\n", structure->name);
    For (idx, structure->fields) {
        Ast_Var *var = structure->fields[idx];
        if (var->is_constant) {
            continue;
        }
        sb->print("    ");
        c_print_var(sb, var);
        sb->print(";\n");
    }
    sb->print("};\n");
    For (idx, structure->operator_overloads) {
        c_print_procedure(sb, structure->operator_overloads[idx]);
    }
}

String_Builder generate_c_main_file(Ast_Block *global_scope) {
    // todo(josh): I think there's a bug in my String_Buffer implementation
    //             as this crashes on resize sometimes
    String_Builder sb = make_string_builder(default_allocator(), 10 * 1024);

    sb.print("#include <stdint.h>\n");
    sb.print("#include <stdbool.h>\n");
    sb.print("#include <stdio.h>\n");
    sb.print("#include <stdlib.h>\n");
    sb.print("#include <string.h>\n");

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

    sb.print("typedef struct {\n");
    sb.print("    char *data;\n");
    sb.print("    i64 count;\n");
    sb.print("} String;\n");

    sb.print("String MAKE_STRING(char *data, i64 count) {\n");
    sb.print("    String string;\n");
    sb.print("    string.data = data;\n");
    sb.print("    string.count = count;\n");
    sb.print("    return string;\n");
    sb.print("};\n");

    sb.print("typedef struct {\n");
    sb.print("    void *data;\n");
    sb.print("    i64 count;\n");
    sb.print("} Slice;\n");

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
                if (!procedure->procedure->header->is_foreign) {
                    c_print_procedure_header(&sb, procedure->procedure->header);
                    sb.print(";\n");
                }
                break;
            }
            case DECL_TYPE: {
                Type_Declaration *type_decl = (Type_Declaration *)decl;
                if (is_type_array(type_decl->type)) {
                    assert(type_decl->parent_block == nullptr);
                    Type_Array *array_type = (Type_Array *)type_decl->type;
                    c_print_type(&sb, array_type, "");
                    sb.printf(";\n");
                }
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
                c_print_struct(&sb, structure->structure);
                break;
            }
            case DECL_VAR: {
                Var_Declaration *var = (Var_Declaration *)decl;
                if (!var->var->is_constant) {
                    c_print_var(&sb, var->var);
                    if (var->var->expr == nullptr) {
                        sb.printf(" = {0}");
                    }
                    sb.print(";\n");
                }
                break;
            }
            case DECL_PROC: {
                Proc_Declaration *procedure = (Proc_Declaration *)decl;
                if (!procedure->procedure->header->is_foreign) {
                    c_print_procedure(&sb, procedure->procedure);
                }
                break;
            }
            case DECL_TYPE: {
                Type_Declaration *type_decl = (Type_Declaration *)decl;
                if (is_type_array(type_decl->type)) {
                    assert(type_decl->parent_block == nullptr);
                    Type_Array *array_type = (Type_Array *)type_decl->type;
                    c_print_type(&sb, array_type, "");
                    sb.printf(" {\n");
                    sb.printf("    ");
                    c_print_type(&sb, array_type->array_of, "");
                    sb.printf(" elements[%d];\n", array_type->count);
                    sb.printf("};\n");
                }
                break;
            }
        }
    }
    return sb;
}