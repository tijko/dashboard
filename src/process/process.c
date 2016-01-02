#define _POSIX_C_SOURCE 200890L

#include <pwd.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "process.h"
#include "../ipc/ipc.h"
#include "../cpu/cpu.h"
#include "../io/disk.h"
#include "../memory/memory.h"
#include "../util/file_utils.h"


proc_t *build_process_list(int memtotal, uid_t euid)
{
    proc_t *process_list = create_proc();
    if (process_list == NULL)
        return NULL;

    process_list->prev = NULL;
    proc_t *head = process_list;
 
    char process_path[MAXPROCPATH];

    int nproc = 0;

    struct dirent *curr;
    struct stat currp;

    DIR *dir = opendir(PROC);

    while ((curr = readdir(dir))) {

        snprintf(process_path, MAXPROCPATH - 1, "%s%s", PROC, curr->d_name);
        stat(process_path, &currp);

        if ((currp.st_mode & S_IFDIR) && is_pid(curr->d_name)) {
            process_list->pidstr = curr->d_name;
            process_list->pid = atoi(curr->d_name);

            process_list->name = get_process_name(process_list);
            if (!process_list->name)
                continue;

            process_list->cpuset = current_cpus(process_list->pid);
            if (process_list->cpuset < 1) 
                continue;

            proc_user(process_list);
            if (!process_list->user) 
                continue;

            process_list->mempcent = memory_percentage(process_list->pidstr, memtotal);
            if (process_list->mempcent == -1)
                continue;

            process_list->nice = nice(process_list->pid);
            if (process_list->nice == 100)
                continue;

            process_list->ioprio = ioprio_class(process_list->pid);
            if (!process_list->ioprio) 
                continue;

            process_list->state = state(process_list->pidstr);
            if (!process_list->state) 
                continue;

            process_list->open_fds = current_fds(process_list->pidstr);
            process_list->pte = get_field(process_list->pidstr, PTE);
            process_list->rss = get_field(process_list->pidstr, RSS);
            process_list->vmem = get_field(process_list->pidstr, VMEM);
            process_list->thrcnt = get_field(process_list->pidstr, THRS);

            if (euid == 0) {
                process_list->io_read = get_process_taskstat_io(process_list->pid, 'o');
                process_list->io_write = get_process_taskstat_io(process_list->pid, 'i');
                process_list->invol_sw = get_process_ctxt_switches(process_list->pid);
            }

            process_list->next = create_proc();
            process_list->next->prev = process_list;
            process_list = process_list->next;
            process_list->proc_no = ++nproc;
        }
    }

    closedir(dir);
    process_list->prev->next = NULL;
    free(process_list);

    return head; 
}

bool is_pid(char *process_name)
{
    unsigned int pos;

    for (pos=0; pos < strlen(process_name) && 
         isdigit(process_name[pos]); pos++);

    return pos == strlen(process_name) ? true : false;
}

char *get_process_name(proc_t *proc)
{
    char *comm = construct_path(3, PROC, proc->pidstr, COMM);

    int comm_fd = open(comm, O_RDONLY);

    if (comm_fd == -1) {
        free(comm);
        return NULL;
    }

    char *process_name = malloc(sizeof(char) * PROCNAME_MAX);
    read(comm_fd, process_name, PROCNAME_MAX - 1);

    char *newline = strchr(process_name, '\n');
    if (newline == NULL)
        return NULL;

    *newline = '\0';

    free(comm);
    close(comm_fd);

    return process_name;
} 

void proc_user(proc_t *proc)
{
    proc->uid = get_field(proc->pidstr, UID);
    struct passwd *getuser = getpwuid(proc->uid);
    proc->user = NULL;
    if (getuser) 
        proc->user = strdup(getuser->pw_name);
}

int get_field(char *pid, char *field) // XXX fix
{
    char *path, *field_str_value;
    int value;

    path = construct_path(3, PROC, pid, STATUS);
    
    field_str_value = proc_parser(path, field);
    free(path);

    value = 0;

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
            
void free_procs(proc_t *process_list)
{
    for (proc_t *tmp=process_list; process_list != NULL; tmp=process_list) {
        process_list = process_list->next;
        free(tmp->name);
        free(tmp->user);
        free(tmp->ioprio);
        free(tmp->state);
        free(tmp);
    }
}
