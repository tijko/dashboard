#ifndef DASHBOARD_H
#define DASHBOARD_H

#include <sys/types.h>

#include "src/process/process.h"
#include "src/system/sys_stats.h"


typedef struct {
    int max_x;
    int max_y;
    int prev_x;
    int prev_y;
    char *fieldbar;
    sysaux_t *system;
    proc_t *process_list;
    proc_tree_t *process_tree;
} board_t;    

void print_usage(void);

char set_sort_option(char *opt);

void dashboard_mainloop(char attr_sort);

//void update_process_stats(board_t *dashboard);
void update_process_stats(proc_tree_t *ps_tree, proc_t *ps, sysaux_t *sys);
 
board_t *init_board(void);

void free_board(board_t *board);

#endif
