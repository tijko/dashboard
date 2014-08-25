#include <stdio.h>
#include "../src/process.h"

#define LINE_X 2 

#define PROCLN 11

#define DELAY 5

#define KEY_C 99
#define KEY_E 101
#define KEY_M 109
#define KEY_N 110
#define KEY_P 112
#define KEY_R 114
#define KEY_V 118
#define KEY_ESCAPE 113

#define FIELDS 10

void init_screen(void);

void dashboard_loop(void);

void update_screen(proc_t *processes, char *fstype, int plineno);

char *fieldbar_builder(void);

char *add_space(char *fieldbar, char *field, int spaces, size_t max);
