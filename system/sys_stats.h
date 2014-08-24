#ifndef SYSSTATS_H
#define SYSSTATS_H

#define MAXTOT 16

#define BASE 1024

#define MEMINFO "/proc/meminfo"

#define MEMFREE "MemFree"

#define SECS 60


void build_info(char *fstype);

char *mem_avail(unsigned long memory, unsigned long base);

void current_uptime(unsigned long seconds, int y, int x);

#endif
