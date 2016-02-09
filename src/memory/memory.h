#ifndef MEM_H
#define MEM_H

#include "../util/file_utils.h"

#define TOTALMEM "MemTotal:"
#define VMSIZE "VmSize:\t"

#define MAXFILE 256

#define MINMEM 0.0

long total_memory(void);

float memory_percentage(char *path, long totalmem);

#endif
