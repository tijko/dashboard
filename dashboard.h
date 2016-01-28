#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <sys/types.h>

#include "src/process/process.h"


typedef struct {
    uid_t euid;
    int max_x;
    int max_y;
    int prev_x;
    int prev_y;
    char *fieldbar;
    long memtotal;
    proc_t *process_list;
} board_t;    

void print_usage(void);

char set_sort_option(char *opt);

void dashboard_mainloop(char attr_sort);

void update_process_stats(board_t *dashboard);
 
board_t *init_board(void);

void free_board(board_t *board);

#endif
