#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>

#include "common.h"
#include "lexer.h"
#include "parser.h"
#include "checker.h"
#include "c_backend.h"

/*
TODO:
-put struct constants in a different block than struct instance members
-error when instantiating a polymorphic struct without parameters
-implicit polymorphism
-function pointers
-varargs
-runtime bounds checks
-constant bounds checks i.e. var arr: [4]int; arr[232];
-right now operator overloading requires the first parameter to be the struct that you are overloading for. this is not ideal because you want to be able to do float * vector
-reference-to-reference parsing bug: var x: >>int lexes as a shift-right
-assigning to reference-to-reference doesn't work. I'm not sure what the behaviour should be
-enum field expressions
-foreach loops
-slicing
-transmute
-switch statements
-#caller_location
-unions
-tagged unions
-defer
-#if
-type_info
-figure out if I should allow shadowing (maybe with a keyword?)
-generate C instead of C++. I think the only thing from C++ I'm using is for the templated Static_Array for array-by-value semantics
*/

void main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage:\n  sif <build|run> <file>\n");
        return;
    }

    bool is_run   = strcmp(argv[1], "run")   == 0;
    bool is_build = strcmp(argv[1], "build") == 0;

    init_lexer_globals();
    init_parser();
    init_checker();

    Ast_Block *global_scope = begin_parsing(argv[2]);
    if (!global_scope) {
        printf("There were errors.\n");
        return;
    }

    add_global_declarations(global_scope);

    bool check_success = typecheck_global_scope(global_scope);
    if (!check_success) {
        printf("There were errors.\n");
        return;
    }

    String_Builder c_code = generate_c_main_file(global_scope);
    write_entire_file("output.cpp", c_code.string());

    if (system("cmd.exe /c \"cl output.cpp\"") != 0) {
        printf("sif encountered an error when compiling C output. Exiting.\n");
        return;
    }

    if (is_run) {
        system("cmd.exe /c \"output.exe\"");
    }
    else {
        assert(is_build);
    }
}