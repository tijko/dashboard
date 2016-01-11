#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "memory.h"
#include "../util/file_utils.h"


long total_memory(void)
{
    char *memory = proc_parser(MEMINFO, TOTALMEM);

    if (memory != NULL) {
        long value = strtol(memory, NULL, 10);
        free(memory);
        return value;
    }

    return -1;
}

float memory_percentage(char *pidstr, long totalmem)
{
    float total_usage, total_mem_percent;

    size_t path_length = strlen(pidstr) + STATUS_LEN;
    char *path = malloc(sizeof(char) * path_length + 1);

    snprintf(path, path_length, STATUS, pidstr);

    char *percentage = proc_parser(path, VMEM);

    if (percentage != NULL) {
        total_usage = (float) strtol(percentage, NULL, 10);
        free(percentage);
        total_mem_percent = (total_usage / totalmem) * 100;
    } else 
        total_mem_percent = MINMEM;
    
    free(path);

    return total_mem_percent;
}
