#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include <unistd.h>
#include <ncurses.h>
#include "../src/process.h"


#define DASHBOARD "DASHBOARD"
#define REFRESH_RATE 10

#define LINE_X 2 
#define LINE_Y 9
#define PROCLN 11

enum {
    LPID   = 18,
    LUSER  = 26,
    LCPU   = 36,
    LMEM   = 40,
    LLNICE = 48,
    LMNICE = 49,
    LNNICE = 50,
    LPRIO  = 54,
    LSTATE = 63,
    LVMEM  = 69,
    LPTE   = 79,
    LRSS   = 88,
    LREAD  = 97,
    LWRITE = 111,
    LFDS   = 124,
    LINVOL = 133,
    LTHRDS = 144
};

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
#define KEY_T 116
#define KEY_V 118
#define KEY_ESCAPE 113

#define FIELDS 15

#define ALLOC_ALIGNTO 8L
#define ALLOC_ALIGN(size) (size + ALLOC_ALIGNTO - 1) & ~(ALLOC_ALIGNTO - 1)

static char const *fieldattrs[] = {"  NAME", "PID", "USER", "CPU", "MEM%%", 
                                   "NI", "IO", "ST", "VMEM", "PTE", "RES",
                                   "READ", "WRITE", "FDS", "NIVCSW", "THRS", 
                                                                           ""};

static const unsigned int fieldattr_size = ((sizeof(fieldattrs) / 
                                             sizeof( typeof(fieldattrs) )) - 1);

static const int attrspace[] = {13, 5, 5, 2, 5, 4, 5, 5, 
                                6, 6, 6, 10, 8, 6, 4, 0, 0};

uid_t euid;

void init_screen(int log_opt, char attr_sort);

void dashboard_loop(int log_opt, char attr_sort);

int update_screen(proc_t *processes, char *fstype, int plineno);

char *fieldbar_builder(void);

void add_space(char *fieldbar, char *field, int strterm, int spaces);

#endif
