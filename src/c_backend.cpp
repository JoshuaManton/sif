#include "c_backend.h"
#include "os_windows.h"

extern Timer g_global_timer;
extern bool g_no_type_info;
extern bool g_is_debug_build;

extern Ast_Proc *g_main_proc;

// todo(josh): @Multithreading is this an issue?
int g_total_num_temporaries_emitted = 0;

// todo(josh): @Multithreading is this an issue?
int g_num_short_circuit_labels = 0;

void c_print_type(Chunked_String_Builder *sb, Type *type, const char *var_name);
void c_print_type_plain(Chunked_String_Builder *sb, Type *type, const char *var_name);
char *c_print_expr(Chunked_String_Builder *sb, Ast_Expr *expr, int indent_level, Type *target_type = nullptr, bool is_reference_declaration = false);
void print_indents(Chunked_String_Builder *sb, int indent_level);
void c_print_procedure_header(Chunked_String_Builder *sb, Ast_Proc_Header *header);

void init_c_backend() {
}

void c_print_type_prefix(Chunked_String_Builder *sb, Type *type) {
    if (type == nullptr) {
        sb->printf("void ");
        return;
    }

    switch (type->kind) {
        case TYPE_PRIMITIVE: {
            Type_Primitive *type_primitive = (Type_Primitive *)type;
            if (type_primitive->name == g_interned_string_string) {
                sb->print("String ");
            }
            else if (type_primitive->name == g_interned_rawptr_string) {
                sb->print("void *");
            }
            else if (type_primitive->name == g_interned_any_string) {
                sb->print("Any ");
            }
            else if (type_primitive->name == g_interned_typeid_string) {
                sb->print("u64 ");
            }
            else if (type_primitive->name == g_interned_cstring_string) {
                sb->print("char *");
            }
            else {
                sb->printf("%s ", type_primitive->name);
            }
            break;
        }
        case TYPE_STRUCT: {
            Type_Struct *type_struct = (Type_Struct *)type;
            sb->printf("%s %s ", (type_struct->is_union ? "union" : "struct"), type_struct->name);
            break;
        }
        case TYPE_ENUM: {
            Type_Enum *enum_type = (Type_Enum *)type;
            assert(enum_type->base_type != nullptr);
            c_print_type_prefix(sb, enum_type->base_type);
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
        case TYPE_VARARGS: {
            Type_Varargs *varargs = (Type_Varargs *)type;
            if (varargs->is_c_varargs) {
                sb->print("...");
            }
            else {
                sb->print("Slice ");
            }
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

void c_print_line_directive(Chunked_String_Builder *sb, Location location, char *comment = "") {
    if (g_is_debug_build) {
        sb->printf("#line %d \"%s\"", location.line, location.filepath);
        if (comment != nullptr) {
            sb->printf(" // %s\n", comment);
        }
        else {
            sb->printf("\n");
        }
    }
}

void c_print_type_postfix(Chunked_String_Builder *sb, Type *type) {
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
        case TYPE_VARARGS: {
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

void c_print_type(Chunked_String_Builder *sb, Type *type, const char *var_name) {
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

void c_print_type_plain_prefix(Chunked_String_Builder *sb, Type *type) {
    if (type == nullptr) {
        sb->printf("void_");
        return;
    }

    switch (type->kind) {
        case TYPE_PRIMITIVE: {
            Type_Primitive *type_primitive = (Type_Primitive *)type;
            if (type_primitive->name == g_interned_string_string) {
                sb->print("String");
            }
            else if (type_primitive->name == g_interned_rawptr_string) {
                sb->print("voidpointer");
            }
            else if (type_primitive->name == g_interned_any_string) {
                sb->print("Any");
            }
            else if (type_primitive->name == g_interned_typeid_string) {
                sb->print("u64 ");
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
            sb->print("function_pointer_");
            c_print_type_plain_prefix(sb, type_procedure->return_type);
            break;
        }
        default: {
            assert(false);
        }
    }
}

void c_print_type_plain_postfix(Chunked_String_Builder *sb, Type *type) {
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
            sb->print("parameters_");
            For (idx, type_procedure->parameter_types) {
                c_print_type_plain(sb, type_procedure->parameter_types[idx], "");
                if (idx != (type_procedure->parameter_types.count-1)) {
                    sb->print("_");
                }
            }
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

void c_print_type_plain(Chunked_String_Builder *sb, Type *type, const char *var_name) {
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

void c_print_procedure_header(Chunked_String_Builder *sb, Ast_Proc_Header *header) {
    assert(header->declaration->name != nullptr);
    Chunked_String_Builder header_name_sb = make_chunked_string_builder(g_global_linear_allocator, 128);
    header_name_sb.print(header->declaration->link_name);
    header_name_sb.print("(");
    For (idx, header->parameters) {
        Ast_Var *parameter = header->parameters[idx];
        assert(!parameter->is_constant);
        assert(!parameter->is_polymorphic_value);
        if (idx != 0) {
            header_name_sb.print(", ");
        }
        const char *name_to_print = parameter->declaration->link_name;
        if (is_type_varargs(parameter->type)) {
            Type_Varargs *varargs = (Type_Varargs *)parameter->type;
            if (varargs->is_c_varargs) {
                name_to_print = "";
            }
        }
        c_print_type(&header_name_sb, parameter->type, name_to_print);
    }
    header_name_sb.print(")");
    if (header->procedure == g_main_proc) {
        sb->printf("i32 %s", header_name_sb.make_string());
    }
    else {
        c_print_type(sb, header->type->return_type, header_name_sb.make_string());
    }
}

char *c_temporary() {
    g_total_num_temporaries_emitted += 1;
    Chunked_String_Builder sb = make_chunked_string_builder(g_global_linear_allocator, 128); // todo(josh): this is very dumb. use an arena or something
    sb.printf("__t%d", g_total_num_temporaries_emitted);
    return sb.make_string();
}

void c_print_procedure_call_parameters(Chunked_String_Builder *sb, Ast_Expr *root_expr, Type_Procedure *proc_type, Array<Procedure_Call_Parameter> parameters, int indent_level, Array<char *> *out_temporaries) {
    assert(parameters.count == proc_type->parameter_types.count);
    For (idx, parameters) {
        Procedure_Call_Parameter call_parameter = parameters[idx];
        Type *target_type = proc_type->parameter_types[idx];
        if (!is_type_varargs(target_type)) {
            assert(call_parameter.exprs.count == 1);
            // c_print_line_directive(sb, call_parameter.exprs[0]->location, "proc call parameter");
            char *param = c_print_expr(sb, call_parameter.exprs[0], indent_level, target_type);
            out_temporaries->append(param);
        }
        else {
            Type_Varargs *varargs = (Type_Varargs *)target_type;
            assert(idx == proc_type->parameter_types.count-1);
            int varargs_count = call_parameter.exprs.count;
            if (varargs_count == 1 && (unparen_expr(call_parameter.exprs[0])->expr_kind == EXPR_SPREAD)) {
                assert(!varargs->is_c_varargs);
                Expr_Spread *spread = (Expr_Spread *)unparen_expr(call_parameter.exprs[0]);
                char *spread_name = nullptr;
                if (is_type_array(spread->rhs->operand.type)) {
                    Type_Array *array_type = (Type_Array *)spread->rhs->operand.type;
                    char *rhs_name = c_print_expr(sb, spread->rhs, indent_level);
                    spread_name = c_temporary();
                    print_indents(sb, indent_level);
                    c_print_line_directive(sb, spread->location, "proc call slice spread");
                    sb->printf("Slice %s = MAKE_SLICE(&%s.elements[0], %d);\n", spread_name, rhs_name, array_type->count);
                }
                else {
                    assert(is_type_slice(spread->rhs->operand.type) || is_type_varargs(spread->rhs->operand.type));
                    spread_name = c_print_expr(sb, spread->rhs, indent_level);
                }
                assert(spread_name != nullptr);
                out_temporaries->append(spread_name);
            }
            else {
                if (varargs->is_c_varargs) {
                    For (idx, call_parameter.exprs) {
                        Ast_Expr *param = call_parameter.exprs[idx];
                        char *param_name = c_print_expr(sb, param, indent_level);
                        out_temporaries->append(param_name);
                    }
                }
                else {
                    char *t = c_temporary();
                    c_print_line_directive(sb, root_expr->location, "proc call vararg slice decl");
                    print_indents(sb, indent_level);
                    c_print_type(sb, varargs->varargs_of, t);
                    sb->printf("[%d];\n", max(1, varargs_count));
                    For (idx, call_parameter.exprs) {
                        Ast_Expr *param = call_parameter.exprs[idx];
                        char *param_name = c_print_expr(sb, param, indent_level, varargs->varargs_of);
                        c_print_line_directive(sb, param->location, "proc call vararg expr");
                        print_indents(sb, indent_level);
                        sb->printf("%s[%d] = %s;\n", t, idx, param_name);
                    }
                    char *slice = c_temporary();
                    c_print_line_directive(sb, root_expr->location, "proc call vararg slice decl");
                    print_indents(sb, indent_level);
                    c_print_type(sb, get_or_create_type_slice_of(varargs->varargs_of), slice);
                    sb->print(";\n");
                    c_print_line_directive(sb, root_expr->location, "proc call vararg slice set data");
                    print_indents(sb, indent_level);
                    sb->printf("%s.data = %s;\n", slice, t);
                    c_print_line_directive(sb, root_expr->location, "proc call vararg slice set count");
                    print_indents(sb, indent_level);
                    sb->printf("%s.count = %d;\n", slice, varargs_count);
                    out_temporaries->append(slice);
                }
            }
            return;
        }
    }
}

void c_print_hidden_using_selectors(Chunked_String_Builder *sb, Declaration *decl, int indent_level) {
    Declaration *from_using = decl;
    while (from_using->kind == DECL_USING) {
        Using_Declaration *using_decl = (Using_Declaration *)from_using;
        Type *lhs_type = nullptr;
        assert(using_decl->from_using);
        assert(using_decl->from_using->kind == DECL_VAR);
        Var_Declaration *var_decl = (Var_Declaration *)using_decl->from_using;
        sb->printf("%s", using_decl->from_using->name);
        lhs_type = var_decl->var->type;
        assert(lhs_type != nullptr);
        if (is_type_pointer(lhs_type)) {
            sb->print("->");
        }
        else {
            sb->print(".");
        }
        from_using = using_decl->declaration;
    }
}

char *c_print_gen_location(Chunked_String_Builder *sb, int indent_level, Location location) {
    char *t = c_temporary();
    c_print_line_directive(sb, location, "gen Source_Code_Location");
    print_indents(sb, indent_level);
    sb->printf("struct Source_Code_Location %s = {0};\n", t);
    c_print_line_directive(sb, location, "gen Source_Code_Location");
    print_indents(sb, indent_level);
    sb->printf("%s.filepath = MAKE_STRING(\"%s\", %d);\n", t, location.filepath, strlen(location.filepath));
    c_print_line_directive(sb, location, "gen Source_Code_Location");
    print_indents(sb, indent_level);
    sb->printf("%s.line = %d;\n", t, location.line);
    c_print_line_directive(sb, location, "gen Source_Code_Location");
    print_indents(sb, indent_level);
    sb->printf("%s.character = %d;\n", t, location.character);
    return t;
}

void c_print_null_check(Chunked_String_Builder *sb, int indent_level, char *ptr_name, Location location) {
    char *location_t = c_print_gen_location(sb, indent_level, location);
    c_print_line_directive(sb, location, "null check");
    print_indents(sb, indent_level);
    sb->printf("sif_null_check(%s, %s);\n", ptr_name, location_t);
    c_print_line_directive(sb, location, "null check after");
}

void c_print_bounds_check(Chunked_String_Builder *sb, int indent_level, char *lhs_name, char *count_field, char *index_name, Location location) {
    char *location_t = c_print_gen_location(sb, indent_level, location);
    c_print_line_directive(sb, location, "bounds check");
    print_indents(sb, indent_level);
    sb->printf("sif_bounds_check(%s.%s, %s, %s);\n", lhs_name, count_field, index_name, location_t);
    c_print_line_directive(sb, location, "bounds check after");
}

char *c_print_procedure_call(Chunked_String_Builder *_sb, Ast_Expr *root_expr, const char *procedure_name, Array<Procedure_Call_Parameter> parameters, int indent_level, Type_Procedure *proc_type) {
    Chunked_String_Builder call_sb = make_chunked_string_builder(g_global_linear_allocator, 128);
    Array<char *> params;
    params.allocator = g_global_linear_allocator;
    c_print_procedure_call_parameters(_sb, root_expr, proc_type, parameters, indent_level, &params);
    call_sb.printf("%s(", procedure_name);
    For (idx, params) {
        if (idx != 0) {
            call_sb.print(", ");
        }
        call_sb.print(params[idx]);
    }
    call_sb.print(")");

    if (proc_type->return_type) {
        char *t = c_temporary();
        print_indents(_sb, indent_level);
        c_print_type(_sb, proc_type->return_type, t);
        _sb->printf(" = %s;\n", call_sb.make_string());
        return t;
    }
    else {
        return call_sb.make_string();
    }
}

char *c_print_expr(Chunked_String_Builder *_sb, Ast_Expr *expr, int indent_level, Type *target_type, bool is_reference_declaration) {
    Chunked_String_Builder expr_sb = make_chunked_string_builder(g_global_linear_allocator, 128);

    if (expr->desugared_procedure_to_call != nullptr) {
        char *call_expr = c_print_procedure_call(_sb, expr, expr->desugared_procedure_to_call->header->declaration->link_name, expr->processed_procedure_call_parameters, indent_level, expr->desugared_procedure_to_call->header->type);
        expr_sb.print(call_expr);
    }
    else {
        if (expr->operand.flags & OPERAND_CONSTANT) {
            assert(!is_type_untyped(expr->operand.type));
            if (is_type_float(expr->operand.type)) {
                expr_sb.printf("%f", expr->operand.float_value);
            }
            else if (is_type_integer(expr->operand.type)) {
                // todo(josh): handle uints
                expr_sb.printf("%d", expr->operand.int_value);
            }
            else if (expr->operand.type == type_bool) {
                expr_sb.print(expr->operand.bool_value ? "true" : "false");
            }
            else if (expr->operand.type == type_string) {
                expr_sb.printf("MAKE_STRING(\"%s\", %d)", expr->operand.scanned_string_value, expr->operand.escaped_string_length);
            }
            else if (expr->operand.type == type_cstring) {
                expr_sb.printf("\"%s\"", expr->operand.scanned_string_value);
            }
            else if (is_type_procedure(expr->operand.type)) {
                expr_sb.print(expr->operand.proc_value->declaration->link_name);
            }
            else if (is_type_typeid(expr->operand.type)) {
                expr_sb.printf("%d", expr->operand.type_value->id);
            }
            else if (expr->operand.type->flags & TF_POINTER) {
                expr_sb.printf("((");
                c_print_type(&expr_sb, expr->operand.type, "");
                expr_sb.printf(")%d)", expr->operand.int_value);
            }
            else {
                assert(false);
            }
        }
        else if (is_type_typeid(expr->operand.type) && (expr->operand.flags & OPERAND_CONSTANT)) {
            expr_sb.printf("%d", expr->operand.type_value->id);
        }
        else {
            switch (expr->expr_kind) {
                case EXPR_IDENTIFIER: {
                    Expr_Identifier *identifier = (Expr_Identifier *)expr;
                    Chunked_String_Builder ident_sb = make_chunked_string_builder(g_global_linear_allocator, 128);
                    c_print_hidden_using_selectors(&expr_sb, identifier->resolved_declaration, indent_level);
                    expr_sb.print(identifier->resolved_declaration->link_name);
                    break;
                }
                case EXPR_UNARY: {
                    Expr_Unary *unary = (Expr_Unary *)expr;
                    char *rhs = c_print_expr(_sb, unary->rhs, indent_level);
                    switch (unary->op) {
                        case TK_PLUS: {
                            expr_sb.print("+");
                            break;
                        }
                        case TK_MINUS: {
                            expr_sb.print("-");
                            break;
                        }
                        case TK_NOT: {
                            expr_sb.print("!");
                            break;
                        }
                        case TK_BIT_NOT: {
                            expr_sb.print("~");
                            break;
                        }
                        default: {
                            assert(false);
                        }
                    }
                    expr_sb.printf("%s", rhs);
                    break;
                }
                case EXPR_BINARY: {
                    Expr_Binary *binary = (Expr_Binary *)expr;
                    if (binary->op == TK_EQUAL_TO && (binary->lhs->operand.type == type_string) && (binary->lhs->operand.type == type_string)) {
                        char *lhs = c_print_expr(_sb, binary->lhs, indent_level); assert(lhs);
                        char *rhs = c_print_expr(_sb, binary->rhs, indent_level); assert(rhs);
                        expr_sb.printf("string_eq(%s, %s);\n", lhs, rhs);
                    }
                    else {
                        // todo(josh): compress short circuit AND and OR together
                        if (binary->op == TK_BOOLEAN_AND) {
                            g_num_short_circuit_labels += 1;
                            String_Builder label_sb = make_string_builder(g_global_linear_allocator, 16);
                            label_sb.printf("__short_curcuit_AND_%d", g_num_short_circuit_labels);
                            char *result = c_temporary();
                            c_print_line_directive(_sb, expr->location);
                            print_indents(_sb, indent_level);
                            c_print_type(_sb, type_bool, result);
                            _sb->printf(" = false;\n");
                            char *lhs = c_print_expr(_sb, binary->lhs, indent_level); assert(lhs);
                            c_print_line_directive(_sb, expr->location);
                            print_indents(_sb, indent_level);
                            _sb->printf("if (%s == false) { goto %s; }\n", lhs, label_sb.string());
                            char *rhs = c_print_expr(_sb, binary->rhs, indent_level); assert(rhs);
                            c_print_line_directive(_sb, expr->location);
                            print_indents(_sb, indent_level);
                            _sb->printf("if (%s == false) { goto %s; }\n", rhs, label_sb.string());
                            c_print_line_directive(_sb, expr->location);
                            print_indents(_sb, indent_level);
                            _sb->printf("%s = true;\n", result);
                            c_print_line_directive(_sb, expr->location);
                            print_indents(_sb, indent_level);
                            _sb->printf("%s:;\n", label_sb.string());
                            expr_sb.print(result);
                        }
                        else if (binary->op == TK_BOOLEAN_OR) {
                            g_num_short_circuit_labels += 1;
                            String_Builder label_sb = make_string_builder(g_global_linear_allocator, 16);
                            label_sb.printf("__short_curcuit_OR_%d", g_num_short_circuit_labels);
                            char *result = c_temporary();
                            c_print_line_directive(_sb, expr->location);
                            print_indents(_sb, indent_level);
                            c_print_type(_sb, type_bool, result);
                            _sb->printf(" = true;\n");
                            char *lhs = c_print_expr(_sb, binary->lhs, indent_level); assert(lhs);
                            c_print_line_directive(_sb, expr->location);
                            print_indents(_sb, indent_level);
                            _sb->printf("if (%s == true) { goto %s; }\n", lhs, label_sb.string());
                            char *rhs = c_print_expr(_sb, binary->rhs, indent_level); assert(rhs);
                            c_print_line_directive(_sb, expr->location);
                            print_indents(_sb, indent_level);
                            _sb->printf("if (%s == true) { goto %s; }\n", rhs, label_sb.string());
                            c_print_line_directive(_sb, expr->location);
                            print_indents(_sb, indent_level);
                            _sb->printf("%s = false;\n", result);
                            c_print_line_directive(_sb, expr->location);
                            print_indents(_sb, indent_level);
                            _sb->printf("%s:;\n", label_sb.string());
                            expr_sb.print(result);
                        }
                        else {
                            char *lhs = c_print_expr(_sb, binary->lhs, indent_level); assert(lhs);
                            char *rhs = c_print_expr(_sb, binary->rhs, indent_level); assert(rhs);
                            expr_sb.print(lhs);
                            switch (binary->op) {
                                case TK_PLUS:                     { expr_sb.print(" + ");  break; }
                                case TK_MINUS:                    { expr_sb.print(" - ");  break; }
                                case TK_MULTIPLY:                 { expr_sb.print(" * ");  break; }
                                case TK_DIVIDE:                   { expr_sb.print(" / ");  break; }
                                case TK_MOD:                      { expr_sb.print(" % ");  break; }
                                case TK_LESS_THAN:                { expr_sb.print(" < ");  break; }
                                case TK_LESS_THAN_OR_EQUAL:       { expr_sb.print(" <= "); break; }
                                case TK_GREATER_THAN:             { expr_sb.print(" > ");  break; }
                                case TK_GREATER_THAN_OR_EQUAL:    { expr_sb.print(" >= "); break; }
                                case TK_EQUAL_TO:                 { expr_sb.print(" == "); break; }
                                case TK_NOT_EQUAL_TO:             { expr_sb.print(" != "); break; }
                                case TK_AMPERSAND:                { expr_sb.print(" & ");  break; }
                                case TK_BIT_OR:                   { expr_sb.print(" | ");  break; }
                                case TK_BOOLEAN_AND:              { expr_sb.print(" && "); break; }
                                case TK_BOOLEAN_OR:               { expr_sb.print(" || "); break; }
                                case TK_LEFT_SHIFT:               { expr_sb.print(" << "); break; }
                                case TK_RIGHT_SHIFT:              { expr_sb.print(" >> "); break; }
                                default: {
                                    assert(false);
                                }
                            }
                            expr_sb.printf("%s", rhs);
                        }
                    }
                    break;
                }
                case EXPR_PROCEDURE_CALL: {
                    Expr_Procedure_Call *call = (Expr_Procedure_Call *)expr;
                    char *procedure_name = c_print_expr(_sb, call->lhs, indent_level);
                    char *call_expr = c_print_procedure_call(_sb, expr, procedure_name, call->processed_procedure_call_parameters, indent_level, call->target_procedure_type);
                    expr_sb.print(call_expr);
                    break;
                }
                case EXPR_SUBSCRIPT: {
                    Expr_Subscript *subscript = (Expr_Subscript *)expr;
                    char *lhs = c_print_expr(_sb, subscript->lhs, indent_level);
                    char *index = c_print_expr(_sb, subscript->index, indent_level);
                    if (is_type_slice(subscript->lhs->operand.type) || is_type_varargs(subscript->lhs->operand.type)) {
                        Type_Slice *slice_type = nullptr;
                        if (is_type_slice(subscript->lhs->operand.type)) {
                            slice_type = (Type_Slice *)subscript->lhs->operand.type;
                        }
                        else if (is_type_varargs(subscript->lhs->operand.type)) {
                            slice_type = ((Type_Varargs *)subscript->lhs->operand.type)->slice_type;
                        }
                        c_print_bounds_check(_sb, indent_level, lhs, "count", index, subscript->location);
                        assert(slice_type != nullptr);
                        expr_sb.print("((");
                        c_print_type(&expr_sb, slice_type->data_pointer_type, "");
                        expr_sb.print(")");
                        expr_sb.print(lhs);
                        expr_sb.print(".data)[");
                        expr_sb.print(index);
                        expr_sb.print("]");
                    }
                    else if (is_type_array(subscript->lhs->operand.type)) {
                        // todo(josh): assert not out of bounds
                        expr_sb.printf("%s.elements[%s]", lhs, index);
                    }
                    else if (subscript->lhs->operand.type == type_string) {
                        c_print_bounds_check(_sb, indent_level, lhs, "count", index, subscript->location);
                        expr_sb.printf("%s.data[%s]", lhs, index);
                    }
                    else {
                        assert(false);
                    }
                    break;
                }
                case EXPR_COMPOUND_LITERAL: {
                    Expr_Compound_Literal *compound_literal = (Expr_Compound_Literal *)expr;
                    Array<char *> expr_names;
                    expr_names.allocator = g_global_linear_allocator;
                    For (idx, compound_literal->exprs) {
                        char *name = c_print_expr(_sb, compound_literal->exprs[idx], indent_level);
                        expr_names.append(name);
                    }
                    char *t = c_temporary();
                    c_print_line_directive(_sb, compound_literal->location, "compound literal decl");
                    print_indents(_sb, indent_level);
                    c_print_type(_sb, compound_literal->operand.type, t);
                    _sb->print(" = {0};\n");

                    Type *compound_literal_type = compound_literal->operand.type;
                    if (is_type_array(compound_literal_type)) {
                        Type_Array *array_type = (Type_Array *)compound_literal_type;
                        For (idx, expr_names) {
                            c_print_line_directive(_sb, compound_literal->location, "compound literal assignment");
                            print_indents(_sb, indent_level);
                            _sb->printf("%s.elements[%d] = %s;\n", t, idx, expr_names[idx]);
                        }
                    }
                    else {
                        For (idx, expr_names) {
                            c_print_line_directive(_sb, compound_literal->location, "compound literal assignment");
                            print_indents(_sb, indent_level);
                            Struct_Field target_field = compound_literal_type->variable_fields[idx];
                            assert(!(target_field.operand.flags & OPERAND_CONSTANT));
                            assert(target_field.operand.flags & OPERAND_LVALUE);
                            _sb->printf("%s.%s = %s;\n", t, target_field.name, expr_names[idx]);
                        }
                    }
                    expr_sb.printf("%s", t);
                    break;
                }
                case EXPR_ADDRESS_OF: {
                    Expr_Address_Of *address_of = (Expr_Address_Of *)expr;
                    char *rhs = c_print_expr(_sb, address_of->rhs, indent_level);
                    expr_sb.printf("&%s", rhs);
                    break;
                }
                case EXPR_DEREFERENCE: {
                    Expr_Dereference *dereference = (Expr_Dereference *)expr;
                    char *lhs = c_print_expr(_sb, dereference->lhs, indent_level);
                    c_print_null_check(_sb, indent_level, lhs, dereference->lhs->location);
                    expr_sb.printf("(*%s)", lhs);
                    break;
                }
                case EXPR_SELECTOR: {
                    Expr_Selector *selector = (Expr_Selector *)expr;
                    c_print_line_directive(_sb, selector->location, "expr selector");
                    char *lhs = c_print_expr(_sb, selector->lhs, indent_level);
                    assert(lhs);
                    bool is_accessing_slice_data_field = false;
                    if ((selector->lookup.type_with_field->kind == TYPE_SLICE) && (selector->field_name == g_interned_data_string)) {
                        is_accessing_slice_data_field = true;
                        Type_Slice *slice_type = (Type_Slice *)selector->lookup.type_with_field;
                        expr_sb.print("*((");
                        c_print_type(&expr_sb, get_or_create_type_pointer_to(slice_type->data_pointer_type), "");
                        expr_sb.print(")&");
                    }
                    expr_sb.print(lhs);
                    if (is_type_pointer(selector->lhs->operand.type)) {
                        c_print_null_check(_sb, indent_level, lhs, selector->lhs->location);
                        expr_sb.print("->");
                    }
                    else {
                        expr_sb.print(".");
                    }
                    Declaration *right_most_decl = selector->lookup.declaration;
                    c_print_hidden_using_selectors(&expr_sb, right_most_decl, indent_level);
                    expr_sb.printf("%s", right_most_decl->name);
                    if (is_accessing_slice_data_field) {
                        expr_sb.print(")");
                    }
                    break;
                }
                case EXPR_CAST: {
                    Expr_Cast *expr_cast = (Expr_Cast *)expr;
                    char *rhs = c_print_expr(_sb, expr_cast->rhs, indent_level);
                    expr_sb.print("((");
                    c_print_type(&expr_sb, expr_cast->type_expr->operand.type_value, "");
                    expr_sb.printf(")%s)", rhs);
                    break;
                }
                case EXPR_TRANSMUTE: {
                    Expr_Transmute *transmute = (Expr_Transmute *)expr;
                    char *rhs = c_print_expr(_sb, transmute->rhs, indent_level);
                    expr_sb.print("*((");
                    c_print_type(&expr_sb, get_or_create_type_pointer_to(transmute->type_expr->operand.type_value), "");
                    expr_sb.printf(")&%s)", rhs);
                    break;
                }
                case EXPR_PAREN: {
                    Expr_Paren *paren = (Expr_Paren *)expr;
                    char *nested = c_print_expr(_sb, paren->nested, indent_level, target_type);
                    expr_sb.printf("%s", nested);
                    break;
                }
                case EXPR_NULL: {
                    expr_sb.print("NULL");
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
                    assert(false);
                    break;
                }
                case EXPR_ARRAY_TYPE: {
                    assert(false);
                    break;
                }
                case EXPR_SIZEOF: {
                    assert(false);
                    break;
                }
                case EXPR_TYPEOF: {
                    assert(false);
                    break;
                }
                default: {
                    printf("unhandled case: %d\n", expr->expr_kind);
                    assert(false);
                }
            }
        }
    }

    Chunked_String_Builder tsb = make_chunked_string_builder(g_global_linear_allocator, 32);

    c_print_line_directive(_sb, expr->location);
    print_indents(_sb, indent_level);
    char *t = c_temporary();
    tsb.printf("(");
    if (expr->operand.flags & OPERAND_LVALUE) {
        tsb.printf("*");
        if (expr->operand.reference_type == nullptr) {
            c_print_type(_sb, get_or_create_type_pointer_to(expr->operand.type), t);
        }
        else {
            // todo(josh): do while loop to assign through reference-to-reference stuff
            if (!is_reference_declaration) {
                tsb.printf("*");
            }
            c_print_type(_sb, get_or_create_type_pointer_to(expr->operand.reference_type), t);
        }
        _sb->printf(" = &%s;\n", expr_sb.make_string());
    }
    else if (expr->operand.flags & OPERAND_RVALUE) {
        c_print_type(_sb, expr->operand.type, t);
        _sb->printf(" = %s;\n", expr_sb.make_string());
    }
    else {
        assert(expr->operand.flags & OPERAND_NO_VALUE);
        _sb->printf("%s", expr_sb.make_string());
    }
    tsb.printf("%s)", t);

    if (target_type) {
        if (target_type == type_any) {
            if (expr->operand.type != type_any) {
                Chunked_String_Builder any_sb = make_chunked_string_builder(g_global_linear_allocator, 64);
                any_sb.printf("MAKE_ANY(&%s, %d)", tsb.make_string(), expr->operand.type->id);
                tsb.clear();
                tsb.print(any_sb.make_string());
            }
        }
        else if (is_type_reference(target_type)) {
            Chunked_String_Builder reference_sb = make_chunked_string_builder(g_global_linear_allocator, 64);
            reference_sb.printf("&%s", tsb.make_string());
            tsb.clear();
            tsb.print(reference_sb.make_string());
        }
    }

    return tsb.make_string();
}

void print_indents(Chunked_String_Builder *sb, int indent_level) {
    if (g_is_debug_build) {
        for (int i = 0; i < indent_level; i++) {
            sb->print("    ");
        }
    }
}

void c_print_block(Chunked_String_Builder *sb, Ast_Block *block, int indent_level);

void c_print_var_statement(Chunked_String_Builder *sb, Ast_Var *var, int indent_level) {
    if (!var->is_constant) {
        char *rhs = nullptr;
        if (var->expr) {
            rhs = c_print_expr(sb, var->expr, indent_level, var->type);
        }
        c_print_line_directive(sb, var->location, "var decl");
        print_indents(sb, indent_level);
        c_print_type(sb, var->type, var->declaration->name);
        sb->print(";\n");
        char *lhs = c_print_expr(sb, var->name_expr, indent_level, nullptr, var->name_expr->operand.reference_type != nullptr);
        c_print_line_directive(sb, var->location, "var decl");
        print_indents(sb, indent_level);
        if (rhs) {
            sb->printf("%s = %s;\n", lhs, rhs);
        }
        else {
            sb->printf("memset(&%s, 0, %d);\n", lhs, var->type->size);
        }
    }
    else {
        sb->printf("// constant declaration omitted: %s\n", var->name);
    }
}

void c_print_statement(Chunked_String_Builder *sb, Ast_Block *block, Ast_Node *node, int indent_level);

void c_print_defers_from_block_to_block(Chunked_String_Builder *sb, int indent_level, Ast_Block *from, Ast_Block *to) {
    Ast_Block *current = from;
    while (true) {
        for (int idx = current->c_gen_defer_stack.count-1; idx >= 0; idx -= 1) {
            Ast_Defer *ast_defer = (Ast_Defer *)current->c_gen_defer_stack[idx];
            print_indents(sb, indent_level);
            sb->printf("// defer %d\n", idx);
            c_print_statement(sb, current, ast_defer->node_to_defer, indent_level);
        }
        if (current == to) {
            break;
        }
        current = current->parent_block;
    }
}

void c_print_statement(Chunked_String_Builder *sb, Ast_Block *block, Ast_Node *node, int indent_level) {
    switch (node->ast_kind) {
        case AST_VAR: {
            Ast_Var *var = (Ast_Var *)node;
            c_print_var_statement(sb, var, indent_level);
            break;
        }

        case AST_USING: {
            Ast_Using *ast_using = (Ast_Using *)node;
            assert(ast_using->expr);
            break;
        }

        case AST_ASSIGN: {
            Ast_Assign *assign = (Ast_Assign *)node;
            char *assign_op = nullptr;
            switch (assign->op) {
                case TK_ASSIGN:             assign_op = " = ";   break;
                case TK_PLUS_ASSIGN:        assign_op = " += ";  break;
                case TK_MINUS_ASSIGN:       assign_op = " -= ";  break;
                case TK_MULTIPLY_ASSIGN:    assign_op = " *= ";  break;
                case TK_DIVIDE_ASSIGN:      assign_op = " /= ";  break;
                case TK_MOD_ASSIGN:         assign_op = " %= ";  break;
                case TK_LEFT_SHIFT_ASSIGN:  assign_op = " <<= "; break;
                case TK_RIGHT_SHIFT_ASSIGN: assign_op = " >>= "; break;
                case TK_BIT_AND_ASSIGN:     assign_op = " &= ";  break;
                case TK_BIT_OR_ASSIGN:      assign_op = " |= ";  break;
                case TK_BOOLEAN_AND_ASSIGN: assign_op = " &&= "; break;
                case TK_BOOLEAN_OR_ASSIGN:  assign_op = " ||= "; break;
                default: {
                    assert(false);
                }
            }
            assert(assign->lhs->exprs.count == assign->rhs->exprs.count);
            Array<char *> temps;
            temps.allocator = g_global_linear_allocator;
            For (idx, assign->rhs->exprs) {
                Ast_Expr *rhs_expr = assign->rhs->exprs[idx];
                char *rhs = c_print_expr(sb, rhs_expr, indent_level, rhs_expr->operand.type);
                char *t = c_temporary();
                c_print_line_directive(sb, assign->location, "assign statement");
                print_indents(sb, indent_level);
                c_print_type(sb, rhs_expr->operand.type, t);
                sb->printf(" = %s;\n", rhs);
                temps.append(t);
            }
            assert(temps.count == assign->lhs->exprs.count);
            For (idx, assign->lhs->exprs) {
                Ast_Expr *lhs_expr = assign->lhs->exprs[idx];
                char *lhs = c_print_expr(sb, lhs_expr, indent_level);
                char *rhs = temps[idx];
                c_print_line_directive(sb, assign->location, "assign statement");
                print_indents(sb, indent_level);
                sb->print(lhs);
                sb->print(assign_op);
                sb->print(rhs);
                sb->print(";\n");
            }
            break;
        }

        case AST_STATEMENT_EXPR: {
            Ast_Statement_Expr *statement = (Ast_Statement_Expr *)node;
            c_print_expr(sb, statement->expr, indent_level);
            sb->print(";\n");
            break;
        }

        case AST_BLOCK_STATEMENT: {
            Ast_Block_Statement *block_statement = (Ast_Block_Statement *)node;
            c_print_line_directive(sb, block_statement->location, "block statement");
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
            c_print_line_directive(sb, ast_if->location, "if block");
            print_indents(sb, indent_level);
            sb->print("{\n");
            indent_level += 1;
            if (ast_if->pre_statement) {
                c_print_statement(sb, ast_if->if_block, ast_if->pre_statement, indent_level);
            }
            // c_print_line_directive(sb, ast_if->location, "if statement expr");
            char *cond_name = c_print_expr(sb, ast_if->condition, indent_level);
            c_print_line_directive(sb, ast_if->location, "if statement");
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
            indent_level -= 1;
            print_indents(sb, indent_level);
            sb->print("}\n");
            break;
        }

        case AST_FOR_LOOP: {
            Ast_For_Loop *for_loop = (Ast_For_Loop *)node;
            c_print_line_directive(sb, for_loop->location, "for loop block");
            print_indents(sb, indent_level);
            sb->print("{\n");
            c_print_line_directive(sb, for_loop->location, "for loop pre");
            c_print_statement(sb, block, for_loop->pre, indent_level+1);
            print_indents(sb, indent_level+1);
            c_print_line_directive(sb, for_loop->location, "for loop while");
            sb->print("while (true) {\n");
            char *cond_name = c_print_expr(sb, for_loop->condition, indent_level+2);
            assert(cond_name);
            c_print_line_directive(sb, for_loop->location, "for loop condition");
            print_indents(sb, indent_level+2);
            sb->printf("if (!%s) { break; }\n", cond_name);
            c_print_line_directive(sb, for_loop->body->location, "for loop body");
            c_print_block(sb, for_loop->body, indent_level + 2);
            c_print_line_directive(sb, for_loop->location, "for loop post");
            c_print_statement(sb, block, for_loop->post, indent_level+2);
            print_indents(sb, indent_level+1);
            sb->print("}\n");
            print_indents(sb, indent_level);
            sb->print("}\n");
            break;
        }

        case AST_WHILE_LOOP: {
            Ast_While_Loop *while_loop = (Ast_While_Loop *)node;
            c_print_line_directive(sb, while_loop->location, "while loop");
            print_indents(sb, indent_level);
            sb->print("while (true) {\n");
            char *cond_name = c_print_expr(sb, while_loop->condition, indent_level+1);
            assert(cond_name);
            c_print_line_directive(sb, while_loop->condition->location, "while condition");
            print_indents(sb, indent_level+1);
            sb->printf("if (!%s) { break; }\n", cond_name);
            c_print_line_directive(sb, while_loop->body->location, "while body");
            c_print_block(sb, while_loop->body, indent_level + 1);
            print_indents(sb, indent_level);
            sb->print("}\n");
            break;
        }

        case AST_RETURN: {
            Ast_Return *ast_return = (Ast_Return *)node;
            char *t = nullptr;
            if (ast_return->expr) {
                c_print_line_directive(sb, ast_return->expr->location, "return expr");
                char *expr_name = c_print_expr(sb, ast_return->expr, indent_level, ast_return->matching_procedure->type->return_type);
                // note(josh): extra value copy for defer
                t = c_temporary();
                c_print_line_directive(sb, ast_return->expr->location, "return expr value copy");
                print_indents(sb, indent_level);
                c_print_type(sb, ast_return->matching_procedure->type->return_type, t);
                sb->printf(" = %s;\n", expr_name);
            }
            assert(ast_return->matching_procedure->procedure != nullptr);
            assert(ast_return->matching_procedure->procedure->body != nullptr);
            c_print_defers_from_block_to_block(sb, indent_level, block, ast_return->matching_procedure->procedure->body);
            c_print_line_directive(sb, ast_return->location, "return statement");
            print_indents(sb, indent_level);
            sb->print("return");
            if (t) {
                sb->printf(" %s", t);
            }
            sb->print(";\n");
            break;
        }

        case AST_BREAK: {
            Ast_Break *ast_break = (Ast_Break *)node;
            Ast_Block *loop_block = nullptr;
            if (ast_break->matching_loop->ast_kind == AST_FOR_LOOP) {
                Ast_For_Loop *for_loop = (Ast_For_Loop *)ast_break->matching_loop;
                loop_block = for_loop->body;
            }
            else {
                assert(ast_break->matching_loop->ast_kind == AST_WHILE_LOOP);
                Ast_While_Loop *while_loop = (Ast_While_Loop *)ast_break->matching_loop;
                loop_block = while_loop->body;
            }
            assert(loop_block != nullptr);
            c_print_defers_from_block_to_block(sb, indent_level, block, loop_block);
            c_print_line_directive(sb, ast_break->location, "break statement");
            print_indents(sb, indent_level);
            sb->print("break;\n");
            break;
        }

        case AST_CONTINUE: {
            Ast_Continue *ast_continue = (Ast_Continue *)node;
            assert(ast_continue->matching_loop);
            Ast_Block *loop_block = nullptr;
            if (ast_continue->matching_loop->ast_kind == AST_FOR_LOOP) {
                Ast_For_Loop *for_loop = (Ast_For_Loop *)ast_continue->matching_loop;
                if (for_loop->post) {
                    c_print_statement(sb, block, for_loop->post, indent_level);
                }
                loop_block = for_loop->body;
            }
            else {
                assert(ast_continue->matching_loop->ast_kind == AST_WHILE_LOOP);
                Ast_While_Loop *while_loop = (Ast_While_Loop *)ast_continue->matching_loop;
                loop_block = while_loop->body;
            }
            assert(loop_block != nullptr);
            c_print_defers_from_block_to_block(sb, indent_level, block, loop_block);
            c_print_line_directive(sb, ast_continue->location, "continue statement");
            print_indents(sb, indent_level);
            sb->print("continue;\n");
            break;
        }

        case AST_DEFER: {
            Ast_Defer *ast_defer = (Ast_Defer *)node;
            block->c_gen_defer_stack.append(ast_defer);
            break;
        }

        case AST_DIRECTIVE_ASSERT: {
            break;
        }

        case AST_DIRECTIVE_PRINT: {
            break;
        }

        default: {
            assert(false);
            break;
        }
    }
}

void c_print_block(Chunked_String_Builder *sb, Ast_Block *block, int indent_level) {
    For (idx, block->nodes) {
        Ast_Node *node = block->nodes[idx];
        c_print_statement(sb, block, node, indent_level);
    }
    c_print_defers_from_block_to_block(sb, indent_level, block, block);
}

void c_print_gen_type_info_struct(Chunked_String_Builder *sb, char *ti_name, Type *type) {
    sb->printf("    %s->fields.data = malloc(%d * sizeof(struct Type_Info_Struct_Field));\n", ti_name, type->variable_fields.count);
    sb->printf("    %s->fields.count = %d;\n", ti_name, type->variable_fields.count);
    For (idx, type->variable_fields) {
        Struct_Field field = type->variable_fields[idx];
        sb->printf("    GTISF(%s, \"%s\", %d, %d, %d, %d);\n", ti_name, field.name, strlen(field.name), idx, field.operand.type->id, field.offset);
        if (field.notes.count > 0) {
            // todo(josh): make macros for these
            sb->printf("    ((struct Type_Info_Struct_Field *)%s->fields.data)[%d].notes.data  = malloc(%d * sizeof(String));\n", ti_name, idx, field.notes.count);
            sb->printf("    ((struct Type_Info_Struct_Field *)%s->fields.data)[%d].notes.count = %d;\n", ti_name, idx, field.notes.count);
            For (note_idx, field.notes) {
                char *note = field.notes[note_idx];
                sb->printf("    ((String *)((struct Type_Info_Struct_Field *)%s->fields.data)[%d].notes.data)[%d] = MAKE_STRING(\"%s\", %d);\n", ti_name, idx, note_idx, note, strlen(note));
            }
        }
    }
}

// :TypeInfoKindValues
#define TYPE_INFO_KIND_INTEGER   0
#define TYPE_INFO_KIND_FLOAT     1
#define TYPE_INFO_KIND_BOOL      2
#define TYPE_INFO_KIND_STRING    3
#define TYPE_INFO_KIND_STRUCT    4
#define TYPE_INFO_KIND_UNION     5
#define TYPE_INFO_KIND_ENUM      6
#define TYPE_INFO_KIND_POINTER   7
#define TYPE_INFO_KIND_SLICE     8
#define TYPE_INFO_KIND_ARRAY     9
#define TYPE_INFO_KIND_REFERENCE 10
#define TYPE_INFO_KIND_PROCEDURE 11
#define TYPE_INFO_KIND_TYPEID    12

void c_print_procedure(Chunked_String_Builder *sb, Ast_Proc *proc) {
    assert(proc->body != nullptr);
    assert(!proc->header->is_foreign);

    c_print_line_directive(sb, proc->header->location, "proc location");
    c_print_procedure_header(sb, proc->header);
    sb->print(" {\n");
    if (proc == g_main_proc) {
        c_print_line_directive(sb, proc->header->location, "__init_sif_runtime()");
        print_indents(sb, 1);
        sb->print("__init_sif_runtime();\n");
        For (idx, proc->parent_block->declarations) {
            Declaration *decl = proc->parent_block->declarations[idx];
            if (decl->kind != DECL_VAR) {
                continue;
            }
            Var_Declaration *var = (Var_Declaration *)decl;
            if (var->var->expr) {
                char *rhs = c_print_expr(sb, var->var->expr, 1);
                c_print_line_directive(sb, proc->header->location, "init global variable");
                print_indents(sb, 1);
                sb->printf("%s = %s;\n", var->link_name, rhs);
            }
        }
    }
    c_print_block(sb, proc->body, 1);
    if (proc == g_main_proc) {
        print_indents(sb, 1);
        sb->print("return 0;\n");
    }
    sb->print("}\n");
}



struct Forward_Declarations_Payload {
    Chunked_String_Builder sb;
    int start_index;
    int one_past_last_index;
    bool done;
};

u32 forward_declarations_worker_proc(void *userdata) {
    Forward_Declarations_Payload *payload = (Forward_Declarations_Payload *)userdata;
    Chunked_String_Builder *sb = &payload->sb;
    for (int idx = payload->start_index; idx < payload->one_past_last_index; idx += 1) {
        Declaration *decl = ordered_declarations[idx];
        switch (decl->kind) {
            case DECL_STRUCT: {
                Struct_Declaration *structure = (Struct_Declaration *)decl;
                sb->printf("%s %s", (structure->structure->is_union ? "union" : "struct"), decl->name);
                sb->print(";\n");
                break;
            }
            case DECL_ENUM: {
                Enum_Declaration *enum_decl = (Enum_Declaration *)decl;
                sb->printf("enum %s", enum_decl->name);
                sb->print(";\n");
                break;
            }
            case DECL_PROC: {
                Proc_Declaration *proc_decl = (Proc_Declaration *)decl;
                c_print_procedure_header(sb, proc_decl->header);
                sb->print(";\n");
                break;
            }
            case DECL_TYPE: {
                Type_Declaration *type_decl = (Type_Declaration *)decl;
                if (is_type_array(type_decl->type)) {
                    assert(type_decl->parent_block == nullptr);
                    Type_Array *array_type = (Type_Array *)type_decl->type;
                    c_print_type(sb, array_type, "");
                    sb->printf(";\n");
                }
                break;
            }
            case DECL_VAR: {
                break;
            }
            default: {
                assert(false);
            }
        }
    }
    payload->done = true;
    return 0;
}

struct Actual_Declarations_Payload {
    Chunked_String_Builder sb;
    int start_index;
    int one_past_last_index;
    bool done;
};

u32 actual_declarations_worker_proc(void *userdata) {
    Actual_Declarations_Payload *payload = (Actual_Declarations_Payload *)userdata;
    Chunked_String_Builder *sb = &payload->sb;
    for (int idx = payload->start_index; idx < payload->one_past_last_index; idx += 1) {
        Declaration *decl = ordered_declarations[idx];
        switch (decl->kind) {
            case DECL_STRUCT: {
                Struct_Declaration *struct_decl = (Struct_Declaration *)decl;
                Ast_Struct *structure = struct_decl->structure;
                sb->printf("%s %s {\n", (structure->is_union ? "union" : "struct"), structure->declaration->name);
                bool did_print_at_least_one_member = false;
                For (idx, structure->fields) {
                    Ast_Var *var = structure->fields[idx];
                    if (var->is_constant) {
                        continue;
                    }
                    did_print_at_least_one_member = true;
                    print_indents(sb, 1);
                    c_print_type(sb, var->type, var->declaration->link_name);
                    sb->print(";\n");
                }
                if (!did_print_at_least_one_member) {
                    assert(structure->type->size == 1);
                    sb->print("    bool __dummy;\n");
                }
                sb->print("};\n");
                sb->printf("STATIC_ASSERT(sizeof(%s %s) == %d, %s);\n", (structure->is_union ? "union" : "struct"), structure->declaration->name, structure->type->size, structure->declaration->name);
                break;
            }
            case DECL_ENUM: {
                Enum_Declaration *enum_decl = (Enum_Declaration *)decl;
                For (idx, enum_decl->ast_enum->type->constant_fields) {
                    Struct_Field field = enum_decl->ast_enum->type->constant_fields[idx];
                    assert(field.operand.flags & OPERAND_CONSTANT);
                    sb->printf("%s_%s = %d;\n", enum_decl->name, field.name, field.operand.int_value);
                }
                sb->print(";\n");
                break;
            }
            case DECL_VAR: {
                Var_Declaration *var = (Var_Declaration *)decl;
                if (!var->var->is_constant) {
                    c_print_type(sb, var->var->type, var->link_name);
                    if (var->var->expr == nullptr) {
                        sb->printf(" = {0}");
                    }
                    sb->print(";\n");
                }
                break;
            }
            case DECL_PROC: {
                Proc_Declaration *proc_decl = (Proc_Declaration *)decl;
                if (!proc_decl->header->is_foreign && proc_decl->header->procedure != g_main_proc) {
                    c_print_procedure(sb, proc_decl->header->procedure);
                }
                break;
            }
            case DECL_TYPE: {
                Type_Declaration *type_decl = (Type_Declaration *)decl;
                if (is_type_array(type_decl->type)) {
                    assert(type_decl->parent_block == nullptr);
                    Type_Array *array_type = (Type_Array *)type_decl->type;
                    c_print_type(sb, array_type, "");
                    sb->printf(" {\n");
                    sb->printf("    ");
                    Chunked_String_Builder elements_name_sb = make_chunked_string_builder(g_global_linear_allocator, 128);
                    elements_name_sb.printf("elements[%d]", array_type->count);
                    c_print_type(sb, array_type->array_of, elements_name_sb.make_string());
                    sb->printf(";\n};\n");
                }
                break;
            }
        }
    }
    payload->done = true;
    return 0;
}

struct Type_Info_Generation_Payload {
    Chunked_String_Builder sb;
    int start_index;
    int one_past_last_index;
    bool done;
};

u32 type_info_worker_proc(void *userdata) {
    Type_Info_Generation_Payload *payload = (Type_Info_Generation_Payload *)userdata;
    Chunked_String_Builder *sb = &payload->sb;

    if (!g_no_type_info) {
        String_Builder ti_name_sb = make_string_builder(g_global_linear_allocator, 128);
        for (int idx = payload->start_index; idx < payload->one_past_last_index; idx += 1) {
            Type *type = all_types[idx];
            assert(type->id = idx+1);
            ti_name_sb.clear();
            ti_name_sb.printf("ti%d", type->id);
            char *ti_name = ti_name_sb.string();

            int printable_name_length;
            char *printable_name = type_to_string(type, &printable_name_length);

            switch (type->kind) {
                case TYPE_PRIMITIVE: {
                    if (is_type_integer(type)) {
                        sb->printf("    MTI(%s, MAKE_STRING(\"%s\", %d), Type_Info_Integer, %d, %d, %d, %d);\n", ti_name, printable_name, printable_name_length, TYPE_INFO_KIND_INTEGER, type->id, type->size, type->align);
                        sb->printf("    %s->is_signed = %s;\n", ti_name, (type->flags & TF_SIGNED ? "true" : "false"));
                    }
                    else if (is_type_float(type)) {
                        sb->printf("    MTI(%s, MAKE_STRING(\"%s\", %d), Type_Info_Float, %d, %d, %d, %d);\n", ti_name, printable_name, printable_name_length, TYPE_INFO_KIND_FLOAT, type->id, type->size, type->align);
                    }
                    else if (type == type_bool) {
                        sb->printf("    MTI(%s, MAKE_STRING(\"%s\", %d), Type_Info_Bool, %d, %d, %d, %d);\n", ti_name, printable_name, printable_name_length, TYPE_INFO_KIND_BOOL, type->id, type->size, type->align);
                    }
                    else if (type == type_string) {
                        sb->printf("    MTI(%s, MAKE_STRING(\"%s\", %d), Type_Info_String, %d, %d, %d, %d);\n", ti_name, printable_name, printable_name_length, TYPE_INFO_KIND_STRING, type->id, type->size, type->align);
                    }
                    else if (type == type_rawptr) {
                        sb->printf("    MTI(%s, MAKE_STRING(\"%s\", %d), Type_Info_Pointer, %d, %d, %d, %d);\n", ti_name, printable_name, printable_name_length, TYPE_INFO_KIND_POINTER, type->id, type->size, type->align);
                    }
                    else if (type == type_typeid) {
                        sb->printf("    MTI(%s, MAKE_STRING(\"%s\", %d), Type_Info_Typeid, %d, %d, %d, %d);\n", ti_name, printable_name, printable_name_length, TYPE_INFO_KIND_TYPEID, type->id, type->size, type->align);
                    }
                    else if (type == type_any) {
                        sb->printf("    MTI(%s, MAKE_STRING(\"%s\", %d), %s, %d, %d, %d, %d);\n", ti_name, printable_name, printable_name_length, "Type_Info_Struct", TYPE_INFO_KIND_STRUCT, type->id, type->size, type->align);
                        c_print_gen_type_info_struct(sb, ti_name, type);
                    }
                    else if (type == type_cstring) {
                        sb->printf("    MTI(%s, MAKE_STRING(\"%s\", %d), Type_Info_String, %d, %d, %d, %d);\n", ti_name, printable_name, printable_name_length, TYPE_INFO_KIND_STRING, type->id, type->size, type->align);
                        sb->printf("    %s->is_cstring = true;\n", ti_name);
                    }
                    else {
                        printf("Unhandled primitive type: %\n", type_to_string(type));
                    }
                    break;
                }
                case TYPE_ARRAY: {
                    Type_Array *type_array = (Type_Array *)type;
                    sb->printf("    MTI(%s, MAKE_STRING(\"%s\", %d), Type_Info_Array, %d, %d, %d, %d);\n", ti_name, printable_name, printable_name_length, TYPE_INFO_KIND_ARRAY, type->id, type->size, type->align);
                    sb->printf("    %s->array_of = GTIP(%d);\n", ti_name, type_array->array_of->id);
                    sb->printf("    %s->count = %d;\n", ti_name, type_array->count);
                    break;
                }
                case TYPE_VARARGS:
                case TYPE_SLICE: {
                    Type *slice_of = nullptr;
                    if (is_type_slice(type)) {
                        Type_Slice *type_slice = (Type_Slice *)type;
                        slice_of = type_slice->slice_of;
                    }
                    else {
                        assert(is_type_varargs(type));
                        Type_Varargs *varargs = (Type_Varargs *)type;
                        slice_of = varargs->varargs_of;
                    }
                    sb->printf("    MTI(%s, MAKE_STRING(\"%s\", %d), Type_Info_Slice, %d, %d, %d, %d);\n", ti_name, printable_name, printable_name_length, TYPE_INFO_KIND_SLICE, type->id, type->size, type->align);
                    sb->printf("    %s->slice_of = GTIP(%d);\n", ti_name, slice_of->id);
                    break;
                }
                case TYPE_REFERENCE: {
                    Type_Reference *type_reference = (Type_Reference *)type;
                    sb->printf("    MTI(%s, MAKE_STRING(\"%s\", %d), Type_Info_Reference, %d, %d, %d, %d);\n", ti_name, printable_name, printable_name_length, TYPE_INFO_KIND_REFERENCE, type->id, type->size, type->align);
                    sb->printf("    %s->reference_to = GTIP(%d);\n", ti_name, type_reference->reference_to->id);
                    break;
                }
                case TYPE_STRUCT: {
                    Type_Struct *struct_type = (Type_Struct *)type;
                    sb->printf("    MTI(%s, MAKE_STRING(\"%s\", %d), %s, %d, %d, %d, %d);\n", ti_name, printable_name, printable_name_length, (struct_type->is_union ? "Type_Info_Union" : "Type_Info_Struct"), (struct_type->is_union ? TYPE_INFO_KIND_UNION : TYPE_INFO_KIND_STRUCT), type->id, type->size, type->align);
                    // todo(josh): make macros for these
                    if (struct_type->notes.count > 0) {
                        sb->printf("    %s->notes.data  = malloc(%d * sizeof(String));\n", ti_name, struct_type->notes.count);
                        sb->printf("    %s->notes.count = %d;\n", ti_name, struct_type->notes.count);
                        For (note_idx, struct_type->notes) {
                            char *note = struct_type->notes[note_idx];
                            sb->printf("    ((String *)%s->notes.data)[%d] = MAKE_STRING(\"%s\", %d);\n", ti_name, note_idx, note, strlen(note));
                        }
                    }
                    c_print_gen_type_info_struct(sb, ti_name, type);
                    break;
                }
                case TYPE_POINTER: {
                    Type_Pointer *pointer = (Type_Pointer *)type;
                    sb->printf("    MTI(%s, MAKE_STRING(\"%s\", %d), Type_Info_Pointer, %d, %d, %d, %d);\n", ti_name, printable_name, printable_name_length, TYPE_INFO_KIND_POINTER, type->id, type->size, type->align);
                    sb->printf("    %s->pointer_to = GTIP(%d);\n", ti_name, pointer->pointer_to->id);
                    break;
                }
                case TYPE_ENUM: {
                    Type_Enum *type_enum = (Type_Enum *)type;
                    sb->printf("    MTI(%s, MAKE_STRING(\"%s\", %d), Type_Info_Enum, %d, %d, %d, %d);\n", ti_name, printable_name, printable_name_length, TYPE_INFO_KIND_ENUM, type->id, type->size, type->align);
                    // todo(josh): make macros for these
                    if (type_enum->notes.count > 0) {
                        sb->printf("    %s->notes.data  = malloc(%d * sizeof(String));\n", ti_name, type_enum->notes.count);
                        sb->printf("    %s->notes.count = %d;\n", ti_name, type_enum->notes.count);
                        For (note_idx, type_enum->notes) {
                            char *note = type_enum->notes[note_idx];
                            sb->printf("    ((String *)%s->notes.data)[%d] = MAKE_STRING(\"%s\", %d);\n", ti_name, note_idx, note, strlen(note));
                        }
                    }
                    sb->printf("    %s->base_type = GTIP(%d);\n", ti_name, type_enum->base_type->id);
                    sb->printf("    %s->fields.data = malloc(%d * sizeof(struct Type_Info_Enum_Field));\n", ti_name, type_enum->constant_fields.count);
                    sb->printf("    %s->fields.count = %d;\n", ti_name, type_enum->constant_fields.count);
                    For (idx, type_enum->constant_fields) {
                        Struct_Field field = type_enum->constant_fields[idx];
                        assert(field.offset == -1);
                        assert(field.operand.flags & OPERAND_CONSTANT);
                        sb->printf("    GTIEF(%s, %d, \"%s\", %d, %d);\n", ti_name, idx, field.name, strlen(field.name), field.operand.int_value);
                    }
                    break;
                }
                case TYPE_PROCEDURE: {
                    Type_Procedure *procedure = (Type_Procedure *)type;
                    sb->printf("    MTI(%s, MAKE_STRING(\"%s\", %d), Type_Info_Procedure, %d, %d, %d, %d);\n", ti_name, printable_name, printable_name_length, TYPE_INFO_KIND_PROCEDURE, type->id, type->size, type->align);
                    if (procedure->parameter_types.count) {
                        sb->printf("    %s->parameter_types.data = malloc(%d * sizeof(struct Type_Info *));\n", ti_name, procedure->parameter_types.count);
                        sb->printf("    %s->parameter_types.count = %d;\n", ti_name, procedure->parameter_types.count);
                        For (idx, procedure->parameter_types) {
                            Type *param_type = procedure->parameter_types[idx];
                            sb->printf("    GPPT(%s, %d, %d);\n", ti_name, idx, param_type->id);
                        }
                    }
                    if (procedure->return_type) {
                        sb->printf("    %s->return_type = GTIP(%d);\n", ti_name, procedure->return_type->id);
                    }
                    break;
                }
                default: {
                    assert(false);
                    break;
                }
            }
        }
    }
    payload->done = true;
    return 0;
}



Chunked_String_Builder generate_c_main_file(Ast_Block *global_scope) {
    Actual_Declarations_Payload actual_declarations_payload1 = {};
    actual_declarations_payload1.sb = make_chunked_string_builder(g_global_linear_allocator, 10 * 1024);
    actual_declarations_payload1.start_index = ordered_declarations.count / 2 * 0;
    actual_declarations_payload1.one_past_last_index = ordered_declarations.count / 2 * 1;
    Thread actual_declarations_thread1 = create_thread(actual_declarations_worker_proc, &actual_declarations_payload1);

    Actual_Declarations_Payload actual_declarations_payload2 = {};
    actual_declarations_payload2.sb = make_chunked_string_builder(g_global_linear_allocator, 10 * 1024);
    actual_declarations_payload2.start_index = ordered_declarations.count / 2 * 1;
    actual_declarations_payload2.one_past_last_index = ordered_declarations.count;
    Thread actual_declarations_thread2 = create_thread(actual_declarations_worker_proc, &actual_declarations_payload2);



    Type_Info_Generation_Payload type_info_generation_payload1 = {};
    type_info_generation_payload1.sb = make_chunked_string_builder(g_global_linear_allocator, 10 * 1024);
    type_info_generation_payload1.start_index = 0;
    type_info_generation_payload1.one_past_last_index = all_types.count / 2;
    Thread type_info_thread1 = create_thread(type_info_worker_proc, &type_info_generation_payload1);

    Type_Info_Generation_Payload type_info_generation_payload2 = {};
    type_info_generation_payload2.sb = make_chunked_string_builder(g_global_linear_allocator, 10 * 1024);
    type_info_generation_payload2.start_index = all_types.count / 2;
    type_info_generation_payload2.one_past_last_index = all_types.count;
    Thread type_info_thread2 = create_thread(type_info_worker_proc, &type_info_generation_payload2);



    Forward_Declarations_Payload forward_declarations_payload1 = {};
    forward_declarations_payload1.sb = make_chunked_string_builder(g_global_linear_allocator, 10 * 1024);
    forward_declarations_payload1.start_index = 0;
    forward_declarations_payload1.one_past_last_index = ordered_declarations.count;
    Thread forward_declarations_thread1 = create_thread(forward_declarations_worker_proc, &forward_declarations_payload1);

    // Forward_Declarations_Payload forward_declarations_payload2 = {};
    // forward_declarations_payload2.sb = make_chunked_string_builder(g_global_linear_allocator, 10 * 1024);
    // forward_declarations_payload2.start_index = ordered_declarations.count / 2;
    // forward_declarations_payload2.one_past_last_index = ordered_declarations.count;
    // Thread forward_declarations_thread2 = create_thread(forward_declarations_worker_proc, &forward_declarations_payload2);



    Chunked_String_Builder sb = make_chunked_string_builder(g_global_linear_allocator, 10 * 1024);

    sb.print("#include <stdint.h>\n");
    sb.print("#include <stdbool.h>\n");
    sb.print("#include <stdio.h>\n");
    sb.print("#include <stdlib.h>\n");
    sb.print("#include <string.h>\n");
    sb.print("#include <math.h>\n");

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

    sb.print("Slice MAKE_SLICE(void *data, i64 count) {\n");
    sb.print("    Slice slice;\n");
    sb.print("    slice.data = data;\n");
    sb.print("    slice.count = count;\n");
    sb.print("    return slice;\n");
    sb.print("};\n");

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

    sb.print("void print_char(u8 c) {\n");
    sb.print("    printf(\"%c\", c);\n");
    sb.print("}\n");
    sb.print("void print_int(i64 i) {\n");
    sb.print("    printf(\"%lld\", i);\n");
    sb.print("}\n");
    sb.print("void print_uint(u64 i) {\n");
    sb.print("    printf(\"%llu\", i);\n");
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
    sb.print("void assert(bool condition) {\n");
    sb.print("    if (!condition) {\n");
    sb.print("        printf(\"Assertion failed.\\n\");\n");
    sb.print("        *((int *)0) = 0;\n");
    sb.print("    }\n");
    sb.print("}\n");

    sb.print("#define STATIC_ASSERT(COND,MSG) typedef char static_assertion_##MSG[(COND)?1:-1]\n");


    // todo(josh): @Speed we could clean this up a bunch by introducing some kind of
    // Incomplete_Declaration and only outputting the ones we need to, rather
    // than predeclaring literally everything in the program
    sb.print("\n// Forward declarations\n");
    double forward_decl_wait_start_time = query_timer(&g_global_timer);
    while (!forward_declarations_payload1.done) { sleep(1); }
    sb.print(forward_declarations_payload1.sb.make_string());
    // while (!forward_declarations_payload2.done) { sleep(1); }
    // sb.print(forward_declarations_payload2.sb.make_string());
    double forward_decl_wait_end_time = query_timer(&g_global_timer);

    sb.print("\n// Actual declarations\n");
    double actual_decl_wait_start_time = query_timer(&g_global_timer);
    while (!actual_declarations_payload1.done) { sleep(1); }
    sb.print(actual_declarations_payload1.sb.make_string());
    while (!actual_declarations_payload2.done) { sleep(1); }
    sb.print(actual_declarations_payload2.sb.make_string());
    double actual_decl_wait_end_time = query_timer(&g_global_timer);

    sb.print("void __init_sif_runtime() {\n");
    sb.printf("    int type_info_table_size = %d * sizeof(union Union_All_Type_Infos);\n", all_types.count + 1);
    sb.printf("    _global_type_table.data = malloc(type_info_table_size);\n");
    sb.printf("    zero_pointer(_global_type_table.data, type_info_table_size);\n");
    sb.printf("    _global_type_table.count = %d;\n", all_types.count + 1);
    sb.printf("    // Make Type Info\n");
    sb.printf("    #define MTI(_varname, _printable_name, _type, _kind, _id, _size, _align) struct _type *_varname = ((struct _type *)GTIP(_id)); zero_pointer(_varname, sizeof(*_varname)); _varname->base.printable_name = _printable_name; _varname->base.id = _id; _varname->base.size = _size; _varname->base.align = _align; _varname->base.kind = _kind;\n");
    sb.printf("    // Get Type Info Pointer\n");
    sb.printf("    #define GTIP(_index) ((struct Type_Info *)&((union Union_All_Type_Infos *)_global_type_table.data)[_index])\n");
    sb.printf("    // Gen Type Info Struct Field\n");
    sb.printf("    #define GTISF(_ti_name, _field_name, _field_name_len, _field_index, _field_type_index, _field_offset) ((struct Type_Info_Struct_Field *)_ti_name->fields.data)[_field_index] = (struct Type_Info_Struct_Field){MAKE_STRING(_field_name, _field_name_len), GTIP(_field_type_index), _field_offset}\n");
    sb.printf("    // Gen Type Info Enum Field\n");
    sb.printf("    #define GTIEF(_ti_name, _field_index, _field_name, _field_name_len, _field_value) ((struct Type_Info_Enum_Field *)_ti_name->fields.data)[_field_index] = (struct Type_Info_Enum_Field){MAKE_STRING(_field_name, _field_name_len), _field_value};\n");
    sb.printf("    // Gen Procedure Parameter Type\n");
    sb.printf("    #define GPPT(_varname, _param_index, _type_index) ((struct Type_Info **)_varname->parameter_types.data)[_param_index] = GTIP(_type_index);\n");
    double type_info_wait_start_time = query_timer(&g_global_timer);
    while (!type_info_generation_payload1.done) { sleep(1); }
    sb.print(type_info_generation_payload1.sb.make_string());
    while (!type_info_generation_payload2.done) { sleep(1); }
    sb.print(type_info_generation_payload2.sb.make_string());
    double type_info_wait_end_time = query_timer(&g_global_timer);
    sb.print("}\n");

    // printf("forward:   %f\n", forward_decl_wait_end_time - forward_decl_wait_start_time);
    // printf("actual:    %f\n", actual_decl_wait_end_time - actual_decl_wait_start_time);
    // printf("type info: %f\n", type_info_wait_end_time - type_info_wait_start_time);

    c_print_procedure(&sb, g_main_proc);

    return sb;
}