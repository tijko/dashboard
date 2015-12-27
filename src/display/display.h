#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199310L
#endif

#ifndef DISPLAY_H
#define DISPLAY_H

#include <sys/types.h>

#include "../process/process.h"


#define DASHBOARD "DASHBOARD"
#define REFRESH_RATE 50

#define LINE_X 2 
#define LINE_Y 9
#define PROC_LINE_SIZE 11

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

#define FIELDS 15

#define ALLOC_ALIGNTO 8L
#define ALLOC_ALIGN(size) (size + ALLOC_ALIGNTO - 1) & ~(ALLOC_ALIGNTO - 1)

static char const *fieldattrs[] = {"  NAME", "PID", "USER", "CPU", "MEM%%", 
                                   "NI", "IO", "ST", "VMEM", "PTE", "RES",
                                   "READ", "WRITE", "FDS", "NIVCSW", "THRS", 
                                                                           ""};

static const unsigned int fieldattr_size = (sizeof fieldattrs / 
                                            sizeof( __typeof__(fieldattrs[0]) )) 
                                                                             -1;

static const int attrspace[] = {13, 5, 5, 2, 5, 4, 5, 5, 
                                6, 6, 6, 10, 8, 6, 4, 0, 0};

void init_screen(void);

int update_screen(proc_t *processes, bool sys_fields_refresh, char *fstype, 
                  char *fieldbar, int process_line_num, int max_x, int max_y);

char *build_fieldbar(void);

void add_space(char *fieldbar, char const *field, int strterm, int spaces);

#endif
