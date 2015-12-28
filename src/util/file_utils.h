#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#define PATHLEN 32

#define STATUS "/status"
#define PROC "/proc/"
#define COMM "/comm"
#define FD "/fd"

#define UID "Uid"
#define PTE "VmPTE"
#define RSS "VmRSS"
#define VMEM "VmSize"
#define THRS "Threads"

char *proc_parser(char *file, char *field);

char *construct_path(int pathparts, ...);

#endif
