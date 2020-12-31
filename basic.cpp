#include "basic.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>



bool is_power_of_two(uintptr_t n) {
    return n > 0 && (n & (n-1)) == 0;
}

uintptr_t align_forward(uintptr_t p, uintptr_t align) {
    assert(is_power_of_two(align));
    p = (p + (align - 1)) & (~(align - 1));
    return p;
}

uintptr_t align_backward(uintptr_t p, uintptr_t align) {
    return align_forward(p - align + 1, align);
}

void zero_memory(void *memory_void, int length) {
    char *memory = (char *)memory_void;
    uintptr_t start = (uintptr_t)memory;
    uintptr_t start_aligned = align_forward(start, alignof(uintptr_t));
    uintptr_t end = start + (uintptr_t)length;
    uintptr_t end_aligned = align_backward(end, alignof(uintptr_t));

    for (uintptr_t i = start; i < start_aligned; i++) {
        memory[i-start] = 0;
    }

    for (uintptr_t i = start_aligned; i < end_aligned; i += sizeof(uintptr_t)) {
        *((uintptr_t *)&memory[i-start]) = 0;
    }

    for (uintptr_t i = end_aligned; i < end; i++) {
        memory[i-start] = 0;
    }
}



float min(float a, float b) {
    if (a < b)  return a;
    return b;
}

float max(float a, float b) {
    if (a > b)  return a;
    return b;
}

int min(int a, int b) {
    if (a < b)  return a;
    return b;
}

int max(int a, int b) {
    if (a > b)  return a;
    return b;
}



int modulo(int a, int b) {
    int r = a % b;
    return r < 0 ? r + b : r;
}



void *alloc(Allocator allocator, int size, int alignment) {
    assert(allocator.alloc_proc != nullptr && "Alloc proc was nullptr for allocator");
    void *ptr = allocator.alloc_proc(allocator.data, size, alignment);
    return memset(ptr, 0, size);
}

void free(Allocator allocator, void *ptr) {
    assert(allocator.free_proc != nullptr && "Free proc was nullptr for allocator");
    allocator.free_proc(allocator.data, ptr);
}



void *default_allocator_alloc(void *allocator, int size, int alignment) {
    return malloc(size);
}

void default_allocator_free(void *allocator, void *ptr) {
    free(ptr);
}

Allocator default_allocator() {
    Allocator a = {};
    a.alloc_proc = default_allocator_alloc;
    a.free_proc = default_allocator_free;
    return a;
}



byte *buffer_allocate(byte *buffer, int buffer_len, int *offset, int size, int alignment, bool panic_on_oom) {
    // Don't allow allocations of zero size. This would likely return a
    // pointer to a different allocation, causing many problems.
    if (size == 0) {
        return nullptr;
    }

    // todo(josh): The `align_forward()` call and the `start + size` below
    // that could overflow if the `size` or `align` parameters are super huge

    int start = align_forward(*offset, alignment);

    // Don't allow allocations that would extend past the end of the buffer.
    if ((start + size) > buffer_len) {
        if (panic_on_oom) {
            assert(0 && "buffer_allocate ran out of memory");
        }
        return nullptr;
    }

    *offset = start + size;
    byte *ptr = &buffer[start];
    zero_memory(ptr, size);
    return ptr;
}



void *null_allocator_alloc(void *allocator, int size, int align) {
    ASSERTF(false, "Tried to allocate with the null allocator");
    return nullptr;
}

void null_allocator_free(void *allocator, void *ptr) {
}

Allocator null_allocator() {
    Allocator a = {};
    return a;
}



void init_arena(Arena *arena, byte *backing, int backing_size) {
    arena->memory = backing;
    arena->memory_size = backing_size;
    arena->cur_offset = 0;
}

void *arena_alloc(void *allocator, int size, int align) {
    Arena *arena = (Arena *)allocator;
    return buffer_allocate(arena->memory, arena->memory_size, &arena->cur_offset, size, align);
}

void arena_free(void *allocator, void *ptr) {
    // note(josh): freeing from arenas does nothing.
}

void arena_clear(Arena *arena) {
    arena->cur_offset = 0;
}

Allocator arena_allocator(Arena *arena) {
    Allocator a = {};
    a.data = arena;
    a.alloc_proc = arena_alloc;
    a.free_proc = arena_free;
    return a;
}



void init_pool_allocator(Pool_Allocator *pool, Allocator backing_allocator, int slot_size, int num_slots) {
    assert(slot_size > 0);
    assert(num_slots > 0);
    pool->backing_allocator = backing_allocator;
    pool->memory_size = slot_size * num_slots;
    pool->slot_size = slot_size;
    pool->num_slots = num_slots;
    pool->memory = (byte *)alloc(pool->backing_allocator, pool->memory_size);
    pool->slots_freelist = (int *)alloc(pool->backing_allocator, sizeof(int) * num_slots);
    pool->freelist_count = num_slots;
    pool->generations = (int *)alloc(pool->backing_allocator, sizeof(int) * num_slots);
    int slot_idx = 0;
    for (int idx = num_slots-1; idx >= 0; idx -= 1) {
        pool->slots_freelist[idx] = slot_idx;
        slot_idx += 1;
    }
}

