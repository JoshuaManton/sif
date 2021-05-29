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
#ifdef MEM_DO_ZEROING
    memset(ptr, 0, size);
#endif
    return ptr;
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
    // zero_memory(ptr, size);
    return ptr;
}



void *null_allocator_alloc(void *allocator, int size, int align) {
    assert(false && "Tried to allocate with the null allocator");
    return nullptr;
}

void null_allocator_free(void *allocator, void *ptr) {
}

Allocator null_allocator() {
    Allocator a = {};
    return a;
}



void init_arena(Arena *arena, void *backing, int backing_size, bool panic_on_oom) {
    arena->memory = (byte *)backing;
    arena->memory_size = backing_size;
    arena->cur_offset = 0;
    arena->panic_on_oom = panic_on_oom;
}

void *arena_alloc(void *allocator, int size, int align) {
    Arena *arena = (Arena *)allocator;
    return buffer_allocate(arena->memory, arena->memory_size, &arena->cur_offset, size, align, arena->panic_on_oom);
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



void init_dynamic_arena(Dynamic_Arena *dyn, int starting_chunk_size, Allocator backing_allocator) {
    dyn->chunk_size = starting_chunk_size;
    dyn->backing_allocator = backing_allocator;
    dyn->arenas = make_array<Arena>(backing_allocator);
    dyn->leaked_allocations = make_array<void *>(backing_allocator);
    Arena *arena = dyn->arenas.append();
    init_arena(arena, alloc(backing_allocator, starting_chunk_size), starting_chunk_size, false);
}

void *dynamic_arena_alloc(void *allocator, int size, int align) {
    Dynamic_Arena *dyn = (Dynamic_Arena *)allocator;
    dyn->spinlock.lock();
    defer(dyn->spinlock.unlock());
    Arena *current_arena = &dyn->arenas[dyn->current_arena_index];
    void *ptr = arena_alloc(current_arena, size, align);
    if (!ptr) {
        // we've reached the end of the arena
        if (dyn->current_arena_index+1 >= dyn->arenas.count) {
            // make a new arena
            current_arena = dyn->arenas.append();
            dyn->chunk_size *= 2;
            init_arena(current_arena, alloc(dyn->backing_allocator, dyn->chunk_size), dyn->chunk_size, false);
        }
        else {
            // use the next one in the list
            current_arena = &dyn->arenas[dyn->current_arena_index+1];
        }
        dyn->current_arena_index += 1;
        
        // retry the allocation
        ptr = arena_alloc(current_arena, size, align);
        if (!ptr) {
            // if it STILL failed then that means that the allocation is too big for our chunk size. fallback to the backing allocator
            ptr = alloc(dyn->backing_allocator, size, align);
            assert(ptr != nullptr);
            dyn->leaked_allocations.append(ptr);
        }
    }
    assert(ptr != nullptr);
    return ptr;
}

void dynamic_arena_free(void *allocator, void *ptr) {
    // note(josh): freeing from arenas does nothing
}

void dynamic_arena_clear(Dynamic_Arena *dyn) {
    dyn->spinlock.lock();
    defer(dyn->spinlock.unlock());
    For (idx, dyn->arenas) {
        arena_clear(&dyn->arenas[idx]);
    }
    dyn->current_arena_index = 0;
}

Allocator dynamic_arena_allocator(Dynamic_Arena *dyn) {
    Allocator a = {};
    a.data = dyn;
    a.alloc_proc = dynamic_arena_alloc;
    a.free_proc = dynamic_arena_free;
    return a;
}

void destroy_dynamic_arena(Dynamic_Arena *dyn) {
    dyn->spinlock.lock();
    defer(dyn->spinlock.unlock());
    For (idx, dyn->arenas) {
        free(dyn->backing_allocator, dyn->arenas[idx].memory);
    }
    dyn->arenas.destroy();
    For (idx, dyn->leaked_allocations) {
        free(dyn->backing_allocator, dyn->leaked_allocations[idx]);
    }
    dyn->leaked_allocations.destroy();
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



char *read_entire_file(const char *filename, Allocator allocator, int *len) {
    FILE *file = fopen(filename, "rb");
    if (file == nullptr) {
        return nullptr;
    }
    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);
    char *str = (char *)alloc(allocator, length+1);
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

char *String_Builder::print(char c) {
  buf.reserve(buf.count + 1 + 8);
  char *copy = &buf.data[buf.count];
  *copy = c;
  buf.count += 1;
  BOUNDS_CHECK(buf.count, 0, buf.capacity);
  buf.data[buf.count] = 0;
  return copy;
}

char *String_Builder::print(const char *str) {
    int length = strlen(str);
    buf.reserve(buf.count + length + 8);
    char *copy = &buf.data[buf.count];
    memcpy(copy, str, length);
    buf.count += length;
    BOUNDS_CHECK(buf.count, 0, buf.capacity);
    buf.data[buf.count] = 0;
    return copy;
}

char *String_Builder::printf(const char *fmt, ...) {
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
    char *copy = &buf.data[buf.count];
    buf.data[buf.count+length_would_have_written+1] = 0;
    buf.count += length_would_have_written;
    BOUNDS_CHECK(buf.count, 0, buf.capacity);
    return copy;
}

char *String_Builder::write_with_length(const char *str, int length) {
    buf.reserve(buf.count + length + 8);
    char *copy = &buf.data[buf.count];
    memcpy(copy, str, length);
    buf.count += length;
    BOUNDS_CHECK(buf.count, 0, buf.capacity);
    buf.data[buf.count] = 0;
    return copy;
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



Chunked_String_Builder make_chunked_string_builder(Allocator allocator, int chunk_size) {
    Chunked_String_Builder sb = {};
    sb.max_chunk_size = chunk_size;
    sb.chunks.allocator = allocator;
    sb.allocator = allocator;
    return sb;
}

Chunked_String_Builder_Chunk *Chunked_String_Builder::get_or_make_chunk_buffer_for_length(int length) {
    // todo(josh): should we double the chunk size when allocating new chunks?
    Chunked_String_Builder_Chunk *current_chunk = nullptr;
    assert(max_chunk_size > 0);
    if (chunks.count == 0) {
        current_chunk = chunks.append();
        current_chunk->buffer = (char *)alloc(allocator, max_chunk_size);
        current_chunk->buffer[0] = 0;
        current_chunk->buffer_size = max_chunk_size;
    }
    else {
        current_chunk = &chunks[current_chunk_index];
    }
    if (length > (current_chunk->buffer_size - 1 - current_chunk->cursor)) { // not enough room let in this chunk
        if (length >= max_chunk_size) { // this string is bigger than our max chunk size, copy the string as it's own chunk
            current_chunk = chunks.append();
            current_chunk->buffer = (char *)alloc(allocator, length+1);
            current_chunk->buffer[0] = 0;
            current_chunk->buffer_size = length+1;
            current_chunk_index += 1;
            return current_chunk;
        }
        else {
            current_chunk = chunks.append();
            current_chunk->buffer = (char *)alloc(allocator, max_chunk_size);
            current_chunk->buffer[0] = 0;
            current_chunk->buffer_size = max_chunk_size;
            current_chunk_index += 1;
            return current_chunk;
        }
    }
    else {
        return current_chunk;
    }
}

char *Chunked_String_Builder::print(const char *str) {
    int length = strlen(str);
    return write_with_length(str, length);
}

char *Chunked_String_Builder::printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int length_required = vsnprintf(nullptr, 0, fmt, args);
    va_end(args);
    
    Chunked_String_Builder_Chunk *chunk = get_or_make_chunk_buffer_for_length(length_required);
    char *copy = &chunk->buffer[chunk->cursor];
    va_start(args, fmt);
    int result = vsnprintf(copy, chunk->buffer_size - chunk->cursor, fmt, args);
    va_end(args);
    assert(result == length_required);
    chunk->cursor += length_required;
    chunk->buffer[chunk->cursor] = 0;
    return copy;
}

char *Chunked_String_Builder::write_with_length(const char *str, int length) {
    Chunked_String_Builder_Chunk *chunk = get_or_make_chunk_buffer_for_length(length);
    char *copy = &chunk->buffer[chunk->cursor];
    memcpy(copy, str, length);
    chunk->cursor += length;
    chunk->buffer[chunk->cursor] = 0;
    return copy;
}

void Chunked_String_Builder::append_null() {
    Chunked_String_Builder_Chunk *chunk = get_or_make_chunk_buffer_for_length(1);
    chunk->buffer[chunk->cursor] = 0;
    chunk->cursor += 1;
    chunk->buffer[chunk->cursor] = 0;
}

void Chunked_String_Builder::clear() {
    For (idx, chunks) {
        chunks[idx].cursor = 0;
        chunks[idx].buffer[0] = 0;
    }
    current_chunk_index = 0;
}

char *Chunked_String_Builder::make_string() {
    int required_length = 0;
    for (int idx = 0; idx <= current_chunk_index && idx < chunks.count; idx += 1) {
        Chunked_String_Builder_Chunk chunk = chunks[idx];
        required_length += chunk.cursor;
    }
    required_length += 1; // null term
    char *big_buffer = (char *)alloc(allocator, required_length);
    int current_cursor = 0;
    for (int idx = 0; idx <= current_chunk_index && idx < chunks.count; idx += 1) {
        Chunked_String_Builder_Chunk chunk = chunks[idx];
        memcpy(&big_buffer[current_cursor], chunk.buffer, chunk.cursor);
        current_cursor += chunk.cursor;
    }
    assert(current_cursor+1 == required_length);
    big_buffer[current_cursor] = 0;
    return big_buffer;
}

void Chunked_String_Builder::destroy() {
    For (idx, chunks) {
        free(allocator, chunks[idx].buffer);
    }
    chunks.destroy();
}



bool starts_with(const char *str, const char *start) {
    for (int i = 0; start[i] != 0; i++) {
        if (str[i] != start[i]) {
            return false;
        }
    }
    return true;
}

bool ends_with(const char *str, const char *end) {
    int str_length = strlen(str);
    int end_length = strlen(end);
    if (str_length < end_length) {
        return false;
    }
    for (int i = 0; i < end_length; i++) {
        if (str[str_length-1-i] != end[end_length-1-i]) {
            return false;
        }
    }
    return true;
}

int last_index_of(const char *str, const char *query) {
    int end_of_str = strlen(str)-1;
    int query_len = strlen(query)-1;
    for (int i = end_of_str-query_len; i >= 0; i -= 1) {
        const char *c = &str[i];
        bool equal = true;
        for (int q = 0; q < query_len; q += 1) {
            if (str[i+q] != query[q]) {
                equal = false;
            }
        }
        if (equal) {
            return i;
        }
    }
    return -1;
}

// path/to/file.txt -> path/to
// file.txt         -> <null>
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
    new_str[length_to_end] = 0;
    return new_str;
}

// path/to/file.txt -> file
// file.txt         -> file
// file             -> file
// path/to/         -> <null>
char *path_filename(const char *filepath, Allocator allocator) {
    int length = strlen(filepath);
    int slash_index = length;
    int dot_index = length;
    for (; slash_index >= 0; slash_index--) {
        if (filepath[slash_index] == '/' || filepath[slash_index] == '\\') {
            break;
        }
    }
    for (; dot_index >= 0; dot_index--) {
        if (filepath[dot_index] == '.') {
            break;
        }
    }
    int start = slash_index+1;
    int end = dot_index;
    if (end < start) {
        end = length;
    }
    if (start == end) {
        return nullptr;
    }
    assert(end >= start);
    int length_to_end = end - start;
    char *new_str = (char *)alloc(allocator, length_to_end+1);
    memcpy(new_str, &filepath[start], length_to_end);
    new_str[length_to_end] = 0;
    return new_str;
}

// path/to/file.txt -> file.txt
// file.txt         -> file.txt
// file             -> file
// path/to/         -> <null>
char *path_filename_with_extension(const char *filepath, Allocator allocator) {
    int length = strlen(filepath);
    int slash_index = length;
    for (; slash_index >= 0; slash_index--) {
        if (filepath[slash_index] == '/' || filepath[slash_index] == '\\') {
            break;
        }
    }
    int start = slash_index+1;
    int end = length;
    if (start == end) {
        return nullptr;
    }
    assert(end >= start);
    int length_to_end = end - start;
    char *new_str = (char *)alloc(allocator, length_to_end+1);
    memcpy(new_str, &filepath[start], length_to_end);
    new_str[length_to_end] = 0;
    return new_str;
}