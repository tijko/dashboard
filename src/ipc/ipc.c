#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include "ipc.h"
#include "../util/file_utils.h"


int current_fds(char *path)
{
    struct dirent *fd_file;

    int open_fds = 0;
 
    DIR *fd_dir = opendir(path);

    if (fd_dir == NULL) 
        return -1;

    while ((fd_file = readdir(fd_dir))) {

        if (!isdigit(fd_file->d_name[0]))
            continue;
        else
            open_fds++;
    }

    closedir(fd_dir);
    
    return open_fds;
}
