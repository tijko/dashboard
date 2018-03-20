#ifndef DASHBOARD_H
#define DASHBOARD_H


#include "src/util/taskstats.h"
#include "src/process/process.h"
#include "src/system/sys_stats.h"

#define PTS_DIR "/dev/pts"

typedef struct {
    int max_x;
    int max_y;
    int prev_x;
    int prev_y;
    char screen;
    char *fieldbar;
    char *(*construct_fieldbar)(void);
    void (*display_window)(WINDOW *ps_window, ps_node const *ps_list,
               char const *fieldbar, int process_list_num, int max_y);
    sysaux *system;
    struct nl_session *nls;
    ps_node *process_list;
    Tree *process_tree;
} Board;    

#endif
