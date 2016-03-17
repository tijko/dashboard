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
    sysaux *system;
    ps_node *process_list;
    Tree *process_tree;
} Board;    

void print_usage(void);

char set_sort_option(char *opt);

void dashboard_mainloop(char attr_sort);

void update_process_stats(Tree *ps_tree, ps_node *ps, sysaux *sys);
 
int calculate_ln_diff(Board *board, int ln, int prev_ln);

Board *init_board(void);

void free_board(Board *board);

#endif
