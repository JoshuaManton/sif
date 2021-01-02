#pragma once

#include <cstdlib>

#include "basic.h"
#include "common.h"
#include "parser.h"

enum Type_Kind {
    TYPE_INVALID,
    TYPE_PRIMITIVE,
    TYPE_STRUCT,
    TYPE_ENUM,
    TYPE_POINTER,
    TYPE_REFERENCE,
    TYPE_ARRAY,
    TYPE_POLYMORPHIC,
    TYPE_SLICE,
    TYPE_VARARGS,
    TYPE_PROCEDURE,

    TYPE_COUNT,
};

enum Type_Flags {
    TF_INTEGER      = 1 << 0,
    TF_FLOAT        = 1 << 1,
    TF_SIGNED       = 1 << 2,
    TF_UNSIGNED     = 1 << 3,
    TF_UNTYPED      = 1 << 4,
    TF_NUMBER       = 1 << 5,
    TF_POINTER      = 1 << 6,  // todo(josh): probably don't need this as it's redundant with `TYPE_KIND`
    TF_ARRAY        = 1 << 7,  // todo(josh): probably don't need this as it's redundant with `TYPE_KIND`
    TF_STRUCT       = 1 << 8,  // todo(josh): probably don't need this as it's redundant with `TYPE_KIND`
    TF_INCOMPLETE   = 1 << 9,
    TF_PROCEDURE    = 1 << 10, // todo(josh): probably don't need this as it's redundant with `TYPE_KIND`
    TF_SLICE        = 1 << 11, // todo(josh): probably don't need this as it's redundant with `TYPE_KIND`
    TF_REFERENCE    = 1 << 12, // todo(josh): probably don't need this as it's redundant with `TYPE_KIND`
    TF_POLYMORPHIC  = 1 << 13, // todo(josh): probably don't need this as it's redundant with `TYPE_KIND`
    TF_VARARGS      = 1 << 14, // todo(josh): probably don't need this as it's redundant with `TYPE_KIND`
    TF_ANY          = 1 << 15, // todo(josh): probably don't need this as it's redundant with `TYPE_KIND`
    TF_ENUM         = 1 << 16, // todo(josh): probably don't need this as it's redundant with `TYPE_KIND`
};

enum Check_State {
    CS_NOT_CHECKED,
    CS_CHECKING,
    CS_CHECKED,
};

struct Struct_Field {
    const char *name = {};
    Operand operand = {};
    int offset = {}; // -1 if is_constant
};
struct Type {
    i64 id = {};
    Type_Kind kind = {};
    int size = {};
    int align = {};
    u64 flags = {};
    Check_State check_state = {};
    Array<Struct_Field> fields = {};
    int num_variable_fields = {};
    int num_constant_fields = {};
    Type(Type_Kind kind)
    : kind(kind)
    {
        fields.allocator = default_allocator();
    }
};

struct Type_Primitive : public Type {
    char *name = nullptr;
    Type_Primitive(char *name, int size, int align)
    : Type(TYPE_PRIMITIVE)
    , name(name)
    {
        this->size = size;
        this->align = align;
    }
};

struct Type_Struct : public Type {
    char *name = nullptr;
    Ast_Struct *ast_struct = {};
    Ast_Struct *is_polymorph_of = {};
    Array<Operand> polymorphic_parameter_values = {};
    Type_Struct(Ast_Struct *structure)
    : Type(TYPE_STRUCT)
    , name(structure->name)
    , ast_struct(structure)
    {
        polymorphic_parameter_values.allocator = default_allocator();
    }
};

struct Type_Enum : public Type {
    char *name = nullptr;
    Type_Enum(char *name)
    : Type(TYPE_ENUM)
    , name(name)
    {
        flags |= TF_INTEGER;
    }
};

struct Type_Pointer : public Type {
    Type *pointer_to = {};
    Type_Pointer(Type *pointer_to)
    : Type(TYPE_POINTER)
    , pointer_to(pointer_to)
    {}
};

struct Type_Reference : public Type {
    Type *reference_to = {};
    Type_Reference(Type *reference_to)
    : Type(TYPE_REFERENCE)
    , reference_to(reference_to)
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

struct Type_Slice : public Type {
    Type *slice_of = {};
    Type *data_pointer_type = {};
    Type_Slice(Type *slice_of, Type *data_pointer_type)
    : Type(TYPE_SLICE)
    , slice_of(slice_of)
    , data_pointer_type(data_pointer_type)
    {}
};

struct Type_Varargs : public Type {
    Type *varargs_of = {};
    Type_Pointer *data_pointer_type = {};
    Type_Slice *slice_type = {};
    Type_Varargs(Type *varargs_of, Type_Pointer *data_pointer_type, Type_Slice *slice_type)
    : Type(TYPE_VARARGS)
    , varargs_of(varargs_of)
    , slice_type(slice_type)
    , data_pointer_type(data_pointer_type)
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

extern Array<Declaration *> ordered_declarations;
extern Array<Type *> all_types;
extern Type *type_i8;
extern Type *type_i16;
extern Type *type_i32;
extern Type *type_i64;
extern Type *type_u8;
extern Type *type_u16;
extern Type *type_u32;
extern Type *type_u64;
extern Type *type_f32;
extern Type *type_f64;
extern Type *type_bool;
extern Type *type_untyped_number;
extern Type *type_untyped_null;
extern Type *type_typeid;
extern Type *type_string;
extern Type *type_any;

void init_checker();
void add_global_declarations(Ast_Block *block);
bool typecheck_global_scope(Ast_Block *block);

bool is_type_pointer    (Type *type);
bool is_type_polymorphic(Type *type);
bool is_type_reference  (Type *type);
bool is_type_array      (Type *type);
bool is_type_slice      (Type *type);
bool is_type_number     (Type *type);
bool is_type_integer    (Type *type);
bool is_type_float      (Type *type);
bool is_type_bool       (Type *type);
bool is_type_untyped    (Type *type);
bool is_type_unsigned   (Type *type);
bool is_type_signed     (Type *type);
bool is_type_struct     (Type *type);
bool is_type_incomplete (Type *type);
bool is_type_typeid     (Type *type);
bool is_type_string     (Type *type);
bool is_type_varargs    (Type *type);
bool is_type_enum       (Type *type);

Type_Pointer *get_or_create_type_pointer_to(Type *type);
Type_Reference *get_or_create_type_reference_to(Type *type);
Type_Array *get_or_create_type_array_of(Type *type, int count);
Type_Slice *get_or_create_type_slice_of(Type *type);
Type_Varargs *get_or_create_type_varargs_of(Type *type);
bool complete_type(Type *type);
Ast_Expr *unparen_expr(Ast_Expr *expr);
