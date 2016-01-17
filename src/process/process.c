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

#include "process.h"
#include "../util/file_utils.h"


proc_t *build_process_list(void)
{
    proc_t *process_entry = create_proc();
    if (process_entry == NULL)
        return NULL;

    process_entry->proc_no = 1;
    proc_t *process_list = process_entry;

    char *current_pids[1000];
    get_current_pids(current_pids);
 
    int i;
    for (i=0; current_pids[i] != NULL; i++) {
        process_entry->pidstr = current_pids[i];
        set_process_fields(process_entry);
        process_entry->next = create_proc();
        process_entry->next->prev = process_entry;
        process_entry = process_entry->next;
        process_entry->proc_no = process_entry->prev->proc_no + 1;
    }

    process_entry->prev->next = NULL;
    free(process_entry);

    return process_list; 
}

void set_process_fields(proc_t *process)
{
    process->pid = atoi(process->pidstr);
    process->name = get_process_name(process->pidstr);
}

bool update_process_list(proc_t *process_list)
{
    proc_t *process_entry = get_tail(process_list);
    char *current_pids[1000]; // Set as a Macro/constant (limit?)
    get_current_pids(current_pids);

    bool alteration = false;

    int i;
    for (i=0; current_pids[i] != NULL; i++) {
        if (!process_list_member(process_list, current_pids[i])) {

            process_entry->next = create_proc();
            process_entry->next->prev = process_entry;
            process_entry = process_entry->next;
            process_entry->pidstr = current_pids[i];
            process_entry->pid = atoi(process_entry->pidstr);
            process_entry->name = get_process_name(process_entry->pidstr);

            process_entry->proc_no = process_entry->prev->proc_no + 1; // remove field

            alteration = true;
        }
    }

    for (; process_list != NULL; process_list=process_list->next) {
        for (i=0; current_pids[i] != NULL; i++) {
            if (process_list != NULL && 
                strcmp(current_pids[i], process_list->pidstr) == 0)
                break;
        }

        // XXX check for null on process_list after free_process call
        if (current_pids[i] == NULL) {
            process_list = free_process(process_list);
            alteration = true;
        }
    }

    return alteration;
}

proc_t *get_tail(proc_t *process_list)
{
    while (process_list->next != NULL)
        process_list = process_list->next;
    return process_list;
}

void get_current_pids(char **pid_list)
{
    struct dirent **proc_dir;

    int total_processes = scandir(PROC, &proc_dir, is_pid, NULL);
    if (total_processes == -1) {
        pid_list[0] = NULL;
        return;
    }

    int i;
    for (i=0; i < total_processes; i++) {
        pid_list[i] = strdup(proc_dir[i]->d_name);
        free(proc_dir[i]);
    }

    free(proc_dir);
    pid_list[i] = NULL;
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

int is_pid(const struct dirent *directory)
{
    int name_length = strlen(directory->d_name);

    int i;
    for (i=0; i < name_length; i++) 
        if(!isdigit(directory->d_name[i]))
            return 0;
    return 1;
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
    int count = 0;

    while (process_list->next) {
        count++;
        process_list = process_list->next;
    }

    return count;
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

proc_t *free_process(proc_t *process_list)
{
    if (process_list == NULL)
        return NULL;

    proc_t *process = process_list; 

    if (process->prev != NULL) {
        process_list = process->prev;
        process_list->next = process->next;
        if (process_list->next != NULL)
            process_list->next->prev = process_list;
    } else if (process->next != NULL) {
        process_list = process->next;
        process_list->prev = process->prev;
    } else
        process_list = NULL;

    process->next = NULL;
    process->prev = NULL;

    free_process_list(process);

    return process_list;
}
            
void free_process_list(proc_t *process_list)
{
    for (proc_t *tmp=process_list; process_list != NULL; tmp=process_list) {
        process_list = process_list->next;
        if (tmp->pidstr != NULL)
            free(tmp->pidstr);
        if (tmp->name != NULL)
            free(tmp->name);
        if (tmp->user != NULL)
            free(tmp->user);
        if (tmp->ioprio != NULL)
            free(tmp->ioprio);
        if (tmp->state != NULL)
            free(tmp->state);
        free(tmp);
    }
}
