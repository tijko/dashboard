#include <stdio.h>
#include "../src/process.h"

#define LINE_X 2 

void init_screen(void);

void dashboard_loop(void);

void update_screen(proc_t *processes, char *fstype, int plineno);

char *fieldbar_builder(void);

char *add_space(char *fieldbar, char *field, int spaces, size_t max);
