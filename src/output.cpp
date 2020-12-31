#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <cstring>
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
Slice slice_ptr__polymorph_0(i64 *ptr, i64 count);
struct Foo__polymorph_1;
void pointer_proc__polymorph_2(Foo__polymorph_1 *foo);
struct Foo__polymorph_3;
void slice_proc__polymorph_4(Slice foo);
void slice_proc__polymorph_5(Slice foo);
void array_proc__polymorph_6(Static_Array<Foo__polymorph_1 *, 4> foo);
void array_proc_fixed__polymorph_7(Static_Array<Foo__polymorph_1 *, 4> foo);
void array_proc__polymorph_8(Static_Array<Foo__polymorph_1 *, 2> foo);
void main();

// Actual declarations
Slice slice_ptr__polymorph_0(i64 *ptr, i64 count) {
    Slice slice = {};
    *((i64 **)&slice.data) = ptr;
    slice.count = count;
    return slice;
}
struct Foo__polymorph_1 {
    f32 data;
    Static_Array<i64 , 16> arr;
};
void pointer_proc__polymorph_2(Foo__polymorph_1 *foo) {
}
struct Foo__polymorph_3 {
    f32 data;
    Static_Array<i64 , 8> arr;
};
void slice_proc__polymorph_4(Slice foo) {
}
void slice_proc__polymorph_5(Slice foo) {
}
void array_proc__polymorph_6(Static_Array<Foo__polymorph_1 *, 4> foo) {
}
void array_proc_fixed__polymorph_7(Static_Array<Foo__polymorph_1 *, 4> foo) {
}
void array_proc__polymorph_8(Static_Array<Foo__polymorph_1 *, 2> foo) {
}
void main() {
    Static_Array<i64 , 10> array_of_ints = {};
    Slice slice_arr = slice_ptr__polymorph_0(&array_of_ints.elements[0], 10);
    for (i64 i = 0; i < slice_arr.count; i += 1) {
        print_int(((i64 *)slice_arr.data)[i]);
    }
    Foo__polymorph_1 foo = {};
    pointer_proc__polymorph_2(&foo);
    Slice slice1 = {};
    Slice slice2 = {};
    slice_proc__polymorph_4(slice1);
    slice_proc__polymorph_5(slice2);
    Static_Array<Foo__polymorph_1 *, 4> arr = {};
    array_proc__polymorph_6(arr);
    array_proc_fixed__polymorph_7(arr);
    Static_Array<Foo__polymorph_1 *, 2> arr2 = {};
    array_proc__polymorph_8(arr2);
}
