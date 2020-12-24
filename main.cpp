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
-enum field expressions
-foreach loops
-switch statements
-function pointers
-unions
-tagged unions
-operator overloading
-polymorphism
-figure out if I should allow shadowing (maybe with a keyword?)
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