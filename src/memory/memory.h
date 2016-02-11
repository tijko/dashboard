#ifndef MEM_H
#define MEM_H

#include "../util/file_utils.h"

#define TOTALMEM "MemTotal:"
#define VMSIZE "VmSize:\t"
#define PTE "VmPTE:\t"

#define RSS 24
#define VMEM 23

#define MAXFILE 256

#define MINMEM 0.0

long total_memory(void);

float memory_percentage(char *path, long totalmem);

char *get_page_table_entries(char *proc_path);

char *get_resident_set_size(char *pid);

char *get_virtual_memory(char *pid);

#endif
