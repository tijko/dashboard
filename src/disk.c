#include <fstab.h>
#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <sys/syscall.h>

#include "cpu.h"
#include "disk.h"


char *filesystem_type(void)
{
    int ret;

    ret = setfsent();
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
    long ioprio;
    ioprio = syscall(IOPRIO_GET, IOPRIO_WHO_PROCESS, pid);
    return ioprio;
}

char *ioprio_class(int pid)
{
    int ioprio;
    char *class;

    char *ioprio_classes[4] = {"", "Rt", "Be", "Id"};
    class = malloc(sizeof(char) * PRIOLEN);
    
    ioprio = ioprio_get(pid);
    ioprio >>= IOPRIO_SHIFT;
    if (ioprio != 0) {
        snprintf(class, PRIOLEN, "%s", ioprio_classes[ioprio]);
    } else {
        int niceness;
        int ioprio_level;

        ioprio = sched_getscheduler(pid);
        niceness = nice(pid);
        ioprio_level = (niceness + 20) / 5;
        
        if (ioprio == SCHED_FIFO || ioprio == SCHED_RR)
            snprintf(class, PRIOLEN, "%s/%d", ioprio_classes[1], ioprio_level);
        else if (ioprio == SCHED_OTHER)
            snprintf(class, PRIOLEN, "%s/%d", ioprio_classes[2], ioprio_level);
        else
            snprintf(class, PRIOLEN, "%s", ioprio_classes[3]);
    }
    return class;
}
