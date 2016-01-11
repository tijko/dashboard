#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <sys/types.h>

#include "src/process/process.h"

#define STAT_PATHMAX 32


typedef struct {
    uid_t euid;
    int max_x;
    int max_y;
    int prev_x;
    int prev_y;
    char path[STAT_PATHMAX];
    char *fieldbar;
    long memtotal;
    proc_t *process_list;
} board_t;    

void print_usage(void);

char set_sort_option(char *opt);

void dashboard_mainloop(char attr_sort);

board_t *init_board(void);

void get_process_stats(board_t *dashboard);

#endif
