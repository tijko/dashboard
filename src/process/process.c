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

    proc_t *process_list = process_entry;

    char *current_pids[MAX_PIDS];
    get_current_pids(current_pids);
 
    int i;
    for (i=0; current_pids[i] != NULL; i++) {
        process_entry->pidstr = current_pids[i];
        set_process_fields(process_entry);
        process_entry->next = create_proc();
        process_entry->next->prev = process_entry;
        process_entry = process_entry->next;
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

proc_t *update_process_list(proc_t *process_list, int *redraw)
{
    proc_t *process_entry = get_tail(process_list);
    char *current_pids[MAX_PIDS];
    memset(current_pids, 0, sizeof(char *) * MAX_PIDS);
    get_current_pids(current_pids);

    int i;
    for (i=0; current_pids[i] != NULL; i++) {
        if (!process_list_member(process_list, current_pids[i])) {

            process_entry->next = create_proc();
            process_entry->next->prev = process_entry;
            process_entry = process_entry->next;
            process_entry->pidstr = current_pids[i];
            process_entry->pid = atoi(process_entry->pidstr);
            process_entry->name = get_process_name(process_entry->pidstr);

            *redraw = 1;
        }
    }

    process_list = filter_process_list(process_list);

    return process_list;
}

proc_t *get_tail(proc_t *process_list)
{
    while (process_list->next != NULL)
        process_list = process_list->next;
    return process_list;
}

proc_t *get_head(proc_t *process_list)
{
    while (process_list->prev != NULL)
        process_list = process_list->prev;
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

int get_field(char *path, char *field)
{
    char *field_str_value = proc_parser(path, field);
    if (field_str_value == NULL)
        return 0;

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

proc_t *filter_process_list(proc_t *process_list)
{
    if (process_list == NULL)
        return NULL;

    proc_t *head = NULL, *current = NULL;
    proc_t *process_list_head = process_list;

    while (process_list != NULL) {

        if (process_list->state != NULL && 
            process_list->ioprio != NULL &&
            process_list->name != NULL &&
            process_list->pidstr != NULL &&
            process_list->user != NULL) {

            if (current == NULL) {
                current = copy_proc(process_list);
                current->prev = NULL;
                head = current;
            } else {
                current->next = copy_proc(process_list);
                current->next->prev = current; 
                current = current->next;
            }
        }

        process_list = process_list->next;
    }

    current->next = NULL;

    free_process_list(process_list_head);

    return head;
}

proc_t *copy_proc(proc_t *process)
{
    proc_t *copy = create_proc();

    copy->ioprio = strdup(process->ioprio);
    copy->pidstr = strdup(process->pidstr);
    copy->state = strdup(process->state);
    copy->name = strdup(process->name);
    copy->user = strdup(process->user);

    copy->pid = process->pid;
    copy->uid = process->uid;
    copy->cpuset = process->cpuset;
    copy->nice = process->nice;
    copy->open_fds = process->open_fds;
    copy->invol_sw = process->invol_sw;
    copy->mempcent = process->mempcent;
    copy->vmem = process->vmem;
    copy->thrcnt = process->thrcnt;
    copy->pte = process->pte;
    copy->rss = process->rss;
    copy->io_read = process->io_read;
    copy->io_write = process->io_write;

    return copy;
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
    /*
        XXX Keeping track of how many processes there are currently running
        and then tracing back to a non-null or end/beginning of the list

        or use a filter to rebuild on any null-attribute fields.
    */

    if (process_list == NULL)
        return NULL;

    if (process_list->next == NULL) {
        process_list->prev = NULL;
        free_process_list(process_list);
        return NULL;
    }

    proc_t *process = process_list; 
    process_list = process_list->next;
    if (process_list->prev->prev) {
        process_list->prev = process_list->prev->prev;
        process_list->prev->next = process_list;
    } else
        process_list->prev = NULL;

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
