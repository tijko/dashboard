#define _POSIX_C_SOURCE 200810L

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "file_utils.h"


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
