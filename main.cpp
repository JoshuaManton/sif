#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>

#include "lexer.h"
#include "parser.h"

void main() {
    init_lexer();

    char *text =
        "var x: int = 123;"
        "var y: int = !x;"
        // "var z: int = 33 - 2;"
        ;
    Lexer lexer;
    lexer.text = text;

    Ast_Block *block = parse_block(&lexer);
}