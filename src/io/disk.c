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
#include "../util/file_utils.h"


static char *ioprio_classes[4] = {"", "Rt", "Be", "Id"};

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

uint64_t get_process_taskstat_io(int pid, int conn, char field)
{
    return task_req(pid, conn, field);
} 

char *get_user_ps_write(char *pid)
{
    char path[MAXPATH];
    snprintf(path, MAXPATH, IO_STAT, pid);

    char *wbytes = parse_proc(path, PROC_WRITE);

    if (wbytes == NULL)
        return NULL;

    char *write_conv = calculate_size(wbytes, 0);

    return write_conv;
}

char *get_user_ps_read(char *pid)
{
    char path[MAXPATH];
    snprintf(path, MAXPATH, IO_STAT, pid);

    char *rbytes = parse_proc(path, PROC_READ);

    if (rbytes == NULL)
        return NULL;

    char *read_conv = calculate_size(rbytes, 0);

    return read_conv;
}
