#ifndef DASHBOARD_H
#define DASHBOARD_H

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

#endif
