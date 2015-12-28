#ifndef MEM_H
#define MEM_H

#define VMEM "VmSize"
#define TOTALMEM "MemTotal"
#define MEMINFO "/proc/meminfo"

#define MAXFILE 256

#define MINMEM 0.0

int total_memory(void);

float memory_percentage(char *pidstr, int totalmem);

#endif
