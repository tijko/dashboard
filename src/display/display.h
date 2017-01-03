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
    LCPU   = 36,
    LLNICE = 42,
    LMNICE = 43,
    LNNICE = 43,
    LPRIO  = 49,
    LSTATE = 57,
    LVMEM  = 63,
    LRSS   = 77,
    LREAD  = 91,
    LWRITE = 109,
    LINVOL = 144,
    LTHRDS = 157
};

// LUPTM or uptime display on proc specific screen

enum {
    LCMD  = 3,
    LPID  = 19,
    LPPID = 27,
    LUSER = 36,
    LTTY  = 50,
    LUTM  = 57,
    LSTM  = 67,
    LFDS  = 77,
    LNLWP = 90
};


#define DELAY 1 

#define FIELDS 14

#define ALLOC_ALIGNTO 8L
#define ALLOC_ALIGN(size) (size + ALLOC_ALIGNTO - 1) & ~(ALLOC_ALIGNTO - 1)

/*
 * Separate into groups based off of screen type
 */

static char const *default_attrs[] = {"  CMD", "PID", "PPID", "USER", "TTY",
                                      "UTIME", "STIME", "FDS","NLWP", ""};

static const unsigned int default_attrsize = (sizeof default_attrs / 
                                              sizeof(
                                              __typeof__(default_attrs[0]) ))
                                                                          -1;

static const int default_attrspace[] = {13, 5, 5, 10, 4, 5, 5, 10, 12};

void init_windows(WINDOW **windows);

void update_system_window(WINDOW *system_window, sysaux *sys);

void update_process_window(WINDOW *ps_window, ps_node const *ps_list,
                           char const *fieldbar, int process_line_num, int max_y);

char *build_fieldbar(void);

#endif
