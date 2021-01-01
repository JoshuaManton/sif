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
String string_ptr(u8 *ptr, i64 count);
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
    bool __t1 = a.count != b.count;
    if (__t1) {
        return false;
    }
    {
        i64 i = 0;
        while (true) {
            bool __t2 = i < a.count;
            if (!__t2) { break; }
            bool __t3 = a.data[i] != b.data[i];
            if (__t3) {
                return false;
            }
            i += 1;
        }
    }
    return true;
}
String string_ptr(u8 *ptr, i64 count) {
    String str = {0};
    str.data = ptr;
    str.count = count;
    return str;
}
i64 factorial(i64 n) {
    bool __t4 = n == 1;
    if (__t4) {
        return 1;
    }
    i64 __t6 = n - 1;
    i64 __t7 = factorial(__t6);
    i64 __t5 = n * __t7;
    return __t5;
}
void basic_stuff() {
    print_string(MAKE_STRING("\n\n---- basic_stuff ----\n", 24));
    i64 a = 123;
    i64 b1 = 123;
    assert(true);
    f32 b2 = 123.000000;
    assert(true);
    i64 __t8 = factorial(5);
    i64 fact = __t8;
    bool __t9 = fact == 120;
    assert(__t9);
    assert(true);
    i64 c = 12;
    f32 d = 12.000000;
    f32 e = 5.500000;
    bool __t10 = e == 5.500000;
    assert(__t10);
    i64 f = 5;
    bool __t11 = f == 5;
    assert(__t11);
    f32 g = 5.500000;
    bool __t12 = g == 5.500000;
    assert(__t12);
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
    bool __t13 = my_array.elements[0] == 1;
    assert(__t13);
    bool __t14 = my_array.elements[1] == 2;
    assert(__t14);
    bool __t15 = my_array.elements[2] == 3;
    assert(__t15);
    bool __t16 = my_array.elements[3] == 4;
    assert(__t16);
    arrays_by_value(my_array);
    print_int(my_array.elements[2]);
    bool __t17 = my_array.elements[2] == 3;
    assert(__t17);
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
    bool __t18 = string_eq(hello, MAKE_STRING("Hello", 5));
    assert(__t18);
    print_string(hello);
    print_string(MAKE_STRING("\n", 1));
    String world = {0};
    world.data = &a.data[7];
    world.count = 5;
    bool __t19 = string_eq(world, MAKE_STRING("World", 5));
    assert(__t19);
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
    bool __t20 = f.a == 0;
    assert(__t20);
    f.a = 123;
    f.str = MAKE_STRING("foozle", 6);
    i64 __t22 = factorial(5);
    bool __t21 = 120 == __t22;
    f.t = __t21;
    struct Foo __t23 = {0};
    __t23.a = 149;
    __t23.str = MAKE_STRING("hellooo", 7);
    __t23.t = false;
    f = __t23;
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
    bool __t24 = my_int == 123;
    assert(__t24);
    i64 *int_reference = &my_int;
    (*int_reference) = 789;
    bool __t25 = (*int_reference) == 789;
    assert(__t25);
    bool __t26 = my_int == 789;
    assert(__t26);
    print_int((*int_reference));
    bool __t27 = my_int == 789;
    assert(__t27);
    change_by_reference(&my_int, 456);
    bool __t28 = my_int == 456;
    assert(__t28);
    print_int(my_int);
}
struct Vector3 {
    f32 x;
    f32 y;
    f32 z;
};
struct Vector3 __operator_overload_Vector3_TK_PLUS_Vector3(struct Vector3 a, struct Vector3 b) {
    f32 __t29 = a.x + b.x;
    f32 __t30 = a.y + b.y;
    f32 __t31 = a.z + b.z;
    struct Vector3 __t32 = {0};
    __t32.x = __t29;
    __t32.y = __t30;
    __t32.z = __t31;
    return __t32;
}
struct Vector3 __operator_overload_Vector3_TK_MINUS_Vector3(struct Vector3 a, struct Vector3 b) {
    f32 __t33 = a.x - b.x;
    f32 __t34 = a.y - b.y;
    f32 __t35 = a.z - b.z;
    struct Vector3 __t36 = {0};
    __t36.x = __t33;
    __t36.y = __t34;
    __t36.z = __t35;
    return __t36;
}
struct Vector3 __operator_overload_Vector3_TK_MULTIPLY_Vector3(struct Vector3 a, struct Vector3 b) {
    f32 __t37 = a.x * b.x;
    f32 __t38 = a.y * b.y;
    f32 __t39 = a.z * b.z;
    struct Vector3 __t40 = {0};
    __t40.x = __t37;
    __t40.y = __t38;
    __t40.z = __t39;
    return __t40;
}
struct Vector3 __operator_overload_Vector3_TK_DIVIDE_Vector3(struct Vector3 a, struct Vector3 b) {
    f32 __t41 = a.x / b.x;
    f32 __t42 = a.y / b.y;
    f32 __t43 = a.z / b.z;
    struct Vector3 __t44 = {0};
    __t44.x = __t41;
    __t44.y = __t42;
    __t44.z = __t43;
    return __t44;
}
struct Vector3 __operator_overload_Vector3_TK_MULTIPLY_f32(struct Vector3 a, f32 f) {
    f32 __t45 = a.x * f;
    f32 __t46 = a.y * f;
    f32 __t47 = a.z * f;
    struct Vector3 __t48 = {0};
    __t48.x = __t45;
    __t48.y = __t46;
    __t48.z = __t47;
    return __t48;
}
void operator_overloading() {
    print_string(MAKE_STRING("\n\n---- operator_overloading ----\n", 33));
    struct Vector3 __t49 = {0};
    __t49.x = 1.000000;
    __t49.y = 2.000000;
    __t49.z = 3.000000;
    struct Vector3 v1 = __t49;
    struct Vector3 __t50 = {0};
    __t50.x = 1.000000;
    __t50.y = 4.000000;
    __t50.z = 9.000000;
    struct Vector3 v2 = __t50;
    struct Vector3 __t51 = __operator_overload_Vector3_TK_PLUS_Vector3(v1, v2);
    struct Vector3 v3 = __t51;
    struct Vector3 __t52 = __operator_overload_Vector3_TK_MULTIPLY_f32(v3, 5.000000);
    struct Vector3 v4 = __t52;
    print_float(v4.x);
    print_float(v4.y);
    print_float(v4.z);
    bool __t53 = v4.x == 10.000000;
    assert(__t53);
    bool __t54 = v4.y == 30.000000;
    assert(__t54);
    bool __t55 = v4.z == 60.000000;
    assert(__t55);
}
i64 value_poly__polymorph_0() {
    return 4;
}
i64 type_poly__polymorph_1(i64 a) {
    i64 __t56 = a * a;
    return __t56;
}
f32 type_poly__polymorph_2(f32 a) {
    f32 __t57 = a * a;
    return __t57;
}
i64 value_and_type_poly__polymorph_3() {
    return 49;
}
f32 value_and_type_poly__polymorph_4() {
    return 64.000000;
}
void procedural_polymorphism() {
    print_string(MAKE_STRING("\n\n---- procedural_polymorphism ----\n", 36));
    i64 __t58 = value_poly__polymorph_0();
    print_int(__t58);
    i64 __t60 = value_poly__polymorph_0();
    bool __t59 = __t60 == 4;
    assert(__t59);
    i64 __t61 = type_poly__polymorph_1(3);
    print_int(__t61);
    i64 __t63 = type_poly__polymorph_1(3);
    bool __t62 = __t63 == 9;
    assert(__t62);
    f32 __t64 = type_poly__polymorph_2(4.000000);
    print_float(__t64);
    f32 __t66 = type_poly__polymorph_2(4.000000);
    bool __t65 = __t66 == 16.000000;
    assert(__t65);
    i64 a = 5;
    f32 f = 6.000000;
    i64 __t67 = type_poly__polymorph_1(a);
    print_int(__t67);
    f32 __t68 = type_poly__polymorph_2(f);
    print_float(__t68);
    i64 __t70 = type_poly__polymorph_1(a);
    bool __t69 = __t70 == 25;
    assert(__t69);
    f32 __t72 = type_poly__polymorph_2(f);
    bool __t71 = __t72 == 36.000000;
    assert(__t71);
    i64 __t73 = value_and_type_poly__polymorph_3();
    print_int(__t73);
    f32 __t74 = value_and_type_poly__polymorph_4();
    print_float(__t74);
    i64 __t76 = value_and_type_poly__polymorph_3();
    bool __t75 = __t76 == 49;
    assert(__t75);
    f32 __t78 = value_and_type_poly__polymorph_4();
    bool __t77 = __t78 == 64.000000;
    assert(__t77);
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
    i64 *__t79 = __operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(&array_of_ints, 4);
    (*__t79) = 124;
    i64 *__t80 = __operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(&array_of_ints, 4);
    print_int((*__t80));
    i64 *__t82 = __operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(&array_of_ints, 4);
    bool __t81 = (*__t82) == 124;
    assert(__t81);
}
struct Dynamic_Array__polymorph_6 {
    Slice array;
    i64 count;
};
struct Vector3 *__operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(struct Dynamic_Array__polymorph_6 *dyn, i64 index) {
    return &((struct Vector3 *)(*dyn).array.data)[index];
}
void append__polymorph_7(struct Dynamic_Array__polymorph_6 *dyn, struct Vector3 value) {
    bool __t83 = dyn->count == dyn->array.count;
    if (__t83) {
        struct Vector3 *old_data = *((struct Vector3 **)&dyn->array.data);
        i64 __t85 = dyn->array.count * 2;
        i64 __t84 = 8 + __t85;
        i64 new_cap = __t84;
        i64 __t86 = new_cap * 12;
        void *__t87 = alloc(__t86);
        struct Vector3 *__t88 = ((struct Vector3 *)__t87);
        *((struct Vector3 **)&dyn->array.data) = __t88;
        dyn->array.count = new_cap;
        bool __t89 = old_data != NULL;
        if (__t89) {
            i64 __t90 = dyn->count * 12;
            u32 __t91 = ((u32 )__t90);
            void *__t92 = memcpy(*((struct Vector3 **)&dyn->array.data), old_data, __t91);
            free(old_data);
        }
    }
    bool __t93 = dyn->count < dyn->array.count;
    assert(__t93);
    ((struct Vector3 *)dyn->array.data)[dyn->count] = value;
    dyn->count += 1;
}
void destroy_dynamic_array__polymorph_8(struct Dynamic_Array__polymorph_6 dyn) {
    bool __t94 = *((struct Vector3 **)&dyn.array.data) != NULL;
    if (__t94) {
        free(*((struct Vector3 **)&dyn.array.data));
    }
}
void dynamic_arrays() {
    print_string(MAKE_STRING("\n\n---- dynamic_arrays ----\n", 27));
    struct Dynamic_Array__polymorph_6 dyn = {0};
    struct Vector3 __t95 = {0};
    __t95.x = 1.000000;
    __t95.y = 2.000000;
    __t95.z = 3.000000;
    append__polymorph_7(&dyn, __t95);
    struct Vector3 __t96 = {0};
    __t96.x = 1.000000;
    __t96.y = 4.000000;
    __t96.z = 9.000000;
    append__polymorph_7(&dyn, __t96);
    struct Vector3 __t97 = {0};
    __t97.x = 2.000000;
    __t97.y = 8.000000;
    __t97.z = 18.000000;
    append__polymorph_7(&dyn, __t97);
    struct Vector3 *__t99 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, 1);
    bool __t98 = (*__t99).x == 1.000000;
    assert(__t98);
    struct Vector3 *__t101 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, 1);
    bool __t100 = (*__t101).y == 4.000000;
    assert(__t100);
    struct Vector3 *__t103 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, 1);
    bool __t102 = (*__t103).z == 9.000000;
    assert(__t102);
    {
        i64 i = 0;
        while (true) {
            bool __t104 = i < dyn.count;
            if (!__t104) { break; }
            print_int(i);
            struct Vector3 *__t105 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, i);
            print_float((*__t105).x);
            struct Vector3 *__t106 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, i);
            print_float((*__t106).y);
            struct Vector3 *__t107 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, i);
            print_float((*__t107).z);
            i += 1;
        }
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
