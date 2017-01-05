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

// static
enum {
    LCMD  = 3,
    LPID  = 19,
    LPPID = 27,
    LUSER = 36
};

// default
enum {
    LTTY  = 50,
    LUTM  = 57,
    LSTM  = 67,
    LFDS  = 77,
    LNLWP = 90
};

// memory
enum {
    LSIZE  = 50,
    LRSS   = 58,
    LSHR   = 66,
    LLCK   = 76,
    LDATA  = 90,
    LSWAP  = 103,
    LLIB   = 117
};

// LUPTM or uptime display on proc specific screen

#define DELAY 1 

#define ALLOC_ALIGNTO 8L
#define ALLOC_ALIGN(size) (size + ALLOC_ALIGNTO - 1) & ~(ALLOC_ALIGNTO - 1)

#define ATTRSIZE(attrs) (sizeof attrs / (sizeof( __typeof__(attrs[0]) ))) - 1

static char const *default_attrs[] = { "  CMD", "PID", "PPID", "USER", "TTY",
                                       "UTIME", "STIME", "FDS","NLWP", "" };

static char const *memory_attrs[] = { "  CMD", "PID", "PPID", "USER", 
                                      "SIZE", "RSS", "SHARE", "LOCK", 
                                      "DATA", "SWAP", "LIB", "" };

static const int default_attrspace[] = { 13, 5, 5, 10, 4, 5, 5, 10, 12 };

static const int memory_attrspace[] = { 13, 5, 5, 10, 4, 5, 5, 10, 9, 10, 5 };

void init_windows(WINDOW **windows);

void update_system_window(WINDOW *system_window, sysaux *sys);

void update_default_window(WINDOW *ps_window, ps_node const *ps_list,
               char const *fieldbar, int process_line_num, int max_y);

void update_memory_window(WINDOW *ps_window, ps_node const *ps_list,
              char const *fieldbar, int process_line_num, int max_y);

char *build_default_fieldbar(void);

char *build_memory_fieldbar(void);

#endif
