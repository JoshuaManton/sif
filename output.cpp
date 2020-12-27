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
i64 foo_proc__polymorph_0(i64 a, i64 b);
i64 foo_proc__polymorph_1(i64 a, i64 b);
f32 foo_proc__polymorph_2(f32 a, f32 b);
void main();

// Actual declarations
i64 foo_proc__polymorph_0(i64 a, i64 b) {
    return a + b;
}
i64 foo_proc__polymorph_1(i64 a, i64 b) {
    return a + b;
}
f32 foo_proc__polymorph_2(f32 a, f32 b) {
    return a + b;
}
void main() {
    i64 i1 = 1;
    i64 i2 = 2;
    i64 i3 = foo_proc__polymorph_0(i1, i2);
    i3 = foo_proc__polymorph_1(i1, i2);
    print_int(i3);
    f32 f1 = 1.000000;
    f32 f2 = 2.000000;
    f32 f3 = foo_proc__polymorph_2(f1, f2);
    print_float(f3);
}
