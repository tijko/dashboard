#ifndef PROC_H
#define PROC_H


#include <stdint.h>
#include <dirent.h>
#include <stdbool.h>


#define PROCNAME_MAX 1024

#define MAXPROCPATH 2048


typedef struct process_attr {
    char *name;
    char *pidstr;
    char *user;
    int pid;
    int uid;
    int cpuset;
    int nice;
    int open_fds;
    int invol_sw;
    int proc_no;
    char *ioprio;
    char *state;
    float mempcent;
    int vmem;
    int pte;
    int rss;
    int thrcnt;
    uint64_t io_read;
    uint64_t io_write;
    struct process_attr *prev;
    struct process_attr *next;
} proc_t;

proc_t *build_process_list(void);

bool update_process_list(proc_t *process_list);

bool process_list_member(proc_t *process_list, char *pid);

int get_numberof_processes(proc_t *process_list);

proc_t *create_proc(void);

void free_process_list(proc_t *process_list);

proc_t *free_process(proc_t *process_list);

proc_t *get_tail(proc_t *process_list);

int is_pid(const struct dirent *directory);

char *proc_user(char *path);

void set_process_fields(proc_t *process);

void get_current_pids(char **pid_list);

int get_field(char *path, char *field);

char *get_process_name(char *process);

#endif
