#pragma once

#include "basic.h"

struct Timer {
    double frequency = {};
};

void init_timer(Timer *timer);
double query_timer(Timer *timer);



char *get_absolute_path(const char *filename, Allocator);
wchar_t *cstring_to_wide(char *str, Allocator allocator);
char *wide_to_cstring(wchar_t *wide, Allocator allocator);
char *get_current_exe_name(Allocator);

void delete_file(char *);



void sleep(u64 ms);

typedef void *Handle;

struct Mutex {
    Handle handle;
};

Mutex create_mutex();
bool wait_for_mutex(Mutex mutex);
void release_mutex(Mutex mutex);

struct Thread {
    Handle handle;
};

Thread create_thread(u32 (*function)(void *userdata), void *userdata);
void wait_for_thread(Thread thread);