#pragma once

#include "basic.h"

extern char *sif_core_lib_path;
extern Allocator g_global_linear_allocator;

#define SIF_NEW(type) ((type *)alloc(g_global_linear_allocator, sizeof(type), true))

template<typename T>
T *SIF_NEW_CLONE(T t) {
    T *ptr = SIF_NEW(T);
    *ptr = t;
    return ptr;
}

extern char *g_interned_string_return;
extern char *g_interned_string_var;
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
extern char *g_interned_rawptr_string;
extern char *g_interned_any_string;
extern char *g_interned_typeid_string;
extern char *g_interned_data_string;
extern char *g_interned_count_string;

void init_interned_strings();
char *intern_string(char *str, int length_override = -1);