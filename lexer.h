#pragma once

struct Lexer {
    char *text = nullptr;
    char *filepath = nullptr;
    int index = 0;
    int line = 1;
    int character = 1;
    bool reported_error = false;
};

enum Token_Kind {
    TK_INVALID,
    TK_IDENTIFIER,
    TK_NUMBER,
    TK_STRING,

    TK_VAR,
    TK_PROC,
    TK_STRUCT,
    TK_RETURN,
    TK_NULL,
    TK_TRUE,
    TK_FALSE,

    TK_ASSIGN,

    TK_PLUS,
    TK_PLUS_EQUALS,
    TK_MINUS,
    TK_MINUS_EQUALS,
    TK_MULTIPLY,
    TK_MULTIPLY_EQUALS,
    TK_DIVIDE,
    TK_DIVIDE_EQUALS,
    TK_LEFT_SHIFT,
    TK_LEFT_SHIFT_EQUALS,
    TK_RIGHT_SHIFT,
    TK_RIGHT_SHIFT_EQUALS,

    TK_BIT_AND,
    TK_BIT_AND_EQUALS,
    TK_BIT_OR,
    TK_BIT_OR_EQUALS,

    TK_NOT,

    TK_EQUAL_TO,
    TK_NOT_EQUAL_TO,
    TK_LESS_THAN,
    TK_LESS_THAN_OR_EQUAL,
    TK_GREATER_THAN,
    TK_GREATER_THAN_OR_EQUAL,

    TK_BOOLEAN_AND,
    TK_BOOLEAN_AND_EQUALS,
    TK_BOOLEAN_OR,
    TK_BOOLEAN_OR_EQUALS,

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
    TK_AMPERSAND,
    TK_CARET,

    TK_COMMENT,

    TK_COUNT,
};

struct Location {
    char *filepath = nullptr;
    int line = 0;
    int character = 0;
};

struct Token {
    char *text = nullptr;
    Token_Kind kind = TK_INVALID;
    Location location = {};
};

void init_lexer_globals();

bool get_next_token(Lexer *lexer, Token *out_token);
bool peek_next_token(Lexer *lexer, Token *out_token);
void unexpected_token(Lexer *lexer, Token token, Token_Kind expected = TK_INVALID);
bool expect_token(Lexer *lexer, Token_Kind kind, Token *out_token = nullptr);
void print_token(Token token);
char *token_string(Token_Kind kind);