#define _GNU_SOURCE

#include <stdio.h>
#include <sched.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/resource.h>

#include "cpu.h"
#include "../util/taskstats.h"
#include "../util/file_utils.h"


int current_cpus(int pid)
{
    cpu_set_t proc_cpus;

    size_t mask_size = sizeof proc_cpus;

    int ret = sched_getaffinity(pid, mask_size, &proc_cpus);

    if (ret == -1)
        return errno;

    int cpu_affinity = CPU_COUNT(&proc_cpus);

    return cpu_affinity;
}

int nice(int pid)
{
    return getpriority(PRIO_PROCESS, pid);
}

void ctxt_switch(proc_t *procs)
{
    task_req(procs, 's');
}

void state(proc_t *procs)
{
    char *field = "State";
    size_t fieldlen = strlen(field);

    char *path = construct_path(3, PROC, procs->pidstr, STATUS);

    FILE *fp = fopen(path, "r");

    if (fp == NULL) {
        free(path);
        procs->state = NULL;
        return;
    }
            
    int i;
    size_t n;

    char *proc_state = NULL, *ln = NULL;

    for (n=0; getline(&ln, &n, fp) !=0;) {
        *(ln + fieldlen) = '\0';
        if (!strcmp(ln, field)) {
            proc_state = malloc(sizeof(char) * STATE);
            for (i=0; !isspace(ln[i++]); );
            for (; !isalpha(ln[i++]); );
            *proc_state = *(ln + i - 1);
            *(proc_state + 1) = '\0';
            break;
        }
    }

    procs->state = strdup(proc_state);
    fclose(fp);
    free(ln);
    free(path);
    free(proc_state);
}    
