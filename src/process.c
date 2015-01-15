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
#include "../display/display.h"


int current_procs(proc_t *procs, int memtotal)
{
    int nproc;
    DIR *dir;
    proc_t *last;
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

            if (egid == 0) {
                proc_io(procs);
            } else {
                procs->io_read = 0;
                procs->io_write = 0;
            }

            procs->next = malloc(sizeof *(procs->next));

            last = procs;
            procs = procs->next;
            procs->prev = last;
            nproc++;
        }
    }

    closedir(dir);

    free(last->next);
    last->next = NULL;
 
    return nproc;
}

bool is_pid(char *process_name)
{
    char letter;
    signed int strpos;
    ssize_t proclen;

    proclen = strlen(process_name);
    for (strpos=0; strpos < proclen; ++strpos) {
        letter = *(process_name + strpos);
        if (!isdigit(letter)) 
            return false;
    }

    return true;
}

void name_pid(proc_t *proc)
{
    int ofd, i;
    void *buf;
    char *comm, *process_name;

    comm = malloc(sizeof(char) * COMMLEN);
    snprintf(comm, COMMLEN, "/proc/%s/comm", proc->pidstr);

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

void current_fds(proc_t *proc)
{
    char *path;

    struct dirent *fd_file;
    DIR *fd_dir;

    path = malloc(sizeof(char) * MAXPROCPATH);
    snprintf(path, MAXPROCPATH, "/proc/%s/fd/", proc->pidstr);
   
    proc->open_fds = 0;
 
    fd_dir = opendir(path);
    if (fd_dir == NULL) 
        goto free_path;

    while ((fd_file = readdir(fd_dir))) {
        if (!isdigit(fd_file->d_name[0]))
            continue;
        else
            proc->open_fds++;
    }

    closedir(fd_dir);
    
    free_path:
        free(path);

    return;
}
            
void free_procs(proc_t *procs)
{
    proc_t *tmp;
    tmp = procs->next;

    for (; procs->next; tmp=procs->next) {
        free(procs->name);
        free(procs->user);
        free(procs->ioprio);
        free(procs->state);
        free(procs);
        procs = tmp;
    }
}
