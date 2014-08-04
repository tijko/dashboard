#include <pwd.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>

#include "cpu.h"
#include "process.h"


void current_procs(proc_t *procs)
{
    DIR *dir = opendir(PROC);
    struct dirent *curr = malloc(sizeof *curr);
    while ((curr = readdir(dir))) {
        if (curr->d_type == DT_DIR && is_pid(curr->d_name)) {
            procs->pidstr = curr->d_name;
            procs->pid = atoi(curr->d_name);
            name_pid(procs);
            procs->cpuset = current_cpus(procs->pid);
            proc_user(procs);
            procs->next = malloc(sizeof *(procs->next));
            procs = procs->next;
        }
    }
    closedir(dir);
    procs->next = NULL;
}

int is_pid(char *process_name)
{
    char letter;
    int strpos;
    size_t proclen;
    proclen = strlen(process_name);
    for (strpos=0; strpos < proclen; ++strpos) {
        letter = *(process_name + strpos);
        if (!isdigit(letter)) {
            return 0;
        }
    }
    return 1;
}

void name_pid(proc_t *proc)
{
    char *process_name;
    char *comm = malloc(sizeof(char) * COMMLEN);
    snprintf(comm, COMMLEN, "/proc/%s/comm", proc->pidstr);
    int ofd = open(comm, O_RDONLY);
    if (ofd == -1)
        return;
    void *buf = calloc(sizeof(char) * PROCNAME_MAX, sizeof(char));
    read(ofd, buf, 256);
    process_name = (char *) buf;
    int i;
    for (i=0; *(process_name + i) != '\n'; ++i)
        ;
    *(process_name + i) = '\0'; 
    proc->name = (char *) process_name;
    close(ofd);
} 

void proc_user(proc_t *proc)
{
    int uid;
    uid = get_uid(proc->pidstr);
    struct passwd *getuser = getpwuid(uid);
    if (getuser) {
        proc->user = malloc(sizeof(char) * strlen(getuser->pw_name) + 1);
        proc->user = strdup(getuser->pw_name);
    }
}    

int get_uid(char *pid)
{
    size_t n;
    char *ln = malloc(sizeof(char) * 32);
    char *path = malloc(sizeof(char) * 32);
    char *uidstr = malloc(sizeof(char) * 4);
    char *field = malloc(sizeof(char) * 8);
    FILE *fp;
    snprintf(path, 32, "/proc/%s/status", pid);
    fp = fopen(path, "r");
    if (fp == NULL)
        return -1;
    while (getline(&ln, &n, fp)) {
        int i;
        for (i=0; i < 3; i++)
            *(field + i) = *(ln + i);
        *(field + i) = '\0';
        if (!(strcmp("Uid", field))) {
            int l = 0;
            for (;!(isdigit(*(ln + i))); i++)
                ;
            for (;!(isspace(*(ln + i))); i++) {
                *(uidstr + l) = *(ln + i);
                l++;
            }
            *(uidstr + l) = '\0';
            free(path);
            free(field);
            free(ln);
            fclose(fp);
            return atoi(uidstr);
        }
    }
    return -1;
}   

void free_procs(proc_t *procs)
{
    proc_t *tmp;
    while (procs->next) {
        tmp = procs->next;
        free(procs);
        procs = tmp;
    }
}
