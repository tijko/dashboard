#define _GNU_SOURCE

#include <stdio.h>
#include <fstab.h>
#include <sched.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/resource.h>

#include "disk.h"
#include "../cpu/cpu.h"
#include "../util/taskstats.h"


char *ioprio_classes[4] = {"", "Rt", "Be", "Id"};

char *filesystem_type(void)
{
    int ret = setfsent();
    if (ret != 1)
        return NULL;

    struct fstab *file_system = getfsent();
    endfsent();
    if (!file_system)
        return NULL;
    return file_system->fs_vfstype;
}

int ioprio_get(int pid)
{
    return syscall(IOPRIO_GET, IOPRIO_WHO_PROCESS, pid);
}

char *ioprio_class(int pid)
{
    char *class = NULL;

    int ioprio = ioprio_get(pid);
    if (ioprio == -1)
        return NULL;

    if (ioprio >> IOPRIO_CLASS_SHIFT != 0) { 
        class = malloc(sizeof(char) * PRIOLEN);
        snprintf(class, PRIOLEN, "%s/%ld", 
                 ioprio_classes[ioprio >> IOPRIO_CLASS_SHIFT], 
                 ioprio & IOPRIO_PRIO_MASK);
    } else 
        class = ioprio_class_nice(pid);

    return class;
}

char *ioprio_class_nice(int pid)
{
    char *class = malloc(sizeof(char) * PRIOLEN);
    if (class == NULL)
        return NULL;
        
    int prio = sched_getscheduler(pid);
    int niceness = getpriority(PRIO_PROCESS, pid);
    int ioprio_level = (niceness + 20) / 5;
    
    if (prio == SCHED_FIFO || prio == SCHED_RR)
        snprintf(class, PRIOLEN, "%s/%d", ioprio_classes[1], ioprio_level);
    else if (prio == SCHED_OTHER)
        snprintf(class, PRIOLEN, "%s/%d", ioprio_classes[2], ioprio_level);
    else
        snprintf(class, PRIOLEN, "%s", ioprio_classes[3]);
    return class;
}

uint64_t get_process_taskstat_io(int pid, char field)
{
    return task_req(pid, field);
}    
