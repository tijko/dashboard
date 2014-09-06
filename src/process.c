#include <pwd.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include "cpu.h"
#include "disk.h"
#include "memory.h"
#include "process.h"
#include "util/parser.h"


int current_procs(proc_t *procs, int memtotal)
{
    int nproc = 0;
    proc_t *last;
    DIR *dir = opendir(PROC);
    struct dirent *curr = malloc(sizeof *curr);
    while ((curr = readdir(dir))) {
        if (curr->d_type == DT_DIR && is_pid(curr->d_name)) {
            procs->pidstr = curr->d_name;
            procs->pid = atoi(curr->d_name);
            name_pid(procs);
            if (!procs->name)
                continue;
            procs->cpuset = current_cpus(procs->pid);
            if (procs->cpuset < 1)
                continue;
            proc_user(procs);
            if (!procs->user)
                continue;
            memory_percentage(procs, memtotal);
            if (procs->mempcent == -1)
                continue;
            procs->nice = nice(procs->pid);
            if (procs->nice == 100)
                continue;
            procs->ioprio = ioprio_class(procs->pid);
            if (!procs->ioprio)
                continue;
            state(procs);
            if (!procs->state)
                continue;
            procs->pte = get_field(procs->pidstr, PTE);
            procs->rss = get_field(procs->pidstr, RSS);
            procs->vmem = get_field(procs->pidstr, VMEM);
            proc_io(procs);
            procs->next = malloc(sizeof *(procs->next));
            last = procs;
            procs = procs->next;
            procs->prev = last;
            nproc++;
        }
    }
    closedir(dir);
    if (!procs->name || procs->cpuset < 1 || !procs->user || 
        procs->mempcent == -1 || procs->nice == 100) {
        procs->prev->next = NULL;
    } else {
        procs->next = NULL;
    }
    return nproc;
}

int is_pid(char *process_name)
{
    char letter;
    signed int strpos;
    ssize_t proclen;
    proclen = strlen(process_name);
    for (strpos=0; strpos < proclen; ++strpos) {
        letter = *(process_name + strpos);
        if (!isdigit(letter)) {
            return 0;
        }
    }
    return 1;
}

void name_pid(proc_t *proc)
{
    void *buf;
    char *comm;
    char *process_name;
    comm = malloc(sizeof(char) * COMMLEN);
    snprintf(comm, COMMLEN, "/proc/%s/comm", proc->pidstr);

    int ofd = open(comm, O_RDONLY);
    if (ofd == -1) {
        free(comm);
        proc->name = NULL;
        return;
    }

    buf = calloc(sizeof(char) * PROCNAME_MAX, sizeof(char));
    read(ofd, buf, PROCNAME_MAX);
    process_name = (char *) buf;

    int i;
    for (i=0; *(process_name + i) != '\n'; ++i)
        ;

    *(process_name + i) = '\0'; 
    proc->name = (char *) process_name;
    free(comm);
    close(ofd);
} 

void proc_user(proc_t *proc)
{
    proc->uid = get_field(proc->pidstr, UID);
    struct passwd *getuser = getpwuid(proc->uid);
    if (getuser) {
        proc->user = malloc(sizeof(char) * strlen(getuser->pw_name) + 1);
        proc->user = strdup(getuser->pw_name);
        return;
    }
    proc->user = NULL;
}

int get_field(char *pid, char *field)
{
    char *path;
    char *field_str_value;
    int value;

    path = malloc(sizeof(char) * MAXPROCPATH);
    snprintf(path, MAXPROCPATH, "/proc/%s/status", pid);
    
    field_str_value = proc_parser(path, field);
    free(path);

    value = 0;
    if (field_str_value) {
        value = strtol(field_str_value, NULL, 10);
        free(field_str_value);
    }
    return value;
}
            
void free_procs(proc_t *procs)
{
    proc_t *tmp;
    while (procs->next) {
        tmp = procs->next;
        free(procs->name);
        free(procs->user);
        free(procs->ioprio);
        free(procs->state);
        free(procs);
        procs = tmp;
    }
}
