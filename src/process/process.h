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

typedef struct process_attr {
    char *name;
    char *pidstr;
    char *user;
    int pid;
    int uid;
    int cpuset;
    int nice;
    int open_fds;
    char *ioprio;
    char *state;
    float mempcent;
    char *pte;
    char *rss;
    char *vmem;
    char *thrcnt;
    char *io_read;
    char *io_write;
    char *invol_sw;
    struct process_attr *prev;
    struct process_attr *next;
} proc_t;

proc_t *build_process_list(sysaux_t *system);

proc_t *update_process_list(proc_t *process_list, sysaux_t *system, int *redraw);

bool process_list_member(proc_t *process_list, char *pid);

int get_numberof_processes(proc_t *process_list);

proc_t *create_proc(void);

proc_t *filter_process_list(proc_t *process_list, int *redraw);

void get_process_stats(proc_t *process, sysaux_t *system);

void free_process_list(proc_t *process_list);

proc_t *copy_proc(proc_t *process_list);

proc_t *get_tail(proc_t *process_list);

proc_t *get_head(proc_t *process_list);

char *proc_user(char *path);

void add_process_link(proc_t *link, char *pid);

void get_current_pids(char **pid_list);

char *get_process_name(char *process);

bool is_valid_process(proc_t *process);

void free_process_fields(proc_t *process);

#endif
