#ifndef CPU_H
#define CPU_H

#include <stdint.h>


#define MAXPATH 32 
#define STATE 2 

int current_cpus(int pid);

int nice(int pid);

char *state(char *pidstr);

uint64_t get_process_ctxt_switches(int pid);

#endif
