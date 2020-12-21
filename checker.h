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
    TYPE_PROCEDURE,

    TYPE_COUNT,
};

enum Type_Flags {
    TF_INTEGER    = 1 << 0,
    TF_FLOAT      = 1 << 1,
    TF_SIGNED     = 1 << 2,
    TF_UNSIGNED   = 1 << 3,
    TF_UNTYPED    = 1 << 4,
    TF_NUMBER     = 1 << 5,
    TF_POINTER    = 1 << 6,  // todo(josh): probably don't need this as it's redundant with `TYPE_KIND`
    TF_ARRAY      = 1 << 7,  // todo(josh): probably don't need this as it's redundant with `TYPE_KIND`
    TF_STRUCT     = 1 << 8,  // todo(josh): probably don't need this as it's redundant with `TYPE_KIND`
    TF_INCOMPLETE = 1 << 9,
    TF_PROCEDURE  = 1 << 10, // todo(josh): probably don't need this as it's redundant with `TYPE_KIND`
};

enum Check_State {
    CS_NOT_CHECKED,
    CS_CHECKING,
    CS_CHECKED,
};

struct Type {
    Type_Kind kind = {};
    int size = {};
    u64 flags = {};
    Check_State check_state = {};
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
    int offset = 0;
};
struct Type_Struct : public Type {
    char *name = nullptr;
    Array<Struct_Field> fields = {};
    Ast_Struct *ast_struct = {};

    Type_Struct(Ast_Struct *structure)
    : Type(TYPE_STRUCT)
    , name(structure->name)
    , ast_struct(structure)
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

struct Type_Procedure : public Type {
    Array<Type *> parameter_types = {};
    Type *return_type = {};
    Type_Procedure(Array<Type *> parameter_types, Type *return_type)
    : Type(TYPE_PROCEDURE)
    , parameter_types(parameter_types)
    , return_type(return_type)
    {}
};

void init_checker();
void add_global_declarations(Ast_Block *block);
void typecheck_global_scope(Ast_Block *block);
