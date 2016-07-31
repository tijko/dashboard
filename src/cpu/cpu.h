#ifndef CPU_H
#define CPU_H

#include <stdint.h>

#include "../util/taskstats.h"

#define STATE 3
#define THREADS 20

#define SWITCHES "nonvoluntary_ctxt_switches:\t"

int current_cpus(int pid);

int nice(int pid);

uint64_t get_process_ctxt_switches(int pid, struct nl_session *nls);

char *get_user_ps_ctxt_switches(char *pid);

char *get_state(char *pid);

char *get_thread_count(char *pid);

#endif
