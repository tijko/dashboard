#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <dirent.h>

#include "ipc.h"
#include "../util/file_utils.h"


int current_fds(char *pidstr)
{
    struct dirent *fd_file;

    char *path = construct_path(3, PROC, pidstr, FD);
   
    int open_fds = 0;
 
    DIR *fd_dir = opendir(path);
    if (fd_dir == NULL) 
        goto free_path;

    while ((fd_file = readdir(fd_dir))) {

        if (!isdigit(fd_file->d_name[0]))
            continue;
        else
            open_fds++;
    }

    closedir(fd_dir);
    
    free_path:
        free(path);

    return open_fds;
}
