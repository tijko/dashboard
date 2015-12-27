#ifndef CPU_H
#define CPU_H

#include "../process/process.h"

#define MAXPATH 32 
#define STATE 2 

int current_cpus(int pid);

int nice(int pid);

void state(proc_t *procs);

void ctxt_switch(proc_t *procs);

#endif
