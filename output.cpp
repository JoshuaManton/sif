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
i64 holy_wow__polymorph_0();
void main();

// Actual declarations
i64 holy_wow__polymorph_0() {
    i64 a = 246;
    return a;
}
void main() {
    i64 aaa = holy_wow__polymorph_0();
    print_int(aaa);
}
