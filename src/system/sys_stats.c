#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <mntent.h>
#include <stdbool.h>
#include <ncurses.h>
#include <sys/sysinfo.h>

#include "sys_stats.h"
#include "../util/file_utils.h"


char *mem_avail(unsigned long memory, unsigned long base)
{
    char *size_names[5] = {"B", "kB", "mB", "gB", "tB"};

    float total = (float) (memory * base);

    int high;
    for (high=0; total > BASE; high++, total /= BASE)
        ;

    char *memsz = malloc(sizeof(char) * MAXTOT);
    snprintf(memsz, MAXTOT, "%.2f %s\n", total, size_names[high]);
    return memsz;
}

void build_sys_info(char *fstype)
{
    int cur_y, cur_x, max_x, inc_x;
    char *totalfree_str, *memsz;

    long totalfree; 
    struct sysinfo *info;
    int ptys;

    ptys = nr_ptys();
    max_x = getmaxx(stdscr);
    cur_y = 3;
    cur_x = 2;
    inc_x = max_x / 5; 

    info = malloc(sizeof *info);
    sysinfo(info);

    totalfree_str = proc_parser(MEMINFO, MEMFREE);
    totalfree = atol(totalfree_str) * BASE;

    current_uptime(info->uptime, cur_y, cur_x);

    cur_x += inc_x;
    mvwprintw(stdscr, cur_y, cur_x, "Processes: %d", info->procs);
    cur_x += inc_x;
    mvwprintw(stdscr, cur_y, cur_x, "FileSystem: %s", fstype);
    cur_x += inc_x;
    mvwprintw(stdscr, cur_y++, cur_x, "PTYs: %d", ptys);
    cur_x = 2;

    memsz = mem_avail(info->totalram, info->mem_unit);
    mvwprintw(stdscr, ++cur_y, cur_x, "TotalMem: %s", memsz);
    free(memsz);

    memsz = mem_avail(totalfree, info->mem_unit);
    mvwprintw(stdscr, ++cur_y, cur_x, "FreeMem: %s", memsz);
    free(memsz);

    cur_x += inc_x;
    memsz = mem_avail(info->bufferram, info->mem_unit);
    mvwprintw(stdscr, --cur_y, cur_x, "Buffer: %s", memsz);
    free(memsz);

    memsz = mem_avail(info->sharedram, info->mem_unit);
    mvwprintw(stdscr, ++cur_y, cur_x, "Shared: %s", memsz);
    free(memsz);

    cur_x += inc_x;
    memsz = mem_avail(info->totalswap, info->mem_unit);
    mvwprintw(stdscr, --cur_y, cur_x, "Total Swap: %s", memsz);
    free(memsz);

    memsz = mem_avail(info->loads[0], info->mem_unit);
    mvwprintw(stdscr, cur_y, cur_x + inc_x, "AvgLoad: %s", memsz);
    free(memsz);

    memsz = mem_avail(info->freeswap, info->mem_unit);
    mvwprintw(stdscr, ++cur_y, cur_x, "Free Swap: %s", memsz);
    free(memsz);

    cur_x = 2;
    memsz = mem_avail(info->totalram - totalfree, info->mem_unit);
    mvwprintw(stdscr, ++cur_y, cur_x, "UsedMem: %s", memsz); 

    free(info);
    free(totalfree_str);
    free(memsz);
}

void current_uptime(unsigned long seconds, int y, int x)
{
    int hour, minute;

    for (hour=0, minute=0; seconds >= SECS; seconds -= SECS, minute++) {
        if (minute > SECS) {
            hour++; 
            minute = 0;
        }
    }

    mvwprintw(stdscr, y, x, "Hrs: %d Mins: %d Secs: %lu\n", hour, minute, seconds);    
}

int nr_ptys(void)
{
    int o_rd, r_bytes, bufsiz, ptys;

    bufsiz = 64;
    char buf[bufsiz];

    o_rd = open(NRPTYS, O_RDONLY);
    if (o_rd == -1)
        return -1;

    r_bytes = read(o_rd, buf, bufsiz);   
    if (r_bytes == -1) {
        close(o_rd);
        return -1;
    }

    ptys = strtol(buf, NULL, 10);
    close(o_rd);
    return ptys;
}

bool is_sysfield_timer_expired(int sys_timer_fd)
{
    uint64_t time_read = TIME_READ_DEFAULT;
    read(sys_timer_fd, &time_read, sizeof(uint64_t));

    return time_read > 0 ? true : false;
}

int set_sys_timer(struct itimerspec *sys_timer)
{
    struct timespec current_time;
    clock_gettime(CLOCK_REALTIME, &current_time);

    sys_timer->it_interval.tv_sec = SYS_TIMER_EXPIRED_SEC;
    sys_timer->it_interval.tv_nsec = SYS_TIMER_EXPIRED_NSEC;

    sys_timer->it_value.tv_sec = SYS_TIMER_LENGTH + current_time.tv_sec;
    sys_timer->it_value.tv_nsec = current_time.tv_nsec;

    int timer_fd = timerfd_create(CLOCK_REALTIME, TFD_NONBLOCK);
    timerfd_settime(timer_fd, TFD_TIMER_ABSTIME, sys_timer, NULL);
    
    return timer_fd;
}