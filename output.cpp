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
struct Vector3;
void foo(i64 *a);
void references();
void vector_proc(Vector3 v);
struct Some_Struct;
i64 return_stuff();
void main();
struct T;
struct B;
struct A;
struct Contains_Pointers1;
struct Self_Pointer;
void AAA();
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
void foo(i64 *a) {
    (*a) = 149;
    i64 b = 45768;
    (*a) = b;
}
void references() {
    {
        i64 cc = 332;
        print_int(cc);
        foo(&cc);
        print_int(cc);
        i64 *cref = &cc;
        (*cref) += 20;
        print_int((*cref));
        print(MAKE_STRING("--------------", 14));
        Vector3 __generated_compound_literal_5 = {};
        __generated_compound_literal_5.x = 1.000000;
        __generated_compound_literal_5.y = 4.000000;
        __generated_compound_literal_5.z = 9.000000;
        Vector3 v = __generated_compound_literal_5;
        Vector3 *vref = &v;
        (*vref).x = 1.000000;
        (*vref).y = 2.000000;
        (*vref).z = 3.000000;
        print_float((*vref).x);
        print_float((*vref).y);
        print_float((*vref).z);
    }
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
void vector_proc(Vector3 v) {
}
struct Some_Struct {
    i64 x;
};
i64 return_stuff() {
    if (true) {
        return 123;
    }
    return 321;
}
void main() {
    f32 xx = 1.000000;
    f32 yy = 4.000000;
    f32 zz = 9.000000;
    Vector3 __generated_compound_literal_6 = {};
    __generated_compound_literal_6.x = xx;
    __generated_compound_literal_6.y = yy;
    __generated_compound_literal_6.z = zz;
    Vector3 __generated_compound_literal_7 = {};
    __generated_compound_literal_7.x = __generated_compound_literal_6.y;
    __generated_compound_literal_7.y = yy;
    __generated_compound_literal_7.z = zz;
    Vector3 __generated_compound_literal_8 = {};
    __generated_compound_literal_8.x = xx;
    __generated_compound_literal_8.y = yy;
    __generated_compound_literal_8.z = zz;
    f32 some_float = __generated_compound_literal_7.x + __generated_compound_literal_8.z;
    print_float(some_float);
    Vector3 __generated_compound_literal_9 = {};
    __generated_compound_literal_9.x = xx;
    __generated_compound_literal_9.y = yy;
    __generated_compound_literal_9.z = zz;
    vector_proc(__generated_compound_literal_9);
    Vector3 __generated_compound_literal_10 = {};
    __generated_compound_literal_10.x = 2.000000;
    __generated_compound_literal_10.y = 8.000000;
    __generated_compound_literal_10.z = 18.000000;
    Vector3 my_vector = __generated_compound_literal_10;
    assert(my_vector.x == 2.000000);
    assert(my_vector.y == 8.000000);
    assert(my_vector.z == 18.000000);
    print_float(my_vector.x);
    print_float(my_vector.y);
    print_float(my_vector.z);
    i64 an_enum = 0;
    an_enum = 1;
    an_enum = 2;
    i64 asd = 2;
    // constant declaration omitted: N
    // constant declaration omitted: My_Int_Type
    // constant declaration omitted: MY_INT
    i64 my_int = 321;
    i64 value = 0;
    while (value < 10) {
        print_int(value);
        value = value + 1;
    }
    print_int(149);
    Vector3 *memory = ((Vector3 *)alloc(192));
    Slice slice = {};
    *((Vector3 **)&slice.data) = memory;
    slice.count = 16;
    for (i64 i = 0; i < slice.count; i = i + 1) {
        Vector3 *v = &((Vector3 *)slice.data)[i];
        v->x = 9.000000;
        v->y = 4.000000;
        v->z = 1.000000;
    }
    print(MAKE_STRING("Hello, World!", 13));
    Vector3 v_in_slice = ((Vector3 *)slice.data)[9];
    print_float(v_in_slice.x);
    print_float(v_in_slice.y);
    print_float(v_in_slice.z);
    Vector3 *v_ptr = ((Vector3 *)alloc(12));
    v_ptr->x = 1.000000;
    v_ptr->y = 4.000000;
    v_ptr->z = 9.000000;
    print_float(v_ptr->x);
    print_float(v_ptr->y);
    print_float(v_ptr->z);
    free(v_ptr);
    String str = MAKE_STRING("Hello, World", 12);
    print(str);
    print_int(MAKE_STRING("asd123", 6).count);
    Static_Array<i64 , 3> arr = {};
    arr.elements[0] = 1;
    arr.elements[1] = 4;
    arr.elements[2] = 9;
    for (i64 i = 0; i < 3; i = i + 1) {
        print_int(arr.elements[i]);
    }
    Vector3 v = {};
    v.x = 1.500000;
    v.y = 4.400000;
    v.z = 9.300000;
    print_float(v.x);
    print_float(v.y);
    print_float(v.z);
    Static_Array<i64 , 4> a = {};
    i64 x = *(&a.elements[2]);
    Vector3 *v_ptr2 = &v;
    v_ptr2->x = 2.000000;
    v_ptr2->y = ((f32 )return_stuff());
    f32 *x_ptr = ((f32 *)v_ptr2);
    *x_ptr = 149.000000;
    if (*x_ptr == 40.000000) {
        return;
    }
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
