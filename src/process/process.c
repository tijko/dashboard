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
#include "../cpu/cpu.h"
#include "../ipc/ipc.h"
#include "../io/disk.h"
#include "../memory/memory.h"
#include "../util/file_utils.h"
#include "../system/sys_stats.h"


proc_t *build_process_list(sysaux_t *system)
{
    char path[STAT_PATHMAX];
    memset(path, 0, STAT_PATHMAX);

    proc_t *process = create_proc();
    if (process == NULL)
        return NULL;

    process->prev = NULL;
    proc_t *process_list = process;
    get_current_pids(system->current_pids);

    process->pidstr = strdup(system->current_pids[0]);
    process->pid = atoi(process->pidstr);
    process->name = get_process_name(process->pidstr);

    snprintf(path, STAT_PATHMAX - 1, STATUS, process->pidstr);
    process->user = proc_user(path);
    get_process_stats(process, system);
    free(system->current_pids[0]);
 
    for (int i=1; system->current_pids[i] != NULL; i++) { 
        add_process_link(process, system->current_pids[i]);
        free(system->current_pids[i]);
        process = process->next;
        get_process_stats(process, system);
    }

    return process_list; 
}

void get_process_stats(proc_t *process, sysaux_t *system)
{
    char path[STAT_PATHMAX];
    memset(path, 0, STAT_PATHMAX);
    snprintf(path, STAT_PATHMAX - 1, STATUS, process->pidstr);

    free_process_fields(process);

    process->cpuset = current_cpus(process->pid);

    process->nice = nice(process->pid);
    process->ioprio = ioprio_class(process->pid);

//    process->pte = parse_proc(path, PTE); XXX change type

    process->mempcent = memory_percentage(path, system->memtotal);
    process->state = parse_stat(process->pidstr, ST);
    process->rss = parse_stat(process->pidstr, RSS);
    process->vmem = parse_stat(process->pidstr, VMEM);
    process->thrcnt = parse_stat(process->pidstr, THRS);

    memset(path, 0, STAT_PATHMAX - 1); 
    snprintf(path, STAT_PATHMAX - 1, FD, process->pidstr);
    process->open_fds = current_fds(path);

    if (system->euid == 0) {
        uint64_t io_read = get_process_taskstat_io(process->pid, 'o');
        process->io_read = malloc(sizeof(char) * MAXFIELD);
        snprintf(process->io_read, MAXFIELD - 1, "%llu", io_read);
        uint64_t io_write = get_process_taskstat_io(process->pid, 'i');
        process->io_write = malloc(sizeof(char) * MAXFIELD);
        snprintf(process->io_write, MAXFIELD - 1, "%llu", io_write);
        uint64_t invol_sw = get_process_ctxt_switches(process->pid);
        process->invol_sw = malloc(sizeof(char) * MAXFIELD);
        snprintf(process->invol_sw, MAXFIELD - 1, "%llu", invol_sw);
    } else {
        process->io_write = get_user_ps_write(process->pidstr);
        process->io_read = get_user_ps_read(process->pidstr);
        process->invol_sw = get_user_ps_ctxt_switches(process->pidstr);
    }
}

void free_process_fields(proc_t *process)
{
    if (process->state != NULL) 
        free(process->state);
    if (process->ioprio != NULL)
        free(process->ioprio);
    if (process->thrcnt != NULL)
        free(process->thrcnt);
    if (process->vmem != NULL)
        free(process->vmem);
    if (process->rss != NULL)
        free(process->rss);
    if (process->invol_sw)
        free(process->invol_sw);
    if (process->io_read)
        free(process->io_read);
    if (process->io_write)
        free(process->io_write);
}

