#include "c_backend.h"

void c_print_type(String_Builder *sb, Type *type, const char *var_name);
void c_print_type_plain(String_Builder *sb, Type *type, const char *var_name);
char *c_print_expr(String_Builder *sb, Ast_Expr *expr, int indent_level, Type *target_type = nullptr);
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
            else if (strcmp(type_primitive->name, "any") == 0) {
                sb->print("Any ");
            }
            else if (strcmp(type_primitive->name, "typeid") == 0) {
                sb->print("i64 ");
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
        case TYPE_VARARGS: // note(josh): fallthrough
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
        case TYPE_VARARGS: // note(josh): fallthrough
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
            else if (strcmp(type_primitive->name, "any") == 0) {
                sb->print("Any");
            }
            else if (strcmp(type_primitive->name, "typeid") == 0) {
                sb->print("i64 ");
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

void c_print_var(String_Builder *sb, const char *var_name, Type *type, Ast_Expr *expr, int indent_level) {
    char *rhs = nullptr;
    if (expr) {
        rhs = c_print_expr(sb, expr, indent_level, type);
    }
    print_indents(sb, indent_level);
    c_print_type(sb, type, var_name);
    if (rhs) {
        sb->printf(" = %s", rhs);
    }
}

void c_print_var(String_Builder *sb, Ast_Var *var, int indent_level) {
    assert(!var->is_constant);
    c_print_var(sb, var->name, var->type, var->expr, indent_level);
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
        c_print_var(sb, parameter, 0);
    }
    sb->print(")");
}

int total_num_temporaries_emitted = 0;

char *c_temporary() {
    total_num_temporaries_emitted += 1;
    String_Builder sb = make_string_builder(default_allocator(), 32); // todo(josh): this is very dumb. use an arena or something
    sb.printf("__t%d", total_num_temporaries_emitted);
    return sb.string();
}

void c_print_procedure_call_parameters(String_Builder *sb, Ast_Expr *root_expr, Type_Procedure *proc_type, Array<Ast_Expr *> parameters, int indent_level, Array<char *> *out_temporaries) {
    For (idx, proc_type->parameter_types) {
        if (!is_type_varargs(proc_type->parameter_types[idx])) {
            char *param = c_print_expr(sb, parameters[idx], indent_level, proc_type->parameter_types[idx]);
            out_temporaries->append(param);
        }
        else {
            Type_Varargs *varargs = (Type_Varargs *)proc_type->parameter_types[idx];
            assert(idx == proc_type->parameter_types.count-1);
            int varargs_count = root_expr->vararg_parameters.count;
            if (varargs_count == 1 && (unparen_expr(root_expr->vararg_parameters[0])->expr_kind == EXPR_SPREAD)) {
                Expr_Spread *spread = (Expr_Spread *)unparen_expr(root_expr->vararg_parameters[0]);
                char *spread_name = c_print_expr(sb, spread->rhs, indent_level);
                out_temporaries->append(spread_name);
            }
            else {
                char *t = c_temporary();
                print_indents(sb, indent_level);
                c_print_type(sb, varargs->varargs_of, t);
                sb->printf("[%d];\n", max(1, varargs_count));
                For (idx, root_expr->vararg_parameters) {
                    Ast_Expr *param = root_expr->vararg_parameters[idx];
                    char *param_name = c_print_expr(sb, param, indent_level, varargs->varargs_of);
                    print_indents(sb, indent_level);
                    sb->printf("%s[%d] = %s;\n", t, idx, param_name);
                }
                char *slice = c_temporary();
                print_indents(sb, indent_level);
                c_print_type(sb, get_or_create_type_slice_of(varargs->varargs_of), slice);
                sb->print(";\n");
                print_indents(sb, indent_level);
                sb->printf("%s.data = %s;\n", slice, t);
                print_indents(sb, indent_level);
                sb->printf("%s.count = %d;\n", slice, varargs_count);
                out_temporaries->append(slice);
            }
            return;
        }
    }
}

char *c_print_expr(String_Builder *sb, Ast_Expr *expr, int indent_level, Type *target_type) {
    char *t = nullptr;

    if (!(expr->operand.flags & OPERAND_NO_VALUE)) {
        assert(expr->operand.type != nullptr);
    }

    String_Builder reference_prefix_sb = {};
    reference_prefix_sb.buf.allocator = default_allocator();

    if (expr->operand.reference_type != nullptr) {
        assert(is_type_reference(expr->operand.reference_type));
        reference_prefix_sb.print("(");
        Type *type = expr->operand.reference_type;
        while (is_type_reference(type)) {
            reference_prefix_sb.print("*");
            Type_Reference *reference = (Type_Reference *)type;
            type = reference->reference_to;
        }
    }

    if (expr->desugared_procedure_to_call != nullptr) {
        assert(expr->desugared_procedure_to_call != nullptr);
        assert(expr->desugared_procedure_to_call->header->name != nullptr);
        Array<char *> params;
        params.allocator = default_allocator();
        c_print_procedure_call_parameters(sb, expr, expr->desugared_procedure_to_call->header->type, expr->desugared_procedure_parameters, indent_level, &params);
        print_indents(sb, indent_level);
        if (expr->desugared_procedure_to_call->header->type->return_type) {
            t = c_temporary();
            c_print_type(sb, expr->desugared_procedure_to_call->header->type->return_type, t);
            sb->print(" = ");
        }
        sb->printf("%s(", expr->desugared_procedure_to_call->header->name);
        For (idx, params) {
            if (idx != 0) {
                sb->print(", ");
            }
            sb->print(params[idx]);
        }
        sb->print(");\n");
    }
    else {
        if (expr->operand.flags & OPERAND_CONSTANT) {
            assert(!is_type_untyped(expr->operand.type));
            t = c_temporary();
            print_indents(sb, indent_level);
            c_print_type(sb, expr->operand.type, t);
            sb->print(" = ");
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
            else if (is_type_typeid(expr->operand.type)) {
                sb->printf("%d", expr->operand.type_value->id);
            }
            else {
                assert(false);
            }
            sb->print(";\n");
        }
        else {
            switch (expr->expr_kind) {
                case EXPR_IDENTIFIER: {
                    Expr_Identifier *identifier = (Expr_Identifier *)expr;
                    t = identifier->name;
                    break;
                }
                case EXPR_UNARY: {
                    UNIMPLEMENTED(EXPR_UNARY);
                    Expr_Unary *unary = (Expr_Unary *)expr;
                    break;
                }
                case EXPR_BINARY: {
                    Expr_Binary *binary = (Expr_Binary *)expr;
                    t = c_temporary();
                    char *lhs = c_print_expr(sb, binary->lhs, indent_level); assert(lhs);
                    char *rhs = c_print_expr(sb, binary->rhs, indent_level); assert(rhs);
                    print_indents(sb, indent_level);
                    if (binary->op == TK_EQUAL_TO && is_type_string(binary->lhs->operand.type) && is_type_string(binary->lhs->operand.type)) {
                        // todo(josh): this should probably be a desugared_procedure thing
                        c_print_type(sb, type_bool, t);
                        sb->printf(" = string_eq(%s, %s);\n", lhs, rhs);
                        break;
                    }
                    c_print_type(sb, binary->operand.type, t);
                    sb->print(" = ");
                    sb->print(lhs);
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
                    sb->printf("%s;\n", rhs);
                    break;
                }
                case EXPR_PROCEDURE_CALL: {
                    Expr_Procedure_Call *call = (Expr_Procedure_Call *)expr;
                    assert(call->target_procedure_type != nullptr);
                    assert(!is_type_polymorphic(call->target_procedure_type));
                    char *procedure_name = c_print_expr(sb, call->lhs, indent_level);
                    Array<char *> params;
                    params.allocator = default_allocator();
                    c_print_procedure_call_parameters(sb, call, call->target_procedure_type, call->parameters_to_emit, indent_level, &params);
                    if (call->target_procedure_type->return_type) {
                        t = c_temporary();
                        print_indents(sb, indent_level);
                        c_print_type(sb, call->target_procedure_type->return_type, t);
                        sb->print(" = ");
                    }
                    sb->printf("%s(", procedure_name);
                    For (idx, params) {
                        if (idx != 0) {
                            sb->print(", ");
                        }
                        sb->print(params[idx]);
                    }
                    sb->print(");\n");
                    break;
                }
                case EXPR_SUBSCRIPT: {
                    Expr_Subscript *subscript = (Expr_Subscript *)expr;
                    char *lhs = c_print_expr(sb, subscript->lhs, indent_level);
                    char *index = c_print_expr(sb, subscript->index, indent_level);
                    String_Builder subscript_sb = make_string_builder(default_allocator(), 32);
                    if (is_type_slice(subscript->lhs->operand.type) || is_type_varargs(subscript->lhs->operand.type)) {
                        Type_Slice *slice_type = nullptr;
                        if (is_type_slice(subscript->lhs->operand.type)) {
                            slice_type = (Type_Slice *)subscript->lhs->operand.type;
                        }
                        else if (is_type_varargs(subscript->lhs->operand.type)) {
                            slice_type = ((Type_Varargs *)subscript->lhs->operand.type)->slice_type;
                        }
                        assert(slice_type != nullptr);
                        subscript_sb.print("((");
                        c_print_type(&subscript_sb, slice_type->data_pointer_type, "");
                        subscript_sb.print(")");
                        subscript_sb.print(lhs);
                        subscript_sb.print(".data)[");
                        subscript_sb.print(index);
                        subscript_sb.print("]");
                    }
                    else if (is_type_array(subscript->lhs->operand.type)) {
                        subscript_sb.printf("%s.elements[%s]", lhs, index);
                    }
                    else if (is_type_string(subscript->lhs->operand.type)) {
                        subscript_sb.printf("%s.data[%s]", lhs, index);
                    }
                    else {
                        assert(false);
                    }
                    t = subscript_sb.string();
                    break;
                }
                case EXPR_COMPOUND_LITERAL: {
                    Expr_Compound_Literal *compound_literal = (Expr_Compound_Literal *)expr;
                    Array<char *> expr_names;
                    expr_names.allocator = default_allocator();
                    For (idx, compound_literal->exprs) {
                        char *name = c_print_expr(sb, compound_literal->exprs[idx], indent_level);
                        expr_names.append(name);
                    }
                    t = c_temporary();
                    print_indents(sb, indent_level);
                    c_print_type(sb, compound_literal->operand.type, t);
                    sb->print(" = {0};\n");

                    Type *compound_literal_type = compound_literal->operand.type;
                    if (is_type_array(compound_literal_type)) {
                        Type_Array *array_type = (Type_Array *)compound_literal_type;
                        For (idx, expr_names) {
                            print_indents(sb, indent_level);
                            sb->printf("%s.elements[%d] = %s;\n", t, idx, expr_names[idx]);
                        }
                    }
                    else {
                        int variable_field_index = -1;
                        For (idx, expr_names) {
                            variable_field_index += 1;
                            while (compound_literal_type->fields[variable_field_index].operand.flags & OPERAND_CONSTANT) {
                                variable_field_index += 1;
                            }
                            print_indents(sb, indent_level);
                            Struct_Field target_field = compound_literal_type->fields[variable_field_index];
                            assert(!(target_field.operand.flags & OPERAND_CONSTANT));
                            assert(target_field.operand.flags & OPERAND_LVALUE);
                            sb->printf("%s.%s = %s;\n", t, target_field.name, expr_names[idx]);
                        }
                    }
                    break;
                }
                case EXPR_ADDRESS_OF: {
                    Expr_Address_Of *address_of = (Expr_Address_Of *)expr;
                    char *rhs = c_print_expr(sb, address_of->rhs, indent_level);
                    String_Builder address_sb = make_string_builder(default_allocator(), 64);
                    address_sb.printf("&%s", rhs);
                    t = address_sb.string();
                    break;
                }
                case EXPR_DEREFERENCE: {
                    Expr_Dereference *dereference = (Expr_Dereference *)expr;
                    char *lhs = c_print_expr(sb, dereference->lhs, indent_level);
                    String_Builder deref_sb = make_string_builder(default_allocator(), 64);
                    deref_sb.printf("(*%s)", lhs);
                    t = deref_sb.string();
                    break;
                }
                case EXPR_SELECTOR: {
                    Expr_Selector *selector = (Expr_Selector *)expr;
                    char *lhs = c_print_expr(sb, selector->lhs, indent_level);
                    assert(lhs);
                    String_Builder selector_sb = make_string_builder(default_allocator(), 64);
                    bool is_accessing_slice_data_field = false;
                    if ((selector->type_with_field->kind == TYPE_SLICE) && (strcmp(selector->field_name, "data") == 0)) {
                        is_accessing_slice_data_field = true;
                        Type_Slice *slice_type = (Type_Slice *)selector->type_with_field;
                        selector_sb.print("*((");
                        c_print_type(&selector_sb, get_or_create_type_pointer_to(slice_type->data_pointer_type), ""); // todo(josh): @Speed we should be able to cache a pointer-to-pointer-to-element-type
                        selector_sb.print(")&");
                    }
                    selector_sb.print(lhs);
                    if (is_type_pointer(selector->lhs->operand.type)) {
                        selector_sb.print("->");
                    }
                    else {
                        selector_sb.print(".");
                    }
                    selector_sb.print(selector->field_name);
                    if (is_accessing_slice_data_field) {
                        selector_sb.print(")");
                    }
                    t = selector_sb.string();
                    break;
                }
                case EXPR_CAST: {
                    Expr_Cast *expr_cast = (Expr_Cast *)expr;
                    char *rhs = c_print_expr(sb, expr_cast->rhs, indent_level);
                    t = c_temporary();
                    print_indents(sb, indent_level);
                    c_print_type(sb, expr_cast->type_expr->operand.type_value, t);
                    sb->print(" = ((");
                    c_print_type(sb, expr_cast->type_expr->operand.type_value, "");
                    sb->printf(")%s);\n", rhs);
                    break;
                }
                case EXPR_NUMBER_LITERAL: {
                    assert(false);
                    break;
                }
                case EXPR_CHAR_LITERAL: {
                    assert(false);
                    break;
                }
                case EXPR_STRING_LITERAL: {
                    assert(false);
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
                    t = c_temporary();
                    char *nested = c_print_expr(sb, paren->nested, indent_level, target_type);
                    print_indents(sb, indent_level);
                    c_print_type(sb, expr->operand.type, t);
                    sb->printf(" = %s;\n", nested);
                    break;
                }
                case EXPR_NULL: {
                    t = "NULL";
                    break;
                }
                case EXPR_TRUE: {
                    assert(false);
                    break;
                }
                case EXPR_FALSE: {
                    assert(false);
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
    }

    if (target_type) {
        if (is_type_reference(target_type)) {
            reference_prefix_sb.printf("&%s", t);
            t = reference_prefix_sb.string();
        }
    }

    if (expr->operand.reference_type != nullptr) {
        assert(is_type_reference(expr->operand.reference_type));
        reference_prefix_sb.printf("%s)", t);
        t = reference_prefix_sb.string();
    }

    if (target_type) {
        if (target_type == type_any && (expr->operand.type != type_any)) {
            String_Builder any_sb = make_string_builder(default_allocator(), 32);
            any_sb.printf("MAKE_ANY(&%s, %d)", t, expr->operand.type->id);
            t = any_sb.string();
        }
    }

    return t;
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
                c_print_var(sb, var, indent_level);
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
            char *lhs = c_print_expr(sb, assign->lhs, indent_level);
            char *rhs = c_print_expr(sb, assign->rhs, indent_level, assign->lhs->operand.type);
            print_indents(sb, indent_level);
            sb->print(lhs);
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
            sb->print(rhs);
            if (print_semicolon) {
                sb->print(";\n");
            }
            break;
        }

        case AST_STATEMENT_EXPR: {
            Ast_Statement_Expr *statement = (Ast_Statement_Expr *)node;
            c_print_expr(sb, statement->expr, indent_level);
            break;
        }

        case AST_BLOCK_STATEMENT: {
            Ast_Block_Statement *block_statement = (Ast_Block_Statement *)node;
            print_indents(sb, indent_level);
            sb->print("{\n");
            c_print_block(sb, block_statement->block, indent_level + 1);
            print_indents(sb, indent_level);
            sb->print("}\n");
            break;
        }

        case AST_IF: {
            Ast_If *ast_if = (Ast_If *)node;
            assert(ast_if->condition != nullptr);
            assert(ast_if->body != nullptr);
            char *cond_name = c_print_expr(sb, ast_if->condition, indent_level);
            print_indents(sb, indent_level);
            sb->printf("if (%s) {\n", cond_name);
            c_print_block(sb, ast_if->body, indent_level+1);
            print_indents(sb, indent_level);
            sb->print("}\n");
            if (ast_if->else_body) {
                print_indents(sb, indent_level);
                sb->print("else {\n");
                c_print_block(sb, ast_if->else_body, indent_level + 1);
                print_indents(sb, indent_level);
                sb->print("}\n");
            }
            break;
        }

        case AST_FOR_LOOP: {
            Ast_For_Loop *for_loop = (Ast_For_Loop *)node;
            print_indents(sb, indent_level);
            sb->print("{\n");
            c_print_statement(sb, for_loop->pre, indent_level+1, true);
            print_indents(sb, indent_level+1);
            sb->print("while (true) {\n");
            char *cond_name = c_print_expr(sb, for_loop->condition, indent_level+2);
            assert(cond_name);
            print_indents(sb, indent_level+2);
            sb->printf("if (!%s) { break; }\n", cond_name);
            c_print_block(sb, for_loop->body, indent_level + 2);
            c_print_statement(sb, for_loop->post, indent_level+2, true);
            print_indents(sb, indent_level+1);
            sb->print("}\n");
            print_indents(sb, indent_level);
            sb->print("}\n");
            break;
        }

        case AST_WHILE_LOOP: {
            Ast_While_Loop *while_loop = (Ast_While_Loop *)node;
            print_indents(sb, indent_level);
            sb->print("while (true) {\n");
            char *cond_name = c_print_expr(sb, while_loop->condition, indent_level+1);
            assert(cond_name);
            print_indents(sb, indent_level+1);
            sb->printf("if (!%s) { break; }\n", cond_name);
            c_print_block(sb, while_loop->body, indent_level + 1);
            print_indents(sb, indent_level);
            sb->print("}\n");
            break;
        }

        case AST_RETURN: {
            Ast_Return *ast_return = (Ast_Return *)node;
            char *expr_name = nullptr;
            if (ast_return->expr) {
                expr_name = c_print_expr(sb, ast_return->expr, indent_level, ast_return->matching_procedure->type->return_type);
            }
            print_indents(sb, indent_level);
            sb->print("return");
            if (expr_name) {
                sb->printf(" %s;\n", expr_name);
            }
            break;
        }

        case AST_BREAK: {
            print_indents(sb, indent_level);
            sb->print("break;\n");
            break;
        }

        case AST_CONTINUE: {
            print_indents(sb, indent_level);
            sb->print("continue;\n");
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

String_Builder generate_c_main_file(Ast_Block *global_scope) {
    // todo(josh): I think there's a bug in my String_Buffer implementation
    //             as this crashes on resize sometimes
    String_Builder sb = make_string_builder(default_allocator(), 50 * 1024);

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

    sb.print("typedef struct {\n");
    sb.print("    void *data;\n");
    sb.print("    i64 type;\n");
    sb.print("} Any;\n");

    sb.print("Any MAKE_ANY(void *data, i64 type) {\n");
    sb.print("    Any any;\n");
    sb.print("    any.data = data;\n");
    sb.print("    any.type = type;\n");
    sb.print("    return any;\n");
    sb.print("};\n");

    sb.print("void print_int(i64 i) {\n");
    sb.print("    printf(\"%lld\", i);\n");
    sb.print("}\n");
    sb.print("void print_float(float f) {\n");
    sb.print("    printf(\"%f\", f);\n");
    sb.print("}\n");
    sb.print("void print_bool(bool b) {\n");
    sb.print("    printf((b ? \"true\" : \"false\"));\n");
    sb.print("}\n");
    sb.print("void print_string(String string) {\n");
    sb.print("    for (i64 i = 0; i < string.count; i++) {\n");
    sb.print("        char c = string.data[i];\n");
    sb.print("        putchar(c);\n");
    sb.print("    }\n");
    sb.print("}\n");
    sb.print("void *alloc(i64 size) {\n");
    sb.print("    char *memory = (char *)malloc(size);\n");
    sb.print("    return memory;\n");
    sb.print("}\n");
    sb.print("void assert(bool condition) {\n");
    sb.print("    if (!condition) {\n");
    sb.print("        printf(\"Assertion failed.\");\n");
    sb.print("        *((char *)0) = 0;\n");
    sb.print("    }\n");
    sb.print("}\n");

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
                Proc_Declaration *proc_decl = (Proc_Declaration *)decl;
                if (!proc_decl->header->is_foreign) {
                    c_print_procedure_header(&sb, proc_decl->header);
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
                Struct_Declaration *struct_decl = (Struct_Declaration *)decl;
                Ast_Struct *structure = struct_decl->structure;
                sb.printf("struct %s {\n", structure->name);
                For (idx, structure->fields) {
                    Ast_Var *var = structure->fields[idx];
                    if (var->is_constant) {
                        continue;
                    }
                    c_print_var(&sb, var, 1);
                    sb.print(";\n");
                }
                sb.print("};\n");
                For (idx, structure->operator_overloads) {
                    c_print_procedure(&sb, structure->operator_overloads[idx]);
                }
                break;
            }
            case DECL_VAR: {
                Var_Declaration *var = (Var_Declaration *)decl;
                if (!var->var->is_constant) {
                    c_print_var(&sb, var->var, 0);
                    if (var->var->expr == nullptr) {
                        sb.printf(" = {0}");
                    }
                    sb.print(";\n");
                }
                break;
            }
            case DECL_PROC: {
                Proc_Declaration *proc_decl = (Proc_Declaration *)decl;
                if (!proc_decl->header->is_foreign) {
                    c_print_procedure(&sb, proc_decl->header->procedure);
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