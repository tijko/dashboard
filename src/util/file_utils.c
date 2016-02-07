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

    char *parse_proc_buffer = malloc(sizeof(char) * 1024);

    int rbytes = read(open_fd, parse_proc_buffer, 1024);
    close(open_fd);

    if (rbytes < 0) 
        goto error;

    size_t field_length = strlen(field);
    char *proc_field = strtok(parse_proc_buffer, "\n");
    char *field_substr = NULL;

    while (proc_field != NULL && field_substr == NULL) { 
        field_substr = strstr(proc_field, field); 
        proc_field = strtok(NULL, "\n");
    }

    if (field_substr == NULL)
        goto error;

    free(parse_proc_buffer);

    return field_substr + field_length;

error:
    free(parse_proc_buffer);
    return NULL;    
}

char *proc_parser(char *path, char *field)
{
    FILE *fp = fopen(path, "r");
    size_t fieldlen = strlen(field);

    if (fp == NULL)
        return NULL;

    size_t n = 0;
    char *tmp = NULL;

    int i, j;

    char *ln;

    for (n=0, tmp=NULL, ln=NULL; getline(&ln, &n, fp) != -1;) {
        *(ln + fieldlen) = '\0';
        if (!(strcmp(ln, field))) {
            for (i=0; !(isdigit(*(ln + i))); i++)
                ;

            tmp = malloc(sizeof(char) * PATHLEN);
            if (tmp == NULL)
                return NULL;

            for (j=0; isdigit(*(ln + i)); j++, i++)
                *(tmp + j) = *(ln + i);

            *(tmp + j) = '\0';
            break;
        }
    }

    free(ln);
    fclose(fp);

    return tmp;
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

    char *stat_str = strtok(stat_buffer, " ");
    if (stat_str == NULL)
        return NULL;

    for (int i = 1; i < field; i++)
        stat_str = strtok(NULL, " ");

    return strdup(stat_str);
}

int is_pid(const struct dirent *directory)
{
    int name_length = strlen(directory->d_name);

    for (int i=0; i < name_length; i++) 
        if(!isdigit(directory->d_name[i]))
            return 0;
    return 1;
}
