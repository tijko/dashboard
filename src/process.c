#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include "cpu.h"
#include "process.h"
/*
    List priority type e.g.(realtime, besteffort, idle, etc)
    List state type e.g.(running, interruptible, sleeping)
*/

void current_procs(proc_t *procs)
{
    DIR *dir = opendir(PROC);
    struct dirent *curr = malloc(sizeof *curr);
    while ((curr = readdir(dir))) {
        if (curr->d_type == DT_DIR && is_pid(curr->d_name)) {
            procs->pid = curr->d_name;
            name_pid(procs);
            procs->cpuset = current_cpus(procs->pid);
            procs->next = malloc(sizeof *(procs->next));
            procs = procs->next;
        }
    }
    closedir(dir);
    procs->next = NULL;
}

int is_pid(char *process_name)
{
    char letter;
    int strpos;
    size_t proclen;
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
    char *process_name;
    char *comm = malloc(sizeof(char) * COMMLEN);
    snprintf(comm, COMMLEN, "/proc/%s/comm", proc->pid);
    int ofd = open(comm, O_RDONLY);
    void *buf = calloc(sizeof(char) * PROCNAME_MAX, sizeof(char));
    read(ofd, buf, 256);
    process_name = (char *) buf;
    int i;
    for (i=0; *(process_name + i) != '\n'; ++i)
        ;
    *(process_name + i) = '\0'; 
    proc->name = (char *) process_name;
    close(ofd);
} 

void free_procs(proc_t *procs)
{
    proc_t *temp;
    while (procs->next) {
        temp = procs->next;
        free(procs);
        procs = temp;
    }
}
