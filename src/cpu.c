#define _GNU_SOURCE

#include <sched.h>
#include <errno.h>
#include <stdlib.h>

// number of cpus sysconf(_NPROCESSORS_ONLN)

int current_cpus(char *pid)
{
    int ret;
    int cpu_affinity;
    int pid_int;
    size_t mask_size;
    cpu_set_t proc_cpus;

    pid_int = atoi(pid);
    mask_size = sizeof proc_cpus;

    ret = sched_getaffinity(pid_int, mask_size, &proc_cpus);

    if (ret == -1)
        return errno;

    cpu_affinity = CPU_COUNT(&proc_cpus);

    return cpu_affinity;
}
