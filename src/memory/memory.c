#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#include "memory.h"
#include "../util/file_utils.h"


long total_memory(void)
{
    char *memory = parse_proc(MEMINFO, TOTALMEM);

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

    char *percentage = parse_proc(path, VMSIZE);
   

    if (percentage != NULL) {
        total_usage = (float) strtol(percentage, NULL, 10);
        free(percentage);
        total_mem_percent = (total_usage / totalmem) * 100;
    } else 
        total_mem_percent = MINMEM;
    
    return total_mem_percent;
}

char *get_page_table_entries(char *proc_path)
{
    char *pte = parse_proc(proc_path, PTE);

    return pte;    
}

char *get_resident_set_size(char *pid)
{
    char *rss = parse_stat(pid, RSS);

    if (rss == NULL)
        return NULL;

    char *rss_conv = calculate_size(rss, 0);

    return rss_conv;
}

char *get_virtual_memory(char *pid)
{
    char *vm = parse_stat(pid, VMEM);

    if (vm == NULL)
        return NULL;

    char *vm_conv = calculate_size(vm, 0);

    return vm_conv;
}
