#ifndef SYSSTATS_H
#define SYSSTATS_H

void build_info(char *fstype);

char *mem_avail(unsigned long memory, unsigned long base);

void current_uptime(unsigned long seconds, int y, int x);

#endif
