#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cassert>

#include "lexer.h"

static char *token_string_map[TK_COUNT];

void init_lexer() {
    token_string_map[TK_INVALID]               = "INVALID";
    token_string_map[TK_IDENTIFIER]            = "IDENTIFIER";
    token_string_map[TK_NUMBER]                = "NUMBER";
    token_string_map[TK_STRING]                = "STRING";

    token_string_map[TK_VAR]                   = "VAR";
    token_string_map[TK_PROC]                  = "PROC";
    token_string_map[TK_STRUCT]                = "STRUCT";
    token_string_map[TK_RETURN]                = "RETURN";
    token_string_map[TK_NULL]                  = "NULL";
    token_string_map[TK_TRUE]                  = "TRUE";
    token_string_map[TK_FALSE]                 = "FALSE";

    token_string_map[TK_ASSIGN]                = "=";
    token_string_map[TK_PLUS]                  = "+";
    token_string_map[TK_PLUS_EQUALS]           = "+=";
    token_string_map[TK_MINUS]                 = "-";
    token_string_map[TK_MINUS_EQUALS]          = "-=";
    token_string_map[TK_MULTIPLY]              = "*";
    token_string_map[TK_MULTIPLY_EQUALS]       = "*=";
    token_string_map[TK_DIVIDE]                = "/";
    token_string_map[TK_DIVIDE_EQUALS]         = "/=";
    token_string_map[TK_LEFT_SHIFT]            = "<<";
    token_string_map[TK_LEFT_SHIFT_EQUALS]     = "<<=";
    token_string_map[TK_RIGHT_SHIFT]           = ">>";
    token_string_map[TK_RIGHT_SHIFT_EQUALS]    = ">>=";
    token_string_map[TK_BIT_AND]               = "&";
    token_string_map[TK_BIT_AND_EQUALS]        = "&=";
    token_string_map[TK_BIT_OR]                = "|";
    token_string_map[TK_BIT_OR_EQUALS]         = "|=";

    token_string_map[TK_NOT]                   = "!";
    token_string_map[TK_NOT_EQUAL_TO]          = "!=";
    token_string_map[TK_LESS_THAN]             = "<";
    token_string_map[TK_LESS_THAN_OR_EQUAL]    = "<=";
    token_string_map[TK_GREATER_THAN]          = ">";
    token_string_map[TK_GREATER_THAN_OR_EQUAL] = ">=";
    token_string_map[TK_EQUAL_TO]              = "==";
    token_string_map[TK_BOOLEAN_AND]           = "&&";
    token_string_map[TK_BOOLEAN_AND_EQUALS]    = "&&=";
    token_string_map[TK_BOOLEAN_OR]            = "||";
    token_string_map[TK_BOOLEAN_OR_EQUALS]     = "||=";

    token_string_map[TK_LEFT_CURLY]            = "{";
    token_string_map[TK_RIGHT_CURLY]           = "}";
    token_string_map[TK_LEFT_SQUARE]           = "[";
    token_string_map[TK_RIGHT_SQUARE]          = "]";
    token_string_map[TK_LEFT_PAREN]            = "(";
    token_string_map[TK_RIGHT_PAREN]           = ")";
    token_string_map[TK_SEMICOLON]             = ";";
    token_string_map[TK_COLON]                 = ":";
    token_string_map[TK_DOT]                   = ".";
    token_string_map[TK_COMMA]                 = ",";
    token_string_map[TK_AMPERSAND]             = "&";
    token_string_map[TK_CARET]                 = "^";

    token_string_map[TK_COUNT]                 = "COUNT";

    for (int i = 0; i < TK_COUNT; i++) {
        if (token_string_map[i] == nullptr) {
            printf("No token string for token: %d\n", i);
            assert(false);
        }
    }
}

void print_token(Token token) {
    printf("%s %s\n", token_string_map[token.kind], token.text);
}

