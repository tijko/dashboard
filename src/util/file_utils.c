#define _POSIX_C_SOURCE 200810L

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

#include "file_utils.h"


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
