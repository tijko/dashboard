#define _POSIX_C_SOURCE 200810L

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "file_utils.h"


char *construct_path(int pathparts, ...)
{
    va_list part;
    int args;
    char *path_part, *pathname;

    pathname = NULL;
    va_start(part, pathparts);

    for (args=0; args < pathparts; args++) {
        path_part = (char *) va_arg(part, char *);
        if (pathname == NULL)
            pathname = calloc(sizeof(char) * strlen(path_part) + 1, sizeof(char));
        else 
            pathname = realloc(pathname, strlen(pathname) + strlen(path_part) + 1); // XXX fix
        strcat(pathname, path_part);
    }

    va_end(part);

    return pathname;
}

char *proc_parser(char *path, char *field)
{
    int i, j;
    FILE *fp;
    char *ln, *tmp;
    size_t fieldlen, n;

    fp = fopen(path, "r");
    fieldlen = strlen(field);

    if (fp == NULL)
        return NULL;

    n = 0;
    tmp = NULL;

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
