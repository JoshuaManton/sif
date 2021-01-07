#include "common.h"

Chunked_String_Builder g_interned_string_buffer;
Hashtable<u64, char *> g_interned_strings;

char *intern_string(char *str, int length_override) {
    u64 hash = 0xcbf29ce484222325;
    int length = length_override;
    if (length_override == -1) {
        char *c = str;
        for (; *c != '\0'; c++) {
            hash = (hash * 0x100000001b3) ^ u64(*c);
        }
        length = c - str;
    }
    else {
        for (int i = 0; i < length_override; i++) {
            hash = (hash * 0x100000001b3) ^ u64(str[i]);
        }
    }
    char **interned = g_interned_strings.get(hash);
    if (interned) {
        return *interned;
    }
    char *copy = g_interned_string_buffer.write_with_length(str, length);
    g_interned_string_buffer.append_null();
    g_interned_strings.insert(hash, copy);
    return copy;
}

char *g_interned_string_return;
char *g_interned_string_var;
char *g_interned_string_const;
char *g_interned_string_proc;
char *g_interned_string_operator;
char *g_interned_string_struct;
char *g_interned_string_union;
char *g_interned_string_enum;
char *g_interned_string_null;
char *g_interned_string_true;
char *g_interned_string_false;
char *g_interned_string_sizeof;
char *g_interned_string_typeof;
char *g_interned_string_if;
char *g_interned_string_else;
char *g_interned_string_for;
char *g_interned_string_while;
char *g_interned_string_break;
char *g_interned_string_continue;
char *g_interned_string_cast;
char *g_interned_string_transmute;
char *g_interned_string_using;
char *g_interned_string_defer;

char *g_interned_main_string;
char *g_interned_sif_runtime_string;
char *g_interned_string_string;
char *g_interned_rawptr_string;
char *g_interned_any_string;
char *g_interned_typeid_string;
char *g_interned_data_string;
char *g_interned_count_string;

void init_interned_strings() {
    g_interned_string_buffer = make_chunked_string_builder(g_global_linear_allocator, 1 * 1024);
    g_interned_strings = make_hashtable<u64, char *>(g_global_linear_allocator, 10 * 1024);
    g_interned_string_return     = intern_string("return");
    g_interned_string_var        = intern_string("var");
    g_interned_string_const      = intern_string("const");
    g_interned_string_proc       = intern_string("proc");
    g_interned_string_operator   = intern_string("operator");
    g_interned_string_struct     = intern_string("struct");
    g_interned_string_union      = intern_string("union");
    g_interned_string_enum       = intern_string("enum");
    g_interned_string_null       = intern_string("null");
    g_interned_string_true       = intern_string("true");
    g_interned_string_false      = intern_string("false");
    g_interned_string_sizeof     = intern_string("sizeof");
    g_interned_string_typeof     = intern_string("typeof");
    g_interned_string_if         = intern_string("if");
    g_interned_string_else       = intern_string("else");
    g_interned_string_for        = intern_string("for");
    g_interned_string_while      = intern_string("while");
    g_interned_string_break      = intern_string("break");
    g_interned_string_continue   = intern_string("continue");
    g_interned_string_cast       = intern_string("cast");
    g_interned_string_transmute  = intern_string("transmute");
    g_interned_string_using      = intern_string("using");
    g_interned_string_defer      = intern_string("defer");

    g_interned_main_string        = intern_string("main");
    g_interned_sif_runtime_string = intern_string("sif_runtime");
    g_interned_string_string      = intern_string("string");
    g_interned_rawptr_string      = intern_string("rawptr");
    g_interned_any_string         = intern_string("any");
    g_interned_typeid_string      = intern_string("typeid");
    g_interned_data_string        = intern_string("data");
    g_interned_count_string       = intern_string("count");
}