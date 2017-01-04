#ifndef DASHBOARD_H
#define DASHBOARD_H


#include "src/util/taskstats.h"
#include "src/process/process.h"
#include "src/system/sys_stats.h"

typedef struct {
    int max_x;
    int max_y;
    int prev_x;
    int prev_y;
    char screen;
    char *fieldbar;
    sysaux *system;
    struct nl_session *nls;
    ps_node *process_list;
    Tree *process_tree;
} Board;    

#endif
