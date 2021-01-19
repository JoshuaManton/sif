#pragma once

#include "basic.h"

struct Location {
    const char *filepath = nullptr;
    char *text = nullptr;
    int index = 0;
    int line = 0;
    int character = 0;
};

enum Token_Kind {
    TK_INVALID,
    TK_IDENTIFIER,
    TK_NUMBER,
    TK_STRING,
    TK_CHAR,

    TK_CONST,
    TK_PROC,
    TK_OPERATOR,
    TK_STRUCT,
    TK_UNION,
    TK_ENUM,
    TK_RETURN,
    TK_NULL,
    TK_TRUE,
    TK_FALSE,
    TK_CAST,
    TK_TRANSMUTE,
    TK_USING,
    TK_DEFER,

    TK_NOTE,

    TK_IF,
    TK_ELSE,
    TK_FOR,
    TK_WHILE,
    TK_BREAK,
    TK_CONTINUE,

    TK_SIZEOF,
    TK_TYPEOF,
    TK_TYPEOFELEMENT,

    TK_DIRECTIVE_PRINT,
    TK_DIRECTIVE_ASSERT,
    TK_DIRECTIVE_FOREIGN,
    TK_DIRECTIVE_INCLUDE,
    TK_DIRECTIVE_FOREIGN_IMPORT,
    TK_DIRECTIVE_FOREIGN_SYSTEM_IMPORT,
    TK_DIRECTIVE_PARTIAL,
    TK_DIRECTIVE_C_VARARGS,
    TK_DIRECTIVE_CALLER_LOCATION,

    TK_ASSIGN,

    TK_PLUS,
    TK_PLUS_ASSIGN,
    TK_MINUS,
    TK_MINUS_ASSIGN,
    TK_MULTIPLY,
    TK_MULTIPLY_ASSIGN,
    TK_DIVIDE,
    TK_DIVIDE_ASSIGN,
    TK_MOD,
    TK_MOD_ASSIGN,
    TK_LEFT_SHIFT,
    TK_LEFT_SHIFT_ASSIGN,
    TK_RIGHT_SHIFT,
    TK_RIGHT_SHIFT_ASSIGN,

    TK_DOLLAR,

    TK_AMPERSAND, // bitwise AND and address-of

    TK_BIT_AND_ASSIGN,
    TK_BIT_OR,
    TK_BIT_OR_ASSIGN,
    TK_BIT_NOT,

    // todo(josh): bitwise XOR

    TK_NOT,

    TK_EQUAL_TO,
    TK_NOT_EQUAL_TO,
    TK_LESS_THAN,
    TK_LESS_THAN_OR_EQUAL,
    TK_GREATER_THAN,
    TK_GREATER_THAN_OR_EQUAL,

    TK_BOOLEAN_AND,
    TK_BOOLEAN_AND_ASSIGN,
    TK_BOOLEAN_OR,
    TK_BOOLEAN_OR_ASSIGN,

    TK_LEFT_CURLY,
    TK_RIGHT_CURLY,
    TK_LEFT_SQUARE,
    TK_RIGHT_SQUARE,
    TK_LEFT_PAREN,
    TK_RIGHT_PAREN,

    TK_SEMICOLON,
    TK_COLON,
    TK_DOT,
    TK_DOT_DOT,
    TK_COMMA,
    TK_CARET,
    TK_COMPOUND_LITERAL, // note(josh): '.{''

    TK_COMMENT,

    TK_COUNT,
};

struct Token {
    char *text = nullptr;
    char *escaped_text = nullptr;
    Token_Kind kind = TK_INVALID;
    Location location = {};
    bool has_a_dot = false;
    int escaped_length = {};
    int scanner_length = {};
    i64 int_value = {};
    u64 uint_value = {};
    f64 float_value = {};
};

struct Ast_Block;
struct Ast_Proc_Header;

struct Declaration;

struct Lexer {
    char *text = nullptr;
    Location location = {};
    bool errored = false;
    bool has_peeked_token = {};
    Location peeked_location = {};
    Token peeked_token = {};
    Ast_Block *current_block = {};
    Declaration *current_toplevel_declaration = {};
    Ast_Proc_Header *currently_parsing_proc_body = {};
    Allocator allocator = {};
    int num_polymorphic_variables_parsed = 0;
    Lexer(const char *filepath, char *text)
    : text(text)
    {
        location.filepath = filepath;
        location.line = 1;
        location.character = 1;
    }

    Lexer(const char *filepath)
    {
        location.filepath = filepath;
        location.line = 1;
        location.character = 1;
    }
};

void init_lexer_globals();

bool get_next_token(Lexer *lexer, Token *out_token);
void eat_next_token(Lexer *lexer, Token *out_token = nullptr);
bool peek_next_token(Lexer *lexer, Token *out_token);
void unexpected_token(Lexer *lexer, Token token, Token_Kind expected = TK_INVALID);
bool expect_token(Lexer *lexer, Token_Kind kind, Token *out_token = nullptr);
void print_token(Token token);
char *token_string(Token_Kind kind);
char *token_name(Token_Kind kind);

void report_error(Location location, const char *fmt, ...);
void report_info(Location location, const char *fmt, ...);

#define UNIMPLEMENTED(val) assert(false && "Unimplemented case: " #val "\n");

