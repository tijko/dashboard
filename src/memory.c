#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "memory.h"
#include "util/parser.h"


int total_memory(void)
{
    char *mempath = "/proc/meminfo";
    char *memory = proc_parser(mempath, "MemTotal");
    if (memory != NULL) {
        int value = strtol(memory, NULL, 10);
        return value;
    }
    return -1;
}

void memory_percentage(proc_t *procs, int totalmem)
{
    float total_usage = 0;
    char *path = malloc(sizeof(char) * 256);
    snprintf(path, 256, "/proc/%d/status", procs->pid);
    char *percentage = proc_parser(path, "VmSize");
    if (percentage != NULL) {
        total_usage = (float) strtol(percentage, NULL, 10);
        free(percentage);
    }
    procs->mempcent = (total_usage / totalmem) * 100;
    free(path);
}
