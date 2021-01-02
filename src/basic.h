#pragma once

#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <float.h>
#include <string.h>

typedef unsigned char      byte; static_assert(sizeof(byte) == 1, "byte size was not 1");
typedef unsigned char      u8;   static_assert(sizeof(u8)   == 1, "u8 size was not 1");
typedef unsigned short     u16;  static_assert(sizeof(u16)  == 2, "u16 size was not 2");
typedef unsigned int       u32;  static_assert(sizeof(u32)  == 4, "u32 size was not 4");
typedef unsigned long long u64;  static_assert(sizeof(u64)  == 8, "u64 size was not 8");

typedef u32 uint; static_assert(sizeof(uint)  == 4, "uint size was not 4");

typedef char      i8;   static_assert(sizeof(i8)   == 1, "i8 size was not 1");
typedef short     i16;  static_assert(sizeof(i16)  == 2, "i16 size was not 2");
typedef int       i32;  static_assert(sizeof(i32)  == 4, "i32 size was not 4");
typedef long long i64;  static_assert(sizeof(i64)  == 8, "i64 size was not 8");

static_assert(sizeof(int)  == 4, "int size was not 4");

typedef float  f32; static_assert(sizeof(f32) == 4, "f32 size was not 4");
typedef double f64; static_assert(sizeof(f64) == 8, "f64 size was not 8");

static_assert(sizeof(float) == 4, "float size was not 4");

static_assert(sizeof(void *) == 8, "void * size was not 8");

#ifndef ASSERT
#define ASSERT(cond) { if (!(cond)) { printf("<%s:%d> Assertion failed: " #cond "\n", __FILE__, __LINE__); __debugbreak(); } }
#endif

#ifndef ASSERTF
#define ASSERTF(cond, ...) { if (!(cond)) { printf("<%s:%d> Assertion failed: " #cond ". ", __FILE__, __LINE__); printf(__VA_ARGS__); __debugbreak(); } }
#endif

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) \
    ((sizeof(a) / sizeof(*(a))) / \
    static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))
#endif

static void bounds__check(int index, int min, int max_plus_one, char *file, int line) {
    if ((index < min) || (index >= max_plus_one)) {
        printf("<%s:%d> Index %d is out of range %d..<%d\n", file, line, index, min, max_plus_one);
        assert(false);
    }
}

#define BOUNDS_CHECK(index, min, max_plus_one) bounds__check(index, min, max_plus_one, __FILE__, __LINE__)



bool is_power_of_two(uintptr_t n);
uintptr_t align_forward(uintptr_t p, uintptr_t align);
uintptr_t align_backward(uintptr_t p, uintptr_t align);
float min(float a, float b);
float max(float a, float b);
int min(int a, int b);
int max(int a, int b);
int modulo(int a, int b);


byte *buffer_allocate(byte *buffer, int buffer_len, int *offset, int size, int alignment, bool panic_on_oom = true);

#ifndef DEFAULT_ALIGNMENT
#define DEFAULT_ALIGNMENT sizeof(void *) * 2
#endif

struct Allocator {
    void *data;
    void *(*alloc_proc)(void *allocator, int size, int alignment);
    void (*free_proc)(void *allocator, void *ptr);
};

void *alloc(Allocator allocator, int size, int alignment = DEFAULT_ALIGNMENT);
void free(Allocator allocator, void *ptr);
#define NEW(allocator, type) ((type *)alloc(allocator, sizeof(type), alignof(type)))
#define MAKE(allocator, type, count) ((type *)alloc(allocator, sizeof(type) * count, alignof(type)))

void *default_allocator_alloc(void *allocator, int size, int alignment);
void default_allocator_free(void *allocator, void *ptr);
Allocator default_allocator();



Allocator null_allocator();



struct Arena {
    byte *memory;
    int memory_size;
    int cur_offset;
};

void init_arena(Arena *arena, byte *backing, int backing_size);
void *arena_alloc(void *allocator, int size, int align = DEFAULT_ALIGNMENT);
void arena_free(void *allocator, void *ptr);
void arena_clear(Arena *arena);
Allocator arena_allocator(Arena *arena);



struct Pool_Allocator {
    byte *memory;
    int memory_size;
    int slot_size;
    int num_slots;
    int *generations;
    int *slots_freelist;
    int freelist_count;
    Allocator backing_allocator;
};

