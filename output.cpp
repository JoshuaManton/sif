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
    int count;
};
struct Slice {
    void *data;
    int count;
};

void print_int(int i) {
    printf("%d\n", i);
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
char *alloc(int size) {
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
void *alloc(i64 size);
struct Vector3;
void print_float(f32 f);
void free(void *ptr);
void main();
void print_int(i32 i);
void print(String str);
void assert(bool condition);

// Actual declarations
struct Vector3 {
    f32 x;
    f32 y;
    f32 z;
};
void main() {
    Vector3 (*memory) = ((Vector3 (*))alloc(192));
    Slice slice = {};
    *((Vector3 (*(*)))&slice.data) = memory;
    slice.count = 16;
    for (i64 i = 0; i < slice.count; i = i + 1) {
        Vector3 (*v) = &((Vector3 (*))slice.data)[i];
        v->x = 9.000000;
        v->y = 4.000000;
        v->z = 1.000000;
    }
    Vector3 v_in_slice = ((Vector3 (*))slice.data)[9];
    print_float(v_in_slice.x);
    print_float(v_in_slice.y);
    print_float(v_in_slice.z);
    Vector3 (*v) = ((Vector3 (*))alloc(12));
    v->x = 1.000000;
    v->y = 4.000000;
    v->z = 9.000000;
    print_float(v->x);
    print_float(v->y);
    print_float(v->z);
    free(v);
}
