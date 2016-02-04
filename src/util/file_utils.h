#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string.h>
#include <dirent.h>


#define PATHLEN 32

#define PROC "/proc/"
#define STATUS "/proc/%s/status"

#define STAT "/proc/%s/stat"
#define STAT_BUFFER 4096

#define COMM "/proc/%s/comm"
#define COMM_LEN strlen(COMM)

#define FD "/proc/%s/fd"

#define UID "Uid"
#define PTE "VmPTE"

#define ST 3
#define THRS 20
#define VMEM 23
#define RSS  24

char *proc_parser(char *file, char *field);

char *parse_stat(char *pid, int field);

int is_pid(const struct dirent *directory);

#endif
