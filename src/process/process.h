#ifndef PROC_H
#define PROC_H


#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>


#define PROCNAME_MAX 1024

#define MAXPROCPATH 2048

#define COMMLEN 0x20 

//typedef struct process_attr proc_t;

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

proc_t *build_process_list(int memtotal, uid_t euid);

int get_numberof_processes(proc_t *process_list);

proc_t *create_proc(void);

void free_procs(proc_t *process_list);

bool is_pid(char *process_name);

void proc_user(proc_t *proc);

int get_field(char *pid, char *field);

void name_pid(proc_t *procs);

#endif
