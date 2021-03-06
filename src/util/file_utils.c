#define _POSIX_C_SOURCE 200810L

#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "file_utils.h"


char *parse_proc(char *path, char *field)
{
    int open_fd = open(path, O_RDONLY);
    if (open_fd < 0)
        return NULL;

    char parse_proc_buffer[PROC_SIZE];
    memset(parse_proc_buffer, '\0', PROC_SIZE - 1);

    char *delimiter = "\n";

    int rbytes = read(open_fd, parse_proc_buffer, PROC_SIZE - 1);
    close(open_fd);

    if (rbytes < 0) 
        return NULL;

    parse_proc_buffer[PROC_SIZE - 1] = '\0';

    size_t field_length = strlen(field);

    char *proc_field = strtok(parse_proc_buffer, delimiter);

    if (!proc_field)
        return NULL;

    char *field_substr = NULL;

    while (proc_field != NULL && field_substr == NULL) { 
        field_substr = strstr(proc_field, field); 
        proc_field = strtok(NULL, delimiter);
    }

    if (field_substr == NULL)
        return NULL;

    char *field_str = strip(field_substr + field_length);

    return field_str;
}

char *parse_stat(char *pid, int field)
{
    char path[PATHLEN];
    snprintf(path, PATHLEN - 1, STAT, pid);

    char stat_buffer[STAT_BUFFER];

    int stat_fd = open(path, O_RDONLY);
    if (stat_fd < 0)
        return NULL;

    int rbytes = read(stat_fd, stat_buffer, STAT_BUFFER);
    close(stat_fd);
    if (rbytes < 0)
        return NULL;

    char *delimiter = " ";
    char *stat_str = strtok(stat_buffer, delimiter);
    if (stat_str == NULL)
        return NULL;

    for (int i = 1; i < field; i++)
        stat_str = strtok(NULL, delimiter);

    char *ps_stat = strdup(stat_str);

    return ps_stat;
}

int is_pid(const struct dirent *directory)
{
    int name_length = strlen(directory->d_name);

    for (int i=0; i < name_length; i++) 
        if(!isdigit(directory->d_name[i]))
            return 0;
    return 1;
}

char *strip(char *stat)
{
    char *stripped = malloc(sizeof(char) * strlen(stat) + 1);

    int hit_value = 0;    
    int idx;

    for (idx=0; *stat; ) {
        if (isdigit(*stat)) {
            *(stripped + idx++) = *stat;
            hit_value = 1;
        }

        if (!isdigit(*stat++) && hit_value)
            break;
    }

    *(stripped + idx) = '\0';

    return stripped;
}

char *calculate_size(char *field_total, int byte_idx)
{
    char *byte_totals[] = {"B", "KiB", "MiB", "GiB"};

    long double field_value = (long double) strtoll(field_total, NULL, 10);

    for (; byte_idx < 4 && field_value > 1024; byte_idx++)
        field_value /= 1024;

    free(field_total);

    char *field_str = malloc(sizeof(char) * 16);

    snprintf(field_str, 15, "%.2Lf %s", field_value, byte_totals[byte_idx]);

    return field_str;
}

uint64_t value_from_string(char *ps_field_value)
{
    char *field_value = strdup(ps_field_value);    
    char *value = strtok(field_value, " "); 
    char *size = strtok(NULL, " ");

    int mul = 0;
    char *byte_totals[] = {"B", "KiB", "MiB", "GiB"};

    if (size != NULL) {
        for (int i=0; i < 4; i++) {
            if (!strcmp(byte_totals[i], size)) {
                mul = i;
                break;
            }
        }
    }

    uint64_t field = (uint64_t) strtod(value, NULL);

    for (; mul > 0; mul--) {
        field *= 1024;
    }

    free(field_value);

    return field;
}

