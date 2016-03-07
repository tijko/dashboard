#ifndef PROC_H
#define PROC_H


#include <stdint.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>

#include "../system/sys_stats.h"


#define PROCNAME_MAX 1024
#define STAT_PATHMAX 32
#define MAXPROCPATH 2048
#define MAXFIELD 32

typedef int color_t;

enum color_t {
    BLACK = 0,
    RED   = 1
};

typedef struct process_attr {
    int pid;
    int uid;
    int cpuset;
    int nice;
    int open_fds;
    char *pte;
    char *rss;
    char *vmem;
    char *name;
    char *user;
    char *state;
    char *pidstr;
    char *ioprio;
    char *thrcnt;
    char *io_read;
    char *io_write;
    char *invol_sw;
    float mempcent;
    struct process_attr *prev;
    struct process_attr *next;
    struct process_attr *left;
    struct process_attr *right;
    struct process_attr *parent;
    color_t color;
} proc_t;

typedef struct process_tree {
    proc_t *root;
    proc_t *nil;
    int ps_number;
} proc_tree_t;

proc_t *ps_list;

proc_tree_t *init_process_tree(void);

proc_tree_t *build_process_tree(sysaux_t *system);

void update_ps_tree(proc_tree_t *ps_tree, sysaux_t *system, int *redraw);

bool ps_tree_member(proc_tree_t *ps_tree, pid_t pid);

proc_t *init_proc(void);

proc_t *create_proc(char *pid, sysaux_t *system);

void filter_ps_tree(proc_tree_t *ps_tree, proc_t *ps, int *redraw);

void get_process_stats(proc_t *process, sysaux_t *system);

void free_process_list(proc_t *process_list);

proc_t *get_tail(proc_t *process_list);

proc_t *get_head(proc_t *process_list);

char *proc_user(char *path);

int get_current_pids(char **pid_list);

char *get_process_name(char *process);

bool is_valid_process(proc_t *process);

void free_ps_fields(proc_t *ps);

void insert_process(proc_tree_t *tree, proc_t *process, proc_t *new_process);

void delete_process(proc_tree_t *tree, proc_t *process, pid_t pid);

void insert_fixup(proc_tree_t *tree, proc_t *process);

void delete_fixup(proc_tree_t *tree, proc_t *process);

void left_rotate(proc_tree_t *tree, proc_t *process);

void right_rotate(proc_tree_t *tree, proc_t *process);

proc_t *min_tree(proc_tree_t *tree, proc_t *process);

void transplant_process(proc_tree_t *tree, proc_t *process_out, 
                                           proc_t *process_in);

void free_ps_tree(proc_tree_t *ps_tree);

void free_ps_tree_nodes(proc_tree_t *ps_tree, proc_t *ps);

void free_ps(proc_t *ps);

void tree_to_list(proc_tree_t *tree, proc_t *ps);

#endif
