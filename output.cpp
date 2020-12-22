#include <stdint.h>
#include <stdbool.h>
typedef int64_t i64;
typedef float f32;
typedef double f64;
// Forward declarations
struct Vector3;
i64 return_stuff();
void main();
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
void foo(i64 x);
void baz(i64 x);
struct Contains_Pointers2;

// Actual declarations
struct Vector3 {
    f32 x;
    f32 y;
    f32 z;
};
i64 return_stuff() {
    if (true) {
        return 123;
    }
    return 321;
}
void main() {
    Vector3 v = {};
    v.x = 1.500000;
    v.y = 4.000000;
    v.z = 9.000000;
    Vector3 (*v_ptr) = &v;
    v_ptr->x = 2.000000;
    v_ptr->y = ((f32 )return_stuff());
    f32 (*x_ptr) = ((f32 (*))v_ptr);
    *x_ptr = 149.000000;
}
T (*p) = {};
struct T {
    bool a[8];
};
i64 a[8] = {};
struct B {
    A (*a);
};
struct A {
    B b[4];
};
struct Contains_Pointers1 {
    Contains_Pointers2 (*a);
};
struct Self_Pointer {
    Self_Pointer (*b);
};
void AAA() {
    A aa = {};
    B b = aa.b[2];
    Contains_Pointers1 (*c) = {};
    Contains_Pointers2 (*d) = c->a;
    c->a = d;
    Self_Pointer e = {};
}
i64 simple = {};
i64 (*pointer) = {};
i64 array[4] = {};
i64 (*(*pointer_to_pointer)) = {};
i64 array_of_arrays[4][8] = {};
i64 (*pointer_to_array)[4] = {};
i64 (*array_of_pointers[4]) = {};
i64 (*(*wack))[4][8] = {};
i64 (*(*more_wack[4][8])) = {};
i64 (*(*still_more_wack)[4])[8] = {};
i64 (*(*(*(*complicated_garbage))[4])[8][32]) = {};
struct Some_Struct {
    i64 x;
};
void recursion() {
    Some_Struct (*b) = {};
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
    i64 (*y) = {};
    i64 z = *y;
    z = 123 + z;
    z = z + 123;
    z = 24;
}
i64 global_var = {};
void foo(i64 x) {
    bar(1);
    global_var;
    Some_Struct b = {};
    i64 z = {};
    z = 321;
}
void baz(i64 x) {
    foo(x);
}
struct Contains_Pointers2 {
    Contains_Pointers1 (*b);
};
