#include <stdio.h>
#include <ctype.h>
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

float memory_percentage(char *path, long totalmem)
{
    float total_usage, total_mem_percent;

    char *percentage = proc_parser(path, VMEM);

    if (percentage != NULL) {
        total_usage = (float) strtol(percentage, NULL, 10);
        free(percentage);
        total_mem_percent = (total_usage / totalmem) * 100;
    } else 
        total_mem_percent = MINMEM;
    
    return total_mem_percent;
}
