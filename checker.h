#pragma once

#include <cstdlib>

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
    Type_Primitive(int size)
    : Type(TYPE_PRIMITIVE)
    {
        this->size = size;
    }
};

struct Struct_Field {
    char *name;
    Type *type;
};
struct Type_Struct : public Type {
    Array<Struct_Field> fields;
    Type_Struct(Array<Struct_Field> fields)
    : Type(TYPE_STRUCT)
    , fields(fields)
    {}
};

struct Type_Pointer : public Type {
    Type *pointer_to;
    Type_Pointer(Type *pointer_to)
    : Type(TYPE_POINTER)
    , pointer_to(pointer_to)
    {}
};

struct Type_Array : public Type {
    int count;
    Type *array_of;
    Type_Array(int count, Type *array_of)
    : Type(TYPE_ARRAY)
    , count(count)
    , array_of(array_of)
    {}
};

enum Operand_Flags {
    OPERAND_CONSTANT = 1 << 0,
    OPERAND_TYPE     = 1 << 1,
};

struct Operand {
    u64 flags = {};
    Type *type = {};

    i64 int_value      = {};
    f64 float_value    = {};
    char *string_value = {};
    bool bool_value    = {};
    Type *type_value   = {};
};

void init_checker();
void add_global_declarations(Ast_Block *block);
void typecheck_block(Ast_Block *block);
