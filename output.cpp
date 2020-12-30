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
struct Foo__polymorph_0;
void pointer_proc__polymorph_1(Foo__polymorph_0 *foo);
struct Foo__polymorph_2;
void slice_proc__polymorph_3(Slice foo);
void slice_proc__polymorph_4(Slice foo);
void array_proc__polymorph_5(Static_Array<Foo__polymorph_0 *, 4> foo);
void array_proc_fixed__polymorph_6(Static_Array<Foo__polymorph_0 *, 4> foo);
void array_proc__polymorph_7(Static_Array<Foo__polymorph_0 *, 2> foo);
void main();

// Actual declarations
struct Foo__polymorph_0 {
    f32 data;
    Static_Array<i64 , 16> arr;
};
void pointer_proc__polymorph_1(Foo__polymorph_0 *foo) {
}
struct Foo__polymorph_2 {
    f32 data;
    Static_Array<i64 , 8> arr;
};
void slice_proc__polymorph_3(Slice foo) {
}
void slice_proc__polymorph_4(Slice foo) {
}
void array_proc__polymorph_5(Static_Array<Foo__polymorph_0 *, 4> foo) {
}
void array_proc_fixed__polymorph_6(Static_Array<Foo__polymorph_0 *, 4> foo) {
}
void array_proc__polymorph_7(Static_Array<Foo__polymorph_0 *, 2> foo) {
}
void main() {
    Foo__polymorph_0 foo = {};
    pointer_proc__polymorph_1(&foo);
    Slice slice1 = {};
    Slice slice2 = {};
    slice_proc__polymorph_3(slice1);
    slice_proc__polymorph_4(slice2);
    Static_Array<Foo__polymorph_0 *, 4> arr = {};
    array_proc__polymorph_5(arr);
    array_proc_fixed__polymorph_6(arr);
    Static_Array<Foo__polymorph_0 *, 2> arr2 = {};
    array_proc__polymorph_7(arr2);
}
