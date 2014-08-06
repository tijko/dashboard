#include <stdio.h>
#include <stdlib.h>
#include <mntent.h>
#include <ncurses.h>
#include <sys/sysinfo.h>

#include "sys_stats.h"

char *mem_avail(unsigned long memory, unsigned long base)
{
    float total;
    int high;
    int div;
    char *size_names[5] = {"B", "kB", "mB", "gB", "tB"};
    total = (float) (memory * base);
    div = 1024;
    for (high=0; total > div; high++, total /= div)
        ;
    char *memsz = malloc(sizeof(char) * 16);
    snprintf(memsz, 16, "%.2f %s\n", total, size_names[high]);
    return memsz;
}

void build_info(char *fstype)
{
    char *memsz;
    int cur_y, cur_x;
    int max_x;
    max_x = getmaxx(stdscr);
    cur_y = 3;
    cur_x = 2;
    int inc_x = max_x / 5; 
    struct sysinfo *info = malloc(sizeof *info);
    sysinfo(info);

    current_uptime(info->uptime, cur_y, cur_x);
    mvwprintw(stdscr, ++cur_y, cur_x, "Procs: %d", info->procs);
    cur_x += inc_x;
    mvwprintw(stdscr, cur_y, cur_x, "FileSystem: %s", fstype);
    cur_x = 2;

    memsz = mem_avail(info->totalram, info->mem_unit);
    mvwprintw(stdscr, ++cur_y, cur_x, "TotalMem: %s", memsz);

    memsz = mem_avail(info->freeram, info->mem_unit);
    mvwprintw(stdscr, ++cur_y, cur_x, "FreeMem: %s", memsz);

    cur_x += inc_x;
    memsz = mem_avail(info->bufferram, info->mem_unit);
    mvwprintw(stdscr, --cur_y, cur_x, "Buffer: %s", memsz);

    memsz = mem_avail(info->sharedram, info->mem_unit);
    mvwprintw(stdscr, ++cur_y, cur_x, "Shared: %s", memsz);

    cur_x += inc_x;
    memsz = mem_avail(info->totalswap, info->mem_unit);
    mvwprintw(stdscr, --cur_y, cur_x, "Total Swap: %s", memsz);

    memsz = mem_avail(info->freeswap, info->mem_unit);
    mvwprintw(stdscr, ++cur_y, cur_x, "Free Swap: %s", memsz);

    cur_x = 2;
    memsz = mem_avail(info->totalram - info->freeram, info->mem_unit);
    mvwprintw(stdscr, ++cur_y, cur_x, "UsedMem: %s", memsz); 

    free(info);
}

void current_uptime(unsigned long seconds, int y, int x)
{
    int base = 60;

    int hour = 0;
    int minute = 0;
    
    while (seconds >= 60) {
        seconds -= base;
        minute++;
        if (minute > 60) {
            hour++;
            minute = 0;
        }
    }
    mvwprintw(stdscr, y, x, "Hrs: %d Mins: %d Secs: %lu\n", hour, minute, seconds);    
}
