#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// #define MICROSOFT_CRAZINESS_IMPLEMENTATION
// #include "microsoft_craziness.h"

#include "common.h"
#include "lexer.h"
#include "parser.h"
#include "checker.h"
#include "c_backend.h"

/*
TODO:

THINGS NEEDED FOR ME TO DRAW A TRIANGLE
-foreign system + D3D11 bindings
-enum field values (probably)

SMALL
-unions
-constant bounds checks i.e. var arr: [4]int; arr[232];
-deduplicate #include paths
-opt=N
-unknown directives don't stop compilation
-error when instantiating a polymorphic struct without parameters
-block comments
-@notes on declarations
-intern identifiers and remove strcmps

MEDIUM
-allow custom entrypoints
-enforce entrypoints returning i32, or do it implicitly in the backend
-enum field expressions
-defer
-foreign system
-runtime bounds checks
-switch statements
-emit #line directives in backend
-change everything to use custom allocators
-slicing
-transmute
-build to dll
-read command line parameters
-check for use-before-declaration of local vars
-make C output a bit prettier, whatever that means
-good logging of cyclic dependencies
-reference-to-reference parsing bug: var x: >>int lexes as a shift-right
-put struct constants in a different block than struct instance members
-polymorphic structs should register their parameters as local constants
-locally scoped structs and procs
-tagged unions

BIG
-control flow graph analysis
-type_info
-#if
-C varargs for bindings
-cstring type
-default procedure parameters
-#caller_location
-#location()
-multiple return values
-iterative solver for polymorphism
-implicit polymorphism
-namespaced imports i.e. `#include Foo "foo.sif"`
-using
-right now operator overloading requires the first parameter to be the struct that you are overloading for. this is not ideal because you want to be able to do float * vector
-assigning to reference-to-reference doesn't work. I'm not sure what the behaviour should be
-foreach loops
-figure out if I should allow shadowing (maybe with a keyword?)
*/

struct Timer {
    double frequency = {};
};

void init_timer(Timer *timer) {
    LARGE_INTEGER large_integer_frequency = {};
    assert(QueryPerformanceFrequency(&large_integer_frequency) != 0);
    timer->frequency = large_integer_frequency.QuadPart/1000.0;
}

double query_timer(Timer *timer) {
    LARGE_INTEGER large_integer_counter = {};
    assert(QueryPerformanceCounter(&large_integer_counter) != 0);
    return large_integer_counter.QuadPart / timer->frequency;
}

char *wide_to_cstring(wchar_t *wide) {
    int query_result = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    assert(query_result > 0);

    char *cstring = (char *)alloc(default_allocator(), query_result);
    int result = WideCharToMultiByte(CP_UTF8, 0, wide, -1, cstring, query_result, nullptr, nullptr);

    assert(result == query_result);
    return cstring;
}

void print_usage() {
    printf("Usage:\n");
    printf("  sif <build|run> <file> [optional flags]\n\n");

    printf("Optional flags:\n");
    printf("  -show-timings           Print a log of times for different compilation stages.\n");
    printf("  -keep-temp-files        Don't delete intermediate files used for compilation.\n");
    printf("  -o <output>.exe         Override the default output .exe name.\n");
}

char *sif_core_lib_path;

