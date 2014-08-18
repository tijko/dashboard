#include "process.h"

#define VMEM "VmSize"
#define MAXFILE 256

#define MINMEM 0.0

int total_memory(void);

void memory_percentage(proc_t *procs, int totalmem);
