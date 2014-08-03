#include <fstab.h>
#include <ncurses.h>


char *filesystem_type(void)
{
    struct fstab *file_system = getfsent();
    if (!file_system)
        return NULL;
    return file_system->fs_vfstype;
}
