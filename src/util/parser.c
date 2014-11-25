#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"


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

    for (n=0, tmp=NULL; getline(&ln, &n, fp) != -1;) {
        *(ln + fieldlen) = '\0';
        if (!(strcmp(ln, field))) {
            for (i=0; !(isdigit(*(ln + i))); i++)
                ;

            tmp = malloc(sizeof(char) * PATHLEN);

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
