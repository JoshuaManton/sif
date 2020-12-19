#pragma once

#include <cstdlib>

#include "common.h"
#include "parser.h"

enum Type_Kind {
    TYPE_INVALID,
    TYPE_PRIMITIVE,
    TYPE_STRUCT,
    TYPE_POINTER,
    TYPE_ARRAY,

    TYPE_COUNT,
};

enum Type_Flags {
    TF_INTEGER  = 1 << 0,
    TF_FLOAT    = 1 << 1,
    TF_SIGNED   = 1 << 2,
    TF_UNSIGNED = 1 << 3,
    TF_UNTYPED  = 1 << 4,
    TF_NUMBER   = 1 << 5,
    TF_POINTER  = 1 << 6,
    TF_ARRAY    = 1 << 7,
};

struct Type {
    Type_Kind kind = {};
    int size = {};
    u64 flags = {};
    Type(Type_Kind kind)
    : kind(kind)
    {}
};

struct Type_Primitive : public Type {
    char *name = nullptr;
    Type_Primitive(char *name, int size)
    : Type(TYPE_PRIMITIVE)
    , name(name)
    {
        this->size = size;
    }
};

struct Struct_Field {
    char *name = nullptr;
    Type *type = nullptr;
};
struct Type_Struct : public Type {
    char *name = nullptr;
    Array<Struct_Field> fields = {};
    Type_Struct(char *name, Array<Struct_Field> fields)
    : Type(TYPE_STRUCT)
    , name(name)
    , fields(fields)
    {}
};

struct Type_Pointer : public Type {
    Type *pointer_to = {};
    Type_Pointer(Type *pointer_to)
    : Type(TYPE_POINTER)
    , pointer_to(pointer_to)
    {}
};

struct Type_Array : public Type {
    Type *array_of = {};
    int count = {};
    Type_Array(Type *array_of, int count)
    : Type(TYPE_ARRAY)
    , array_of(array_of)
    , count(count)
    {}
};

enum Operand_Flags {
    OPERAND_CONSTANT = 1 << 0,
    OPERAND_TYPE     = 1 << 1,
    OPERAND_LVALUE   = 1 << 2,
    OPERAND_RVALUE   = 1 << 3,
};

struct Operand {
    Location location = {};

    u64 flags = {};
    Type *type = {};

    i64 int_value      = {};
    f64 float_value    = {};
    char *string_value = {};
    bool bool_value    = {};
    Type *type_value   = {};

    Operand(Location location)
    : location(location)
    {}
};

void init_checker();
void add_global_declarations(Ast_Block *block);
void typecheck_block(Ast_Block *block);