bool is_letter_or_underscore(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

bool is_whitespace(char c) {
    return (c == ' ') || (c == '\n') || (c == '\t') || (c == '\r') || (c == '\v'); // todo(josh): what other whitespace chars are there?
}

bool is_digit(char c) {
    return (c >= '0') && (c <= '9');
}

bool is_one_to_nine(char c) {
    return (c >= '1') && (c <= '9');
}

char *clone_string(char *string, int length) {
    char *new_string = (char *)malloc(length+1);
    memcpy((void *)new_string, (void *)string, length);
    new_string[length] = 0;
    return new_string;
}

char *scan_identifier(char *text, int *out_length) {
    char *start = text;
    while (is_letter_or_underscore(*text)) {
        text += 1;
    }
    int length = text - start;
    *out_length = length;
    char *result = clone_string(start, length);
    return result;
}

// note(josh): returns the string without quotes, out_length includes the quotes though since the lexer needs to know how much to advance by
char *scan_string(char *text, int *out_length) {
    assert(*text == '"');
    char *start = text;
    text += 1;
    bool escaped = false;
    while ((*text != 0) && (*text != '"' || escaped)) {
        escaped = false;
        if (*text == '\\') {
            escaped = true;
        }
        text += 1;
    }
    assert(*text == '"'); // todo(josh): check for EOF
    text += 1;
    int length = text - start;
    *out_length = length;
    char *result = clone_string(start+1, length-2);
    return result;
}

char *scan_number(char *text, int *out_length) {
    char *start = text;

    if (*text == '0') {
        text += 1;
    }
    else if (is_one_to_nine(*text)) {
        while (is_one_to_nine(*text)) {
            text += 1;
        }
    }
    else {
        assert(false && "scan_number called without being at a number");
    }

    if (*text == '.') {
        text += 1;
        while (is_digit(*text)) {
            text += 1;
        }
    }

    int length = text - start;
    *out_length = length;
    char *result = clone_string(start, length);
    return result;
}

bool get_next_token(Lexer *lexer, Token *out_token) {
    while (is_whitespace(lexer->text[lexer->index])) {
        lexer->index += 1;
    }

    if (lexer->text[lexer->index] == 0) {
        return false;
    }

    #define SIMPLE_TOKEN(c, t) if (lexer->text[lexer->index] == c) { \
        lexer->index += 1; \
        out_token->kind = t; \
        out_token->text = token_string_map[t]; \
    }

    if (is_letter_or_underscore(lexer->text[lexer->index])) {
        int length = 0;
        char *identifier = scan_identifier(&lexer->text[lexer->index], &length);
        lexer->index += length;
        out_token->kind = TK_IDENTIFIER;
        out_token->text = identifier;

        #define CHECK_KEYWORD(keyword, token) if (strcmp(identifier, keyword) == 0) { \
            out_token->kind = token; \
        }

        CHECK_KEYWORD("return", TK_RETURN)
        else CHECK_KEYWORD("var", TK_VAR)
        else CHECK_KEYWORD("proc", TK_PROC)
        else CHECK_KEYWORD("struct", TK_STRUCT)
        else CHECK_KEYWORD("null", TK_NULL)
        else CHECK_KEYWORD("true", TK_TRUE)
        else CHECK_KEYWORD("false", TK_FALSE)
    }
    else if (is_digit(lexer->text[lexer->index])) {
        int length = 0;
        char *number = scan_number(&lexer->text[lexer->index], &length);
        lexer->index += length;
        out_token->kind = TK_NUMBER;
        out_token->text = number;
    }
    else if (lexer->text[lexer->index] == '"') {
        int length = 0;
        char *string = scan_string(&lexer->text[lexer->index], &length);
        lexer->index += length;
        out_token->kind = TK_STRING;
        out_token->text = string;
    }
    // todo(josh): make a macro for these operators
    else if (lexer->text[lexer->index] == '+') {
        lexer->index += 1;
        out_token->kind = TK_PLUS;
        out_token->text = "+";
        if (lexer->text[lexer->index] == '=') {
            out_token->kind = TK_PLUS_EQUALS;
            out_token->text = "+=";
        }
    }
    else if (lexer->text[lexer->index] == '-') {
        lexer->index += 1;
        out_token->kind = TK_MINUS;
        out_token->text = "-";
        if (lexer->text[lexer->index] == '=') {
            lexer->index += 1;
            out_token->kind = TK_MINUS_EQUALS;
            out_token->text = "-=";
        }
    }
    else if (lexer->text[lexer->index] == '*') {
        lexer->index += 1;
        out_token->kind = TK_MULTIPLY;
        out_token->text = "*";
        if (lexer->text[lexer->index] == '=') {
            lexer->index += 1;
            out_token->kind = TK_MULTIPLY_EQUALS;
            out_token->text = "*=";
        }
    }
    else if (lexer->text[lexer->index] == '/') {
        lexer->index += 1;
        out_token->kind = TK_DIVIDE;
        out_token->text = "/";
        if (lexer->text[lexer->index] == '=') {
            lexer->index += 1;
            out_token->kind = TK_DIVIDE_EQUALS;
            out_token->text = "/=";
        }
    }
    else if (lexer->text[lexer->index] == '=') {
        lexer->index += 1;
        out_token->kind = TK_ASSIGN;
        out_token->text = "=";
        if (lexer->text[lexer->index] == '=') {
            lexer->index += 1;
            out_token->kind = TK_EQUAL_TO;
            out_token->text = "==";
        }
    }
    else if (lexer->text[lexer->index] == '!') {
        lexer->index += 1;
        out_token->kind = TK_NOT;
        out_token->text = "!";
        if (lexer->text[lexer->index] == '=') {
            lexer->index += 1;
            out_token->kind = TK_NOT_EQUAL_TO;
            out_token->text = "!=";
        }
    }
    else if (lexer->text[lexer->index] == '<') {
        lexer->index += 1;
        out_token->kind = TK_LESS_THAN;
        out_token->text = "<";
        if (lexer->text[lexer->index] == '<') {
            lexer->index += 1;
            out_token->kind = TK_LEFT_SHIFT;
            out_token->text = "<<";
            if (lexer->text[lexer->index] == '=') {
                lexer->index += 1;
                out_token->kind = TK_LEFT_SHIFT_EQUALS;
                out_token->text = "<<=";
            }
        }
        else if (lexer->text[lexer->index] == '=') {
            lexer->index += 1;
            out_token->kind = TK_LESS_THAN_OR_EQUAL;
            out_token->text = "<=";
        }
    }
    else if (lexer->text[lexer->index] == '>') {
        lexer->index += 1;
        out_token->kind = TK_GREATER_THAN;
        out_token->text = ">";
        if (lexer->text[lexer->index] == '>') {
            lexer->index += 1;
            out_token->kind = TK_RIGHT_SHIFT;
            out_token->text = ">>";
            if (lexer->text[lexer->index] == '=') {
                lexer->index += 1;
                out_token->kind = TK_RIGHT_SHIFT_EQUALS;
                out_token->text = ">>=";
            }
        }
        else if (lexer->text[lexer->index] == '=') {
            lexer->index += 1;
            out_token->kind = TK_GREATER_THAN_OR_EQUAL;
            out_token->text = ">=";
        }
    }
    else if (lexer->text[lexer->index] == '&') {
        lexer->index += 1;
        out_token->kind = TK_BIT_AND;
        out_token->text = "&";
        if (lexer->text[lexer->index] == '&') {
            lexer->index += 1;
            out_token->kind = TK_BOOLEAN_AND;
            out_token->text = "&&";
            if (lexer->text[lexer->index] == '=') {
                lexer->index += 1;
                out_token->kind = TK_BOOLEAN_AND_EQUALS;
                out_token->text = "&&=";
            }
        }
        else if (lexer->text[lexer->index] == '=') {
            lexer->index += 1;
            out_token->kind = TK_BIT_AND_EQUALS;
            out_token->text = "&=";
        }
    }
    else if (lexer->text[lexer->index] == '|') {
        lexer->index += 1;
        out_token->kind = TK_BIT_OR;
        out_token->text = "|";
        if (lexer->text[lexer->index] == '|') {
            lexer->index += 1;
            out_token->kind = TK_BOOLEAN_OR;
            out_token->text = "||";
            if (lexer->text[lexer->index] == '=') {
                lexer->index += 1;
                out_token->kind = TK_BOOLEAN_OR_EQUALS;
                out_token->text = "||=";
            }
        }
        else if (lexer->text[lexer->index] == '=') {
            lexer->index += 1;
            out_token->kind = TK_BIT_OR_EQUALS;
            out_token->text = "|=";
        }
    }
    else SIMPLE_TOKEN('{', TK_LEFT_CURLY)
    else SIMPLE_TOKEN('}', TK_RIGHT_CURLY)
    else SIMPLE_TOKEN(';', TK_SEMICOLON)
    else SIMPLE_TOKEN(':', TK_COLON)
    else SIMPLE_TOKEN('.', TK_DOT)
    else SIMPLE_TOKEN(',', TK_COMMA)
    else SIMPLE_TOKEN('&', TK_AMPERSAND)
    else SIMPLE_TOKEN('^', TK_CARET)
    else {
        assert(false);
    }

    return true;
}

bool peek_next_token(Lexer *lexer, Token *out_token) {
    Lexer copy = *lexer;
    bool ok = get_next_token(&copy, out_token);
    return ok;
}

bool expect_token(Lexer *lexer, Token_Kind kind, Token *out_token) {
    Token backup;
    if (out_token == nullptr) {
        out_token = &backup;
    }
    bool ok = get_next_token(lexer, out_token);
    if (!ok) {
        return false;
    }
    return out_token->kind == kind;
}

char *token_string(Token_Kind kind) {
    return token_string_map[kind];
}