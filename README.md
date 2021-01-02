# sif
sif is a simple imperative procedural language made with the goal of being a slightly-higher-level C.

## Hello, World!

```odin
#include "core:basic.sif"

proc main() : i32 {
    print("Hello, World!\n");
    return 0;
}
```

## Features

- The basic stuff: variables, procedures, structs, ints, floats, bool, pointers, arrays, etc
- Length-delimited strings, not null-terminated
- Operator overloading
- Procedural and structural polymorphism
- Order-independent declarations
- `any` type (see demo below)

## Features Planned

- Runtime type information
- Tagged unions
- defer statement
- `using` statement for namespace inclusion
- Runtime bounds checks
- Other small things you'd expect like static #if, switch statements, etc
- Currently the code-generator outputs C code, in the future I'd like to either try to use LLVM or write a custom x64 backend

## Building and Running

1. Clone the repo.
2. Run `build.bat`
3. Make sure that you have run vcvars so `cl` is in your path (this is a temporary step, will work around it later)
4. Put the following into a file called `my_program.sif`:
```odin
#include "core:basic.sif"

proc main() : i32 {
    print("Hello, World!\n");
    return 0;
}
```
5. Run `bin/sif.exe run my_program.sif`

## Demo

The following is a demo program showing off many of the features of sif.

