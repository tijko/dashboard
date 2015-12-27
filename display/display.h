#ifdef _POSIX_C_SOURCE
#undef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199310L
#endif

#ifndef DISPLAY_H
#define DISPLAY_H

#include <time.h>
#include <sys/types.h>
#include <sys/timerfd.h>

#include "../src/process.h"


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

#define TIME_READ_DEFAULT 0 

#define SYS_TIMER_EXPIRED_SEC 0
#define SYS_TIMER_EXPIRED_NSEC 0
#define SYS_TIMER_LENGTH 2000005 // XXX

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

uid_t euid;

void init_screen(char attr_sort);

void dashboard_loop(char attr_sort);

int update_screen(proc_t *processes, bool sys_fields_refresh, 
                  char *fstype, int plineno);

bool is_sysfield_timer_expired(int sys_timer_fd);

int set_sys_timer(struct itimerspec *sys_timer);

char *fieldbar_builder(void);

void add_space(char *fieldbar, char const *field, int strterm, int spaces);

#endif