void  init_pool_allocator(Pool_Allocator *pool, Allocator backing_allocator, int slot_size, int num_slots);
void *pool_get(Pool_Allocator *pool, int *out_generation, int *out_index);
void  pool_return(Pool_Allocator *pool, void *ptr);
int   pool_get_slot_index(Pool_Allocator *pool, void *ptr);
void *pool_get_slot_by_index(Pool_Allocator *pool, int slot);
void *pool_alloc(void *allocator, int size, int align = DEFAULT_ALIGNMENT);
void  pool_free(void *allocator, void *ptr);
Allocator pool_allocator(Pool_Allocator *pool);
void destroy_pool(Pool_Allocator pool);



// todo(josh): read_entire_file should be in a different file I think
char *read_entire_file(const char *filename, int *len);
void write_entire_file(const char *filename, const char *data);

// note(josh): defer implementation stolen from gb.h
#if !defined(GB_NO_DEFER) && defined(__cplusplus) && ((defined(_MSC_VER) && _MSC_VER >= 1400) || (__cplusplus >= 201103L))
extern "C++" {
    // NOTE(bill): Stupid fucking templates
    template <typename T> struct gbRemoveReference       { typedef T Type; };
    template <typename T> struct gbRemoveReference<T &>  { typedef T Type; };
    template <typename T> struct gbRemoveReference<T &&> { typedef T Type; };

    /// NOTE(bill): "Move" semantics - invented because the C++ committee are idiots (as a collective not as indiviuals (well a least some aren't))
    template <typename T> inline T &&gb_forward(typename gbRemoveReference<T>::Type &t)  { return static_cast<T &&>(t); }
    template <typename T> inline T &&gb_forward(typename gbRemoveReference<T>::Type &&t) { return static_cast<T &&>(t); }
    template <typename T> inline T &&gb_move   (T &&t)                                   { return static_cast<typename gbRemoveReference<T>::Type &&>(t); }
    template <typename F>
    struct gbprivDefer {
        F f;
        gbprivDefer(F &&f) : f(gb_forward<F>(f)) {}
        ~gbprivDefer() { f(); }
    };
    template <typename F> gbprivDefer<F> gb__defer_func(F &&f) { return gbprivDefer<F>(gb_forward<F>(f)); }

    #define GB_DEFER_1(x, y) x##y
    #define GB_DEFER_2(x, y) GB_DEFER_1(x, y)
    #define GB_DEFER_3(x)    GB_DEFER_2(x, __COUNTER__)
    #define defer(code)      auto GB_DEFER_3(_defer_) = gb__defer_func([&]()->void{code;})
}
#endif




template<typename T>
struct Array {
    T *data = {};
    int count = {};
    int capacity = {};
    Allocator allocator = {};

    T *append(T element);
    T *insert(int index, T element);
    void reserve(int capacity);
    T pop();
    T ordered_remove(int index);
    T unordered_remove(int index);
    void clear();
    void destroy();

    inline T &operator[](int index) {
        BOUNDS_CHECK(index, 0, count);
        return data[index];
    }
};

template<typename T>
Array<T> make_array(Allocator allocator, int capacity = 16) {
    Array<T> array = {};
    array.allocator = allocator;
    array.reserve(capacity);
    return array;
}

template<typename T>
Array<T> make_array(T *buffer, int capacity) {
    Array<T> array = {};
    array.allocator = null_allocator();
    array.data = buffer;
    array.capacity = capacity;
    return array;
}

template<typename T>
T *Array<T>::append(T element) {
    if (count >= capacity) {
        reserve(8 + (capacity * 2));
    }
    data[count] = element;
    count += 1;
    return &data[count-1];
}

template<typename T>
T *Array<T>::insert(int index, T element) {
    BOUNDS_CHECK(index, 0, count);
    if (count >= capacity) {
        reserve(8 + (capacity * 2));
    }
    for (int i = count; i >= index; i--) {
        data[i] = data[i-1];
    }
    data[index] = element;
    count += 1;
    return &data[index];
}

template<typename T>
void Array<T>::reserve(int capacity) {
    if (this->capacity >= capacity) {
        return;
    }

    assert(allocator.alloc_proc != nullptr);
    void *new_data = alloc(allocator, sizeof(T) * capacity);
    if (data != nullptr) {
        memcpy(new_data, data, sizeof(T) * count);
        free(allocator, data);
    }

    data = (T *)new_data;
    this->capacity = capacity;
}

template<typename T>
void Array<T>::destroy() {
    if (data) {
        free(allocator, data);
    }
}

