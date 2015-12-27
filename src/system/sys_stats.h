#ifndef SYSSTATS_H
#define SYSSTATS_H

#include <stdbool.h>
#include <sys/timerfd.h>


#define MAXTOT 16
#define BASE 1024

#define MEMINFO "/proc/meminfo"
#define MEMFREE "MemFree"

#define SECS 60

#define TIME_READ_DEFAULT 0 

#define SYS_TIMER_EXPIRED_SEC 0
#define SYS_TIMER_EXPIRED_NSEC 0
#define SYS_TIMER_LENGTH 2000005 // XXX

#define NRPTYS "/proc/sys/kernel/pty/nr"

void build_sys_info(char *fstype);

char *mem_avail(unsigned long memory, unsigned long base);

void current_uptime(unsigned long seconds, int y, int x);

int nr_ptys(void);

bool is_sysfield_timer_expired(int sys_timer_fd);

int set_sys_timer(struct itimerspec *sys_timer);

#endif
