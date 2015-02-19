#define _GNU_SOURCE

#include <stdio.h>
#include <sched.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/resource.h>

#include "cpu.h"
#include "process.h"

int current_cpus(int pid)
{
    int ret, cpu_affinity;
    size_t mask_size;
    cpu_set_t proc_cpus;

    mask_size = sizeof proc_cpus;

    ret = sched_getaffinity(pid, mask_size, &proc_cpus);

    if (ret == -1)
        return errno;

    cpu_affinity = CPU_COUNT(&proc_cpus);

    return cpu_affinity;
}

int nice(int pid)
{
    int niceness;
    niceness = getpriority(PRIO_PROCESS, pid);
    return niceness;
}

void state(proc_t *procs)
{
    int i;
    FILE *fp;
    char *ln, *path, *field;
    char *proc_state = NULL;
    size_t n, fieldlen;

    field = "State";
    fieldlen = strlen(field);
    path = malloc(sizeof(char) * MAXPATH);

    snprintf(path, MAXPATH, "/proc/%s/status", procs->pidstr);
    fp = fopen(path, "r");
            
    for (n=0; getline(&ln, &n, fp) !=0;) {
        *(ln + fieldlen) = '\0';
        if (!strcmp(ln, field)) {
            proc_state = malloc(sizeof(char) * STATE);
            for (i=0; !isspace(*(ln + i)); i++)
                ;
            for (; !isalpha(*(ln + i)); i++)
                ;
            *proc_state = *(ln + i);
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
