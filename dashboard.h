#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <sys/types.h>

#include "src/process/process.h"


typedef struct {
    char *fieldbar;
    int max_x;
    int max_y;
    int prev_x;
    int prev_y;
    proc_t *process_list;
} board_t;    

void print_usage(void);

char set_sort_option(char *opt);

void dashboard_mainloop(char attr_sort);

void get_process_stats(proc_t *process_list, uid_t euid, long memtotal);

#endif
