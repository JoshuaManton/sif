#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "os_windows.h"
#include "lexer.h"
#include "parser.h"
#include "checker.h"
#include "c_backend.h"
#include "ir.h"

/*
TODO:

HIGH PRIORITY
-default procedure parameters?
-#caller_location
-handle usage before declaration
-handle unary operators nicer, currently quick-and-dirty
-#print and #assert don't work in global scope right now
-allow operator overloads to be declared outside a struct
-+=, -=, etc for operator overloading
-do proper nested selector expression elimination with 'using'
-#link_name
-new_slice/new_string are not the right word. use 'make'
-namespacing bug {
    proc main() {
        bar: int;
        foo();
        proc foo() {
            printa(bar); // the compiler thinks it knows what 'bar' is, resulting in an error in the C code
        }
    }
}
-get rid of spinlocking in dynamic_arena, just give each thread its own allocator
-special sauce for [] overloads instead of actually having reference types in the language

MEDIUM PRIORITY
-add more comprehensive numbers test
-add binary literals 0b0010110111100001
-enum arrays @EnumArrays
-don't generate short-circuiting jumps if it is unnecessary
-put struct constants in Type_Info
-add type_untyped_number that can convert to int or float so that type_untyped_int can't convert to float in the case of a char literal
-deduplicate #foreign_import/#foreign_system_import
-add assert to c_backend that makes sure all declarations have been checked. looks like some array type decls are bypassing check_declaration
-block comments
-while foo := bar(); (foo != null)
-sized bool types
-underscores in numbers
-uninitialized stack members
-error if you use reserved C identifiers like 'char', 'double', 'signed', etc?
-Expr_Change (v2 := v1.{y=4};)
-slicing
-declare distinct types
-allow passing a ^Derived to a proc that takes ^Base if the first field in Derived is 'using base: Base'
-procedure literal expression handling
-#no_bounds_checks
-#no_null_checks
-remove runtime's dependency on basic
-@UnaryOperatorOverloading
-switch statements
-ternary operator maybe?
-read command line parameters
-good logging of cyclic dependencies
-reference-to-reference parsing bug: var x: >>int lexes as a shift-right
-what should the semantics be for reference-to-reference?
-foreach loops
-#if
-#location()
-multiple return values
-namespaced imports i.e. `#include Foo "foo.sif"`
-append(&dyn, {1, 4, 9}); doesn't work because we can't use expected types when initially checking a polymorphic procedure right now

LOW PRIORITY
-utf8 strings
-enum field @notes?
-#this to refer to struct type for anonymous
-use a custom allocator in __init_sif_runtime
-add allocators to demo
-opt=N
-cast(Type)value syntax in addition to the current cast(Type, value) maybe?
-unknown directives don't stop compilation
-figure out if I should allow shadowing (maybe with a keyword?)
-build to dll
-check for use-before-declaration of local vars
-make C output a bit prettier, whatever that means
-tagged unions?
-allow custom entrypoints
-implicit polymorphism
-loop/block labels a la Odin
-<<< and >>> for shift rotate?
-procedure overloading
*/

void print_usage() {
    printf("sif compiler v0.2.1\n\n");

    printf("Usage:\n");
    printf("  sif <build|run|check> <file> [optional flags]\n\n");

    printf("Optional flags:\n");
    printf("  -o <output>.exe             Override the default output .exe name.\n");
    // printf("  -opt <level>                Optimization level.\n");
    printf("  -debug                      Generate debug information.\n");
    printf("  -log-cl-command             Print the invocation command for cl.exe.\n");
    printf("  -show-timings               Print a log of times for different compilation stages.\n");
    printf("  -keep-temp-files            Don't delete intermediate files used for compilation.\n");
    printf("  -no-type-info               Don't generate runtime type information.\n");
    printf("  -no-threads                 Don't compile with multithreading.\n");
}

char *sif_core_lib_path;

Allocator g_global_linear_allocator;

bool g_logged_error = {};

Timer g_global_timer = {};

// todo(josh): put these in some kind of Build_Options struct in common.h?
bool g_no_type_info = {};
bool g_no_threads = {};
bool g_is_debug_build = {};

