#pragma once

#include "parser.h"
#include "checker.h"

Chunked_String_Builder generate_c_main_file(Ast_Block *global_scope);