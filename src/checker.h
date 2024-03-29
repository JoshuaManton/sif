#pragma once

#include <cstdlib>

#include "os_windows.h"
#include "common.h"
#include "parser.h"

#include "basic.h"

enum Type_Kind {
    TYPE_INVALID,
    TYPE_PRIMITIVE,
    TYPE_TUPLE,
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
    TF_POINTER      = 1 << 6,
    TF_POLYMORPHIC  = 1 << 7,
    TF_STRING       = 1 << 8,
};

enum Type_Completion_Flags {
    TCF_SIZE                    = 1 << 0,
    TCF_USINGS                  = 1 << 1,
    TCF_GENERATE_FIELDS         = 1 << 2, // note(josh): this one is done automatically, no need to pass it to complete_type()
    TCF_ADD_ORDERED_DECLARATION = 1 << 3, // note(josh): this one is done automatically, no need to pass it to complete_type()
    TCF_ALL                     = TCF_SIZE | TCF_USINGS | TCF_GENERATE_FIELDS | TCF_ADD_ORDERED_DECLARATION,
};

struct Type_Pointer;
struct Type_Slice;
struct Type_Varargs;
struct Type_Reference;

struct Struct_Field {
    const char *name = {};
    Operand operand = {};
    int offset = {}; // -1 if is_constant
    Array<char *> notes = {};
};

struct Type {
    i64 id = {};
    Type_Kind kind = {};
    int size = {};
    int align = {};
    u64 flags = {};
    u64 completed_flags = {};            // Type_Completion_Flags
    u64 currently_completing_flags = {}; // Type_Completion_Flags
    Array<Struct_Field> all_fields = {};
    Array<Struct_Field> constant_fields = {};
    Array<Struct_Field> variable_fields = {};
    Ast_Block *declarations_block = {};
    char *printable_name = {};
    int printable_name_length = {}; // note(josh): doesn't include null term
    Type_Pointer *pointer_to_this_type = {};
    Type_Slice *slice_of_this_type = {};
    Type_Varargs *varargs_of_this_type = {};
    Type_Varargs *c_varargs_of_this_type = {};
    Type_Reference *reference_to_this_type = {};
    Type(Type_Kind kind)
    : kind(kind)
    {
        all_fields.allocator = g_global_linear_allocator;
        constant_fields.allocator = g_global_linear_allocator;
        variable_fields.allocator = g_global_linear_allocator;
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

struct Type_Tuple : public Type {
    Array<Type *>types = {};
    Type_Tuple(Array<Type *>types)
    : Type(TYPE_TUPLE)
    , types(types)
    {}
};

struct Type_Struct : public Type {
    char *name = nullptr;
    Ast_Struct *ast_struct = {};
    Ast_Struct *is_polymorph_of = {};
    Array<Operand> polymorphic_parameter_values = {};
    bool is_union = {};
    Array<char *> notes = {};
    Type_Struct(Ast_Struct *structure)
    : Type(TYPE_STRUCT)
    , name(structure->name)
    , ast_struct(structure)
    , notes(structure->declaration->notes)
    {
        polymorphic_parameter_values.allocator = g_global_linear_allocator;
    }
};

struct Type_Enum : public Type {
    const char *name = nullptr;
    Type *base_type = {};
    Array<char *> notes = {};
    Type_Enum(const char *name, Array<char *> notes)
    : Type(TYPE_ENUM)
    , name(name)
    , notes(notes)
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
    bool is_c_varargs = {};
    Type_Varargs(Type *varargs_of, Type_Pointer *data_pointer_type, Type_Slice *slice_type, bool is_c_varargs)
    : Type(TYPE_VARARGS)
    , varargs_of(varargs_of)
    , slice_type(slice_type)
    , data_pointer_type(data_pointer_type)
    , is_c_varargs(is_c_varargs)
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
extern Type *type_cstring;
extern Type *type_string;
extern Type *type_rawptr;
extern Type *type_byte;
extern Type *type_int;
extern Type *type_uint;
extern Type *type_float;
extern Type *type_any;

extern Array<Type *> all_types;
extern Array<Type *> g_completed_types;

extern Ast_Proc *g_main_proc;

extern Spinlock g_ordered_declarations_spinlock;
extern Spinlock g_completed_types_spinlock;

extern bool g_done_typechecking;
extern bool g_done_typechecking_sif_runtime_stuff;

extern Declaration *sif_runtime_bounds_check_proc;
extern Declaration *sif_runtime_null_check_proc;
extern Declaration *sif_runtime_string_eq_proc;
extern Declaration *sif_runtime_zero_pointer_proc;
extern Declaration *sif_runtime_source_code_location;
extern Declaration *sif_runtime_make_source_code_location;
extern Struct_Declaration *sif_runtime_union_all_type_infos;
extern Struct_Declaration *sif_runtime_type_info;
extern Struct_Declaration *sif_runtime_type_info_integer;
extern Struct_Declaration *sif_runtime_type_info_float;
extern Struct_Declaration *sif_runtime_type_info_bool;
extern Struct_Declaration *sif_runtime_type_info_string;
extern Struct_Declaration *sif_runtime_type_info_struct_field;
extern Struct_Declaration *sif_runtime_type_info_struct;
extern Struct_Declaration *sif_runtime_type_info_union;
extern Struct_Declaration *sif_runtime_type_info_enum_field;
extern Struct_Declaration *sif_runtime_type_info_enum;
extern Struct_Declaration *sif_runtime_type_info_pointer;
extern Struct_Declaration *sif_runtime_type_info_slice;
extern Struct_Declaration *sif_runtime_type_info_array;
extern Struct_Declaration *sif_runtime_type_info_reference;
extern Struct_Declaration *sif_runtime_type_info_procedure;
extern Struct_Declaration *sif_runtime_type_info_typeid;

void init_checker();
void add_global_declarations(Ast_Block *block);
bool typecheck_global_scope(Ast_Block *block);

bool is_type_pointer    (Type *type);
bool is_type_procedure  (Type *type);
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

char *type_to_string(Type *type, int *out_length = nullptr);
Type_Pointer *get_or_create_type_pointer_to(Type *type);
Type_Reference *get_or_create_type_reference_to(Type *type);
Type_Array *get_or_create_type_array_of(Type *type, int count);
Type_Slice *get_or_create_type_slice_of(Type *type);
Type_Varargs *get_or_create_type_varargs_of(Type *type, bool is_c_varargs);
bool complete_type(Type *type);
Ast_Expr *unparen_expr(Ast_Expr *expr);
