#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

#include "memory.h"


int total_memory(void)
{
    int i, j;
    char *memory = malloc(sizeof(char) * 32);
    char *mempath = "/proc/meminfo";
    FILE *fp = fopen(mempath, "r");
    char *ln = malloc(sizeof(char) * 256);
    size_t n = 256;
    getline(&ln, &n, fp);
    fclose(fp);
    for (i=0; !(isdigit(*(ln + i))); i++)
        ;
    j=0;
    while (isdigit(*(ln + i))) {
        *(memory + j) = *(ln + i);
        ++j;
        ++i;
    }
    *(memory + j) = '\0';
    free(ln); 
    return atoi(memory);
}

void memory_percentage(proc_t *procs, int totalmem)
{
    float zero = 0;
    char *ln = malloc(sizeof(char) * 256);
    size_t buf = 256;
    char *path = malloc(sizeof(char) * 256);
    snprintf(path, 256, "/proc/%d/status", procs->pid);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        procs->mempcent = (zero / totalmem) * 100; 
        return;
    }
    char *memfield = "VmSize";
    char *field = malloc(sizeof(char) * 8);
    int i;    
    long m;
    while (getline(&ln, &buf, fp) != -1) { 
        for (i=0; i < 6; i++)
            *(field + i) = *(ln + i);
        *(field + i) = '\0';
        if (!(strcmp(field, memfield))) {
            char *mem = malloc(sizeof(char) * 32);
            for (;!(isdigit(*(ln + i))); i++)
                ;     
            int l = 0;
            for (l=0; isdigit(*(ln + i)); i++, l++)
                *(mem + l) = *(ln + i);
            *(mem + l) = '\0';
            m = strtol(mem, NULL, 10);
            if (m == -1) {
                procs->mempcent = (zero / totalmem) * 100; 
            } else {
                procs->mempcent = ((float) m / totalmem) * 100;
            }
            fclose(fp);
            free(ln);
            free(path);
            free(field);
            free(mem); 
            return;
        }
    }
    procs->mempcent = (zero / totalmem) * 100; 
    fclose(fp);
    free(ln);
    free(path);
    free(field);
    return;
}
