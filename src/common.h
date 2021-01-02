#pragma once

#include "basic.h"

extern char *sif_core_lib_path;
extern Allocator g_global_linear_allocator;

#define SIF_NEW(type) ((type *)alloc(g_global_linear_allocator, sizeof(type)))

template<typename T>
T *SIF_NEW_CLONE(T t) {
    T *ptr = SIF_NEW(T);
    *ptr = t;
    return ptr;
}