template<typename T>
void Array<T>::clear() {
    count = 0;
}

template<typename T>
T Array<T>::pop() {
    BOUNDS_CHECK(count-1, 0, count);
    T t = data[count-1];
    count -= 1;
    return t;
}

template<typename T>
T Array<T>::ordered_remove(int index) {
    BOUNDS_CHECK(index, 0, count);
    T t = data[index];
    if (index != (count-1)) {
        for (int i = index+1; i < count; i++) {
            data[i-1] = data[i];
        }
    }
    count -= 1;
    return t;
}

template<typename T>
T Array<T>::unordered_remove(int index) {
    BOUNDS_CHECK(index, 0, count);
    T t = data[index];
    if (index != (count-1)) {
        data[index] = data[count-1];
    }
    count -= 1;
    return t;
}

#define Foreach(var, array) for (auto *var = (array).data; (uintptr_t)var < (uintptr_t)(&(array).data[(array).count]); var++)
#define For(idx, array) for (int idx = 0; idx < (array).count; idx++)



#if 0
struct Array_Test_Struct {
    Vector3 position;
    Vector3 tex_coord;
    Vector4 color;
};

void run_array_tests() {
    // todo(josh): add asserts and whatnot to this

    Array<Array_Test_Struct> array = {};
    array.allocator = default_allocator();
    defer(array.destroy());
    Array_Test_Struct a = {v3(1, 2, 3), v3(0.1f, 0.1f, 0), v4(1, 0, 1, 1)};
    Array_Test_Struct b = {v3(4, 5, 6), v3(0.2f, 0.2f, 0), v4(0, 1, 1, 1)};
    Array_Test_Struct c = {v3(7, 8, 9), v3(0.4f, 0.4f, 0), v4(1, 1, 0, 1)};
    array.append(a);
    array.append(b);
    array.append(c);
    array.unordered_remove(0);
    array.insert(1, a);

    printf("--------------\n");
    Foreach(v, array) {
        printf("%f %f %f\n", v->position.x, v->position.y, v->position.z);
    }

    Array_Test_Struct last_vertex = array.pop();
    printf("--------------\n");
    printf("%f %f %f\n", last_vertex.position.x, last_vertex.position.y, last_vertex.position.z);

    printf("--------------\n");
    Foreach(v, array) {
        printf("%f %f %f\n", v->position.x, v->position.y, v->position.z);
    }

    Array_Test_Struct crazy_vertex = {v3(9, 9, 9), v3(1, 1, 1), v4(1, 2, 3, 4)};
    array[1] = crazy_vertex;
    printf("--------------\n");
    Foreach(v, array) {
        printf("%f %f %f\n", v->position.x, v->position.y, v->position.z);
    }
}
#endif



struct String_Builder {
    Array<char> buf;

    void print(const char *str);
    void printf(const char *fmt, ...);
    void clear();
    char *string();
    void destroy();
};

String_Builder make_string_builder(Allocator allocator, int capacity = 16);



template<typename Key>
struct Key_Header {
    bool filled;
    Key  key;
};

template<typename Key, typename Value>
struct Key_Value {
    bool  filled;
    u64   hash;
    Value value;
};

#define INITIAL_HASHTABLE_SIZE 31
template<typename Key, typename Value>
struct Hashtable {
    Key_Header<Key> *key_headers;
    Key_Value<Key, Value> *values;
    i64 count;
    i64 capacity;
    Allocator allocator;

    void insert(Key key, Value value);
    void remove(Key key);
    bool contains(Key key);
    Value *get(Key key);
    void clear();
    void destroy();

    void get_key_header(Key key, Key_Header<Key> **out_header, int *out_index);
};

template<typename Key, typename Value>
Hashtable<Key, Value> make_hashtable(Allocator allocator, int capacity = INITIAL_HASHTABLE_SIZE) {
    Hashtable<Key, Value> hashtable = {};
    hashtable.allocator = allocator;
    hashtable.key_headers = (Key_Header<Key> *)alloc(hashtable.allocator, sizeof(Key_Header<Key>) * capacity);
    hashtable.values = (Key_Value<Key, Value> *)alloc(hashtable.allocator, sizeof(Key_Value<Key, Value>) * capacity);
    hashtable.capacity = capacity;
    return hashtable;
}

static i64 next_power_of_2(i64 n) {
    if (n <= 0) {
        return 0;
    }
    n -= 1;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    n += 1;
    return n;
}

