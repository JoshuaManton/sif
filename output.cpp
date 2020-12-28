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
struct Foo__polymorph_0;
void thing__polymorph_1(Foo__polymorph_0 foo);
i64 value_poly__polymorph_2();
i64 type_poly__polymorph_3(i64 a);
f32 type_poly__polymorph_4(f32 a);
i64 type_poly__polymorph_5(i64 a);
f32 type_poly__polymorph_6(f32 a);
i64 value_and_type_poly__polymorph_7();
f32 value_and_type_poly__polymorph_8();
void main();

// Actual declarations
struct Foo__polymorph_0 {
    f32 data;
    Static_Array<i64 , 32> arr;
};
void thing__polymorph_1(Foo__polymorph_0 foo) {
    print_int(foo.arr.elements[4]);
}
i64 value_poly__polymorph_2() {
    return 4;
}
i64 type_poly__polymorph_3(i64 a) {
    return a * a;
}
f32 type_poly__polymorph_4(f32 a) {
    return a * a;
}
i64 type_poly__polymorph_5(i64 a) {
    return a * a;
}
f32 type_poly__polymorph_6(f32 a) {
    return a * a;
}
i64 value_and_type_poly__polymorph_7() {
    return 49;
}
f32 value_and_type_poly__polymorph_8() {
    return 64.000000;
}
void main() {
    Foo__polymorph_0 poly_struct = {};
    poly_struct.arr.elements[4] = 123;
    print_int(poly_struct.arr.elements[4]);
    thing__polymorph_1(poly_struct);
    print_int(value_poly__polymorph_2());
    print_int(type_poly__polymorph_3(3));
    print_float(type_poly__polymorph_4(4.000000));
    i64 a = 5;
    f32 f = 6.000000;
    print_int(type_poly__polymorph_5(a));
    print_float(type_poly__polymorph_6(f));
    print_int(value_and_type_poly__polymorph_7());
    print_float(value_and_type_poly__polymorph_8());
}
