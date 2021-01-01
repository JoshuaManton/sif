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
    printf((b ? "true" : "false"));
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
void printa(Slice args);
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
void print_things(Slice args);
void any_type();
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
void printa(Slice args) {
    {
        i64 __t30 = 0;
        i64 i = __t30;
        while (true) {
            bool __t31 = i < args.count;
            if (!__t31) { break; }
            i64 __t33 = 0;
            bool __t32 = i != __t33;
            if (__t32) {
                u8 __t34 = 32;
                putchar(__t34);
            }
            Any arg = ((Any *)args.data)[i];
            i64 __t36 = 4;
            bool __t35 = arg.type == __t36;
            if (__t35) {
                i64 *__t37 = ((i64 *)arg.data);
                print_int((*__t37));
            }
            else {
                i64 __t39 = 9;
                bool __t38 = arg.type == __t39;
                if (__t38) {
                    f32 *__t40 = ((f32 *)arg.data);
                    print_float((*__t40));
                }
                else {
                    i64 __t42 = 13;
                    bool __t41 = arg.type == __t42;
                    if (__t41) {
                        String *__t43 = ((String *)arg.data);
                        print_string((*__t43));
                    }
                    else {
                        i64 __t45 = 11;
                        bool __t44 = arg.type == __t45;
                        if (__t44) {
                            bool *__t46 = ((bool *)arg.data);
                            print_bool((*__t46));
                        }
                    }
                }
            }
            i64 __t47 = 1;
            i += __t47;
        }
    }
    u8 __t48 = 10;
    putchar(__t48);
}
bool string_eq(String a, String b) {
    bool __t49 = a.count != b.count;
    if (__t49) {
        bool __t50 = false;
        return __t50;
    }
    {
        i64 __t51 = 0;
        i64 i = __t51;
        while (true) {
            bool __t52 = i < a.count;
            if (!__t52) { break; }
            bool __t53 = a.data[i] != b.data[i];
            if (__t53) {
                bool __t54 = false;
                return __t54;
            }
            i64 __t55 = 1;
            i += __t55;
        }
    }
    bool __t56 = true;
    return __t56;
}
String string_ptr(u8 *ptr, i64 count) {
    String str = {0};
    str.data = ptr;
    str.count = count;
    return str;
}
i64 factorial(i64 n) {
    i64 __t58 = 1;
    bool __t57 = n == __t58;
    if (__t57) {
        i64 __t59 = 1;
        return __t59;
    }
    i64 __t62 = 1;
    i64 __t61 = n - __t62;
    i64 __t63 = factorial(__t61);
    i64 __t60 = n * __t63;
    return __t60;
}
void basic_stuff() {
    String __t64 = MAKE_STRING("\n\n---- basic_stuff ----\n", 24);
    Any __t65[1];
    Slice __t66;
    __t66.data = __t65;
    __t66.count = 0;
    print(__t64, __t66);
    i64 __t67 = 123;
    i64 a = __t67;
    i64 __t68 = 123;
    i64 b1 = __t68;
    bool __t69 = true;
    assert(__t69);
    f32 __t70 = 123.000000;
    f32 b2 = __t70;
    bool __t71 = true;
    assert(__t71);
    i64 __t72 = 5;
    i64 __t73 = factorial(__t72);
    i64 fact = __t73;
    i64 __t75 = 120;
    bool __t74 = fact == __t75;
    assert(__t74);
    bool __t76 = true;
    assert(__t76);
    i64 __t77 = 12;
    i64 c = __t77;
    f32 __t78 = 12.000000;
    f32 d = __t78;
    f32 __t79 = 5.500000;
    f32 e = __t79;
    f32 __t81 = 5.500000;
    bool __t80 = e == __t81;
    assert(__t80);
    i64 __t82 = 5;
    i64 f = __t82;
    i64 __t84 = 5;
    bool __t83 = f == __t84;
    assert(__t83);
    f32 __t85 = 5.500000;
    f32 g = __t85;
    f32 __t87 = 5.500000;
    bool __t86 = g == __t87;
    assert(__t86);
}
struct Static_Array_4_i64  {
    i64  elements[4];
};
void arrays_by_value(struct Static_Array_4_i64 arr) {
    i64 __t88 = 2;
    i64 __t89 = 738;
    arr.elements[__t88] = __t89;
}
void arrays() {
    String __t90 = MAKE_STRING("\n\n---- arrays ----\n", 19);
    Any __t91[1];
    Slice __t92;
    __t92.data = __t91;
    __t92.count = 0;
    print(__t90, __t92);
    struct Static_Array_4_i64 my_array = {0};
    i64 __t93 = 0;
    i64 __t94 = 1;
    my_array.elements[__t93] = __t94;
    i64 __t95 = 1;
    i64 __t96 = 2;
    my_array.elements[__t95] = __t96;
    i64 __t97 = 2;
    i64 __t98 = 3;
    my_array.elements[__t97] = __t98;
    i64 __t99 = 3;
    i64 __t100 = 4;
    my_array.elements[__t99] = __t100;
    String __t101 = MAKE_STRING("%\n", 2);
    Any __t102[1];
    i64 __t103 = 0;
    __t102[0] = MAKE_ANY(&my_array.elements[__t103], 4);
    Slice __t104;
    __t104.data = __t102;
    __t104.count = 1;
    print(__t101, __t104);
    String __t105 = MAKE_STRING("%\n", 2);
    Any __t106[1];
    i64 __t107 = 1;
    __t106[0] = MAKE_ANY(&my_array.elements[__t107], 4);
    Slice __t108;
    __t108.data = __t106;
    __t108.count = 1;
    print(__t105, __t108);
    String __t109 = MAKE_STRING("%\n", 2);
    Any __t110[1];
    i64 __t111 = 2;
    __t110[0] = MAKE_ANY(&my_array.elements[__t111], 4);
    Slice __t112;
    __t112.data = __t110;
    __t112.count = 1;
    print(__t109, __t112);
    String __t113 = MAKE_STRING("%\n", 2);
    Any __t114[1];
    i64 __t115 = 3;
    __t114[0] = MAKE_ANY(&my_array.elements[__t115], 4);
    Slice __t116;
    __t116.data = __t114;
    __t116.count = 1;
    print(__t113, __t116);
    i64 __t118 = 0;
    i64 __t119 = 1;
    bool __t117 = my_array.elements[__t118] == __t119;
    assert(__t117);
    i64 __t121 = 1;
    i64 __t122 = 2;
    bool __t120 = my_array.elements[__t121] == __t122;
    assert(__t120);
    i64 __t124 = 2;
    i64 __t125 = 3;
    bool __t123 = my_array.elements[__t124] == __t125;
    assert(__t123);
    i64 __t127 = 3;
    i64 __t128 = 4;
    bool __t126 = my_array.elements[__t127] == __t128;
    assert(__t126);
    arrays_by_value(my_array);
    String __t129 = MAKE_STRING("%\n", 2);
    Any __t130[1];
    i64 __t131 = 2;
    __t130[0] = MAKE_ANY(&my_array.elements[__t131], 4);
    Slice __t132;
    __t132.data = __t130;
    __t132.count = 1;
    print(__t129, __t132);
    i64 __t134 = 2;
    i64 __t135 = 3;
    bool __t133 = my_array.elements[__t134] == __t135;
    assert(__t133);
}
void slices() {
}
void strings() {
    String __t136 = MAKE_STRING("\n\n---- strings ----\n", 20);
    Any __t137[1];
    Slice __t138;
    __t138.data = __t137;
    __t138.count = 0;
    print(__t136, __t138);
    bool __t139 = true;
    assert(__t139);
    String __t140 = MAKE_STRING("Hello, World!", 13);
    String a = __t140;
    String hello = {0};
    i64 __t141 = 0;
    hello.data = &a.data[__t141];
    i64 __t142 = 5;
    hello.count = __t142;
    String __t144 = MAKE_STRING("Hello", 5);
    bool __t143 = string_eq(hello, __t144);
    assert(__t143);
    String __t145 = MAKE_STRING("%\n", 2);
    Any __t146[1];
    __t146[0] = MAKE_ANY(&hello, 13);
    Slice __t147;
    __t147.data = __t146;
    __t147.count = 1;
    print(__t145, __t147);
    String world = {0};
    i64 __t148 = 7;
    world.data = &a.data[__t148];
    i64 __t149 = 5;
    world.count = __t149;
    String __t151 = MAKE_STRING("World", 5);
    bool __t150 = string_eq(world, __t151);
    assert(__t150);
    String __t152 = MAKE_STRING("%\n", 2);
    Any __t153[1];
    __t153[0] = MAKE_ANY(&world, 13);
    Slice __t154;
    __t154.data = __t153;
    __t154.count = 1;
    print(__t152, __t154);
}
struct Foo {
    i64 a;
    String str;
    bool t;
};
void structs() {
    String __t155 = MAKE_STRING("\n\n---- structs ----\n", 20);
    Any __t156[1];
    Slice __t157;
    __t157.data = __t156;
    __t157.count = 0;
    print(__t155, __t157);
    struct Foo f = {0};
    i64 __t159 = 0;
    bool __t158 = f.a == __t159;
    assert(__t158);
    i64 __t160 = 123;
    f.a = __t160;
    String __t161 = MAKE_STRING("foozle", 6);
    f.str = __t161;
    i64 __t163 = 120;
    i64 __t164 = 5;
    i64 __t165 = factorial(__t164);
    bool __t162 = __t163 == __t165;
    f.t = __t162;
    i64 __t166 = 149;
    String __t167 = MAKE_STRING("hellooo", 7);
    bool __t168 = false;
    struct Foo __t169 = {0};
    __t169.a = __t166;
    __t169.str = __t167;
    __t169.t = __t168;
    f = __t169;
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
    String __t170 = MAKE_STRING("\n\n---- order_independence ----\n", 31);
    Any __t171[1];
    Slice __t172;
    __t172.data = __t171;
    __t172.count = 0;
    print(__t170, __t172);
    struct Loopy loopy = {0};
    bool __t173 = true;
    assert(__t173);
    bool __t174 = true;
    assert(__t174);
}
void procedure_with_varargs(Slice numbers) {
    String __t175 = MAKE_STRING("varargs count: %\n", 17);
    Any __t176[1];
    __t176[0] = MAKE_ANY(&numbers.count, 4);
    Slice __t177;
    __t177.data = __t176;
    __t177.count = 1;
    print(__t175, __t177);
    {
        i64 __t178 = 0;
        i64 i = __t178;
        while (true) {
            bool __t179 = i < numbers.count;
            if (!__t179) { break; }
            String __t180 = MAKE_STRING("  %\n", 4);
            Any __t181[1];
            __t181[0] = MAKE_ANY(&((i64 *)numbers.data)[i], 4);
            Slice __t182;
            __t182.data = __t181;
            __t182.count = 1;
            print(__t180, __t182);
            i64 __t183 = 1;
            i += __t183;
        }
    }
}
void varargs() {
    String __t184 = MAKE_STRING("\n\n---- varargs ----\n", 20);
    Any __t185[1];
    Slice __t186;
    __t186.data = __t185;
    __t186.count = 0;
    print(__t184, __t186);
    i64 __t187[4];
    i64 __t188 = 1;
    __t187[0] = __t188;
    i64 __t189 = 2;
    __t187[1] = __t189;
    i64 __t190 = 3;
    __t187[2] = __t190;
    i64 __t191 = 4;
    __t187[3] = __t191;
    Slice __t192;
    __t192.data = __t187;
    __t192.count = 4;
    procedure_with_varargs(__t192);
    i64 __t193[2];
    i64 __t194 = 5;
    __t193[0] = __t194;
    i64 __t195 = 6;
    __t193[1] = __t195;
    Slice __t196;
    __t196.data = __t193;
    __t196.count = 2;
    procedure_with_varargs(__t196);
    i64 __t197[1];
    Slice __t198;
    __t198.data = __t197;
    __t198.count = 0;
    procedure_with_varargs(__t198);
}
void change_by_reference(i64 *a, i64 value) {
    (*a) = value;
}
void references() {
    String __t199 = MAKE_STRING("\n\n---- references ----\n", 23);
    Any __t200[1];
    Slice __t201;
    __t201.data = __t200;
    __t201.count = 0;
    print(__t199, __t201);
    i64 __t202 = 123;
    i64 my_int = __t202;
    String __t203 = MAKE_STRING("%\n", 2);
    Any __t204[1];
    __t204[0] = MAKE_ANY(&my_int, 4);
    Slice __t205;
    __t205.data = __t204;
    __t205.count = 1;
    print(__t203, __t205);
    i64 __t207 = 123;
    bool __t206 = my_int == __t207;
    assert(__t206);
    i64 *int_reference = &my_int;
    i64 __t208 = 789;
    (*int_reference) = __t208;
    i64 __t210 = 789;
    bool __t209 = (*int_reference) == __t210;
    assert(__t209);
    i64 __t212 = 789;
    bool __t211 = my_int == __t212;
    assert(__t211);
    String __t213 = MAKE_STRING("%\n", 2);
    Any __t214[1];
    __t214[0] = MAKE_ANY(&(*int_reference), 4);
    Slice __t215;
    __t215.data = __t214;
    __t215.count = 1;
    print(__t213, __t215);
    i64 __t217 = 789;
    bool __t216 = my_int == __t217;
    assert(__t216);
    i64 __t218 = 456;
    change_by_reference(&my_int, __t218);
    i64 __t220 = 456;
    bool __t219 = my_int == __t220;
    assert(__t219);
    String __t221 = MAKE_STRING("%\n", 2);
    Any __t222[1];
    __t222[0] = MAKE_ANY(&my_int, 4);
    Slice __t223;
    __t223.data = __t222;
    __t223.count = 1;
    print(__t221, __t223);
}
struct Vector3 {
    f32 x;
    f32 y;
    f32 z;
};
struct Vector3 __operator_overload_Vector3_TK_PLUS_Vector3(struct Vector3 a, struct Vector3 b) {
    f32 __t224 = a.x + b.x;
    f32 __t225 = a.y + b.y;
    f32 __t226 = a.z + b.z;
    struct Vector3 __t227 = {0};
    __t227.x = __t224;
    __t227.y = __t225;
    __t227.z = __t226;
    return __t227;
}
struct Vector3 __operator_overload_Vector3_TK_MINUS_Vector3(struct Vector3 a, struct Vector3 b) {
    f32 __t228 = a.x - b.x;
    f32 __t229 = a.y - b.y;
    f32 __t230 = a.z - b.z;
    struct Vector3 __t231 = {0};
    __t231.x = __t228;
    __t231.y = __t229;
    __t231.z = __t230;
    return __t231;
}
struct Vector3 __operator_overload_Vector3_TK_MULTIPLY_Vector3(struct Vector3 a, struct Vector3 b) {
    f32 __t232 = a.x * b.x;
    f32 __t233 = a.y * b.y;
    f32 __t234 = a.z * b.z;
    struct Vector3 __t235 = {0};
    __t235.x = __t232;
    __t235.y = __t233;
    __t235.z = __t234;
    return __t235;
}
struct Vector3 __operator_overload_Vector3_TK_DIVIDE_Vector3(struct Vector3 a, struct Vector3 b) {
    f32 __t236 = a.x / b.x;
    f32 __t237 = a.y / b.y;
    f32 __t238 = a.z / b.z;
    struct Vector3 __t239 = {0};
    __t239.x = __t236;
    __t239.y = __t237;
    __t239.z = __t238;
    return __t239;
}
struct Vector3 __operator_overload_Vector3_TK_MULTIPLY_f32(struct Vector3 a, f32 f) {
    f32 __t240 = a.x * f;
    f32 __t241 = a.y * f;
    f32 __t242 = a.z * f;
    struct Vector3 __t243 = {0};
    __t243.x = __t240;
    __t243.y = __t241;
    __t243.z = __t242;
    return __t243;
}
void operator_overloading() {
    String __t244 = MAKE_STRING("\n\n---- operator_overloading ----\n", 33);
    Any __t245[1];
    Slice __t246;
    __t246.data = __t245;
    __t246.count = 0;
    print(__t244, __t246);
    f32 __t247 = 1.000000;
    f32 __t248 = 2.000000;
    f32 __t249 = 3.000000;
    struct Vector3 __t250 = {0};
    __t250.x = __t247;
    __t250.y = __t248;
    __t250.z = __t249;
    struct Vector3 v1 = __t250;
    f32 __t251 = 1.000000;
    f32 __t252 = 4.000000;
    f32 __t253 = 9.000000;
    struct Vector3 __t254 = {0};
    __t254.x = __t251;
    __t254.y = __t252;
    __t254.z = __t253;
    struct Vector3 v2 = __t254;
    struct Vector3 __t255 = __operator_overload_Vector3_TK_PLUS_Vector3(v1, v2);
    struct Vector3 v3 = __t255;
    f32 __t256 = 5.000000;
    struct Vector3 __t257 = __operator_overload_Vector3_TK_MULTIPLY_f32(v3, __t256);
    struct Vector3 v4 = __t257;
    String __t258 = MAKE_STRING("%\n", 2);
    Any __t259[1];
    __t259[0] = MAKE_ANY(&v4.x, 9);
    Slice __t260;
    __t260.data = __t259;
    __t260.count = 1;
    print(__t258, __t260);
    String __t261 = MAKE_STRING("%\n", 2);
    Any __t262[1];
    __t262[0] = MAKE_ANY(&v4.y, 9);
    Slice __t263;
    __t263.data = __t262;
    __t263.count = 1;
    print(__t261, __t263);
    String __t264 = MAKE_STRING("%\n", 2);
    Any __t265[1];
    __t265[0] = MAKE_ANY(&v4.z, 9);
    Slice __t266;
    __t266.data = __t265;
    __t266.count = 1;
    print(__t264, __t266);
    f32 __t268 = 10.000000;
    bool __t267 = v4.x == __t268;
    assert(__t267);
    f32 __t270 = 30.000000;
    bool __t269 = v4.y == __t270;
    assert(__t269);
    f32 __t272 = 60.000000;
    bool __t271 = v4.z == __t272;
    assert(__t271);
}
i64 value_poly__polymorph_0() {
    i64 __t273 = 4;
    return __t273;
}
i64 type_poly__polymorph_1(i64 a) {
    i64 __t274 = a * a;
    return __t274;
}
f32 type_poly__polymorph_2(f32 a) {
    f32 __t275 = a * a;
    return __t275;
}
i64 value_and_type_poly__polymorph_3() {
    i64 __t276 = 49;
    return __t276;
}
f32 value_and_type_poly__polymorph_4() {
    f32 __t277 = 64.000000;
    return __t277;
}
void procedural_polymorphism() {
    String __t278 = MAKE_STRING("\n\n---- procedural_polymorphism ----\n", 36);
    Any __t279[1];
    Slice __t280;
    __t280.data = __t279;
    __t280.count = 0;
    print(__t278, __t280);
    String __t281 = MAKE_STRING("%\n", 2);
    Any __t282[1];
    i64 __t283 = value_poly__polymorph_0();
    __t282[0] = MAKE_ANY(&__t283, 4);
    Slice __t284;
    __t284.data = __t282;
    __t284.count = 1;
    print(__t281, __t284);
    i64 __t286 = value_poly__polymorph_0();
    i64 __t287 = 4;
    bool __t285 = __t286 == __t287;
    assert(__t285);
    String __t288 = MAKE_STRING("%\n", 2);
    Any __t289[1];
    i64 __t290 = 3;
    i64 __t291 = type_poly__polymorph_1(__t290);
    __t289[0] = MAKE_ANY(&__t291, 4);
    Slice __t292;
    __t292.data = __t289;
    __t292.count = 1;
    print(__t288, __t292);
    i64 __t294 = 3;
    i64 __t295 = type_poly__polymorph_1(__t294);
    i64 __t296 = 9;
    bool __t293 = __t295 == __t296;
    assert(__t293);
    f32 __t297 = 4.000000;
    f32 __t298 = type_poly__polymorph_2(__t297);
    print_float(__t298);
    f32 __t300 = 4.000000;
    f32 __t301 = type_poly__polymorph_2(__t300);
    f32 __t302 = 16.000000;
    bool __t299 = __t301 == __t302;
    assert(__t299);
    i64 __t303 = 5;
    i64 a = __t303;
    f32 __t304 = 6.000000;
    f32 f = __t304;
    String __t305 = MAKE_STRING("%\n", 2);
    Any __t306[1];
    i64 __t307 = type_poly__polymorph_1(a);
    __t306[0] = MAKE_ANY(&__t307, 4);
    Slice __t308;
    __t308.data = __t306;
    __t308.count = 1;
    print(__t305, __t308);
    String __t309 = MAKE_STRING("%\n", 2);
    Any __t310[1];
    f32 __t311 = type_poly__polymorph_2(f);
    __t310[0] = MAKE_ANY(&__t311, 9);
    Slice __t312;
    __t312.data = __t310;
    __t312.count = 1;
    print(__t309, __t312);
    i64 __t314 = type_poly__polymorph_1(a);
    i64 __t315 = 25;
    bool __t313 = __t314 == __t315;
    assert(__t313);
    f32 __t317 = type_poly__polymorph_2(f);
    f32 __t318 = 36.000000;
    bool __t316 = __t317 == __t318;
    assert(__t316);
    String __t319 = MAKE_STRING("%\n", 2);
    Any __t320[1];
    i64 __t321 = value_and_type_poly__polymorph_3();
    __t320[0] = MAKE_ANY(&__t321, 4);
    Slice __t322;
    __t322.data = __t320;
    __t322.count = 1;
    print(__t319, __t322);
    String __t323 = MAKE_STRING("%\n", 2);
    Any __t324[1];
    f32 __t325 = value_and_type_poly__polymorph_4();
    __t324[0] = MAKE_ANY(&__t325, 9);
    Slice __t326;
    __t326.data = __t324;
    __t326.count = 1;
    print(__t323, __t326);
    i64 __t328 = value_and_type_poly__polymorph_3();
    i64 __t329 = 49;
    bool __t327 = __t328 == __t329;
    assert(__t327);
    f32 __t331 = value_and_type_poly__polymorph_4();
    f32 __t332 = 64.000000;
    bool __t330 = __t331 == __t332;
    assert(__t330);
}
struct Custom_Array_Type__polymorph_5 {
    struct Static_Array_8_i64 array;
};
i64 *__operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(struct Custom_Array_Type__polymorph_5 *my_array, i64 index) {
    return &(*my_array).array.elements[index];
}
void structural_polymorphism() {
    String __t333 = MAKE_STRING("\n\n---- structural_polymorphism ----\n", 36);
    Any __t334[1];
    Slice __t335;
    __t335.data = __t334;
    __t335.count = 0;
    print(__t333, __t335);
    struct Custom_Array_Type__polymorph_5 array_of_ints = {0};
    i64 __t336 = 4;
    i64 *__t337 = __operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(&array_of_ints, __t336);
    i64 __t338 = 124;
    (*__t337) = __t338;
    String __t339 = MAKE_STRING("%\n", 2);
    Any __t340[1];
    i64 __t341 = 4;
    i64 *__t342 = __operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(&array_of_ints, __t341);
    __t340[0] = MAKE_ANY(&(*__t342), 4);
    Slice __t343;
    __t343.data = __t340;
    __t343.count = 1;
    print(__t339, __t343);
    i64 __t345 = 4;
    i64 *__t346 = __operator_overload_Custom_Array_Type__polymorph_5_TK_LEFT_SQUARE_i64(&array_of_ints, __t345);
    i64 __t347 = 124;
    bool __t344 = (*__t346) == __t347;
    assert(__t344);
}
void print_things(Slice args) {
    {
        i64 __t348 = 0;
        i64 i = __t348;
        while (true) {
            bool __t349 = i < args.count;
            if (!__t349) { break; }
            Any arg = ((Any *)args.data)[i];
            i64 __t351 = 4;
            bool __t350 = arg.type == __t351;
            if (__t350) {
                String __t352 = MAKE_STRING("%\n", 2);
                Any __t353[1];
                i64 *__t354 = ((i64 *)arg.data);
                __t353[0] = MAKE_ANY(&(*__t354), 4);
                Slice __t355;
                __t355.data = __t353;
                __t355.count = 1;
                print(__t352, __t355);
            }
            else {
                i64 __t357 = 13;
                bool __t356 = arg.type == __t357;
                if (__t356) {
                    String __t358 = MAKE_STRING("%\n", 2);
                    Any __t359[1];
                    String *__t360 = ((String *)arg.data);
                    __t359[0] = MAKE_ANY(&(*__t360), 13);
                    Slice __t361;
                    __t361.data = __t359;
                    __t361.count = 1;
                    print(__t358, __t361);
                }
                else {
                    i64 __t363 = 11;
                    bool __t362 = arg.type == __t363;
                    if (__t362) {
                        String __t364 = MAKE_STRING("%\n", 2);
                        Any __t365[1];
                        bool *__t366 = ((bool *)arg.data);
                        __t365[0] = MAKE_ANY(&(*__t366), 11);
                        Slice __t367;
                        __t367.data = __t365;
                        __t367.count = 1;
                        print(__t364, __t367);
                    }
                    else {
                        i64 __t369 = 9;
                        bool __t368 = arg.type == __t369;
                        if (__t368) {
                            String __t370 = MAKE_STRING("%\n", 2);
                            Any __t371[1];
                            f32 *__t372 = ((f32 *)arg.data);
                            __t371[0] = MAKE_ANY(&(*__t372), 9);
                            Slice __t373;
                            __t373.data = __t371;
                            __t373.count = 1;
                            print(__t370, __t373);
                        }
                    }
                }
            }
            i64 __t374 = 1;
            i += __t374;
        }
    }
}
void any_type() {
    String __t375 = MAKE_STRING("\n\n---- any ----\n", 16);
    Any __t376[1];
    Slice __t377;
    __t377.data = __t376;
    __t377.count = 0;
    print(__t375, __t377);
    i64 __t378 = 123;
    i64 some_int = __t378;
    Any a = MAKE_ANY(&some_int, 4);
    bool __t379 = a.data == &some_int;
    assert(__t379);
    i64 __t381 = 4;
    bool __t380 = a.type == __t381;
    assert(__t380);
    i64 *__t382 = ((i64 *)a.data);
    i64 b = (*__t382);
    String __t383 = MAKE_STRING("%\n", 2);
    Any __t384[1];
    __t384[0] = MAKE_ANY(&b, 4);
    Slice __t385;
    __t385.data = __t384;
    __t385.count = 1;
    print(__t383, __t385);
    Any __t386[4];
    bool __t387 = true;
    __t386[0] = MAKE_ANY(&__t387, 11);
    i64 __t388 = 456;
    __t386[1] = MAKE_ANY(&__t388, 4);
    f32 __t389 = 78.900000;
    __t386[2] = MAKE_ANY(&__t389, 9);
    String __t390 = MAKE_STRING("hello", 5);
    __t386[3] = MAKE_ANY(&__t390, 13);
    Slice __t391;
    __t391.data = __t386;
    __t391.count = 4;
    print_things(__t391);
}
struct Dynamic_Array__polymorph_6 {
    Slice array;
    i64 count;
};
struct Vector3 *__operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(struct Dynamic_Array__polymorph_6 *dyn, i64 index) {
    return &((struct Vector3 *)(*dyn).array.data)[index];
}
void append__polymorph_7(struct Dynamic_Array__polymorph_6 *dyn, struct Vector3 value) {
    bool __t392 = dyn->count == dyn->array.count;
    if (__t392) {
        struct Vector3 *old_data = *((struct Vector3 **)&dyn->array.data);
        i64 __t394 = 8;
        i64 __t396 = 2;
        i64 __t395 = dyn->array.count * __t396;
        i64 __t393 = __t394 + __t395;
        i64 new_cap = __t393;
        i64 __t398 = 12;
        i64 __t397 = new_cap * __t398;
        void *__t399 = alloc(__t397);
        struct Vector3 *__t400 = ((struct Vector3 *)__t399);
        *((struct Vector3 **)&dyn->array.data) = __t400;
        dyn->array.count = new_cap;
        bool __t401 = old_data != NULL;
        if (__t401) {
            i64 __t403 = 12;
            i64 __t402 = dyn->count * __t403;
            u32 __t404 = ((u32 )__t402);
            void *__t405 = memcpy(*((struct Vector3 **)&dyn->array.data), old_data, __t404);
            free(old_data);
        }
    }
    bool __t406 = dyn->count < dyn->array.count;
    assert(__t406);
    ((struct Vector3 *)dyn->array.data)[dyn->count] = value;
    i64 __t407 = 1;
    dyn->count += __t407;
}
void destroy_dynamic_array__polymorph_8(struct Dynamic_Array__polymorph_6 dyn) {
    bool __t408 = *((struct Vector3 **)&dyn.array.data) != NULL;
    if (__t408) {
        free(*((struct Vector3 **)&dyn.array.data));
    }
}
void dynamic_arrays() {
    String __t409 = MAKE_STRING("\n\n---- dynamic_arrays ----\n", 27);
    Any __t410[1];
    Slice __t411;
    __t411.data = __t410;
    __t411.count = 0;
    print(__t409, __t411);
    struct Dynamic_Array__polymorph_6 dyn = {0};
    f32 __t412 = 1.000000;
    f32 __t413 = 2.000000;
    f32 __t414 = 3.000000;
    struct Vector3 __t415 = {0};
    __t415.x = __t412;
    __t415.y = __t413;
    __t415.z = __t414;
    append__polymorph_7(&dyn, __t415);
    f32 __t416 = 1.000000;
    f32 __t417 = 4.000000;
    f32 __t418 = 9.000000;
    struct Vector3 __t419 = {0};
    __t419.x = __t416;
    __t419.y = __t417;
    __t419.z = __t418;
    append__polymorph_7(&dyn, __t419);
    f32 __t420 = 2.000000;
    f32 __t421 = 8.000000;
    f32 __t422 = 18.000000;
    struct Vector3 __t423 = {0};
    __t423.x = __t420;
    __t423.y = __t421;
    __t423.z = __t422;
    append__polymorph_7(&dyn, __t423);
    i64 __t425 = 1;
    struct Vector3 *__t426 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, __t425);
    f32 __t427 = 1.000000;
    bool __t424 = (*__t426).x == __t427;
    assert(__t424);
    i64 __t429 = 1;
    struct Vector3 *__t430 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, __t429);
    f32 __t431 = 4.000000;
    bool __t428 = (*__t430).y == __t431;
    assert(__t428);
    i64 __t433 = 1;
    struct Vector3 *__t434 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, __t433);
    f32 __t435 = 9.000000;
    bool __t432 = (*__t434).z == __t435;
    assert(__t432);
    {
        i64 __t436 = 0;
        i64 i = __t436;
        while (true) {
            bool __t437 = i < dyn.count;
            if (!__t437) { break; }
            String __t438 = MAKE_STRING("%\n", 2);
            Any __t439[1];
            __t439[0] = MAKE_ANY(&i, 4);
            Slice __t440;
            __t440.data = __t439;
            __t440.count = 1;
            print(__t438, __t440);
            String __t441 = MAKE_STRING("%\n", 2);
            Any __t442[1];
            struct Vector3 *__t443 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, i);
            __t442[0] = MAKE_ANY(&(*__t443).x, 9);
            Slice __t444;
            __t444.data = __t442;
            __t444.count = 1;
            print(__t441, __t444);
            String __t445 = MAKE_STRING("%\n", 2);
            Any __t446[1];
            struct Vector3 *__t447 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, i);
            __t446[0] = MAKE_ANY(&(*__t447).y, 9);
            Slice __t448;
            __t448.data = __t446;
            __t448.count = 1;
            print(__t445, __t448);
            String __t449 = MAKE_STRING("%\n", 2);
            Any __t450[1];
            struct Vector3 *__t451 = __operator_overload_Dynamic_Array__polymorph_6_TK_LEFT_SQUARE_i64(&dyn, i);
            __t450[0] = MAKE_ANY(&(*__t451).z, 9);
            Slice __t452;
            __t452.data = __t450;
            __t452.count = 1;
            print(__t449, __t452);
            i64 __t453 = 1;
            i += __t453;
        }
    }
    destroy_dynamic_array__polymorph_8(dyn);
}
i32 main() {
    String __t454 = MAKE_STRING("-------------------------\n", 26);
    Any __t455[1];
    Slice __t456;
    __t456.data = __t455;
    __t456.count = 0;
    print(__t454, __t456);
    String __t457 = MAKE_STRING("|   sif language demo   |\n", 26);
    Any __t458[1];
    Slice __t459;
    __t459.data = __t458;
    __t459.count = 0;
    print(__t457, __t459);
    String __t460 = MAKE_STRING("-------------------------\n", 26);
    Any __t461[1];
    Slice __t462;
    __t462.data = __t461;
    __t462.count = 0;
    print(__t460, __t462);
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
    dynamic_arrays();
    i32 __t463 = 0;
    return __t463;
}
