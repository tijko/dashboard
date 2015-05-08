#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "memory.h"


int total_memory(void)
{
    char *mempath = "/proc/meminfo";
    char *memory = proc_parser(mempath, "MemTotal");
    if (memory != NULL) {
        int value = strtol(memory, NULL, 10);
        free(memory);
        return value;
    }
    return -1;
}

void memory_percentage(proc_t *procs, int totalmem)
{
    float total_usage;
    char *percentage;
    char *path;

    path = construct_path(3, PROC, procs->pidstr, STATUS);

    percentage = proc_parser(path, VMEM);

    if (percentage != NULL) {
        total_usage = (float) strtol(percentage, NULL, 10);
        free(percentage);
        procs->mempcent = (total_usage / totalmem) * 100;
    } else {
        procs->mempcent = MINMEM;
    }
    free(path);
}
