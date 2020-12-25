#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <cstdlib>
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;
struct String {
    char *data;
    i64 count;
};
String MAKE_STRING(char *data, i64 count) {
    String string;
    string.data = data;
    string.count = count;
    return string;
};
struct Slice {
    void *data;
    i64 count;
};
template<typename T, int N>
struct Static_Array {
    T elements[N];
};

void print_int(i64 i) {
    printf("%lld\n", i);
}
void print_float(float f) {
    printf("%f\n", f);
}
void print(String string) {
    for (int i = 0; i < string.count; i++) {
        char c = string.data[i];
        printf("%c", c);
    }
    printf("\n");
}
void *alloc(i64 size) {
    char *memory = (char *)malloc(size);
    return memory;
}
void assert(bool condition) {
    if (!condition) {
        printf("Assertion failed.");
        *((char *)0) = 0;
    }
}

// Forward declarations
void print_int(i64 i);
void print_float(f32 f);
void print(String str);
void *alloc(i64 size);
void free(void *ptr);
void assert(bool condition);
struct Dynamic_Array_Int;
void maybe_grow(Dynamic_Array_Int *dyn);
void append(Dynamic_Array_Int *dyn, i64 value);
i64 pop(Dynamic_Array_Int *dyn);
void clear_dynamic_array(Dynamic_Array_Int *dyn);
void destroy_dynamic_array(Dynamic_Array_Int *dyn);
void main();

// Actual declarations
struct Dynamic_Array_Int {
    Slice array;
    i64 count;
};
i64 __operator_overload_Dynamic_Array_Int_TK_LEFT_SQUARE(Dynamic_Array_Int dyn, i64 index) {
    return ((i64 *)dyn.array.data)[index];
}
void maybe_grow(Dynamic_Array_Int *dyn) {
    if (dyn->count == dyn->array.count) {
        if (*((i64 **)&dyn->array.data) != nullptr) {
            free(*((i64 **)&dyn->array.data));
        }
        i64 new_cap = 8 + dyn->array.count * 2;
        *((i64 **)&dyn->array.data) = ((i64 *)alloc(new_cap * 8));
        dyn->array.count = new_cap;
    }
}
void append(Dynamic_Array_Int *dyn, i64 value) {
    maybe_grow(dyn);
    assert(dyn->count < dyn->array.count);
    ((i64 *)dyn->array.data)[dyn->count] = value;
    dyn->count += 1;
}
i64 pop(Dynamic_Array_Int *dyn) {
    assert(dyn->count > 0);
    i64 value = ((i64 *)dyn->array.data)[dyn->count - 1];
    dyn->count -= 1;
    return value;
}
void clear_dynamic_array(Dynamic_Array_Int *dyn) {
    dyn->count = 0;
}
void destroy_dynamic_array(Dynamic_Array_Int *dyn) {
    if (*((i64 **)&dyn->array.data) != nullptr) {
        free(*((i64 **)&dyn->array.data));
    }
}
void main() {
    Dynamic_Array_Int arr = {};
    append(&arr, 1);
    append(&arr, 4);
    append(&arr, 9);
    print_int(__operator_overload_Dynamic_Array_Int_TK_LEFT_SQUARE(arr, 1));
}