void *pool_get(Pool_Allocator *pool, int *out_generation, int *out_index) {
    assert(pool->freelist_count > 0);
    int slot = pool->slots_freelist[pool->freelist_count-1];
    pool->generations[slot] += 1;
    if (out_index) {
        *out_index = slot;
    }
    if (out_generation) {
        *out_generation = pool->generations[slot];
    }
    pool->freelist_count -= 1;
    return memset(&pool->memory[pool->slot_size * slot], 0, pool->slot_size);
}

int pool_get_slot_index(Pool_Allocator *pool, void *ptr) {
    int slot = ((uintptr_t)ptr - (uintptr_t)pool->memory) / pool->slot_size;
    return slot;
}

void *pool_get_slot_by_index(Pool_Allocator *pool, int slot) {
    BOUNDS_CHECK(slot, 0, pool->num_slots);
    void *ptr = pool->memory + (pool->slot_size * slot);
    return ptr;
}

void pool_return(Pool_Allocator *pool, void *ptr) {
    assert((pool->freelist_count+1) <= pool->num_slots);
    int slot = pool_get_slot_index(pool, ptr);
    pool->slots_freelist[pool->freelist_count] = slot;
    pool->freelist_count += 1;
}

void *pool_alloc(void *allocator, int size, int align) {
    Pool_Allocator *pool = (Pool_Allocator *)allocator;
    assert(pool != nullptr);
    return pool_get(pool, nullptr, nullptr);
}

void pool_free(void *allocator, void *ptr) {
    Pool_Allocator *pool = (Pool_Allocator *)allocator;
    assert(pool != nullptr);
    pool_return(pool, ptr);
}

Allocator pool_allocator(Pool_Allocator *pool) {
    Allocator a = {};
    a.data = pool;
    a.alloc_proc = pool_alloc;
    a.free_proc = pool_free;
    return a;
}

void destroy_pool(Pool_Allocator pool) {
    if (pool.memory) free(pool.backing_allocator, pool.memory);
    if (pool.slots_freelist) free(pool.backing_allocator, pool.slots_freelist);
}



// todo(josh): custom allocator
char *read_entire_file(const char *filename, int *len) {
    FILE *file = fopen(filename, "rb");
    if (file == nullptr) {
        return nullptr;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *str = (char *)malloc(length + 1);
    fread(str, 1, length, file);
    fclose(file);

    str[length] = 0;
    *len = length+1;
    return str;
}

void write_entire_file(const char *filename, const char *data) {
    FILE *file = fopen(filename, "w");
    assert(file != nullptr);
    fputs(data, file);
    fclose(file);
}



String_Builder make_string_builder(Allocator allocator, int capacity) {
    String_Builder sb = {};
    sb.buf = make_array<char>(allocator, capacity);
    return sb;
}

void String_Builder::print(const char *str) {
    int length = strlen(str);
    buf.reserve(buf.count + length + 8);
    memcpy(&buf.data[buf.count], str, length);
    buf.count += length;
    BOUNDS_CHECK(buf.count, 0, buf.capacity);
    buf.data[buf.count] = 0;
}

void String_Builder::printf(const char *fmt, ...) {
    va_list args;
    buf.reserve(32); // ensure at least 32 bytes in the buffer
    int length_would_have_written = 0;
    do {
        buf.reserve(buf.count + length_would_have_written + 8);
        va_start(args, fmt);
        length_would_have_written = vsnprintf(&buf.data[buf.count], buf.capacity - buf.count, fmt, args);
        va_end(args);
    } while (length_would_have_written >= (buf.capacity - buf.count));

    assert(length_would_have_written < buf.capacity - buf.count);
    buf.data[buf.count+length_would_have_written+1] = '\0';
    buf.count += length_would_have_written;
    BOUNDS_CHECK(buf.count, 0, buf.capacity);
}

void String_Builder::clear() {
    buf.clear();
    BOUNDS_CHECK(buf.count, 0, buf.capacity);
    buf.data[buf.count] = 0;
}

char *String_Builder::string() {
    return buf.data;
}

void String_Builder::destroy() {
    buf.destroy();
}



bool starts_with(char *str, char *start) {
    for (int i = 0; start[i] != '\0'; i++) {
        if (str[i] != start[i]) {
            return false;
        }
    }
    return true;
}

// path/to/file.txt -> path/to
// returns null if it doesn't hit a '/' or '\\'
char *path_directory(const char *filepath, Allocator allocator) {
    int length = strlen(filepath);
    int slash_index = length;
    for (; slash_index >= 0; slash_index--) {
        if (filepath[slash_index] == '/' || filepath[slash_index] == '\\') {
            break;
        }
    }
    if (slash_index == -1) {
        return nullptr;
    }
    int length_to_end = length - (length - slash_index);
    char *new_str = (char *)alloc(allocator, length_to_end+1);
    memcpy(new_str, filepath, length_to_end);
    new_str[length_to_end] = '\0';
    return new_str;
}