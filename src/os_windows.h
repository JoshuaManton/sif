#include "basic.h"

struct Timer {
    double frequency = {};
};

void init_timer(Timer *timer);
double query_timer(Timer *timer);



char *get_absolute_path(const char *filename, Allocator);
char *wide_to_cstring(wchar_t *wide, Allocator);
char *get_current_exe_name(Allocator);

void delete_file(char *);