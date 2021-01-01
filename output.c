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
void a(Slice args);
void b(Slice args);
struct Static_Array_2_i64 ;
Slice slice_ptr__polymorph_0(i64 *ptr, i64 count);
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
void a(Slice args) {
    String __t57 = MAKE_STRING("---------\n", 10);
    Any __t58[1];
    Slice __t59;
    __t59.data = __t58;
    __t59.count = 0;
    print(__t57, __t59);
    {
        i64 __t60 = 0;
        i64 i = __t60;
        while (true) {
            bool __t61 = i < args.count;
            if (!__t61) { break; }
            String __t62 = MAKE_STRING("%\n", 2);
            Any __t63[1];
            __t63[0] = MAKE_ANY(&((i64 *)args.data)[i], 4);
            Slice __t64;
            __t64.data = __t63;
            __t64.count = 1;
            print(__t62, __t64);
            i64 __t65 = 1;
            i += __t65;
        }
    }
}
void b(Slice args) {
    a(args);
}
struct Static_Array_2_i64  {
    i64  elements[2];
};
Slice slice_ptr__polymorph_0(i64 *ptr, i64 count) {
    Slice slice = {0};
    *((i64 **)&slice.data) = ptr;
    slice.count = count;
    return slice;
}
i32 main() {
    struct Static_Array_2_i64 arr = {0};
    i64 __t66 = 0;
    i64 __t67 = 1;
    arr.elements[__t66] = __t67;
    i64 __t68 = 1;
    i64 __t69 = 2;
    arr.elements[__t68] = __t69;
    i64 __t70 = 0;
    i64 __t71 = 2;
    Slice __t72 = slice_ptr__polymorph_0(&arr.elements[__t70], __t71);
    Slice slice = __t72;
    b(slice);
    i32 __t73 = 0;
    return __t73;
}