```odin
#include "core:basic.sif"

proc main() : i32 {
    print("-------------------------\n");
    print("|   sif language demo   |\n");
    print("-------------------------\n");

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
    any_type();
    function_pointers();
    dynamic_arrays();
    return 0;
}



proc factorial(var n: int) : int {
    if (n == 1) {
        return 1;
    }
    return n * factorial(n-1);
}
proc basic_stuff() {
    print("\n\n---- basic_stuff ----\n");

    // declaration syntax:
    // var <variable name>: <type> = <expression>;
    var a: int = 123;

    // type can be omitted if it can be inferred from the right-hand-side expression
    var b1 = 123;
    assert(typeof(b1) == int);
    var b2 = 123.0;
    assert(typeof(b2) == float);

    // since factorial() returns an int, the type can be inferred here too
    var fact = factorial(5);
    assert(fact == 120);
    assert(typeof(fact) == int);

    // number literals are "untyped" and can coerce when needed
    var c: int = 12;
    var d: float = 12;

    // even when nested under binary expressions
    var e: float = 11 / 2;
    assert(e == 5.5);

    // without type inference the "untyped" numbers default to int, as above
    var f = 11 / 2;
    assert(f == 5);

    // if a `.` is present, float will be assumed, as above
    var g = 11.0 / 2;
    assert(g == 5.5);
}



proc arrays_by_value(arr: [4]int) {
    arr[2] = 738;
}
proc arrays() {
    print("\n\n---- arrays ----\n");
    var my_array: [4]int;
    my_array[0] = 1;
    my_array[1] = 2;
    my_array[2] = 3;
    my_array[3] = 4;
    print("%\n", my_array[0]);
    print("%\n", my_array[1]);
    print("%\n", my_array[2]);
    print("%\n", my_array[3]);
    assert(my_array[0] == 1);
    assert(my_array[1] == 2);
    assert(my_array[2] == 3);
    assert(my_array[3] == 4);
    arrays_by_value(my_array);
    print("%\n", my_array[2]);
    assert(my_array[2] == 3);
}



proc slices() {
    // todo(josh)
    // print("\n\n---- slices ----\n");
}



proc strings() {
    print("\n\n---- strings ----\n");

    // strings in sif are length-delimited rather than null-terminated.
    // the string type is implemented as:
    // struct String {
    //     var data: rawptr;
    //     var count: int;
    // }
    assert(sizeof(string) == 16);

    // since strings are length-delimited this means you can trivially
    // take a substring without allocating and copying
    var a = "Hello, World!";
    var hello: string;
    hello.data = &a[0];
    hello.count = 5;
    assert(hello == "Hello");
    print("%\n", hello);

    var world: string;
    world.data = &a[7];
    world.count = 5;
    assert(world == "World");
    print("%\n", world);
}



struct Foo {
    var a: int;
    var str: string;
    var t: bool;
}
proc structs() {
    print("\n\n---- structs ----\n");

    var f: Foo;
    assert(f.a == 0); // everything is zero initialized by default
    f.a = 123;
    f.str = "foozle";
    f.t = 120 == factorial(5);

    // can also use compound literals to initialize
    f = Foo{149, "hellooo", false};
}



proc enums() {
    // todo(josh)
    // print("\n\n---- enums ----\n");
}



proc order_independence() {
    print("\n\n---- order_independence ----\n");

    // declaration order does not matter. the compiler figures
    // out what depends on what for you. delete all your .h files :)

    var loopy: Loopy;
    assert(sizeof(typeof(loopy.a)) == 8);
    assert(sizeof(typeof(nesty)) == 64);
}
var nesty: [sizeof(typeof(ptr_to_loopy^))]int;
var ptr_to_loopy: ^Loopy;
struct Loopy {
    var a: [sizeof(typeof(ptr_to_loopy))]bool;
}



proc procedure_with_varargs(numbers: ..int) {
    print("varargs count: %\n", numbers.count);
    for (var i = 0; i < numbers.count; i += 1) {
        print("  %\n", numbers[i]);
    }
}
proc varargs() {
    print("\n\n---- varargs ----\n");

    procedure_with_varargs(1, 2, 3, 4);
    procedure_with_varargs(5, 6);
    procedure_with_varargs();
}



proc change_by_reference(a: >int, value: int) {
    // since `a` is a reference, it is implicitly a pointer
    // and dereferences/indirections are inserted automatically
    a = value;
}
proc references() {
    print("\n\n---- references ----\n");

    var my_int = 123;
    print("%\n", my_int);
    assert(my_int == 123);

    // reference implicitly take/dereference pointers depending on context
    // and are (currently, will likely change) denoted by a `>` before the type
    var int_reference: >int = my_int; // implicit `&my_int` after `=`
    int_reference = 789;              // implicit `int_reference^` before `=`
    assert(int_reference == 789);
    assert(my_int == 789);
    print("%\n", int_reference);

    // you can also define procedures that take (and return) references, of course
    assert(my_int == 789);
    change_by_reference(my_int, 456);
    assert(my_int == 456);
    print("%\n", my_int);

    // the only real reason I implemented references is for overloading []
    // which you can see below in the structural_polymorphism() and
    // dynamic_array() procedures
}



struct Vector3 {
    var x: float;
    var y: float;
    var z: float;
    operator +(a: Vector3, b: Vector3) : Vector3 {
        return Vector3{a.x + b.x, a.y + b.y, a.z + b.z};
    }
    operator -(a: Vector3, b: Vector3) : Vector3 {
        return Vector3{a.x - b.x, a.y - b.y, a.z - b.z};
    }
    operator *(a: Vector3, b: Vector3) : Vector3 {
        return Vector3{a.x * b.x, a.y * b.y, a.z * b.z};
    }
    operator /(a: Vector3, b: Vector3) : Vector3 {
        return Vector3{a.x / b.x, a.y / b.y, a.z / b.z};
    }

    operator *(a: Vector3, f: float) : Vector3 {
        return Vector3{a.x * f, a.y * f, a.z * f};
    }
}
proc operator_overloading() {
    print("\n\n---- operator_overloading ----\n");

    var v1 = Vector3{1, 2, 3};
    var v2 = Vector3{1, 4, 9};
    var v3 = v1 + v2;
    var v4 = v3 * 5;
    print("%\n", v4.x);
    print("%\n", v4.y);
    print("%\n", v4.z);
    assert(v4.x == 10);
    assert(v4.y == 30);
    assert(v4.z == 60);
}



// $ on the name of a parameter means this parameter MUST be constant
// and a new polymorph of the procedure will be baked with that constant.
// for example if you called `value_poly(4)` a new version of `value_poly`
// would be generated that replaces each use of the parameter `a` with the
// `4` that you passed. this means the return statement will constant fold
// into a simple `return 16;`
proc value_poly($a: int) : int {
    return a * a;
}
// $ on the type of a parameter means that this parameter can be ANY
// type, and a new version of this procedure will be generated for
// that type. for example `type_poly(124)` would generate a version
// of `type_poly` where `T` is `int`. `type_poly(Vector3{1, 4, 9})`
// would generate a version where `T` is `Vector3`
proc type_poly(a: $T) : T {
    return a * a;
}
// $ on both the name and the type means that this procedure can accept
// any type and it must be a constant. simply a combination of the two
// cases above.
proc value_and_type_poly($a: $T) : T {
    return a * a;
}
proc procedural_polymorphism() {
    print("\n\n---- procedural_polymorphism ----\n");

    print("%\n", value_poly(2));
    assert(value_poly(2) == 4);

    print("%\n", type_poly(3));
    assert(type_poly(3) == 9);
    print_float(type_poly(4.0));
    assert(type_poly(4.0) == 16);

    var a: int = 5;
    var f: float = 6;
    print("%\n", type_poly(a));
    print("%\n", type_poly(f));
    assert(type_poly(a) == 25);
    assert(type_poly(f) == 36);

    print("%\n", value_and_type_poly(7));
    print("%\n", value_and_type_poly(8.0));
    assert(value_and_type_poly(7) == 49);
    assert(value_and_type_poly(8.0) == 64);
}



// same with procedural polymorphism, a new version of the struct
// Custom_Array_Type will be generated for each set of parameters
// that are passed, creating a brand new type.
// for structs, unlike procedures, these values must all be constant
// values in order to maintain compile-time typesafety
struct Custom_Array_Type!($N: int, $T: typeid) {
    var array: [N]T;
    operator [](my_array: >Custom_Array_Type!(N, T), index: int) : >T {
        // implicitly takes a pointer to `my_array.array[index]` since this
        // procedure returns a reference
        return my_array.array[index];
    }
}
proc structural_polymorphism() {
    print("\n\n---- structural_polymorphism ----\n");

    var array_of_ints: Custom_Array_Type!(8, int);
    array_of_ints[4] = 124; // calls [] operator overload
    print("%\n", array_of_ints[4]);
    assert(array_of_ints[4] == 124);
}



proc any_type() {
    print("\n\n---- any ----\n");

    // an `any` is defined as the following:
    // struct Any {
    //     var data: rawptr;
    //     var type: typeid;
    // }
    // any type will implicitly convert to an `any` by taking a pointer to
    // the data and setting the type field to the appropriate typeid
    var some_int = 123;
    var a: any = some_int;
    assert(a.data == &some_int);
    assert(a.type == int);

    // you can access the data pointed to by an `any` by casting the data
    // pointer and dereferencing
    var b: int = cast(^int, a.data)^;
    print("%\n", b);

    // the most notable use of `any` is for a print routine. here is an
    // example using varargs
    print_things(true, 456, 78.9, "hello");
}
proc print_things(args: ..any) {
    for (var i = 0; i < args.count; i += 1) {
        var arg = args[i];
        if (arg.type == int) {
            print("%\n", cast(^int, arg.data)^);
        }
        else if (arg.type == string) {
            print("%\n", cast(^string, arg.data)^);
        }
        else if (arg.type == bool) {
            print("%\n", cast(^bool, arg.data)^);
        }
        else if (arg.type == float) {
            print("%\n", cast(^float, arg.data)^);
        }
    }
}



proc function_pointers() {
    print("\n\n---- any ----\n");

    var my_print1 = print; // uses type inference
    my_print1("This is a function pointer! %\n", "cool");

    var my_print2: proc(fmt: string, args: ..any) = print; // without type inference
    my_print2("This is also % a function % pointer\n", 45.6, true);
}



// the following is a barebones implementation of a resizable array
// using many of the features you've seen thus far including
// structural and procedural polymorphism and operator overloading

struct Dynamic_Array!($T: typeid) {
    var array: []T;
    var count: int;
    operator [](dyn: >Dynamic_Array!(T), index: int) : >T {
        return dyn.array[index];
    }
}

proc append(dyn: ^Dynamic_Array!($T), value: T) {
    if (dyn.count == dyn.array.count) {
        var old_data = dyn.array.data;
        var new_cap = 8 + dyn.array.count * 2;
        dyn.array.data = cast(^T, alloc(new_cap * sizeof(T)));
        dyn.array.count = new_cap;
        if (old_data != null) {
            memcpy(dyn.array.data, old_data, cast(u64, dyn.count * sizeof(T)));
            free(old_data);
        }
    }
    assert(dyn.count < dyn.array.count);
    dyn.array[dyn.count] = value;
    dyn.count += 1;
}

proc pop(dyn: ^Dynamic_Array!($T)) : T {
    assert(dyn.count > 0);
    var value = dyn[dyn.count-1];
    dyn.count -= 1;
    return value;
}

proc clear_dynamic_array(dyn: ^Dynamic_Array!($T)) {
    dyn.count = 0;
}

proc destroy_dynamic_array(dyn: Dynamic_Array!($T)) {
    if (dyn.array.data != null) {
        free(dyn.array.data);
    }
}

proc dynamic_arrays() {
    print("\n\n---- dynamic_arrays ----\n");

    var dyn: Dynamic_Array!(Vector3);
    append(&dyn, Vector3{1, 2, 3});
    append(&dyn, Vector3{1, 4, 9});
    append(&dyn, Vector3{2, 8, 18});
    assert(dyn[1].x == 1);
    assert(dyn[1].y == 4);
    assert(dyn[1].z == 9);
    for (var i = 0; i < dyn.count; i += 1) {
        print("%\n", i);
        print("%\n", dyn[i].x);
        print("%\n", dyn[i].y);
        print("%\n", dyn[i].z);
    }
    destroy_dynamic_array(dyn);
}
```
