#pragma once

#include "common.h"

struct Location {
    const char *filepath = nullptr;
    int line = 0;
    int character = 0;
};

struct Lexer {
    char *text = nullptr;
    int index = 0;
    Location location = {};
    bool errored = false;
    bool allow_compound_literals = true;
    Lexer(const char *filepath, char *text)
    : text(text)
    {
        location.filepath = filepath;
        location.line = 1;
        location.character = 1;
    }
};

enum Token_Kind {
    TK_INVALID,
    TK_IDENTIFIER,
    TK_NUMBER,
    TK_STRING,

    TK_VAR,
    TK_CONST,
    TK_PROC,
    TK_STRUCT,
    TK_ENUM,
    TK_RETURN,
    TK_NULL,
    TK_TRUE,
    TK_FALSE,
    TK_CAST,

    TK_IF,
    TK_ELSE,
    TK_FOR,
    TK_WHILE,
    TK_BREAK,
    TK_CONTINUE,

    TK_SIZEOF,
    TK_TYPEOF,

    TK_DIRECTIVE_PRINT,
    TK_DIRECTIVE_ASSERT,
    TK_DIRECTIVE_FOREIGN,
    TK_DIRECTIVE_C_CODE,
    TK_DIRECTIVE_INCLUDE,

    TK_ASSIGN,

    TK_PLUS,
    TK_PLUS_ASSIGN,
    TK_MINUS,
    TK_MINUS_ASSIGN,
    TK_MULTIPLY,
    TK_MULTIPLY_ASSIGN,
    TK_DIVIDE,
    TK_DIVIDE_ASSIGN,
    TK_LEFT_SHIFT,
    TK_LEFT_SHIFT_ASSIGN,
    TK_RIGHT_SHIFT,
    TK_RIGHT_SHIFT_ASSIGN,

    TK_AMPERSAND, // bitwise AND and address-of

    TK_BIT_AND_ASSIGN,
    TK_BIT_OR,
    TK_BIT_OR_ASSIGN,

    // todo(josh): bitwise XOR
    // todo(josh): bitwise NOR

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
    TK_COMMA,
    TK_CARET,

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
};

void init_lexer_globals();

bool get_next_token(Lexer *lexer, Token *out_token);
void eat_next_token(Lexer *lexer);
bool peek_next_token(Lexer *lexer, Token *out_token);
void unexpected_token(Lexer *lexer, Token token, Token_Kind expected = TK_INVALID);
bool expect_token(Lexer *lexer, Token_Kind kind, Token *out_token = nullptr);
void print_token(Token token);
char *token_string(Token_Kind kind);

void report_error(Location location, const char *fmt, ...);
void report_info(Location location, const char *fmt, ...);

#define UNIMPLEMENTED(val) assert(false && "Unimplemented case: " #val "\n");