template<typename Key, typename Value>
void Hashtable<Key, Value>::insert(Key key, Value value) {
    if ((f64)count >= ((f64)capacity*0.75)) {
        Key_Header<Key> *old_key_headers = key_headers;
        Key_Value<Key, Value> *old_values = values;
        i64 old_length = capacity;
        i64 old_count = count;

        i64 new_capacity = next_power_of_2(INITIAL_HASHTABLE_SIZE + old_length);
        key_headers = (Key_Header<Key> *)alloc(allocator, sizeof(Key_Header<Key>) * new_capacity);
        values = (Key_Value<Key, Value> *)alloc(allocator, sizeof(Key_Value<Key, Value>) * new_capacity);
        count = 0;
        capacity = new_capacity;

        for (i64 header_idx = 0; header_idx < old_length; header_idx++) {
            Key_Header<Key> *old_key_header = &old_key_headers[header_idx];
            if (old_key_header->filled) {
                insert(old_key_header->key, old_values[header_idx].value);
            }
        }

        free(allocator, old_key_headers);
        free(allocator, old_values);
    }

    i64 key_value_index = -1;
    u64 h = hash_key(key);
    u64 hash_idx = h % capacity;
    for (i64 idx = hash_idx; idx < capacity; idx++) {
        Key_Value<Key, Value> *value_ptr = &values[idx];
        if (!value_ptr->filled) {
            key_value_index = idx;
            goto end_search;
        }
    }
    for (i64 idx = 0; idx < hash_idx; idx += 1) {
        Key_Value<Key, Value> *value_ptr = &values[idx];
        if (!value_ptr->filled) {
            key_value_index = idx;
            goto end_search;
        }
    }
    end_search:;
    assert(key_value_index >= 0);

    values[key_value_index] = {true, h, value};
    key_headers[key_value_index] = {true, key};
    count += 1;
}

template<typename Key>
u64 hash_key(Key key) {
    // note(josh): fnv64
    u64 h = 0xcbf29ce484222325;
    byte *key_byte_ptr = (byte *)&key;
    for (int i = 0; i < sizeof(Key); i++) {
        h = (h * 0x100000001b3) ^ u64(key_byte_ptr[i]);
    }
    return h;
}

template<typename Key, typename Value>
bool Hashtable<Key, Value>::contains(Key key) {
    Key_Header<Key> *header = {};
    int index = {};
    get_key_header(key, &header, &index);
    return header != nullptr;
}

template<typename Key, typename Value>
Value *Hashtable<Key, Value>::get(Key key) {
    Key_Header<Key> *header = {};
    int index = {};
    get_key_header(key, &header, &index);
    if (header == nullptr) {
        return nullptr;
    }
    return &values[index].value;
}

template<typename Key, typename Value>
void Hashtable<Key, Value>::remove(Key key) {
    Key_Header<Key> *key_header = {};
    int key_value_idx = {};
    get_key_header(key, &key_header, &key_value_idx);
    if (key_header == nullptr) {
        return;
    }
    Key_Value<Key, Value> *key_value = &values[key_value_idx];
    u64 hash_idx = key_value->hash % capacity;
    Key_Value<Key, Value> *last_thing_that_hashed_to_the_same_idx = {};
    u64 last_thing_index = 0;
    for (int idx = key_value_idx+1; idx < capacity; idx++) {
        Key_Value<Key, Value> *value = &values[idx];
        if (!value->filled) {
            goto end_search;
        }
        if ((value->hash % capacity) == hash_idx) {
            last_thing_that_hashed_to_the_same_idx = value;
            last_thing_index = idx;
        }
    }
    for (int idx = 0; idx < hash_idx; idx++) {
        Key_Value<Key, Value> *value = &values[idx];
        if (!value->filled) {
            goto end_search;
        }
        if ((value->hash % capacity) == hash_idx) {
            last_thing_that_hashed_to_the_same_idx = value;
            last_thing_index = idx;
        }
    }
    end_search:;

    if (last_thing_that_hashed_to_the_same_idx != nullptr) {
        *key_header = key_headers[last_thing_index];
        *key_value  = *last_thing_that_hashed_to_the_same_idx;
        key_headers[last_thing_index].filled = false;
        last_thing_that_hashed_to_the_same_idx->filled = false;
    }
    else {
        key_header->filled = false;
        key_value->filled = false;
    }

    count -= 1;
}