proc_t *update_process_list(proc_t *process_list, sysaux_t *system, int *redraw)
{
    proc_t *process_tail = get_tail(process_list);

    get_current_pids(system->current_pids);
    for (int i=0; system->current_pids[i] != NULL; i++) {
        if (!process_list_member(process_list, system->current_pids[i])) {
            add_process_link(process_tail, system->current_pids[i]);
            process_tail = process_tail->next;
            get_process_stats(process_tail, system);
            *redraw = 1;
        }

        free(system->current_pids[i]);
    }


    process_list = filter_process_list(process_list, redraw);

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

void add_process_link(proc_t *tail, char *pid)
{
    char path[STAT_PATHMAX];
    memset(path, 0, STAT_PATHMAX);
    snprintf(path, STAT_PATHMAX - 1, STATUS, pid);

    tail->next = create_proc();
    tail->next->prev = tail;
    tail->next->pidstr = strdup(pid);
    tail->next->pid = atoi(tail->next->pidstr);
    tail->next->name = get_process_name(tail->next->pidstr);
    tail->next->user = proc_user(path);
}

void get_current_pids(char **pid_list)
{
    struct dirent **proc_dir;

    int total_processes = scandir(PROC, &proc_dir, is_pid, NULL);
    if (total_processes == -1) {
        pid_list[0] = NULL;
        return;
    }

    int i = 0;
    for (; i < total_processes; i++) {
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
    char *uid = parse_proc(path, UID);
    if (uid != NULL) 
        return NULL;
    struct passwd *getuser = getpwuid(atoi(uid));
    if (getuser == NULL)
        return NULL;
    return strdup(getuser->pw_name);
}
/*
int get_field(char *path, char *field)
{
    char *field_str_value = parse_(path, field);
    if (field_str_value == NULL)
        return 0;

    int value = 0;

    if (field_str_value) {
        value = strtol(field_str_value, NULL, 10);
        free(field_str_value);
    }

    return value;
}
*/
int get_numberof_processes(proc_t *process_list)
{
    int count = 0;

    while (process_list->next) {
        count++;
        process_list = process_list->next;
    }

    return count;
}

proc_t *filter_process_list(proc_t *process_list, int *redraw)
{
    if (process_list == NULL)
        return NULL;

    proc_t *head = NULL, *current = NULL;
    proc_t *process_list_head = process_list;

    while (process_list != NULL) {

        if (is_valid_process(process_list)) {

            if (current == NULL) {
                current = copy_proc(process_list);
                current->prev = NULL;
                head = current;
            } else {
                current->next = copy_proc(process_list);
                current->next->prev = current; 
                current = current->next;
            }
        } else
            *redraw = 1;
        process_list = process_list->next;
    }

    free_process_list(process_list_head);
    current->next = NULL;

    return head;
}

bool is_valid_process(proc_t *process)
{
    if (process->state != NULL && process->pidstr != NULL &&
        process->user != NULL && process->name != NULL &&
        process->ioprio != NULL && process->thrcnt != NULL)
        return true;
    return false;
}

proc_t *copy_proc(proc_t *process)
{
    proc_t *copy = create_proc();

    if (process->ioprio)
        copy->ioprio = strdup(process->ioprio);
    if (process->pidstr != NULL)
        copy->pidstr = strdup(process->pidstr);
    if (process->state != NULL)
        copy->state = strdup(process->state);
    if (process->name)
        copy->name = strdup(process->name);
    if (process->user)
        copy->user = strdup(process->user);
    if (process->thrcnt)
        copy->thrcnt = strdup(process->thrcnt);
    if (process->vmem)
        copy->vmem = strdup(process->vmem);
    if (process->rss)
        copy->rss = strdup(process->rss);
    if (process->invol_sw)
        copy->invol_sw = strdup(process->invol_sw);
    if(process->io_read)
        copy->io_read = strdup(process->io_read);
    if (process->io_write)
        copy->io_write = strdup(process->io_write);

    copy->pid = process->pid;
    copy->uid = process->uid;
    copy->cpuset = process->cpuset;
    copy->nice = process->nice;
    copy->open_fds = process->open_fds;
    copy->mempcent = process->mempcent;
    copy->pte = process->pte;

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
    p->invol_sw = NULL;
    p->ioprio = NULL;
    p->state = NULL;
    p->mempcent = 0;
    p->vmem = NULL;
    p->thrcnt = NULL;
    p->rss = NULL;
    p->pte = 0;
    p->io_read = NULL;
    p->io_write = NULL;
    p->prev = NULL;
    p->next = NULL;
    return p;
}

/*
    XXX Keeping track of how many processes there are currently running
    and then tracing back to a non-null or end/beginning of the list

    or use a filter to rebuild on any null-attribute fields.
*/
            
void free_process_list(proc_t *process_list)
{
    for (proc_t *tmp=process_list; process_list != NULL; tmp=process_list) {
        process_list = process_list->next;
        if (tmp->pidstr)
            free(tmp->pidstr);
        if (tmp->name)
            free(tmp->name);
        if (tmp->user)
            free(tmp->user);
        if (tmp->ioprio)
            free(tmp->ioprio);
        if (tmp->state)
            free(tmp->state);
        if (tmp->thrcnt)
            free(tmp->thrcnt);
        if (tmp->vmem)
            free(tmp->vmem);
        if (tmp->rss)
            free(tmp->rss);
        if (tmp->io_read)
            free(tmp->io_read);
        if (tmp->io_write)
            free(tmp->io_write);
        if (tmp->invol_sw)
            free(tmp->invol_sw);
        free(tmp);
    }
}
