#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include <stdint.h>
#include <string.h>
#include <dirent.h>


#define PATHLEN 32
#define MAXPATH 1024

#define PROC "/proc/"

#define MEMINFO "/proc/meminfo"
#define STATUS "/proc/%s/status"

#define IO_STAT "/proc/%s/io"

#define STAT "/proc/%s/stat"
#define STAT_BUFFER 4096

#define COMM "/proc/%s/comm"
#define COMM_LEN strlen(COMM)

#define FD "/proc/%s/fd"

#define UID "Uid:\t"

char *parse_stat(char *pid, int field);

char *parse_proc(char *path, char *field);

int is_pid(const struct dirent *directory);

char *strip(char *stat);

char *calculate_size(char *field_total, int byte_idx);

uint64_t value_from_string(char *ps_field_value);

#endif
