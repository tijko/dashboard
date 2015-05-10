#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include "../src/process.h"


#define REFRESH_RATE 10

#define LINE_X 2 

#define PROCLN 11

#define DELAY 5

#define KEY_C 99
#define KEY_D 100
#define KEY_E 101
#define KEY_I 105
#define KEY_M 109
#define KEY_N 110
#define KEY_O 111
#define KEY_P 112
#define KEY_R 114
#define KEY_S 115
#define KEY_V 118
#define KEY_ESCAPE 113

#define FIELDS 14

uid_t euid;

void init_screen(int log_opt, char attr_sort);

void dashboard_loop(int log_opt, char attr_sort);

int update_screen(proc_t *processes, char *fstype, int plineno);

char *fieldbar_builder(void);

char *add_space(char *fieldbar, char *field, int spaces, size_t max);