template<typename Key, typename Value>
void Hashtable<Key, Value>::clear() {
    for (int idx = 0; idx < capacity; idx++) {
        key_headers[idx].filled = false;
        values[idx].filled = false;
    }
    count = 0;
}

template<typename Key, typename Value>
void Hashtable<Key, Value>::destroy() {
    if (key_headers != nullptr) {
        assert(values != nullptr);
        free(allocator, key_headers);
        free(allocator, values);
    }
}

template<typename Key, typename Value>
void Hashtable<Key, Value>::get_key_header(Key key, Key_Header<Key> **out_header, int *out_index) {
    u64 h = hash_key(key);
    u64 hash_idx = h % capacity;
    for (int idx = hash_idx; idx < capacity; idx++) {
        Key_Header<Key> *header = &key_headers[idx];
        if (!header->filled) {
            return;
        }
        if (header->key == key) {
            *out_header = header;
            *out_index = idx;
            return;
        }
    }
    for (int idx = 0; idx < hash_idx; idx += 1) {
        Key_Header<Key> *header = &key_headers[idx];
        if (!header->filled) {
            return;
        }
        if (header->key == key) {
            *out_header = header;
            *out_index = idx;
            return;
        }
    }
}

// main :: proc() {
//  freq := get_freq();

//  // NUM_ELEMS :: 10;
//  NUM_ELEMS :: 1024 * 10000;

//  my_table: Hashtable(int, int);
//  {
//      insert_start := get_time();
//      for i in 0..NUM_ELEMS {
//          insert(&my_table, i, i * 3);
//      }
//      insert_end := get_time();
//      logging.ln("My map inserting ", NUM_ELEMS, " elements:   ", (insert_end-insert_start)/freq, "s");
//  }

//  odin_table: map[int]int;
//  {
//      insert_start := get_time();
//      for i in 0..NUM_ELEMS {
//          odin_table[i] = i * 3;
//      }
//      insert_end := get_time();
//      logging.ln("Odin map inserting ", NUM_ELEMS, " elements: ", (insert_end-insert_start)/freq, "s");
//  }

//  {
//      lookup_start := get_time();
//      for i in 0..NUM_ELEMS {
//          val, ok := get(&my_table, i);
//          assert(ok); assert(val == i * 3);
//      }
//      lookup_end := get_time();
//      logging.ln("My map retrieving ", NUM_ELEMS, " elements:   ", (lookup_end-lookup_start)/freq, "s");
//  }

//  {
//      lookup_start := get_time();
//      for i in 0..NUM_ELEMS {
//          val, ok := odin_table[i];
//          assert(ok); assert(val == i * 3);
//      }
//      lookup_end := get_time();
//      logging.ln("Odin map retrieving ", NUM_ELEMS, " elements: ", (lookup_end-lookup_start)/freq, "s");
//  }

//  {
//      iterate_start := get_time();
//      for header, idx in my_table.key_headers {
//          if !header.filled do continue;
//          key := header.key;
//          value := my_table.values[idx].value;
//          assert(value == key * 3);
//      }
//      iterate_end := get_time();
//      logging.ln("My map iterating ", NUM_ELEMS, " elements:   ", (iterate_end-iterate_start)/freq, "s");
//  }

//  {
//      iterate_start := get_time();
//      for key, value in odin_table {
//          assert(value == key * 3);
//      }
//      iterate_end := get_time();
//      logging.ln("Odin map iterating ", NUM_ELEMS, " elements: ", (iterate_end-iterate_start)/freq, "s");
//  }

//  {
//      removal_start := get_time();
//      for i in 0..NUM_ELEMS {
//          remove(&my_table, i);
//      }
//      removal_end := get_time();
//      logging.ln("My map removing ", NUM_ELEMS, " elements:   ", (removal_end-removal_start)/freq, "s");
//  }

//  {
//      removal_start := get_time();
//      for i in 0..NUM_ELEMS {
//          delete_key(&odin_table, i);
//      }
//      removal_end := get_time();
//      logging.ln("Odin map removing ", NUM_ELEMS, " elements: ", (removal_end-removal_start)/freq, "s");
//  }
// }

bool starts_with(char *str, char *start);
bool ends_with(char *str, char *end);
char *path_directory(const char *filepath, Allocator allocator);
char *path_filename(const char *filepath, Allocator allocator);
char *path_filename_with_extension(const char *filepath, Allocator allocator);
