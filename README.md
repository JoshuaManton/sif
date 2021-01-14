# sif
sif is a simple imperative procedural language made with the goal of being a slightly-higher-level C.

## Hello, World!

```odin
#include "core:basic.sif"

proc main() {
    print("Hello, World!\n");
}
```

## Features (see demo below for examples)

- The basic stuff: variables, procedures, structs, ints, floats, bool, pointers, arrays, etc
- Length-delimited strings, not null-terminated
- Order-independent declarations
- Operator overloading
- Procedural and structural polymorphism
- `using` statement for composition
- Runtime type information
- `any` type
- `defer` statement

## Building and Running

1. Clone the repo.
2. Run `build.bat`
3. Make sure that you have run vcvars so `cl` is in your path (this is a temporary step, will work around it later)
4. Put the following into a file called `my_program.sif`:
```odin
#include "core:basic.sif"

proc main() {
    print("Hello, World!\n");
}
```
5. Run `bin/sif.exe run my_program.sif`

## Demo

The following is a demo program showing off many of the features of sif.

```odin
#include "core:basic.sif"

proc main() {
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
    function_pointers();

    // cool stuff starts here
    operator_overloading();
    procedural_polymorphism();
    structural_polymorphism();
    runtime_type_information();
    using_statement();
    defer_statement();
    any_type();
    dynamic_arrays();
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
// the keyword 'var' can be omitted in procedure parameter lists, since
// variables are the only thing allowed there
proc factorial(n: int) : int {
    if (n == 1) {
        return 1;
    }
    return n * factorial(n-1);
}



proc arrays() {
    print("\n\n---- arrays ----\n");
    var my_array: [4]int;
    my_array[0] = 1;
    my_array[1] = 2;
    my_array[2] = 3;
    my_array[3] = 4;
    print("%\n", my_array);

    // As with structs, compound literals work with arrays
    my_array = {4, 3, 2, 1};
    print("%\n", my_array);

    // Arrays are value types, so calling a function that changes the array
    // parameter will not affect the array at the call-site
    arrays_by_value(my_array);
    print("%\n", my_array);
    assert(my_array[0] == 4);
    assert(my_array[1] == 3);
    assert(my_array[2] == 2);
    assert(my_array[3] == 1);
}
proc arrays_by_value(arr: [4]int) {
    arr[0] = 123;
    arr[1] = 345;
    arr[2] = 567;
    arr[3] = 789;
}



proc slices() {
    print("\n\n---- slices ----\n");

    var array: [4]int;
    array[2] = 124;
    print("%\n", array[2]);
    var slice = slice_ptr(&array[0], array.count);
    assert(slice[2] == 124);
    slice[2] = 421;
    print("%\n", slice[2]);
    assert(array[2] == 421);
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

    // todo(josh): cstrings
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

    // Can also use compound literals to initialize
    f = Foo{149, "hellooo", false};

    // The type of the compound literal can be inferred from context.
    // Since the compiler knows the type of 'f', you don't have to specify
    // it in the compound literal.
    f = {149, "hellooo", false};

    // using runtime type information, you can print whole structs, more on that below.
    print("%\n", f);

    // Type inference also works when calling procedures, since the target type is known
    takes_a_foo({123, "wow", true});
}
proc takes_a_foo(foo: Foo) {
    print("%\n", foo);
}



enum My_Enum {
    FOO;
    BAR;
    BAZ;
}
proc enums() {
    print("\n\n---- enums ----\n");

    var f = My_Enum.FOO;
    print("%\n", f);

    // If the enum type can be inferred from context, you don't have to
    // specify it on the right-hand-side.
    var b: My_Enum = .BAR; // implicit enum selection
    print("%\n", b);

    // Implicit enum selection also works in binary expressions
    if (b == .BAR) {

    }

    // Note: the following will NOT work because the compiler can't
    // figure out the enum type to search for the fields FOO and BAR.
    // if (.FOO == .BAR) {
    // }

    // If at least one of them supplies the target enum, all good.
    if (.FOO == My_Enum.BAR) {
    }

    // As with compound literals, the type can also be inferred from
    // procedure parameters, allowing you to omit the enum name.
    takes_my_enum(.BAZ);
}
proc takes_my_enum(e: My_Enum) {
    print("%\n", e);
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



proc runtime_type_information() {
    print("\n\n---- runtime_type_information ----\n");

    // Every type in your program gets it's own Type_Info struct that holds
    // information about that type, depending on the kind of type it is. If
    // it's a struct it has information about all the fields, if it is an
    // array it has the count and type that it is an array of, etc.
    // you can view all of the Type_Info structs in core/runtime.sif.



    // Using get_type_info() you can get information about a type.
    // The thing returned from get_type_info() is a ^Type_Info, which contains
    // a field `kind` indicating what _kind_ of type it is, such as INTEGER,
    // FLOAT, STRUCT, PROCEDURE, etc. Type_Info just contains the data relevant
    // to all types, such as size and alignment. To get more detailed information
    // about a type, check the `kind` field and cast the ^Type_Info to the appropriate
    // derived type:
    // ^Type_Info_Integer if `kind` is INTEGER
    // ^Type_Info_Struct if `kind` is STRUCT
    // ^Type_Info_Procedure if `kind` is PROCEDURE
    // etc.
    // Again, you can view all of the Type_Info structs in core/runtime.sif.

    // Type_Info_Integer just has a bool indicating whether it's signed or unsigned
    var int_type_info: ^Type_Info = get_type_info(int);
    assert(int_type_info.kind == Type_Info_Kind.INTEGER);
    var int_type_info_kind = cast(^Type_Info_Integer, int_type_info);
    assert(int_type_info_kind.is_signed == true);

    var uint_type_info: ^Type_Info = get_type_info(uint);
    assert(uint_type_info.kind == Type_Info_Kind.INTEGER);
    var uint_type_info_kind = cast(^Type_Info_Integer, uint_type_info);
    assert(uint_type_info_kind.is_signed == false);



    // Type_Info_Struct is a lot more interesting. It contains a list of all the fields,
    // and their types, and their byte offsets
    var my_cool_struct_ti = get_type_info(My_Cool_Struct);
    assert(my_cool_struct_ti.kind == Type_Info_Kind.STRUCT);
    var my_cool_struct_ti_kind = cast(^Type_Info_Struct, my_cool_struct_ti);
    print("My_Cool_Struct\n");
    for (var i = 0; i < my_cool_struct_ti_kind.fields.count; i += 1) {
        var field = my_cool_struct_ti_kind.fields[i];
        print("  field: %, type: %, offset: %\n",
            field.name, field.type.printable_name, field.offset);
    }



    // using runtime type information, the print() function can print whole
    // structs intelligently. you can view the implementation of print()
    // in core/basic.sif
    print("%\n", int_type_info_kind^);
    // prints: Type_Info_Integer{base = Type_Info{printable_name = "i64",
    //         kind = Type_Info_Kind.INTEGER, id = i64, size = 8, align = 8},
    //         is_signed = true}
}
struct My_Cool_Struct {
    var foo: int;
    var bar: string;
    var nested: Nested_Struct;
}
struct Nested_Struct {
    var more_things: [4]Vector3;
}



enum My_Enum_For_Using {
    FOO;
    BAR;
    BAZ;
}
proc using_statement() {
    print("\n\n---- using_statement ----\n");

    // 'using' pulls a namespace's declarations into the namespace
    // of the using.

    // For example if we wanted to print the elements from My_Enum_For_Using
    // you would have to do it like this:
    print("%\n", My_Enum_For_Using.FOO);
    print("%\n", My_Enum_For_Using.BAR);
    print("%\n", My_Enum_For_Using.BAZ);

    // But if you didn't want to do all that typing all the time, you could
    // 'using' that enum:
    using My_Enum_For_Using;
    print("%\n", FOO);
    print("%\n", BAR);
    print("%\n", BAZ);

    // You can use 'using' inside structs as well, to get a form of subtyping
    // that is more flexible than conventional inheritance.
    var my_struct: My_Struct_With_Using;
    my_struct.some_field = 321; // no need to write 'my_struct.other.some_field'
    print("%\n", my_struct.some_field);
    assert(my_struct.some_field == my_struct.other.some_field);

    using my_struct;
    print("%\n", some_field); // no need to write 'my_struct.other.some_field'
    assert(some_field == my_struct.other.some_field);

    // You can 'using' as many fields as you'd like, provided that the names
    // do not collide.
}
struct My_Struct_With_Using {
    using var other: My_Other_Struct;
}
struct My_Other_Struct {
    var some_field: int;
}



proc defer_statement() {
    print("\n\n---- defer_statement ----\n");

    // 'defer' executes a statement at the end of the current block
    {
        var a = 123;
        {
            defer a = 321;
            assert(a == 123);
        }
        assert(a == 321);
    }

    // a good use-case for defer is memory management, keeping the
    // freeing of resources nearby the acquision site
    {
        var some_allocated_memory = new_slice(int, 16, default_allocator());
        defer delete_slice(some_allocated_memory, default_allocator());

        // ... do a bunch of work with some_allocated_memory
    }

    // if there are multiple defers, they are executed in reverse order
    {
        defer print("Will print third\n");
        defer print("Will print second\n");
        defer print("Will print first\n");
    }

    // in addition to automatically running at the end of blocks, normal
    // control flow statements like 'break', 'continue', and 'return' also
    // invoke defers
    {
        var a = 123;
        for (var i = 0; i < 10; i += 1) {
            if (i == 7) {
                defer a = 321;
                break;
            }
            assert(a == 123);
        }
        assert(a == 321);
    }
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
        else {
            print("<unknown type>\n");
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
        var old_data = dyn.array;
        var new_cap = 8 + dyn.array.count * 2;
        dyn.array = new_slice(T, new_cap, default_allocator());
        if (old_data.data != null) {
            copy_slice(dyn.array, old_data);
            delete_slice(old_data, default_allocator());
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
        print("index %\n", i);
        print("  %\n", dyn[i].x);
        print("  %\n", dyn[i].y);
        print("  %\n", dyn[i].z);
    }
    destroy_dynamic_array(dyn);
}
```
