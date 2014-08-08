#define _GNU_SOURCE

#include <sched.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/resource.h>

// number of cpus sysconf(_NPROCESSORS_ONLN)

int current_cpus(int pid)
{
    int ret;
    int cpu_affinity;
    size_t mask_size;
    cpu_set_t proc_cpus;

    mask_size = sizeof proc_cpus;

    ret = sched_getaffinity(pid, mask_size, &proc_cpus);

    if (ret == -1)
        return errno;

    cpu_affinity = CPU_COUNT(&proc_cpus);

    return cpu_affinity;
}

int nice(int pid)
{
    int niceness;
    errno = 0;
    niceness = getpriority(PRIO_PROCESS, pid);
    if (errno != 0)
        return 100; // set-up an error display.
    return niceness;
}
