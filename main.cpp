#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>

#include "common.h"
#include "lexer.h"
#include "parser.h"
#include "checker.h"

bool g_reported_error = false;

void print_block_contents(Ast_Block *block, int indent = 0) {
    For (idx, block->nodes) {
        for (int i = 0; i < indent; i++) {
            printf(" ");
        }
        indent += 4;
        Ast_Node *node = block->nodes[idx];
        switch (node->ast_kind) {
            case AST_VAR: {
                Ast_Var *thing = (Ast_Var *)node;
                printf("var %s\n", thing->name);
                break;
            }

            case AST_PROC: {
                Ast_Proc *thing = (Ast_Proc *)node;
                printf("proc %s\n", thing->name);
                For (param_idx, thing->parameters) {
                    printf("    param %s\n", thing->parameters[param_idx]->name);
                }
                print_block_contents(thing->body, indent);
                break;
            }

            case AST_STRUCT: {
                Ast_Struct *thing = (Ast_Struct *)node;
                printf("struct %s\n", thing->name);
                For (param_idx, thing->fields) {
                    printf("    field %s\n", thing->fields[param_idx]->name);
                }
                break;
            }

            case AST_BLOCK: {
                indent += 4;
                print_block_contents((Ast_Block *)node, indent);
                break;
            }
        }
        indent -= 4;
    }
}

void main(int argc, char **argv) {
    if (argc <= 1) {
        printf("Usage:\n  sif <file>\n");
        return;
    }

    char *root_file = argv[1];
    int len = 0;
    char *root_file_text = read_entire_file(root_file, &len);
    defer(free(root_file_text));

    init_lexer_globals();
    init_parser();
    init_checker();

    Lexer lexer(root_file, root_file_text);

    Ast_Block *global_scope = parse_block(&lexer);
    if (g_reported_error) {
        return;
    }
    // print_block_contents(global_scope);

    add_global_declarations(global_scope);
    resolve_identifiers();
    if (g_reported_error) {
        return;
    }

    make_incomplete_types_for_all_structs();
    typecheck_block(global_scope);
    if (g_reported_error) {
        return;
    }
}