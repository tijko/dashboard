#include "process.h"

#define MAXPATH 32

int current_cpus(int pid);

int nice(int pid);

void state(proc_t *procs);
