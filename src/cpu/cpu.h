#ifndef CPU_H
#define CPU_H

#include <stdint.h>


#define STATE 2 

#define SWITCHES "nonvoluntary_ctxt_switches:\t"

int current_cpus(int pid);

int nice(int pid);

uint64_t get_process_ctxt_switches(int pid);

char *get_user_ps_ctxt_switches(char *pid);

#endif
