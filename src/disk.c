#include <fstab.h>
#include <unistd.h>
#include <ncurses.h>
#include <sys/syscall.h>

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
