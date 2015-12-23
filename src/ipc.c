#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <dirent.h>

#include "ipc.h"
#include "util/file_utils.h"


void current_fds(proc_t *proc)
{
    char *path;

    struct dirent *fd_file;
    DIR *fd_dir;

    path = construct_path(3, PROC, proc->pidstr, FD);
   
    proc->open_fds = 0;
 
    fd_dir = opendir(path);
    if (fd_dir == NULL) 
        goto free_path;

    while ((fd_file = readdir(fd_dir))) {
        if (!isdigit(fd_file->d_name[0]))
            continue;
        else
            proc->open_fds++;
    }

    closedir(fd_dir);
    
    free_path:
        free(path);
}
