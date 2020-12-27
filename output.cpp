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
void basic_stuff();
void arrays_by_value(Static_Array<i64 , 4> arr);
void arrays();
void slices();
struct Vector3;
void operator_overloading();
i64 value_poly__polymorph_0();
i64 value_poly__polymorph_1();
i64 value_poly__polymorph_2();
i64 type_poly__polymorph_3(i64 a);
i64 type_poly__polymorph_4(i64 a);
i64 type_poly__polymorph_5(i64 a);
f32 type_poly__polymorph_6(f32 a);
f32 type_poly__polymorph_7(f32 a);
f32 type_poly__polymorph_8(f32 a);
i64 type_poly__polymorph_9(i64 a);
f32 type_poly__polymorph_10(f32 a);
i64 type_poly__polymorph_11(i64 a);
i64 type_poly__polymorph_12(i64 a);
f32 type_poly__polymorph_13(f32 a);
f32 type_poly__polymorph_14(f32 a);
i64 value_and_type_poly__polymorph_15();
f32 value_and_type_poly__polymorph_16();
i64 value_and_type_poly__polymorph_17();
i64 value_and_type_poly__polymorph_18();
f32 value_and_type_poly__polymorph_19();
f32 value_and_type_poly__polymorph_20();
void procedural_polymorphism();
void change_by_reference(i64 *a, i64 value);
void references();
void main();
void vector_proc(Vector3 v);
i64 return_stuff();
struct T;
struct B;
struct A;
struct Contains_Pointers1;
struct Self_Pointer;
void AAA();
struct Some_Struct;
void recursion();
void duo_recursion2();
void duo_recursion1();
void bar(i64 x);
void foo2(i64 x);
void baz(i64 x);
struct Contains_Pointers2;

