#pragma once

#include "basic.h"
#include "os_windows.h"

extern char *sif_core_lib_path;
extern Allocator g_global_linear_allocator;

template<typename T>
T *SIF_NEW_CLONE(T t, Allocator allocator) {
    T *ptr = ((T *)alloc(allocator, sizeof(T), true));
    *ptr = t;
    return ptr;
}

extern char *g_interned_string_return;
extern char *g_interned_string_const;
extern char *g_interned_string_proc;
extern char *g_interned_string_operator;
extern char *g_interned_string_struct;
extern char *g_interned_string_union;
extern char *g_interned_string_enum;
extern char *g_interned_string_null;
extern char *g_interned_string_true;
extern char *g_interned_string_false;
extern char *g_interned_string_sizeof;
extern char *g_interned_string_typeof;
extern char *g_interned_string_typeofelement;
extern char *g_interned_string_if;
extern char *g_interned_string_else;
extern char *g_interned_string_for;
extern char *g_interned_string_while;
extern char *g_interned_string_break;
extern char *g_interned_string_continue;
extern char *g_interned_string_cast;
extern char *g_interned_string_transmute;
extern char *g_interned_string_using;
extern char *g_interned_string_defer;

extern char *g_interned_main_string;
extern char *g_interned_sif_runtime_string;
extern char *g_interned_string_string;
extern char *g_interned_cstring_string;
extern char *g_interned_rawptr_string;
extern char *g_interned_any_string;
extern char *g_interned_typeid_string;
extern char *g_interned_data_string;
extern char *g_interned_type_string;
extern char *g_interned_count_string;

extern char *g_interned_sif_runtime_bounds_check_proc_string;
extern char *g_interned_sif_runtime_null_check_proc_string;
extern char *g_interned_sif_runtime_string_eq_proc_string;
extern char *g_interned_sif_runtime_zero_pointer_proc_string;
extern char *g_interned_sif_runtime_source_code_location_string;
extern char *g_interned_sif_runtime_make_source_code_location_string;

extern char *g_interned_sif_runtime_union_all_type_infos_string;
extern char *g_interned_sif_runtime_type_info_string;
extern char *g_interned_sif_runtime_type_info_integer_string;
extern char *g_interned_sif_runtime_type_info_float_string;
extern char *g_interned_sif_runtime_type_info_bool_string;
extern char *g_interned_sif_runtime_type_info_string_string;
extern char *g_interned_sif_runtime_type_info_struct_field_string;
extern char *g_interned_sif_runtime_type_info_struct_string;
extern char *g_interned_sif_runtime_type_info_union_string;
extern char *g_interned_sif_runtime_type_info_enum_field_string;
extern char *g_interned_sif_runtime_type_info_enum_string;
extern char *g_interned_sif_runtime_type_info_pointer_string;
extern char *g_interned_sif_runtime_type_info_slice_string;
extern char *g_interned_sif_runtime_type_info_array_string;
extern char *g_interned_sif_runtime_type_info_reference_string;
extern char *g_interned_sif_runtime_type_info_procedure_string;
extern char *g_interned_sif_runtime_type_info_typeid_string;

void init_interned_strings();
char *intern_string(char *str, int length_override = -1);