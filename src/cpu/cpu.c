#define _GNU_SOURCE

#include <stdio.h>
#include <sched.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
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

uint64_t get_process_ctxt_switches(int pid)
{
    return task_req(pid, 's');
}

char *state(char *path)
{
    char *field = "State";
    size_t fieldlen = strlen(field);

    FILE *fp = fopen(path, "r");

    if (fp == NULL) 
        return NULL;
            
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

    fclose(fp);
    free(ln);
    
    return proc_state;
}    
