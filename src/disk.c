#include <fstab.h>
#include <ncurses.h>


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

//
