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
-bounds checks
-right now operator overloading requires the first parameter to be the struct that you are overloading for. this is not ideal because you want to be able to do float * vector
-reference-to-reference parsing bug: var x: >>int parses as a shift-right
-assigning to reference-to-reference doesn't work. I'm not sure what the behaviour should be
-enum field expressions
-foreach loops
-slicing
-switch statements
-function pointers
-unions
-tagged unions
-defer
-polymorphism
-figure out if I should allow shadowing (maybe with a keyword?)
-generate C instead of C++. I think the only thing from C++ I'm using is for the templated Static_Array for array-by-value semantics
*/

void main(int argc, char **argv) {
    if (argc <= 1) {
        printf("Usage:\n  sif <file>\n");
        return;
    }

    init_lexer_globals();
    init_parser();
    init_checker();

    Ast_Block *global_scope = begin_parsing(argv[1]);
    if (!global_scope) {
        printf("There were errors.\n");
        return;
    }

    add_global_declarations(global_scope);
    bool resolve_identifiers_success = resolve_identifiers();
    if (!resolve_identifiers_success) {
        printf("There were errors.\n");
        return;
    }

    bool check_success = typecheck_global_scope(global_scope);
    if (!check_success) {
        printf("There were errors.\n");
        return;
    }

    String_Builder c_code = generate_c_main_file(global_scope);
    write_entire_file("output.cpp", c_code.string());
}