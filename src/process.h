#ifndef PROC_H
#define PROC_H

#define PROCNAME_MAX 1024

#define MAXPROCPATH 2048

#define PROC "/proc"

#define COMMLEN 0x20 

#define UID "Uid"
#define VMEM "VmSize"
#define PTE "VmPTE"
#define RSS "VmRSS"

typedef struct process_attr proc_t;

struct process_attr {
    char *name;
    char *pidstr;
    char *user;
    int pid;
    int uid;
    int cpuset;
    int nice;
    char *ioprio;
    char *state;
    float mempcent;
    int vmem;
    int pte;
    int rss;
    proc_t *prev;
    proc_t *next;
};

int current_procs(proc_t *procs, int memtotal);

void free_procs(proc_t *procs);

int is_pid(char *process_name);

void proc_user(proc_t *proc);

int get_field(char *pid, char *field);

void name_pid(proc_t *procs);

#endif
