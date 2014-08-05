#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "memory.h"
#include "util/procparse.h"


int total_memory(void)
{
    char *mempath = "/proc/meminfo";
    void *memory = proc_parser(mempath, "MemTotal");
    if (memory)
        return *(int *) memory;
    return -1;
}

void memory_percentage(proc_t *procs, int totalmem)
{
    float total_usage = 0;
    char *path = malloc(sizeof(char) * 256);
    snprintf(path, 256, "/proc/%d/status", procs->pid);
    void *percentage = proc_parser(path, "VmSize");
    if (percentage)
        total_usage = *(float *) percentage;
    procs->mempcent = (total_usage / totalmem) * 100;
}
