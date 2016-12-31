#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199310L
#endif

#ifndef DISPLAY_H
#define DISPLAY_H

#include <ncurses.h>
#include <sys/types.h>

#include "../process/process.h"


#define DASHBOARD "DASHBOARD"
#define REFRESH_RATE 50

#define LINE_X 2 
#define LINE_Y 9
#define PROC_LINE_SIZE 4 

enum {
    LPID   = 18,
    LUSER  = 26,
    LCPU   = 36,
    LLNICE = 42,
    LMNICE = 44,
    LNNICE = 43,
    LPRIO  = 49,
    LSTATE = 57,
    LVMEM  = 63,
    LRSS   = 77,
    LREAD  = 91,
    LWRITE = 109,
    LFDS   = 125,
    LINVOL = 144,
    LTHRDS = 157
};

#define DELAY 1 

#define FIELDS 14

#define ALLOC_ALIGNTO 8L
#define ALLOC_ALIGN(size) (size + ALLOC_ALIGNTO - 1) & ~(ALLOC_ALIGNTO - 1)

static char const *fieldattrs[] = {"  NAME", "PID", "USER", "CPU", 
                                   "NI", "IO", "ST", "VMEM", "RES",
                                   "READ", "WRITE", "FDS", "NIVCSW", "THRS", 
                                                                           ""};

static const unsigned int fieldattr_size = (sizeof fieldattrs / 
                                            sizeof( __typeof__(fieldattrs[0]) )) 
                                                                             -1;

static const int attrspace[] = {13, 5, 5, 5, 4, 5, 5, 10, 
                                12, 14, 10, 5, 8, 0, 0};

void init_windows(WINDOW **windows);

void update_system_window(WINDOW *system_window, sysaux *sys);

void update_process_window(WINDOW *ps_window, ps_node const *ps_list,
                           char const *fieldbar, int process_line_num, int max_y);

char *build_fieldbar(void);

#endif