// Actual declarations
struct Dynamic_Array_Int {
    Slice array;
    i64 count;
};
i64 *__operator_overload_Dynamic_Array_Int_TK_LEFT_SQUARE(Dynamic_Array_Int *dyn, i64 index) {
    return &((i64 *)(*dyn).array.data)[index];
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
void basic_stuff() {
}
void arrays_by_value(Static_Array<i64 , 4> arr) {
    arr.elements[2] = 738;
}
void arrays() {
    print(MAKE_STRING("\n\n---- arrays ----", 18));
    Static_Array<i64 , 4> my_array = {};
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
struct Vector3 {
    f32 x;
    f32 y;
    f32 z;
};
Vector3 __operator_overload_Vector3_TK_PLUS(Vector3 a, Vector3 b) {
    Vector3 __generated_compound_literal_0 = {};
    __generated_compound_literal_0.x = a.x + b.x;
    __generated_compound_literal_0.y = a.y + b.y;
    __generated_compound_literal_0.z = a.z + b.z;
    return __generated_compound_literal_0;
}
Vector3 __operator_overload_Vector3_TK_MINUS(Vector3 a, Vector3 b) {
    Vector3 __generated_compound_literal_1 = {};
    __generated_compound_literal_1.x = a.x - b.x;
    __generated_compound_literal_1.y = a.y - b.y;
    __generated_compound_literal_1.z = a.z - b.z;
    return __generated_compound_literal_1;
}
Vector3 __operator_overload_Vector3_TK_MULTIPLY(Vector3 a, Vector3 b) {
    Vector3 __generated_compound_literal_2 = {};
    __generated_compound_literal_2.x = a.x * b.x;
    __generated_compound_literal_2.y = a.y * b.y;
    __generated_compound_literal_2.z = a.z * b.z;
    return __generated_compound_literal_2;
}
Vector3 __operator_overload_Vector3_TK_DIVIDE(Vector3 a, Vector3 b) {
    Vector3 __generated_compound_literal_3 = {};
    __generated_compound_literal_3.x = a.x / b.x;
    __generated_compound_literal_3.y = a.y / b.y;
    __generated_compound_literal_3.z = a.z / b.z;
    return __generated_compound_literal_3;
}
Vector3 __operator_overload_Vector3_TK_MULTIPLY(Vector3 a, f32 f) {
    Vector3 __generated_compound_literal_4 = {};
    __generated_compound_literal_4.x = a.x * f;
    __generated_compound_literal_4.y = a.y * f;
    __generated_compound_literal_4.z = a.z * f;
    return __generated_compound_literal_4;
}
void operator_overloading() {
    print(MAKE_STRING("\n\n---- operator_overloading ----", 32));
    Vector3 __generated_compound_literal_5 = {};
    __generated_compound_literal_5.x = 1.000000;
    __generated_compound_literal_5.y = 2.000000;
    __generated_compound_literal_5.z = 3.000000;
    Vector3 v1 = __generated_compound_literal_5;
    Vector3 __generated_compound_literal_6 = {};
    __generated_compound_literal_6.x = 1.000000;
    __generated_compound_literal_6.y = 4.000000;
    __generated_compound_literal_6.z = 9.000000;
    Vector3 v2 = __generated_compound_literal_6;
    Vector3 v3 = __operator_overload_Vector3_TK_PLUS(v1, v2);
    Vector3 v4 = __operator_overload_Vector3_TK_MULTIPLY(v3, 5.000000);
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
i64 value_poly__polymorph_1() {
    return 4;
}
i64 value_poly__polymorph_2() {
    return 4;
}
i64 type_poly__polymorph_3(i64 a) {
    return a * a;
}
i64 type_poly__polymorph_4(i64 a) {
    return a * a;
}
i64 type_poly__polymorph_5(i64 a) {
    return a * a;
}
f32 type_poly__polymorph_6(f32 a) {
    return a * a;
}
f32 type_poly__polymorph_7(f32 a) {
    return a * a;
}
f32 type_poly__polymorph_8(f32 a) {
    return a * a;
}
i64 type_poly__polymorph_9(i64 a) {
    return a * a;
}
f32 type_poly__polymorph_10(f32 a) {
    return a * a;
}
i64 type_poly__polymorph_11(i64 a) {
    return a * a;
}
i64 type_poly__polymorph_12(i64 a) {
    return a * a;
}
f32 type_poly__polymorph_13(f32 a) {
    return a * a;
}
f32 type_poly__polymorph_14(f32 a) {
    return a * a;
}
i64 value_and_type_poly__polymorph_15() {
    return 49;
}
f32 value_and_type_poly__polymorph_16() {
    return 64.000000;
}
i64 value_and_type_poly__polymorph_17() {
    return 49;
}
i64 value_and_type_poly__polymorph_18() {
    return 49;
}
f32 value_and_type_poly__polymorph_19() {
    return 64.000000;
}
f32 value_and_type_poly__polymorph_20() {
    return 64.000000;
}
void procedural_polymorphism() {
    print(MAKE_STRING("\n\n---- procedural_polymorphism ----", 35));
    print_int(value_poly__polymorph_0());
    assert(value_poly__polymorph_2() == 4);
    print_int(type_poly__polymorph_3(3));
    assert(type_poly__polymorph_5(3) == 9);
    print_float(type_poly__polymorph_6(4.000000));
    assert(type_poly__polymorph_8(4.000000) == 16.000000);
    i64 a = 5;
    f32 f = 6.000000;
    print_int(type_poly__polymorph_9(a));
    print_float(type_poly__polymorph_10(f));
    assert(type_poly__polymorph_12(a) == 25);
    assert(type_poly__polymorph_14(f) == 36.000000);
    print_int(value_and_type_poly__polymorph_15());
    print_float(value_and_type_poly__polymorph_16());
    assert(value_and_type_poly__polymorph_18() == 49);
    assert(value_and_type_poly__polymorph_20() == 64.000000);
}
void change_by_reference(i64 *a, i64 value) {
    (*a) = value;
}
void references() {
    print(MAKE_STRING("\n\n---- references ----", 22));
    i64 my_int = 123;
    print_int(my_int);
    assert(my_int == 123);
    change_by_reference(&my_int, 456);
    print_int(my_int);
    assert(my_int == 456);
    i64 *int_reference = &my_int;
    (*int_reference) = 789;
    assert((*int_reference) == 789);
    assert(my_int == 789);
    print_int((*int_reference));
    {
        Dynamic_Array_Int arr = {};
        append(&arr, 1);
        append(&arr, 4);
        append(&arr, 9);
        assert((*__operator_overload_Dynamic_Array_Int_TK_LEFT_SQUARE(&arr, 0)) == 1);
        assert((*__operator_overload_Dynamic_Array_Int_TK_LEFT_SQUARE(&arr, 1)) == 4);
        assert((*__operator_overload_Dynamic_Array_Int_TK_LEFT_SQUARE(&arr, 2)) == 9);
        (*__operator_overload_Dynamic_Array_Int_TK_LEFT_SQUARE(&arr, 1)) = 334;
        assert((*__operator_overload_Dynamic_Array_Int_TK_LEFT_SQUARE(&arr, 1)) == 334);
        for (i64 i = 0; i < arr.count; i += 1) {
            print_int((*__operator_overload_Dynamic_Array_Int_TK_LEFT_SQUARE(&arr, i)));
        }
    }
}
void main() {
    basic_stuff();
    arrays();
    slices();
    operator_overloading();
    procedural_polymorphism();
    references();
}
void vector_proc(Vector3 v) {
}
i64 return_stuff() {
    if (true) {
        return 123;
    }
    return 321;
}
T *p = {};
struct T {
    Static_Array<bool , 8> a;
};
Static_Array<i64 , 8> a = {};
struct B {
    A *a;
};
struct A {
    Static_Array<B , 4> b;
};
struct Contains_Pointers1 {
    Contains_Pointers2 *a;
};
struct Self_Pointer {
    Self_Pointer *b;
};
void AAA() {
    A aa = {};
    B b = aa.b.elements[2];
    Contains_Pointers1 *c = {};
    Contains_Pointers2 *d = c->a;
    c->a = d;
    Self_Pointer e = {};
}
i64 simple = {};
i64 *pointer = {};
Static_Array<i64 , 4> array = {};
i64 **pointer_to_pointer = {};
Static_Array<Static_Array<i64 , 8> , 4> array_of_arrays = {};
Static_Array<i64 , 4> *pointer_to_array = {};
Static_Array<i64 *, 4> array_of_pointers = {};
Static_Array<Static_Array<i64 , 8> , 4> **wack = {};
Static_Array<Static_Array<i64 **, 8> , 4> more_wack = {};
Static_Array<Static_Array<i64 , 8> *, 4> *still_more_wack = {};
Static_Array<Static_Array<Static_Array<i64 *, 32> , 8> *, 4> **complicated_garbage = {};
struct Some_Struct {
    i64 x;
};
void recursion() {
    Some_Struct *b = {};
    i64 x = b->x;
    recursion();
}
void duo_recursion2() {
    Some_Struct a = {};
    duo_recursion1();
}
void duo_recursion1() {
    duo_recursion2();
}
void bar(i64 x) {
    baz(2);
    i64 *y = {};
    i64 z = *y;
    z = 123 + z;
    z = z + 123;
    z = 24;
}
void foo2(i64 x) {
    bar(1);
    Some_Struct b = {};
    Static_Array<bool , 149> neato = {};
    i64 z = {};
    z = 321;
}
void baz(i64 x) {
    foo2(x);
}
i64 global_var = {};
struct Contains_Pointers2 {
    Contains_Pointers1 *b;
};
