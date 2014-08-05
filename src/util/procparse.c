#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>


void *proc_parser(char *path, char *field)
{
    int i, j;
    FILE *fp;
    char *ln;
    size_t n;
    char *tmp;
    void *value;
    size_t fieldlen;

    n = 0;
    fp = fopen(path, "r");
    fieldlen = strlen(field);
    tmp = malloc(sizeof(char) * 32);

    if (fp == NULL)
        return NULL;

    while (getline(&ln, &n, fp) != -1) {
        *(ln + fieldlen) = '\0';
        if (!(strcmp(ln, field))) {
            for (i=0; !(isdigit(*(ln + i))); i++)
                ;
            for (j=0; isdigit(*(ln + i)); j++, i++)
                *(tmp + j) = *(ln + i);
            *(tmp + j) = '\0';
            value = tmp;
            break;
        }
    }
    fclose(fp);
    return value;
}
