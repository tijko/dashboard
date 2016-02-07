#define _GNU_SOURCE

#include <stdio.h>
#include <sched.h>
#include <errno.h>
#include <stdint.h>
#include <sys/resource.h>

#include "cpu.h"
#include "../util/taskstats.h"
#include "../util/file_utils.h"


int current_cpus(int pid)
{
    cpu_set_t proc_cpus;
    size_t mask_size = sizeof proc_cpus;

    int ret = sched_getaffinity(pid, mask_size, &proc_cpus);

    if (ret == -1)
        return errno;

    int cpu_affinity = CPU_COUNT(&proc_cpus);

    return cpu_affinity;
}

int nice(int pid)
{
    return getpriority(PRIO_PROCESS, pid);
}

uint64_t get_process_ctxt_switches(int pid)
{
    return task_req(pid, 's');
}

char *get_user_ps_ctxt_switches(char *pid)
{
    char path[MAXPATH];
    snprintf(path, MAXPATH, STATUS, pid);

    return parse_proc(path, SWITCHES);
}
