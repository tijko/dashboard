#define _POSIX_C_SOURCE 200890L

#include <pwd.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>

#include "process.h"
#include "../util/file_utils.h"


proc_t *build_process_list(void)
{
    proc_t *process_entry = create_proc();
    if (process_entry == NULL)
        return NULL;

    proc_t *process_list = process_entry;
 
    char process_path[MAXPROCPATH];

    process_entry->proc_no = 1;

    struct dirent *current_proc_dir;
    struct stat proc_stat;

    DIR *proc_fs_dir = opendir(PROC);

    while ((current_proc_dir = readdir(proc_fs_dir))) {

        snprintf(process_path, MAXPROCPATH - 1, "%s%s", 
                 PROC, current_proc_dir->d_name);
        stat(process_path, &proc_stat);

        if ((proc_stat.st_mode & S_IFDIR) && is_pid(current_proc_dir->d_name)) {
            // check name field for NULL...
            set_process_fields(process_entry, current_proc_dir->d_name);

            process_entry->next = create_proc();
            process_entry->next->prev = process_entry;
            process_entry = process_entry->next;
            process_entry->proc_no = process_entry->prev->proc_no + 1;
        }
    }

    closedir(proc_fs_dir);
    process_entry->prev->next = NULL;
    free(process_entry);

    return process_list; 
}

void set_process_fields(proc_t *process, char *pidstr)
{
    process->pidstr = strdup(pidstr);
    process->pid = atoi(process->pidstr);
    process->name = get_process_name(process->pidstr);
}

void update_process_list(proc_t *process_list)
{
    char process_path[MAXPROCPATH];

    struct dirent *current_proc_dir;
    struct stat proc_stat;

    DIR *proc_fs_dir = opendir(PROC);

    // grab tail...
    proc_t *process_entry = process_list;
    while (process_entry->next != NULL) 
        process_entry = process_entry->next;

    while ((current_proc_dir = readdir(proc_fs_dir))) {

        snprintf(process_path, MAXPROCPATH - 1, "%s%s", 
                 PROC, current_proc_dir->d_name);
        stat(process_path, &proc_stat);

        if ((proc_stat.st_mode & S_IFDIR) && 
             is_pid(current_proc_dir->d_name) && 
            !process_list_member(process_list, current_proc_dir->d_name)) {

            process_entry->next = create_proc();
            process_entry->next->prev = process_entry;
            process_entry = process_entry->next;
            process_entry->pidstr = strdup(current_proc_dir->d_name);
            process_entry->pid = atoi(process_entry->pidstr);
            process_entry->name = get_process_name(process_entry->pidstr);

            process_entry->proc_no = process_entry->prev->proc_no + 1;
        }
    }

    closedir(proc_fs_dir);
}

bool process_list_member(proc_t *process_list, char *pid)
{
    while (process_list != NULL) {
        if (strcmp(process_list->pidstr, pid) == 0)
            return true;
        process_list = process_list->next;
    }

    return false;
}

bool is_pid(char *process_name)
{
    unsigned int pos;

    for (pos=0; pos < strlen(process_name) && 
         isdigit(process_name[pos]); pos++);

    return pos == strlen(process_name) ? true : false;
}

char *get_process_name(char *process)
{
    size_t path_length = strlen(process) + COMM_LEN;
    char *comm = malloc(sizeof(char) * path_length + 1);

    snprintf(comm, path_length, COMM, process);

    int comm_fd = open(comm, O_RDONLY);

    if (comm_fd == -1) {
        free(comm);
        return NULL;
    }

    char *process_name = malloc(sizeof(char) * PROCNAME_MAX);
    read(comm_fd, process_name, PROCNAME_MAX - 1);

    char *newline = strchr(process_name, '\n');
    if (newline == NULL) {
        free(comm);
        free(process_name);
        close(comm_fd);
        return NULL;
    }

    *newline = '\0';

    free(comm);
    close(comm_fd);

    return process_name;
} 

char *proc_user(char *path)
{
    int uid = get_field(path, UID);
    struct passwd *getuser = getpwuid(uid);
    if (getuser == NULL)
        return NULL;
    return strdup(getuser->pw_name);
}

int get_field(char *path, char *field) // XXX fix
{
    char *field_str_value = proc_parser(path, field);

    int value = 0;

    if (field_str_value) {
        value = strtol(field_str_value, NULL, 10);
        free(field_str_value);
    }

    return value;
}

int get_numberof_processes(proc_t *process_list)
{
    while (process_list->next)
        process_list = process_list->next;
    return process_list->proc_no;
}

proc_t *create_proc(void)
{
    proc_t *p = malloc(sizeof *p);
    p->name = NULL;
    p->pidstr = NULL;
    p->user = NULL;
    p->pid = 0;
    p->uid = 0;
    p->cpuset = 0;
    p->nice = 0;
    p->open_fds = 0;
    p->invol_sw = 0;
    p->ioprio = NULL;
    p->state = NULL;
    p->mempcent = 0;
    p->vmem = 0;
    p->thrcnt = 0;
    p->pte = 0;
    p->rss = 0;
    p->io_read = 0;
    p->io_write = 0;
    p->prev = NULL;
    p->next = NULL;
    return p;
}

void free_process(proc_t *process_list)
{
    if (process_list == NULL)
        return;

    proc_t *process = process_list; 

    if (process->prev != NULL)
        process->prev->next = process->next;
    if (process->next != NULL)
        process->next->prev = process->prev;

    process->next = NULL;
    process->prev = NULL;

    free_process_list(process);
}
            
void free_process_list(proc_t *process_list)
{
    for (proc_t *tmp=process_list; process_list != NULL; tmp=process_list) {
        process_list = process_list->next;
        free(tmp->pidstr);
        free(tmp->name);
        free(tmp->user);
        free(tmp->ioprio);
        free(tmp->state);
        free(tmp);
    }
}