void main(int argc, char **argv) {
    Timer timer = {};
    init_timer(&timer);
    double application_start_time = query_timer(&timer);

    if (argc < 3) {
        print_usage();
        return;
    }

    wchar_t exe_path_wide[MAX_PATH];
    GetModuleFileNameW(nullptr, exe_path_wide, MAX_PATH);
    char *sif_exe_path = wide_to_cstring(exe_path_wide);
    char *sif_root = path_directory(sif_exe_path, default_allocator());
    String_Builder core_lib_builder = make_string_builder(default_allocator(), 64);
    core_lib_builder.printf("%s/../core", sif_root);
    sif_core_lib_path = core_lib_builder.string();

    bool is_run   = false;
    bool is_build = false;

    if (strcmp(argv[1], "run") == 0) {
        is_run = true;
    }
    else if (strcmp(argv[1], "build") == 0) {
        is_build = true;
    }
    else {
        printf("Unknown compiler flag: %s\n", argv[1]);
        print_usage();
        return;
    }

    char *file_to_compile = argv[2];
    if (!ends_with(file_to_compile, ".sif")) {
        printf("File to compile must end with '.sif'");
        return;
    }

    bool show_timings = false;
    bool keep_temp_files = false;

    char *output_exe_name = nullptr;

    for (int i = 3; i < argc; i++) {
        char *arg = argv[i];
        if (strcmp(arg, "-show-timings") == 0) {
            show_timings = true;
        }
        else if (strcmp(arg, "-keep-temp-files") == 0) {
            keep_temp_files = true;
        }
        else if (strcmp(arg, "-o") == 0) {
            if ((i+1) >= argc) {
                printf("Missing argument for -o flag.");
                return;
            }
            if (!ends_with(argv[i+1], ".exe")) {
                printf("Argument for -o must end with '.exe'");
                return;
            }
            output_exe_name = argv[i+1];
            i += 1;
        }
        else {
            printf("Unknown compiler flag: %s\n", arg);
            print_usage();
            return;
        }
    }

    if (output_exe_name == nullptr) {
        char *file_to_compile_without_extension = path_filename(file_to_compile, default_allocator());
        String_Builder output_exe_name_sb = make_string_builder(default_allocator(), 64);
        output_exe_name_sb.printf("%s.exe", file_to_compile_without_extension);
        output_exe_name = output_exe_name_sb.string();
    }

    init_lexer_globals();
    init_parser();
    init_checker();

    double parsing_start_time = query_timer(&timer);

    Ast_Block *global_scope = begin_parsing(file_to_compile);
    if (!global_scope) {
        printf("There were errors.\n");
        return;
    }

    add_global_declarations(global_scope);

    double checking_start_time = query_timer(&timer);

    bool check_success = typecheck_global_scope(global_scope);
    if (!check_success) {
        printf("There were errors.\n");
        return;
    }

    double codegen_start_time = query_timer(&timer);

    String_Builder c_code = generate_c_main_file(global_scope);
    write_entire_file("output.c", c_code.string());

    double c_compile_start_time = query_timer(&timer);

    // Find_Result fr = find_visual_studio_and_windows_sdk();
    // todo(josh): get the vs and windows sdk paths
    // char *exe_path = wide_to_cstring(fr.vs_exe_path);
    // char *lib_path = wide_to_cstring(fr.vs_library_path);
    // char *um_lib_path = wide_to_cstring(fr.windows_sdk_um_library_path);
    // char *ucrt_lib_path = wide_to_cstring(fr.windows_sdk_ucrt_library_path);

    String_Builder command_sb = make_string_builder(default_allocator(), 128);
    command_sb.printf("cmd.exe /c \"cl.exe output.c /nologo /link /OUT:%s\"", output_exe_name);

    if (system(command_sb.string()) != 0) {
        printf("\nInternal compiler error: sif encountered an error when compiling C output. Exiting.\n");
        return;
    }

    if (!keep_temp_files) {
        DeleteFileA("output.c");
        DeleteFileA("output.obj");
    }

    double compilation_end_time = query_timer(&timer);

    if (show_timings) {
        printf("-----------------------------\n");
        printf("|    sif compile timings    |\n");
        printf("-----------------------------\n");
        printf("Setup     time: %fms\n", (parsing_start_time   - application_start_time));
        printf("Parse     time: %fms\n", (checking_start_time  - parsing_start_time));
        printf("Check     time: %fms\n", (codegen_start_time   - checking_start_time));
        printf("Codegen   time: %fms\n", (c_compile_start_time - codegen_start_time));
        printf("C compile time: %fms\n", (compilation_end_time - c_compile_start_time));
        printf("Total     time: %fms\n", (compilation_end_time - application_start_time));
    }

    if (is_run) {
        String_Builder run_command_sb = make_string_builder(default_allocator(), 64);
        run_command_sb.printf("cmd.exe /c \"%s\"", output_exe_name);
        system(run_command_sb.string());
    }
    else {
        assert(is_build);
    }
}