#include <pwd.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include "ipc.h"
#include "cpu.h"
#include "disk.h"
#include "memory.h"
#include "process.h"
#include "../display/display.h"


int current_procs(proc_t *procs, int memtotal)
{
    DIR *dir;
    struct dirent *curr;

    nproc = 0;

    dir = opendir(PROC);

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

            current_fds(procs);
            procs->pte = get_field(procs->pidstr, PTE);
            procs->rss = get_field(procs->pidstr, RSS);
            procs->vmem = get_field(procs->pidstr, VMEM);

            if (euid == 0) {
                proc_io(procs);
                ctxt_switch(procs);
            } else {
                procs->io_read = 0;
                procs->io_write = 0;
                procs->invol_sw = 0;
            }

            procs->next = create_proc();
            procs->next->prev = procs;
            procs = procs->next;
            procs->proc_no = ++nproc;
        }
    }

    closedir(dir);
    procs->prev->next = NULL;
    free(procs);
 
    return nproc;
}

bool is_pid(char *process_name)
{
    unsigned int pos;
    for (pos=0; pos < strlen(process_name) && 
         isdigit(process_name[pos]); pos++);
    return pos == strlen(process_name) ? true : false;
}

void name_pid(proc_t *proc)
{
    int ofd, i;
    void *buf;
    char *comm, *process_name;

    comm = construct_path(3, PROC, proc->pidstr, COMM);

    ofd = open(comm, O_RDONLY);
    if (ofd == -1) {
        free(comm);
        proc->name = NULL;
        return;
    }

    buf = calloc(sizeof(char) * PROCNAME_MAX, sizeof(char));
    read(ofd, buf, PROCNAME_MAX);
    process_name = (char *) buf;

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
    proc->user = NULL;
    if (getuser) 
        proc->user = strdup(getuser->pw_name);
}

int get_field(char *pid, char *field)
{
    char *path, *field_str_value;
    int value;

    path = construct_path(3, PROC, pid, STATUS);
    
    field_str_value = proc_parser(path, field);
    free(path);

    value = 0;

    if (field_str_value) {
        value = strtol(field_str_value, NULL, 10);
        free(field_str_value);
    }

    return value;
}



proc_t *create_proc(void)
{
    proc_t *p = malloc(sizeof *p);
    p->name = NULL;
    p->pidstr = NULL;
    p->user = NULL;
    p->pid = 0;
    p->uid = 0;
    p->cpuset = 0;
    p->nice = 0;
    p->open_fds = 0;
    p->invol_sw = 0;
    p->ioprio = NULL;
    p->state = NULL;
    p->mempcent = 0;
    p->vmem = 0;
    p->pte = 0;
    p->rss = 0;
    p->io_read = 0;
    p->io_write = 0;
    p->prev = NULL;
    p->next = NULL;
    return p;
}
            
void free_procs(proc_t *procs)
{
    proc_t *tmp = NULL;

    while (procs) {
        if (tmp)
            free(tmp);
        free(procs->name);        
        free(procs->user);
        free(procs->ioprio);
        free(procs->state);
        tmp = procs;
        procs = procs->next;
    }

    if (tmp)
        free(tmp);
}
