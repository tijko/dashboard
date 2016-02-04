#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include "ipc.h"
#include "../util/file_utils.h"


int current_fds(char *path)
{
    struct dirent **fd_files;

    int open_fds = scandir(path, &fd_files, is_pid, NULL);
 
    if (open_fds < 0)
        return -1;

    for (int i=0; i < open_fds; i++)
        free(fd_files[i]);
    free(fd_files);

    return open_fds;
}
