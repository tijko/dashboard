#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <string.h>


#define PATHLEN 32

#define PROC "/proc/"
#define STATUS "/proc/%s/status"

#define COMM "/proc/%s/comm"
#define COMM_LEN strlen(COMM)

#define FD "/proc/%s/fd"

#define UID "Uid"
#define PTE "VmPTE"
#define RSS "VmRSS"
#define VMEM "VmSize"
#define THRS "Threads"

char *proc_parser(char *file, char *field);

#endif