int main(int argc, char **argv) {
    init_timer(&g_global_timer);
    double application_start_time = query_timer(&g_global_timer);

    if (argc < 3) {
        print_usage();
        return 0;
    }

    Dynamic_Arena dynamic_arena = {};
    init_dynamic_arena(&dynamic_arena, 25 * 1024 * 1024, default_allocator());
    g_global_linear_allocator = dynamic_arena_allocator(&dynamic_arena);

    char *sif_exe_path = get_current_exe_name(g_global_linear_allocator);
    char *sif_root = path_directory(sif_exe_path, g_global_linear_allocator);
    String_Builder core_lib_builder = make_string_builder(g_global_linear_allocator, 64);
    core_lib_builder.printf("%s/core", sif_root);
    sif_core_lib_path = core_lib_builder.string();

    bool is_run   = false;
    bool is_build = false;
    bool is_check = false;

    if (strcmp(argv[1], "run") == 0) {
        is_run = true;
    }
    else if (strcmp(argv[1], "build") == 0) {
        is_build = true;
    }
    else if (strcmp(argv[1], "check") == 0) {
        is_check = true;
    }
    else {
        printf("Unknown compiler flag: %s\n", argv[1]);
        print_usage();
        return 0;
    }

    char *file_to_compile = argv[2];
    if (!ends_with(file_to_compile, ".sif")) {
        printf("File to compile must end with '.sif'");
        return 0;
    }

    bool show_timings = false;
    bool keep_temp_files = false;
    bool log_cl_command = false;
    bool ir_test = false;

    char *output_exe_name = nullptr;

    for (int i = 3; i < argc; i++) {
        char *arg = argv[i];
        if (strcmp(arg, "-show-timings") == 0) {
            show_timings = true;
        }
        else if (strcmp(arg, "-keep-temp-files") == 0) {
            keep_temp_files = true;
        }
        else if (strcmp(arg, "-debug") == 0) {
            g_is_debug_build = true;
        }
        else if (strcmp(arg, "-no-type-info") == 0) {
            g_no_type_info = true;
        }
        else if (strcmp(arg, "-no-threads") == 0) {
            g_no_threads = true;
        }
        else if (strcmp(arg, "-log-cl-command") == 0) {
            log_cl_command = true;
        }
        else if (strcmp(arg, "-ir-test") == 0) {
            ir_test = true;
        }
        else if (strcmp(arg, "-o") == 0) {
            if ((i+1) >= argc) {
                printf("Missing argument for -o flag.");
                return 0;
            }
            if (!ends_with(argv[i+1], ".exe")) {
                printf("Argument for -o must end with '.exe'");
                return 0;
            }
            output_exe_name = argv[i+1];
            i += 1;
        }
        else {
            printf("Unknown compiler flag: %s\n", arg);
            print_usage();
            return 0;
        }
    }

    if (output_exe_name == nullptr) {
        char *file_to_compile_without_extension = path_filename(file_to_compile, g_global_linear_allocator);
        String_Builder output_exe_name_sb = make_string_builder(g_global_linear_allocator, 64);
        output_exe_name_sb.printf("%s.exe", file_to_compile_without_extension);
        output_exe_name = output_exe_name_sb.string();
    }

    init_lexer_globals();
    init_interned_strings();
    init_parser();
    init_checker();

    double parsing_start_time = query_timer(&g_global_timer);

    Ast_Block *global_scope = begin_parsing(file_to_compile);
    if (!global_scope || g_logged_error) {
        printf("There were errors.\n");
        return 0;
    }

    add_global_declarations(global_scope);

    double checking_start_time = query_timer(&g_global_timer);

    init_c_backend();

    bool check_success = typecheck_global_scope(global_scope);
    if (!check_success) {
        printf("There were errors.\n");
        return 0;
    }

    if (ir_test) {
        IR *ir = SIF_NEW_CLONE(IR(), g_global_linear_allocator);
        IR_Proc *ir_main = generate_ir_proc(ir, g_main_proc);
        For (idx, ir_main->statements) {
            IR_Statement *statement = ir_main->statements[idx];
            printf_ir_statement(statement);
            printf("\n");
        }
    }

    double codegen_start_time = query_timer(&g_global_timer);

    Chunked_String_Builder c_code = generate_c_main_file(global_scope);
    write_entire_file("output.c", c_code.make_string());

    double c_compile_start_time = query_timer(&g_global_timer);

    if (!is_check) {
        String_Builder command_sb = make_string_builder(g_global_linear_allocator, 128);
        command_sb.printf("cmd.exe /O2 /c \"cl.exe ");
        if (g_is_debug_build) {
            command_sb.print("/Zi /Fd ");
        }
        command_sb.print("output.c /nologo /wd4028 ");

        For (idx, g_all_foreign_import_directives) {
            command_sb.printf("\"%s\" ", g_all_foreign_import_directives[idx]->path);
        }
        command_sb.printf("/link /OUT:\"%s\" ", output_exe_name);
        if (g_is_debug_build) {
            command_sb.print("/DEBUG ");
        }
        command_sb.printf("\"");

        if (log_cl_command) {
            printf("%s\n", command_sb.string());
        }

        FILE *pipe = _popen(command_sb.string(), "r");
        char buffer[1024];
        Chunked_String_Builder error_sb = make_chunked_string_builder(g_global_linear_allocator, 1024);
        bool first_thing = true;
        while (fgets(buffer, sizeof(buffer), pipe)) {
            char *str = buffer;
            if (first_thing) {
                if (starts_with(buffer, "output.c\n")) {
                    str += strlen("output.c\n");
                }
            }
            error_sb.print(str);
            first_thing = false;
        }
        char *errors = error_sb.make_string();
        printf(errors);
        if (strlen(errors) != 0) {
            printf("\nInternal compiler error: sif encountered an error when compiling C output. Exiting.\n");
            return 0;
        }

        if (!keep_temp_files) {
            delete_file("output.c");
            delete_file("output.obj");
        }
    }

    double compilation_end_time = query_timer(&g_global_timer);

    if (show_timings) {
        float setup_time   = parsing_start_time   - application_start_time;
        float parse_time   = checking_start_time  - parsing_start_time;
        float check_time   = codegen_start_time   - checking_start_time;
        float codegen_time = c_compile_start_time - codegen_start_time;
        float cl_time      = compilation_end_time - c_compile_start_time;
        float sif_time     = c_compile_start_time - application_start_time;
        float total_time   = compilation_end_time - application_start_time;

        printf("-----------------------------\n");
        printf("|    sif compile timings    |\n");
        printf("-----------------------------\n");
        printf("  Setup time: %.3fms (%.2f%% sif, %.2f%% total)\n", setup_time,   (setup_time   / sif_time * 100), (setup_time   / total_time * 100));
        printf("  Parse time: %.3fms (%.2f%% sif, %.2f%% total)\n", parse_time,   (parse_time   / sif_time * 100), (parse_time   / total_time * 100));
        printf("  Check time: %.3fms (%.2f%% sif, %.2f%% total)\n", check_time,   (check_time   / sif_time * 100), (check_time   / total_time * 100));
        printf("Codegen time: %.3fms (%.2f%% sif, %.2f%% total)\n", codegen_time, (codegen_time / sif_time * 100), (codegen_time / total_time * 100));
        printf(" cl.exe time: %.3fms (%.2f%% sif, %.2f%% total)\n", cl_time,      (cl_time      / sif_time * 100), (cl_time      / total_time * 100));
        printf("\n");
        printf("  Sif time: %fms\n", sif_time);
        printf("Total time: %fms\n", total_time);
        printf("\n");
        printf("  Total lines: %d\n", g_total_lines_parsed);
        printf("Sif lines/sec: %.0f\n", ((float)g_total_lines_parsed) / (sif_time / 1000));
    }

    if (is_run) {
        String_Builder run_command_sb = make_string_builder(g_global_linear_allocator, 64);
        run_command_sb.printf("cmd.exe /c \"%s\"", output_exe_name);
        system(run_command_sb.string());
    }
}