#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
typedef struct {
    char *data;
    i64 count;
} String;
String MAKE_STRING(char *data, i64 count) {
    String string;
    string.data = data;
    string.count = count;
    return string;
};
typedef struct {
    void *data;
    i64 count;
} Slice;

void print_int(i64 i) {
    printf("%lld\n", i);
}
void print_float(float f) {
    printf("%f\n", f);
}
void print_string(String string) {
    for (i64 i = 0; i < string.count; i++) {
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
bool string_eq(String a, String b);
i64 factorial(i64 n);
void basic_stuff();
struct Static_Array_4_i64 ;
void arrays_by_value(struct Static_Array_4_i64 arr);
void arrays();
void slices();
void strings();
struct Foo;
void structs();
void enums();
struct Static_Array_8_bool ;
struct Loopy;
struct Static_Array_8_i64 ;
void order_independence();
void change_by_reference(i64 *a, i64 value);
void references();
struct Vector3;
void operator_overloading();
i64 value_poly__polymorph_0();
i64 type_poly__polymorph_1(i64 a);
f32 type_poly__polymorph_2(f32 a);
i64 value_and_type_poly__polymorph_3();
f32 value_and_type_poly__polymorph_4();
void procedural_polymorphism();
struct Custom_Array_Type__polymorph_5;
void structural_polymorphism();
struct Dynamic_Array__polymorph_6;
void append__polymorph_7(struct Dynamic_Array__polymorph_6 *dyn, struct Vector3 value);
void destroy_dynamic_array__polymorph_8(struct Dynamic_Array__polymorph_6 dyn);
void dynamic_arrays();
i32 main();

// Actual declarations
bool string_eq(String a, String b) {
    if (a.count != b.count) {
        return false;
    }
    for (i64 i = 0; i < a.count; i += 1) {
        if (a.data[i] != b.data[i]) {
            return false;
        }
    }
    return true;
}
i64 factorial(i64 n) {
    if (n == 1) {
        return 1;
    }
    return n * factorial(n - 1);
}
void basic_stuff() {
    print_string(MAKE_STRING("\n\n---- basic_stuff ----\n", 24));
    i64 a = 123;
    i64 b1 = 123;
    assert(true);
    f32 b2 = 123.000000;
    assert(true);
    i64 fact = factorial(5);
    assert(fact == 120);
    assert(true);
    i64 c = 12;
    f32 d = 12.000000;
    f32 e = 5.500000;
    assert(e == 5.500000);
    i64 f = 5;
    assert(f == 5);
    f32 g = 5.500000;
    assert(g == 5.500000);
}
struct Static_Array_4_i64  {
    i64  elements[4];
};
void arrays_by_value(struct Static_Array_4_i64 arr) {
    arr.elements[2] = 738;
}
void arrays() {
    print_string(MAKE_STRING("\n\n---- arrays ----\n", 19));
    struct Static_Array_4_i64 my_array = {0};
    my_array.elements[0] = 1;
    my_array.elements[1] = 2;
    my_array.elements[2] = 3;
    my_array.elements[3] = 4;
    print_int(my_array.elements[0]);
    print_int(my_array.elements[1]);
    print_int(my_array.elements[2]);
    print_int(my_array.elements[3]);
    assert(my_array.elements[0] == 1);
    assert(my_array.elements[1] == 2);
    assert(my_array.elements[2] == 3);
    assert(my_array.elements[3] == 4);
    arrays_by_value(my_array);
    print_int(my_array.elements[2]);
    assert(my_array.elements[2] == 3);
}
void slices() {
}
void strings() {
    print_string(MAKE_STRING("\n\n---- slices ----\n", 19));
    assert(true);
    String a = MAKE_STRING("Hello, World!", 13);
    String hello = {0};
    hello.data = &a.data[0];
    hello.count = 5;
    assert(string_eq(hello, MAKE_STRING("Hello", 5)));
    print_string(hello);
    print_string(MAKE_STRING("\n", 1));
    String world = {0};
    world.data = &a.data[7];
    world.count = 5;
    assert(string_eq(world, MAKE_STRING("World", 5)));
    print_string(world);
    print_string(MAKE_STRING("\n", 1));
}
struct Foo {
    i64 a;
    String str;
    bool t;
};
void structs() {
    print_string(MAKE_STRING("\n\n---- structs ----\n", 20));
    struct Foo f = {0};
    assert(f.a == 0);
    f.a = 123;
    f.str = MAKE_STRING("foozle", 6);
    f.t = 120 == factorial(5);
    struct Foo __generated_compound_literal_0 = {0};
    __generated_compound_literal_0.a = 149;
    __generated_compound_literal_0.str = MAKE_STRING("hellooo", 7);
    __generated_compound_literal_0.t = false;
    f = __generated_compound_literal_0;
}
void enums() {
}
struct Loopy *ptr_to_loopy = {0};
struct Static_Array_8_bool  {
    bool  elements[8];
};
struct Loopy {
    struct Static_Array_8_bool a;
};
struct Static_Array_8_i64  {
    i64  elements[8];
};
struct Static_Array_8_i64 nesty = {0};
void order_independence() {
    print_string(MAKE_STRING("\n\n---- order_independence ----\n", 31));
    struct Loopy loopy = {0};
    assert(true);
    assert(true);
}
void change_by_reference(i64 *a, i64 value) {
    (*a) = value;
}
void references() {
    print_string(MAKE_STRING("\n\n---- references ----\n", 23));
    i64 my_int = 123;
    print_int(my_int);
    assert(my_int == 123);
    i64 *int_reference = &my_int;
    (*int_reference) = 789;
    assert((*int_reference) == 789);
    assert(my_int == 789);
    print_int((*int_reference));
    assert(my_int == 789);
    change_by_reference(&my_int, 456);
    assert(my_int == 456);
    print_int(my_int);
}
struct Vector3 {
    f32 x;
    f32 y;
    f32 z;
};
struct Vector3 __operator_overload_Vector3_TK_PLUS_Vector3(struct Vector3 a, struct Vector3 b) {
    struct Vector3 __generated_compound_literal_1 = {0};
    __generated_compound_literal_1.x = a.x + b.x;
    __generated_compound_literal_1.y = a.y + b.y;
    __generated_compound_literal_1.z = a.z + b.z;
    return __generated_compound_literal_1;
}
struct Vector3 __operator_overload_Vector3_TK_MINUS_Vector3(struct Vector3 a, struct Vector3 b) {
    struct Vector3 __generated_compound_literal_2 = {0};
    __generated_compound_literal_2.x = a.x - b.x;
    __generated_compound_literal_2.y = a.y - b.y;
    __generated_compound_literal_2.z = a.z - b.z;
    return __generated_compound_literal_2;
}
struct Vector3 __operator_overload_Vector3_TK_MULTIPLY_Vector3(struct Vector3 a, struct Vector3 b) {
    struct Vector3 __generated_compound_literal_3 = {0};
    __generated_compound_literal_3.x = a.x * b.x;
    __generated_compound_literal_3.y = a.y * b.y;
    __generated_compound_literal_3.z = a.z * b.z;
    return __generated_compound_literal_3;
}
struct Vector3 __operator_overload_Vector3_TK_DIVIDE_Vector3(struct Vector3 a, struct Vector3 b) {
    struct Vector3 __generated_compound_literal_4 = {0};
    __generated_compound_literal_4.x = a.x / b.x;
    __generated_compound_literal_4.y = a.y / b.y;
    __generated_compound_literal_4.z = a.z / b.z;
    return __generated_compound_literal_4;
}
struct Vector3 __operator_overload_Vector3_TK_MULTIPLY_f32(struct Vector3 a, f32 f) {
    struct Vector3 __generated_compound_literal_5 = {0};
    __generated_compound_literal_5.x = a.x * f;
    __generated_compound_literal_5.y = a.y * f;
    __generated_compound_literal_5.z = a.z * f;
    return __generated_compound_literal_5;
}
void operator_overloading() {
    print_string(MAKE_STRING("\n\n---- operator_overloading ----\n", 33));
    struct Vector3 __generated_compound_literal_6 = {0};
    __generated_compound_literal_6.x = 1.000000;
    __generated_compound_literal_6.y = 2.000000;
    __generated_compound_literal_6.z = 3.000000;
    struct Vector3 v1 = __generated_compound_literal_6;
    struct Vector3 __generated_compound_literal_7 = {0};
    __generated_compound_literal_7.x = 1.000000;
    __generated_compound_literal_7.y = 4.000000;
    __generated_compound_literal_7.z = 9.000000;
    struct Vector3 v2 = __generated_compound_literal_7;
    struct Vector3 v3 = __operator_overload_Vector3_TK_PLUS_Vector3(v1, v2);
    struct Vector3 v4 = __operator_overload_Vector3_TK_MULTIPLY_f32(v3, 5.000000);
    print_float(v4.x);
    print_float(v4.y);
    print_float(v4.z);
    assert(v4.x == 10.000000);
    assert(v4.y == 30.000000);
    assert(v4.z == 60.000000);
}
i64 value_poly__polymorph_0() {
    return 4;
}
i64 type_poly__polymorph_1(i64 a) {
    return a * a;
}
f32 type_poly__polymorph_2(f32 a) {
    return a * a;
}
i64 value_and_type_poly__polymorph_3() {
    return 49;
}
f32 value_and_type_poly__polymorph_4() {
    return 64.000000;
}
void procedural_polymorphism() {
    print_string(MAKE_STRING("\n\n---- procedural_polymorphism ----\n", 36));
    print_int(value_poly__polymorph_0());
    assert(value_poly__polymorph_0() == 4);
    print_int(type_poly__polymorph_1(3));
    assert(type_poly__polymorph_1(3) == 9);
    print_float(type_poly__polymorph_2(4.000000));
    assert(type_poly__polymorph_2(4.000000) == 16.000000);
    i64 a = 5;
    f32 f = 6.000000;
    print_int(type_poly__polymorph_1(a));
    print_float(type_poly__polymorph_2(f));
    assert(type_poly__polymorph_1(a) == 25);
    assert(type_poly__polymorph_2(f) == 36.000000);
    print_int(value_and_type_poly__polymorph_3());
    print_float(value_and_type_poly__polymorph_4());
    assert(value_and_type_poly__polymorph_3() == 49);
    assert(value_and_type_poly__polymorph_4() == 64.000000);
}
struct Custom_Array_Type__polymorph_5 {
    struct Static_Array_8_i64 array;
};
i64 *__operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(struct Custom_Array_Type__polymorph_5 *my_array, i64 index) {
    return &(*my_array).array.elements[index];
}
void structural_polymorphism() {
    print_string(MAKE_STRING("\n\n---- structural_polymorphism ----\n", 36));
    struct Custom_Array_Type__polymorph_5 array_of_ints = {0};
    (*__operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(&array_of_ints, 4)) = 124;
    print_int((*__operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(&array_of_ints, 4)));
    assert((*__operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(&array_of_ints, 4)) == 124);
}
struct Dynamic_Array__polymorph_6 {
    Slice array;
    i64 count;
};
struct Vector3 *__operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(struct Dynamic_Array__polymorph_6 *dyn, i64 index) {
    return &((struct Vector3 *)(*dyn).array.data)[index];
}
void append__polymorph_7(struct Dynamic_Array__polymorph_6 *dyn, struct Vector3 value) {
    if (dyn->count == dyn->array.count) {
        struct Vector3 *old_data = *((struct Vector3 **)&dyn->array.data);
        i64 new_cap = 8 + dyn->array.count * 2;
        *((struct Vector3 **)&dyn->array.data) = ((struct Vector3 *)alloc(new_cap * 12));
        dyn->array.count = new_cap;
        if (old_data != NULL) {
            memcpy(*((struct Vector3 **)&dyn->array.data), old_data, ((u32 )dyn->count * 12));
            free(old_data);
        }
    }
    assert(dyn->count < dyn->array.count);
    ((struct Vector3 *)dyn->array.data)[dyn->count] = value;
    dyn->count += 1;
}
void destroy_dynamic_array__polymorph_8(struct Dynamic_Array__polymorph_6 dyn) {
    if (*((struct Vector3 **)&dyn.array.data) != NULL) {
        free(*((struct Vector3 **)&dyn.array.data));
    }
}
void dynamic_arrays() {
    print_string(MAKE_STRING("\n\n---- dynamic_arrays ----\n", 27));
    struct Dynamic_Array__polymorph_6 dyn = {0};
    struct Vector3 __generated_compound_literal_8 = {0};
    __generated_compound_literal_8.x = 1.000000;
    __generated_compound_literal_8.y = 2.000000;
    __generated_compound_literal_8.z = 3.000000;
    append__polymorph_7(&dyn, __generated_compound_literal_8);
    struct Vector3 __generated_compound_literal_9 = {0};
    __generated_compound_literal_9.x = 1.000000;
    __generated_compound_literal_9.y = 4.000000;
    __generated_compound_literal_9.z = 9.000000;
    append__polymorph_7(&dyn, __generated_compound_literal_9);
    struct Vector3 __generated_compound_literal_10 = {0};
    __generated_compound_literal_10.x = 2.000000;
    __generated_compound_literal_10.y = 8.000000;
    __generated_compound_literal_10.z = 18.000000;
    append__polymorph_7(&dyn, __generated_compound_literal_10);
    assert((*__operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, 1)).x == 1.000000);
    assert((*__operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, 1)).y == 4.000000);
    assert((*__operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, 1)).z == 9.000000);
    for (i64 i = 0; i < dyn.count; i += 1) {
        print_int(i);
        print_float((*__operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, i)).x);
        print_float((*__operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, i)).y);
        print_float((*__operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, i)).z);
    }
    destroy_dynamic_array__polymorph_8(dyn);
}
i32 main() {
    print_string(MAKE_STRING("-------------------------\n", 26));
    print_string(MAKE_STRING("|   sif language demo   |\n", 26));
    print_string(MAKE_STRING("-------------------------\n", 26));
    basic_stuff();
    arrays();
    slices();
    strings();
    structs();
    enums();
    order_independence();
    references();
    operator_overloading();
    procedural_polymorphism();
    structural_polymorphism();
    dynamic_arrays();
    return 0;
}
