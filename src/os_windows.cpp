#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "os_windows.h"

char *get_absolute_path(const char *filename, Allocator allocator) {
    int absolute_path_buffer_size = 128;
    char *absolute_path = (char *)alloc(allocator, absolute_path_buffer_size);
    u32 result = GetFullPathNameA(filename, absolute_path_buffer_size, absolute_path, nullptr);
    if (result >= absolute_path_buffer_size) {
        absolute_path = (char *)alloc(allocator, result);
        u32 new_result = GetFullPathNameA(filename, result, absolute_path, nullptr);
        assert(new_result < result);
    }
    assert(absolute_path != nullptr);
    return absolute_path;
}

char *wide_to_cstring(wchar_t *wide, Allocator allocator) {
    int query_result = WideCharToMultiByte(CP_UTF8, 0, wide, -1, nullptr, 0, nullptr, nullptr);
    assert(query_result > 0);

    char *cstring = (char *)alloc(allocator, query_result);
    int result = WideCharToMultiByte(CP_UTF8, 0, wide, -1, cstring, query_result, nullptr, nullptr);

    assert(result == query_result);
    return cstring;
}

char *get_current_exe_name(Allocator allocator) {
    wchar_t exe_path_wide[MAX_PATH];
    GetModuleFileNameW(nullptr, exe_path_wide, MAX_PATH);
    return wide_to_cstring(exe_path_wide, allocator);
}

void delete_file(char *filename) {
    DeleteFileA(filename);
}



void init_timer(Timer *timer) {
    LARGE_INTEGER large_integer_frequency = {};
    assert(QueryPerformanceFrequency(&large_integer_frequency) != 0);
    timer->frequency = large_integer_frequency.QuadPart/1000.0;
}

double query_timer(Timer *timer) {
    LARGE_INTEGER large_integer_counter = {};
    assert(QueryPerformanceCounter(&large_integer_counter) != 0);
    return large_integer_counter.QuadPart / timer->frequency;
}