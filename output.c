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
typedef struct {
    void *data;
    i64 type;
} Any;
Any MAKE_ANY(void *data, i64 type) {
    Any any;
    any.data = data;
    any.type = type;
    return any;
};

void print_int(i64 i) {
    printf("%lld", i);
}
void print_float(float f) {
    printf("%f", f);
}
void print_bool(bool b) {
    printf("%s", (b ? "true" : "false"));
}
void print_string(String string) {
    for (i64 i = 0; i < string.count; i++) {
        char c = string.data[i];
        putchar(c);
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
void print(String fmt, Slice args);
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
void print(String fmt, Slice args) {
    i64 __t1 = 0;
    i64 arg_index = __t1;
    {
        i64 __t2 = 0;
        i64 i = __t2;
        while (true) {
            bool __t3 = i < fmt.count;
            if (!__t3) { break; }
            u8 c = fmt.data[i];
            u8 __t5 = 37;
            bool __t4 = c == __t5;
            if (__t4) {
                i64 __t9 = 1;
                i64 __t8 = i + __t9;
                i64 __t7 = __t8;
                bool __t6 = __t7 < fmt.count;
                if (__t6) {
                    i64 __t12 = 1;
                    i64 __t11 = i + __t12;
                    u8 __t13 = 37;
                    bool __t10 = fmt.data[__t11] == __t13;
                    if (__t10) {
                        u8 __t14 = 37;
                        putchar(__t14);
                        i64 __t15 = 1;
                        i += __t15;
                        continue;
                    }
                }
                Any arg = ((Any *)args.data)[arg_index];
                i64 __t17 = 4;
                bool __t16 = arg.type == __t17;
                if (__t16) {
                    i64 *__t18 = ((i64 *)arg.data);
                    print_int((*__t18));
                }
                else {
                    i64 __t20 = 9;
                    bool __t19 = arg.type == __t20;
                    if (__t19) {
                        f32 *__t21 = ((f32 *)arg.data);
                        print_float((*__t21));
                    }
                    else {
                        i64 __t23 = 13;
                        bool __t22 = arg.type == __t23;
                        if (__t22) {
                            String *__t24 = ((String *)arg.data);
                            print_string((*__t24));
                        }
                        else {
                            i64 __t26 = 11;
                            bool __t25 = arg.type == __t26;
                            if (__t25) {
                                bool *__t27 = ((bool *)arg.data);
                                print_bool((*__t27));
                            }
                        }
                    }
                }
                i64 __t28 = 1;
                arg_index += __t28;
            }
            else {
                putchar(c);
            }
            i64 __t29 = 1;
            i += __t29;
        }
    }
}
bool string_eq(String a, String b) {
    bool __t30 = a.count != b.count;
    if (__t30) {
        bool __t31 = false;
        return __t31;
    }
    {
        i64 __t32 = 0;
        i64 i = __t32;
        while (true) {
            bool __t33 = i < a.count;
            if (!__t33) { break; }
            bool __t34 = a.data[i] != b.data[i];
            if (__t34) {
                bool __t35 = false;
                return __t35;
            }
            i64 __t36 = 1;
            i += __t36;
        }
    }
    bool __t37 = true;
    return __t37;
}
String string_ptr(u8 *ptr, i64 count) {
    String str = {0};
    str.data = ptr;
    str.count = count;
    return str;
}
i64 factorial(i64 n) {
    i64 __t39 = 1;
    bool __t38 = n == __t39;
    if (__t38) {
        i64 __t40 = 1;
        return __t40;
    }
    i64 __t43 = 1;
    i64 __t42 = n - __t43;
    i64 __t44 = factorial(__t42);
    i64 __t41 = n * __t44;
    return __t41;
}
void basic_stuff() {
    String __t45 = MAKE_STRING("\n\n---- basic_stuff ----\n", 24);
    Any __t46[1];
    Slice __t47;
    __t47.data = __t46;
    __t47.count = 0;
    print(__t45, __t47);
    i64 __t48 = 123;
    i64 a = __t48;
    i64 __t49 = 123;
    i64 b1 = __t49;
    bool __t50 = true;
    assert(__t50);
    f32 __t51 = 123.000000;
    f32 b2 = __t51;
    bool __t52 = true;
    assert(__t52);
    i64 __t53 = 5;
    i64 __t54 = factorial(__t53);
    i64 fact = __t54;
    i64 __t56 = 120;
    bool __t55 = fact == __t56;
    assert(__t55);
    bool __t57 = true;
    assert(__t57);
    i64 __t58 = 12;
    i64 c = __t58;
    f32 __t59 = 12.000000;
    f32 d = __t59;
    f32 __t60 = 5.500000;
    f32 e = __t60;
    f32 __t62 = 5.500000;
    bool __t61 = e == __t62;
    assert(__t61);
    i64 __t63 = 5;
    i64 f = __t63;
    i64 __t65 = 5;
    bool __t64 = f == __t65;
    assert(__t64);
    f32 __t66 = 5.500000;
    f32 g = __t66;
    f32 __t68 = 5.500000;
    bool __t67 = g == __t68;
    assert(__t67);
}
struct Static_Array_4_i64  {
    i64  elements[4];
};
void arrays_by_value(struct Static_Array_4_i64 arr) {
    i64 __t69 = 2;
    i64 __t70 = 738;
    arr.elements[__t69] = __t70;
}
void arrays() {
    String __t71 = MAKE_STRING("\n\n---- arrays ----\n", 19);
    Any __t72[1];
    Slice __t73;
    __t73.data = __t72;
    __t73.count = 0;
    print(__t71, __t73);
    struct Static_Array_4_i64 my_array = {0};
    i64 __t74 = 0;
    i64 __t75 = 1;
    my_array.elements[__t74] = __t75;
    i64 __t76 = 1;
    i64 __t77 = 2;
    my_array.elements[__t76] = __t77;
    i64 __t78 = 2;
    i64 __t79 = 3;
    my_array.elements[__t78] = __t79;
    i64 __t80 = 3;
    i64 __t81 = 4;
    my_array.elements[__t80] = __t81;
    String __t82 = MAKE_STRING("%\n", 2);
    Any __t83[1];
    i64 __t84 = 0;
    __t83[0] = MAKE_ANY(&my_array.elements[__t84], 4);
    Slice __t85;
    __t85.data = __t83;
    __t85.count = 1;
    print(__t82, __t85);
    String __t86 = MAKE_STRING("%\n", 2);
    Any __t87[1];
    i64 __t88 = 1;
    __t87[0] = MAKE_ANY(&my_array.elements[__t88], 4);
    Slice __t89;
    __t89.data = __t87;
    __t89.count = 1;
    print(__t86, __t89);
    String __t90 = MAKE_STRING("%\n", 2);
    Any __t91[1];
    i64 __t92 = 2;
    __t91[0] = MAKE_ANY(&my_array.elements[__t92], 4);
    Slice __t93;
    __t93.data = __t91;
    __t93.count = 1;
    print(__t90, __t93);
    String __t94 = MAKE_STRING("%\n", 2);
    Any __t95[1];
    i64 __t96 = 3;
    __t95[0] = MAKE_ANY(&my_array.elements[__t96], 4);
    Slice __t97;
    __t97.data = __t95;
    __t97.count = 1;
    print(__t94, __t97);
    i64 __t99 = 0;
    i64 __t100 = 1;
    bool __t98 = my_array.elements[__t99] == __t100;
    assert(__t98);
    i64 __t102 = 1;
    i64 __t103 = 2;
    bool __t101 = my_array.elements[__t102] == __t103;
    assert(__t101);
    i64 __t105 = 2;
    i64 __t106 = 3;
    bool __t104 = my_array.elements[__t105] == __t106;
    assert(__t104);
    i64 __t108 = 3;
    i64 __t109 = 4;
    bool __t107 = my_array.elements[__t108] == __t109;
    assert(__t107);
    arrays_by_value(my_array);
    String __t110 = MAKE_STRING("%\n", 2);
    Any __t111[1];
    i64 __t112 = 2;
    __t111[0] = MAKE_ANY(&my_array.elements[__t112], 4);
    Slice __t113;
    __t113.data = __t111;
    __t113.count = 1;
    print(__t110, __t113);
    i64 __t115 = 2;
    i64 __t116 = 3;
    bool __t114 = my_array.elements[__t115] == __t116;
    assert(__t114);
}
void slices() {
}
void strings() {
    String __t117 = MAKE_STRING("\n\n---- strings ----\n", 20);
    Any __t118[1];
    Slice __t119;
    __t119.data = __t118;
    __t119.count = 0;
    print(__t117, __t119);
    bool __t120 = true;
    assert(__t120);
    String __t121 = MAKE_STRING("Hello, World!", 13);
    String a = __t121;
    String hello = {0};
    i64 __t122 = 0;
    hello.data = &a.data[__t122];
    i64 __t123 = 5;
    hello.count = __t123;
    String __t125 = MAKE_STRING("Hello", 5);
    bool __t124 = string_eq(hello, __t125);
    assert(__t124);
    String __t126 = MAKE_STRING("%\n", 2);
    Any __t127[1];
    __t127[0] = MAKE_ANY(&hello, 13);
    Slice __t128;
    __t128.data = __t127;
    __t128.count = 1;
    print(__t126, __t128);
    String world = {0};
    i64 __t129 = 7;
    world.data = &a.data[__t129];
    i64 __t130 = 5;
    world.count = __t130;
    String __t132 = MAKE_STRING("World", 5);
    bool __t131 = string_eq(world, __t132);
    assert(__t131);
    String __t133 = MAKE_STRING("%\n", 2);
    Any __t134[1];
    __t134[0] = MAKE_ANY(&world, 13);
    Slice __t135;
    __t135.data = __t134;
    __t135.count = 1;
    print(__t133, __t135);
}
struct Foo {
    i64 a;
    String str;
    bool t;
};
void structs() {
    String __t136 = MAKE_STRING("\n\n---- structs ----\n", 20);
    Any __t137[1];
    Slice __t138;
    __t138.data = __t137;
    __t138.count = 0;
    print(__t136, __t138);
    struct Foo f = {0};
    i64 __t140 = 0;
    bool __t139 = f.a == __t140;
    assert(__t139);
    i64 __t141 = 123;
    f.a = __t141;
    String __t142 = MAKE_STRING("foozle", 6);
    f.str = __t142;
    i64 __t144 = 120;
    i64 __t145 = 5;
    i64 __t146 = factorial(__t145);
    bool __t143 = __t144 == __t146;
    f.t = __t143;
    i64 __t147 = 149;
    String __t148 = MAKE_STRING("hellooo", 7);
    bool __t149 = false;
    struct Foo __t150 = {0};
    __t150.a = __t147;
    __t150.str = __t148;
    __t150.t = __t149;
    f = __t150;
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
    String __t151 = MAKE_STRING("\n\n---- order_independence ----\n", 31);
    Any __t152[1];
    Slice __t153;
    __t153.data = __t152;
    __t153.count = 0;
    print(__t151, __t153);
    struct Loopy loopy = {0};
    bool __t154 = true;
    assert(__t154);
    bool __t155 = true;
    assert(__t155);
}
void procedure_with_varargs(Slice numbers) {
    String __t156 = MAKE_STRING("varargs count: %\n", 17);
    Any __t157[1];
    __t157[0] = MAKE_ANY(&numbers.count, 4);
    Slice __t158;
    __t158.data = __t157;
    __t158.count = 1;
    print(__t156, __t158);
    {
        i64 __t159 = 0;
        i64 i = __t159;
        while (true) {
            bool __t160 = i < numbers.count;
            if (!__t160) { break; }
            String __t161 = MAKE_STRING("  %\n", 4);
            Any __t162[1];
            __t162[0] = MAKE_ANY(&((i64 *)numbers.data)[i], 4);
            Slice __t163;
            __t163.data = __t162;
            __t163.count = 1;
            print(__t161, __t163);
            i64 __t164 = 1;
            i += __t164;
        }
    }
}
void varargs() {
    String __t165 = MAKE_STRING("\n\n---- varargs ----\n", 20);
    Any __t166[1];
    Slice __t167;
    __t167.data = __t166;
    __t167.count = 0;
    print(__t165, __t167);
    i64 __t168[4];
    i64 __t169 = 1;
    __t168[0] = __t169;
    i64 __t170 = 2;
    __t168[1] = __t170;
    i64 __t171 = 3;
    __t168[2] = __t171;
    i64 __t172 = 4;
    __t168[3] = __t172;
    Slice __t173;
    __t173.data = __t168;
    __t173.count = 4;
    procedure_with_varargs(__t173);
    i64 __t174[2];
    i64 __t175 = 5;
    __t174[0] = __t175;
    i64 __t176 = 6;
    __t174[1] = __t176;
    Slice __t177;
    __t177.data = __t174;
    __t177.count = 2;
    procedure_with_varargs(__t177);
    i64 __t178[1];
    Slice __t179;
    __t179.data = __t178;
    __t179.count = 0;
    procedure_with_varargs(__t179);
}
void change_by_reference(i64 *a, i64 value) {
    (*a) = value;
}
void references() {
    String __t180 = MAKE_STRING("\n\n---- references ----\n", 23);
    Any __t181[1];
    Slice __t182;
    __t182.data = __t181;
    __t182.count = 0;
    print(__t180, __t182);
    i64 __t183 = 123;
    i64 my_int = __t183;
    String __t184 = MAKE_STRING("%\n", 2);
    Any __t185[1];
    __t185[0] = MAKE_ANY(&my_int, 4);
    Slice __t186;
    __t186.data = __t185;
    __t186.count = 1;
    print(__t184, __t186);
    i64 __t188 = 123;
    bool __t187 = my_int == __t188;
    assert(__t187);
    i64 *int_reference = &my_int;
    i64 __t189 = 789;
    (*int_reference) = __t189;
    i64 __t191 = 789;
    bool __t190 = (*int_reference) == __t191;
    assert(__t190);
    i64 __t193 = 789;
    bool __t192 = my_int == __t193;
    assert(__t192);
    String __t194 = MAKE_STRING("%\n", 2);
    Any __t195[1];
    __t195[0] = MAKE_ANY(&(*int_reference), 4);
    Slice __t196;
    __t196.data = __t195;
    __t196.count = 1;
    print(__t194, __t196);
    i64 __t198 = 789;
    bool __t197 = my_int == __t198;
    assert(__t197);
    i64 __t199 = 456;
    change_by_reference(&my_int, __t199);
    i64 __t201 = 456;
    bool __t200 = my_int == __t201;
    assert(__t200);
    String __t202 = MAKE_STRING("%\n", 2);
    Any __t203[1];
    __t203[0] = MAKE_ANY(&my_int, 4);
    Slice __t204;
    __t204.data = __t203;
    __t204.count = 1;
    print(__t202, __t204);
}
struct Vector3 {
    f32 x;
    f32 y;
    f32 z;
};
struct Vector3 __operator_overload_Vector3_TK_PLUS_Vector3(struct Vector3 a, struct Vector3 b) {
    f32 __t205 = a.x + b.x;
    f32 __t206 = a.y + b.y;
    f32 __t207 = a.z + b.z;
    struct Vector3 __t208 = {0};
    __t208.x = __t205;
    __t208.y = __t206;
    __t208.z = __t207;
    return __t208;
}
struct Vector3 __operator_overload_Vector3_TK_MINUS_Vector3(struct Vector3 a, struct Vector3 b) {
    f32 __t209 = a.x - b.x;
    f32 __t210 = a.y - b.y;
    f32 __t211 = a.z - b.z;
    struct Vector3 __t212 = {0};
    __t212.x = __t209;
    __t212.y = __t210;
    __t212.z = __t211;
    return __t212;
}
struct Vector3 __operator_overload_Vector3_TK_MULTIPLY_Vector3(struct Vector3 a, struct Vector3 b) {
    f32 __t213 = a.x * b.x;
    f32 __t214 = a.y * b.y;
    f32 __t215 = a.z * b.z;
    struct Vector3 __t216 = {0};
    __t216.x = __t213;
    __t216.y = __t214;
    __t216.z = __t215;
    return __t216;
}
struct Vector3 __operator_overload_Vector3_TK_DIVIDE_Vector3(struct Vector3 a, struct Vector3 b) {
    f32 __t217 = a.x / b.x;
    f32 __t218 = a.y / b.y;
    f32 __t219 = a.z / b.z;
    struct Vector3 __t220 = {0};
    __t220.x = __t217;
    __t220.y = __t218;
    __t220.z = __t219;
    return __t220;
}
struct Vector3 __operator_overload_Vector3_TK_MULTIPLY_f32(struct Vector3 a, f32 f) {
    f32 __t221 = a.x * f;
    f32 __t222 = a.y * f;
    f32 __t223 = a.z * f;
    struct Vector3 __t224 = {0};
    __t224.x = __t221;
    __t224.y = __t222;
    __t224.z = __t223;
    return __t224;
}
void operator_overloading() {
    String __t225 = MAKE_STRING("\n\n---- operator_overloading ----\n", 33);
    Any __t226[1];
    Slice __t227;
    __t227.data = __t226;
    __t227.count = 0;
    print(__t225, __t227);
    f32 __t228 = 1.000000;
    f32 __t229 = 2.000000;
    f32 __t230 = 3.000000;
    struct Vector3 __t231 = {0};
    __t231.x = __t228;
    __t231.y = __t229;
    __t231.z = __t230;
    struct Vector3 v1 = __t231;
    f32 __t232 = 1.000000;
    f32 __t233 = 4.000000;
    f32 __t234 = 9.000000;
    struct Vector3 __t235 = {0};
    __t235.x = __t232;
    __t235.y = __t233;
    __t235.z = __t234;
    struct Vector3 v2 = __t235;
    struct Vector3 __t236 = __operator_overload_Vector3_TK_PLUS_Vector3(v1, v2);
    struct Vector3 v3 = __t236;
    f32 __t237 = 5.000000;
    struct Vector3 __t238 = __operator_overload_Vector3_TK_MULTIPLY_f32(v3, __t237);
    struct Vector3 v4 = __t238;
    String __t239 = MAKE_STRING("%\n", 2);
    Any __t240[1];
    __t240[0] = MAKE_ANY(&v4.x, 9);
    Slice __t241;
    __t241.data = __t240;
    __t241.count = 1;
    print(__t239, __t241);
    String __t242 = MAKE_STRING("%\n", 2);
    Any __t243[1];
    __t243[0] = MAKE_ANY(&v4.y, 9);
    Slice __t244;
    __t244.data = __t243;
    __t244.count = 1;
    print(__t242, __t244);
    String __t245 = MAKE_STRING("%\n", 2);
    Any __t246[1];
    __t246[0] = MAKE_ANY(&v4.z, 9);
    Slice __t247;
    __t247.data = __t246;
    __t247.count = 1;
    print(__t245, __t247);
    f32 __t249 = 10.000000;
    bool __t248 = v4.x == __t249;
    assert(__t248);
    f32 __t251 = 30.000000;
    bool __t250 = v4.y == __t251;
    assert(__t250);
    f32 __t253 = 60.000000;
    bool __t252 = v4.z == __t253;
    assert(__t252);
}
i64 value_poly__polymorph_0() {
    i64 __t254 = 4;
    return __t254;
}
i64 type_poly__polymorph_1(i64 a) {
    i64 __t255 = a * a;
    return __t255;
}
f32 type_poly__polymorph_2(f32 a) {
    f32 __t256 = a * a;
    return __t256;
}
i64 value_and_type_poly__polymorph_3() {
    i64 __t257 = 49;
    return __t257;
}
f32 value_and_type_poly__polymorph_4() {
    f32 __t258 = 64.000000;
    return __t258;
}
void procedural_polymorphism() {
    String __t259 = MAKE_STRING("\n\n---- procedural_polymorphism ----\n", 36);
    Any __t260[1];
    Slice __t261;
    __t261.data = __t260;
    __t261.count = 0;
    print(__t259, __t261);
    String __t262 = MAKE_STRING("%\n", 2);
    Any __t263[1];
    i64 __t264 = value_poly__polymorph_0();
    __t263[0] = MAKE_ANY(&__t264, 4);
    Slice __t265;
    __t265.data = __t263;
    __t265.count = 1;
    print(__t262, __t265);
    i64 __t267 = value_poly__polymorph_0();
    i64 __t268 = 4;
    bool __t266 = __t267 == __t268;
    assert(__t266);
    String __t269 = MAKE_STRING("%\n", 2);
    Any __t270[1];
    i64 __t271 = 3;
    i64 __t272 = type_poly__polymorph_1(__t271);
    __t270[0] = MAKE_ANY(&__t272, 4);
    Slice __t273;
    __t273.data = __t270;
    __t273.count = 1;
    print(__t269, __t273);
    i64 __t275 = 3;
    i64 __t276 = type_poly__polymorph_1(__t275);
    i64 __t277 = 9;
    bool __t274 = __t276 == __t277;
    assert(__t274);
    f32 __t278 = 4.000000;
    f32 __t279 = type_poly__polymorph_2(__t278);
    print_float(__t279);
    f32 __t281 = 4.000000;
    f32 __t282 = type_poly__polymorph_2(__t281);
    f32 __t283 = 16.000000;
    bool __t280 = __t282 == __t283;
    assert(__t280);
    i64 __t284 = 5;
    i64 a = __t284;
    f32 __t285 = 6.000000;
    f32 f = __t285;
    String __t286 = MAKE_STRING("%\n", 2);
    Any __t287[1];
    i64 __t288 = type_poly__polymorph_1(a);
    __t287[0] = MAKE_ANY(&__t288, 4);
    Slice __t289;
    __t289.data = __t287;
    __t289.count = 1;
    print(__t286, __t289);
    String __t290 = MAKE_STRING("%\n", 2);
    Any __t291[1];
    f32 __t292 = type_poly__polymorph_2(f);
    __t291[0] = MAKE_ANY(&__t292, 9);
    Slice __t293;
    __t293.data = __t291;
    __t293.count = 1;
    print(__t290, __t293);
    i64 __t295 = type_poly__polymorph_1(a);
    i64 __t296 = 25;
    bool __t294 = __t295 == __t296;
    assert(__t294);
    f32 __t298 = type_poly__polymorph_2(f);
    f32 __t299 = 36.000000;
    bool __t297 = __t298 == __t299;
    assert(__t297);
    String __t300 = MAKE_STRING("%\n", 2);
    Any __t301[1];
    i64 __t302 = value_and_type_poly__polymorph_3();
    __t301[0] = MAKE_ANY(&__t302, 4);
    Slice __t303;
    __t303.data = __t301;
    __t303.count = 1;
    print(__t300, __t303);
    String __t304 = MAKE_STRING("%\n", 2);
    Any __t305[1];
    f32 __t306 = value_and_type_poly__polymorph_4();
    __t305[0] = MAKE_ANY(&__t306, 9);
    Slice __t307;
    __t307.data = __t305;
    __t307.count = 1;
    print(__t304, __t307);
    i64 __t309 = value_and_type_poly__polymorph_3();
    i64 __t310 = 49;
    bool __t308 = __t309 == __t310;
    assert(__t308);
    f32 __t312 = value_and_type_poly__polymorph_4();
    f32 __t313 = 64.000000;
    bool __t311 = __t312 == __t313;
    assert(__t311);
}
struct Custom_Array_Type__polymorph_5 {
    struct Static_Array_8_i64 array;
};
i64 *__operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(struct Custom_Array_Type__polymorph_5 *my_array, i64 index) {
    return &(*my_array).array.elements[index];
}
void structural_polymorphism() {
    String __t314 = MAKE_STRING("\n\n---- structural_polymorphism ----\n", 36);
    Any __t315[1];
    Slice __t316;
    __t316.data = __t315;
    __t316.count = 0;
    print(__t314, __t316);
    struct Custom_Array_Type__polymorph_5 array_of_ints = {0};
    i64 __t317 = 4;
    i64 *__t318 = __operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(&array_of_ints, __t317);
    i64 __t319 = 124;
    (*__t318) = __t319;
    String __t320 = MAKE_STRING("%\n", 2);
    Any __t321[1];
    i64 __t322 = 4;
    i64 *__t323 = __operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(&array_of_ints, __t322);
    __t321[0] = MAKE_ANY(&(*__t323), 4);
    Slice __t324;
    __t324.data = __t321;
    __t324.count = 1;
    print(__t320, __t324);
    i64 __t326 = 4;
    i64 *__t327 = __operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(&array_of_ints, __t326);
    i64 __t328 = 124;
    bool __t325 = (*__t327) == __t328;
    assert(__t325);
}
struct Dynamic_Array__polymorph_6 {
    Slice array;
    i64 count;
};
struct Vector3 *__operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(struct Dynamic_Array__polymorph_6 *dyn, i64 index) {
    return &((struct Vector3 *)(*dyn).array.data)[index];
}
void append__polymorph_7(struct Dynamic_Array__polymorph_6 *dyn, struct Vector3 value) {
    bool __t329 = dyn->count == dyn->array.count;
    if (__t329) {
        struct Vector3 *old_data = *((struct Vector3 **)&dyn->array.data);
        i64 __t331 = 8;
        i64 __t333 = 2;
        i64 __t332 = dyn->array.count * __t333;
        i64 __t330 = __t331 + __t332;
        i64 new_cap = __t330;
        i64 __t335 = 12;
        i64 __t334 = new_cap * __t335;
        void *__t336 = alloc(__t334);
        struct Vector3 *__t337 = ((struct Vector3 *)__t336);
        *((struct Vector3 **)&dyn->array.data) = __t337;
        dyn->array.count = new_cap;
        bool __t338 = old_data != NULL;
        if (__t338) {
            i64 __t340 = 12;
            i64 __t339 = dyn->count * __t340;
            u32 __t341 = ((u32 )__t339);
            void *__t342 = memcpy(*((struct Vector3 **)&dyn->array.data), old_data, __t341);
            free(old_data);
        }
    }
    bool __t343 = dyn->count < dyn->array.count;
    assert(__t343);
    ((struct Vector3 *)dyn->array.data)[dyn->count] = value;
    i64 __t344 = 1;
    dyn->count += __t344;
}
void destroy_dynamic_array__polymorph_8(struct Dynamic_Array__polymorph_6 dyn) {
    bool __t345 = *((struct Vector3 **)&dyn.array.data) != NULL;
    if (__t345) {
        free(*((struct Vector3 **)&dyn.array.data));
    }
}
void dynamic_arrays() {
    String __t346 = MAKE_STRING("\n\n---- dynamic_arrays ----\n", 27);
    Any __t347[1];
    Slice __t348;
    __t348.data = __t347;
    __t348.count = 0;
    print(__t346, __t348);
    struct Dynamic_Array__polymorph_6 dyn = {0};
    f32 __t349 = 1.000000;
    f32 __t350 = 2.000000;
    f32 __t351 = 3.000000;
    struct Vector3 __t352 = {0};
    __t352.x = __t349;
    __t352.y = __t350;
    __t352.z = __t351;
    append__polymorph_7(&dyn, __t352);
    f32 __t353 = 1.000000;
    f32 __t354 = 4.000000;
    f32 __t355 = 9.000000;
    struct Vector3 __t356 = {0};
    __t356.x = __t353;
    __t356.y = __t354;
    __t356.z = __t355;
    append__polymorph_7(&dyn, __t356);
    f32 __t357 = 2.000000;
    f32 __t358 = 8.000000;
    f32 __t359 = 18.000000;
    struct Vector3 __t360 = {0};
    __t360.x = __t357;
    __t360.y = __t358;
    __t360.z = __t359;
    append__polymorph_7(&dyn, __t360);
    i64 __t362 = 1;
    struct Vector3 *__t363 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, __t362);
    f32 __t364 = 1.000000;
    bool __t361 = (*__t363).x == __t364;
    assert(__t361);
    i64 __t366 = 1;
    struct Vector3 *__t367 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, __t366);
    f32 __t368 = 4.000000;
    bool __t365 = (*__t367).y == __t368;
    assert(__t365);
    i64 __t370 = 1;
    struct Vector3 *__t371 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, __t370);
    f32 __t372 = 9.000000;
    bool __t369 = (*__t371).z == __t372;
    assert(__t369);
    {
        i64 __t373 = 0;
        i64 i = __t373;
        while (true) {
            bool __t374 = i < dyn.count;
            if (!__t374) { break; }
            String __t375 = MAKE_STRING("%\n", 2);
            Any __t376[1];
            __t376[0] = MAKE_ANY(&i, 4);
            Slice __t377;
            __t377.data = __t376;
            __t377.count = 1;
            print(__t375, __t377);
            String __t378 = MAKE_STRING("%\n", 2);
            Any __t379[1];
            struct Vector3 *__t380 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, i);
            __t379[0] = MAKE_ANY(&(*__t380).x, 9);
            Slice __t381;
            __t381.data = __t379;
            __t381.count = 1;
            print(__t378, __t381);
            String __t382 = MAKE_STRING("%\n", 2);
            Any __t383[1];
            struct Vector3 *__t384 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, i);
            __t383[0] = MAKE_ANY(&(*__t384).y, 9);
            Slice __t385;
            __t385.data = __t383;
            __t385.count = 1;
            print(__t382, __t385);
            String __t386 = MAKE_STRING("%\n", 2);
            Any __t387[1];
            struct Vector3 *__t388 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, i);
            __t387[0] = MAKE_ANY(&(*__t388).z, 9);
            Slice __t389;
            __t389.data = __t387;
            __t389.count = 1;
            print(__t386, __t389);
            i64 __t390 = 1;
            i += __t390;
        }
    }
    destroy_dynamic_array__polymorph_8(dyn);
}
i32 main() {
    String __t391 = MAKE_STRING("-------------------------\n", 26);
    Any __t392[1];
    Slice __t393;
    __t393.data = __t392;
    __t393.count = 0;
    print(__t391, __t393);
    String __t394 = MAKE_STRING("|   sif language demo   |\n", 26);
    Any __t395[1];
    Slice __t396;
    __t396.data = __t395;
    __t396.count = 0;
    print(__t394, __t396);
    String __t397 = MAKE_STRING("-------------------------\n", 26);
    Any __t398[1];
    Slice __t399;
    __t399.data = __t398;
    __t399.count = 0;
    print(__t397, __t399);
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
    i32 __t400 = 0;
    return __t400;
}
