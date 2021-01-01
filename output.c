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
void procedure_with_varargs(Slice numbers);
void varargs();
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
    print_string(MAKE_STRING("\n\n---- strings ----\n", 20));
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
void procedure_with_varargs(Slice numbers) {
    print_int(numbers.count);
    {
        i64 i = 0;
        while (true) {
            bool __t24 = i < numbers.count;
            if (!__t24) { break; }
            print_string(MAKE_STRING("    ", 4));
            print_int(((i64 *)numbers.data)[i]);
            i += 1;
        }
    }
}
void varargs() {
    print_string(MAKE_STRING("\n\n---- varargs ----\n", 20));
    i64 __t25[4];
    __t25[0] = 1;
    __t25[1] = 2;
    __t25[2] = 3;
    __t25[3] = 4;
    Slice __t26;
    __t26.data = __t25;
    __t26.count = 4;
    procedure_with_varargs(__t26);
    i64 __t27[2];
    __t27[0] = 1;
    __t27[1] = 2;
    Slice __t28;
    __t28.data = __t27;
    __t28.count = 2;
    procedure_with_varargs(__t28);
    i64 __t29[1];
    Slice __t30;
    __t30.data = __t29;
    __t30.count = 0;
    procedure_with_varargs(__t30);
}
void change_by_reference(i64 *a, i64 value) {
    (*a) = value;
}
void references() {
    print_string(MAKE_STRING("\n\n---- references ----\n", 23));
    i64 my_int = 123;
    print_int(my_int);
    bool __t31 = my_int == 123;
    assert(__t31);
    i64 *int_reference = &my_int;
    (*int_reference) = 789;
    bool __t32 = (*int_reference) == 789;
    assert(__t32);
    bool __t33 = my_int == 789;
    assert(__t33);
    print_int((*int_reference));
    bool __t34 = my_int == 789;
    assert(__t34);
    change_by_reference(&my_int, 456);
    bool __t35 = my_int == 456;
    assert(__t35);
    print_int(my_int);
}
struct Vector3 {
    f32 x;
    f32 y;
    f32 z;
};
struct Vector3 __operator_overload_Vector3_TK_PLUS_Vector3(struct Vector3 a, struct Vector3 b) {
    f32 __t36 = a.x + b.x;
    f32 __t37 = a.y + b.y;
    f32 __t38 = a.z + b.z;
    struct Vector3 __t39 = {0};
    __t39.x = __t36;
    __t39.y = __t37;
    __t39.z = __t38;
    return __t39;
}
struct Vector3 __operator_overload_Vector3_TK_MINUS_Vector3(struct Vector3 a, struct Vector3 b) {
    f32 __t40 = a.x - b.x;
    f32 __t41 = a.y - b.y;
    f32 __t42 = a.z - b.z;
    struct Vector3 __t43 = {0};
    __t43.x = __t40;
    __t43.y = __t41;
    __t43.z = __t42;
    return __t43;
}
struct Vector3 __operator_overload_Vector3_TK_MULTIPLY_Vector3(struct Vector3 a, struct Vector3 b) {
    f32 __t44 = a.x * b.x;
    f32 __t45 = a.y * b.y;
    f32 __t46 = a.z * b.z;
    struct Vector3 __t47 = {0};
    __t47.x = __t44;
    __t47.y = __t45;
    __t47.z = __t46;
    return __t47;
}
struct Vector3 __operator_overload_Vector3_TK_DIVIDE_Vector3(struct Vector3 a, struct Vector3 b) {
    f32 __t48 = a.x / b.x;
    f32 __t49 = a.y / b.y;
    f32 __t50 = a.z / b.z;
    struct Vector3 __t51 = {0};
    __t51.x = __t48;
    __t51.y = __t49;
    __t51.z = __t50;
    return __t51;
}
struct Vector3 __operator_overload_Vector3_TK_MULTIPLY_f32(struct Vector3 a, f32 f) {
    f32 __t52 = a.x * f;
    f32 __t53 = a.y * f;
    f32 __t54 = a.z * f;
    struct Vector3 __t55 = {0};
    __t55.x = __t52;
    __t55.y = __t53;
    __t55.z = __t54;
    return __t55;
}
void operator_overloading() {
    print_string(MAKE_STRING("\n\n---- operator_overloading ----\n", 33));
    struct Vector3 __t56 = {0};
    __t56.x = 1.000000;
    __t56.y = 2.000000;
    __t56.z = 3.000000;
    struct Vector3 v1 = __t56;
    struct Vector3 __t57 = {0};
    __t57.x = 1.000000;
    __t57.y = 4.000000;
    __t57.z = 9.000000;
    struct Vector3 v2 = __t57;
    struct Vector3 __t58 = __operator_overload_Vector3_TK_PLUS_Vector3(v1, v2);
    struct Vector3 v3 = __t58;
    struct Vector3 __t59 = __operator_overload_Vector3_TK_MULTIPLY_f32(v3, 5.000000);
    struct Vector3 v4 = __t59;
    print_float(v4.x);
    print_float(v4.y);
    print_float(v4.z);
    bool __t60 = v4.x == 10.000000;
    assert(__t60);
    bool __t61 = v4.y == 30.000000;
    assert(__t61);
    bool __t62 = v4.z == 60.000000;
    assert(__t62);
}
i64 value_poly__polymorph_0() {
    return 4;
}
i64 type_poly__polymorph_1(i64 a) {
    i64 __t63 = a * a;
    return __t63;
}
f32 type_poly__polymorph_2(f32 a) {
    f32 __t64 = a * a;
    return __t64;
}
i64 value_and_type_poly__polymorph_3() {
    return 49;
}
f32 value_and_type_poly__polymorph_4() {
    return 64.000000;
}
void procedural_polymorphism() {
    print_string(MAKE_STRING("\n\n---- procedural_polymorphism ----\n", 36));
    i64 __t65 = value_poly__polymorph_0();
    print_int(__t65);
    i64 __t67 = value_poly__polymorph_0();
    bool __t66 = __t67 == 4;
    assert(__t66);
    i64 __t68 = type_poly__polymorph_1(3);
    print_int(__t68);
    i64 __t70 = type_poly__polymorph_1(3);
    bool __t69 = __t70 == 9;
    assert(__t69);
    f32 __t71 = type_poly__polymorph_2(4.000000);
    print_float(__t71);
    f32 __t73 = type_poly__polymorph_2(4.000000);
    bool __t72 = __t73 == 16.000000;
    assert(__t72);
    i64 a = 5;
    f32 f = 6.000000;
    i64 __t74 = type_poly__polymorph_1(a);
    print_int(__t74);
    f32 __t75 = type_poly__polymorph_2(f);
    print_float(__t75);
    i64 __t77 = type_poly__polymorph_1(a);
    bool __t76 = __t77 == 25;
    assert(__t76);
    f32 __t79 = type_poly__polymorph_2(f);
    bool __t78 = __t79 == 36.000000;
    assert(__t78);
    i64 __t80 = value_and_type_poly__polymorph_3();
    print_int(__t80);
    f32 __t81 = value_and_type_poly__polymorph_4();
    print_float(__t81);
    i64 __t83 = value_and_type_poly__polymorph_3();
    bool __t82 = __t83 == 49;
    assert(__t82);
    f32 __t85 = value_and_type_poly__polymorph_4();
    bool __t84 = __t85 == 64.000000;
    assert(__t84);
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
    i64 *__t86 = __operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(&array_of_ints, 4);
    (*__t86) = 124;
    i64 *__t87 = __operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(&array_of_ints, 4);
    print_int((*__t87));
    i64 *__t89 = __operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(&array_of_ints, 4);
    bool __t88 = (*__t89) == 124;
    assert(__t88);
}
struct Dynamic_Array__polymorph_6 {
    Slice array;
    i64 count;
};
struct Vector3 *__operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(struct Dynamic_Array__polymorph_6 *dyn, i64 index) {
    return &((struct Vector3 *)(*dyn).array.data)[index];
}
void append__polymorph_7(struct Dynamic_Array__polymorph_6 *dyn, struct Vector3 value) {
    bool __t90 = dyn->count == dyn->array.count;
    if (__t90) {
        struct Vector3 *old_data = *((struct Vector3 **)&dyn->array.data);
        i64 __t92 = dyn->array.count * 2;
        i64 __t91 = 8 + __t92;
        i64 new_cap = __t91;
        i64 __t93 = new_cap * 12;
        void *__t94 = alloc(__t93);
        struct Vector3 *__t95 = ((struct Vector3 *)__t94);
        *((struct Vector3 **)&dyn->array.data) = __t95;
        dyn->array.count = new_cap;
        bool __t96 = old_data != NULL;
        if (__t96) {
            i64 __t97 = dyn->count * 12;
            u32 __t98 = ((u32 )__t97);
            void *__t99 = memcpy(*((struct Vector3 **)&dyn->array.data), old_data, __t98);
            free(old_data);
        }
    }
    bool __t100 = dyn->count < dyn->array.count;
    assert(__t100);
    ((struct Vector3 *)dyn->array.data)[dyn->count] = value;
    dyn->count += 1;
}
void destroy_dynamic_array__polymorph_8(struct Dynamic_Array__polymorph_6 dyn) {
    bool __t101 = *((struct Vector3 **)&dyn.array.data) != NULL;
    if (__t101) {
        free(*((struct Vector3 **)&dyn.array.data));
    }
}
void dynamic_arrays() {
    print_string(MAKE_STRING("\n\n---- dynamic_arrays ----\n", 27));
    struct Dynamic_Array__polymorph_6 dyn = {0};
    struct Vector3 __t102 = {0};
    __t102.x = 1.000000;
    __t102.y = 2.000000;
    __t102.z = 3.000000;
    append__polymorph_7(&dyn, __t102);
    struct Vector3 __t103 = {0};
    __t103.x = 1.000000;
    __t103.y = 4.000000;
    __t103.z = 9.000000;
    append__polymorph_7(&dyn, __t103);
    struct Vector3 __t104 = {0};
    __t104.x = 2.000000;
    __t104.y = 8.000000;
    __t104.z = 18.000000;
    append__polymorph_7(&dyn, __t104);
    struct Vector3 *__t106 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, 1);
    bool __t105 = (*__t106).x == 1.000000;
    assert(__t105);
    struct Vector3 *__t108 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, 1);
    bool __t107 = (*__t108).y == 4.000000;
    assert(__t107);
    struct Vector3 *__t110 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, 1);
    bool __t109 = (*__t110).z == 9.000000;
    assert(__t109);
    {
        i64 i = 0;
        while (true) {
            bool __t111 = i < dyn.count;
            if (!__t111) { break; }
            print_int(i);
            struct Vector3 *__t112 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, i);
            print_float((*__t112).x);
            struct Vector3 *__t113 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, i);
            print_float((*__t113).y);
            struct Vector3 *__t114 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, i);
            print_float((*__t114).z);
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
    varargs();
    references();
    operator_overloading();
    procedural_polymorphism();
    structural_polymorphism();
    dynamic_arrays();
    return 0;
}
