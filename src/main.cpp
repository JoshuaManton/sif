#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>

// todo(josh): microsoft craziness
// #define MICROSOFT_CRAZINESS_IMPLEMENTATION
// #include "microsoft_craziness.h"

#include "common.h"
#include "os_windows.h"
#include "lexer.h"
#include "parser.h"
#include "checker.h"
#include "c_backend.h"

/*
TODO:

SMALL
-handle unary operators nicer, currently quick-and-dirty
-opt=N
-unknown directives don't stop compilation
-block comments
-intern identifiers and remove strcmps
-add allocators to demo
-underscores in numbers
-crash when you don't pass enough parameters?
-cstring type
-prevent identifiers from being C keywords in the backend, like `signed`

MEDIUM
-defer
-runtime bounds checks
-switch statements
-emit #line directives in backend
-do entrypoint returning i32 implicitly in backend
-slicing
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
-use microsoft_craziness.h
-enum arrays
-allow custom entrypoints

BIG
-control flow graph analysis
-type_info
-#if
-C varargs for bindings
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

void print_usage() {
    printf("Usage:\n");
    printf("  sif <build|run> <file> [optional flags]\n\n");

    printf("Optional flags:\n");
    printf("  -show-timings           Print a log of times for different compilation stages.\n");
    printf("  -keep-temp-files        Don't delete intermediate files used for compilation.\n");
    printf("  -o <output>.exe         Override the default output .exe name.\n");
}

char *sif_core_lib_path;

Allocator g_global_linear_allocator;

void main(int argc, char **argv) {
    Timer timer = {};
    init_timer(&timer);
    double application_start_time = query_timer(&timer);

    if (argc < 3) {
        print_usage();
        return;
    }

    Dynamic_Arena dynamic_arena = {};
    init_dynamic_arena(&dynamic_arena, 10 * 1024 * 1024, default_allocator());
    g_global_linear_allocator = dynamic_arena_allocator(&dynamic_arena);

    char *sif_exe_path = get_current_exe_name(g_global_linear_allocator);
    char *sif_root = path_directory(sif_exe_path, g_global_linear_allocator);
    String_Builder core_lib_builder = make_string_builder(g_global_linear_allocator, 64);
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
    bool build_debug = false;
    bool log_cl_command = false;

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
            build_debug = true;
        }
        else if (strcmp(arg, "-log-cl-command") == 0) {
            log_cl_command = true;
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
        char *file_to_compile_without_extension = path_filename(file_to_compile, g_global_linear_allocator);
        String_Builder output_exe_name_sb = make_string_builder(g_global_linear_allocator, 64);
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

    // todo(josh): microsoft craziness
    // Find_Result fr = find_visual_studio_and_windows_sdk();
    // char *windows_sdk_root = wide_to_cstring(fr.windows_sdk_root);
    // char *exe_path = wide_to_cstring(fr.vs_exe_path);
    // char *lib_path = wide_to_cstring(fr.vs_library_path);
    // char *um_lib_path = wide_to_cstring(fr.windows_sdk_um_library_path);
    // char *ucrt_lib_path = wide_to_cstring(fr.windows_sdk_ucrt_library_path);
    // printf("win sdk root   %s\n", windows_sdk_root);
    // printf("exe_path:      %s\n", exe_path);
    // printf("lib_path:      %s\n", lib_path);
    // printf("um_lib_path:   %s\n", um_lib_path);
    // printf("ucrt_lib_path: %s\n", ucrt_lib_path);

    String_Builder command_sb = make_string_builder(g_global_linear_allocator, 128);
    command_sb.printf("cmd.exe /c \"cl.exe ");
    if (build_debug) {
        command_sb.print("/Zi /Fd ");
    }
    command_sb.print("output.c /nologo /wd4028 ");
    For (idx, g_all_foreign_import_directives) {
        command_sb.printf("%s ", g_all_foreign_import_directives[idx]->path);
    }
    command_sb.printf("/link /OUT:%s ", output_exe_name);
    // todo(josh): microsoft craziness
    // command_sb.printf("/libpath:\"%s\" ", lib_path);
    // command_sb.printf("/libpath:\"%s\" ", ucrt_lib_path);
    // command_sb.printf("/libpath:\"%s\" ", um_lib_path);
    // command_sb.printf("-nodefaultlib ");
    if (build_debug) {
        command_sb.print("/DEBUG ");
    }
    command_sb.printf("\"", output_exe_name);

    if (log_cl_command) {
        printf("%s\n", command_sb.string());
    }

    if (system(command_sb.string()) != 0) {
        printf("\nInternal compiler error: sif encountered an error when compiling C output. Exiting.\n");
        return;
    }

    if (!keep_temp_files) {
        delete_file("output.c");
        delete_file("output.obj");
    }

    double compilation_end_time = query_timer(&timer);

    if (show_timings) {
        printf("-----------------------------\n");
        printf("|    sif compile timings    |\n");
        printf("-----------------------------\n");
        printf("  Setup time: %fms\n", (parsing_start_time   - application_start_time));
        printf("  Parse time: %fms\n", (checking_start_time  - parsing_start_time));
        printf("  Check time: %fms\n", (codegen_start_time   - checking_start_time));
        printf("Codegen time: %fms\n", (c_compile_start_time - codegen_start_time));
        printf(" cl.exe time: %fms\n", (compilation_end_time - c_compile_start_time));
        printf("\n");
        printf("  Sif time: %fms\n", (c_compile_start_time - application_start_time));
        printf("Total time: %fms\n", (compilation_end_time - application_start_time));
    }

    if (is_run) {
        String_Builder run_command_sb = make_string_builder(g_global_linear_allocator, 64);
        run_command_sb.printf("cmd.exe /c \"%s\"", output_exe_name);
        system(run_command_sb.string());
    }
    else {
        assert(is_build);
    }
}