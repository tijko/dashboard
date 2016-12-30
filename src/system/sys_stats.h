#ifndef SYSSTATS_H
#define SYSSTATS_H

#include <stdbool.h>
#include <ncurses.h>
#include <sys/timerfd.h>

#include "../util/file_utils.h"


#define MAXTOT 16
#define BASE 1024

#define MAXPIDS "/proc/sys/kernel/pid_max"

#define MEMFREE "MemAvailable:"

#define SECS 60

#define TIME_READ_DEFAULT 0 

#define SYS_TIMER_EXPIRED_SEC 0
#define SYS_TIMER_EXPIRED_NSEC 0
#define SYS_TIMER_LENGTH 1 

#define PTY_BUFFER_SZ 64
#define NRPTYS "/proc/sys/kernel/pty/nr"

typedef struct {
    char *fstype;
    uid_t euid;
    int max_pids;
    long memtotal;
} sysaux;

void build_sys_info(WINDOW *system_window, char *fstype);

char *mem_avail(unsigned long memory, unsigned long base);

void current_uptime(WINDOW *system_window, unsigned long seconds, int y, int x);

int nr_ptys(void);

bool is_sysfield_timer_expired(int sys_timer_fd);

int set_sys_timer(struct itimerspec *sys_timer);

int max_pids(void);

#endif
