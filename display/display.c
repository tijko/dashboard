#define _POSIX_C_SOURCE 199310L

#include <sys/timerfd.h>
#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#include "display.h"
#include "../src/cpu.h"
#include "../src/disk.h"
#include "../src/memory.h"
#include "../system/sys_stats.h"
#include "../src/util/sort_fields.h"


void init_screen(char attr_sort)
{
    initscr();
    noecho();
    halfdelay(DELAY);
    keypad(stdscr, TRUE);
    curs_set(0);

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    dashboard_loop(attr_sort);
}

void dashboard_loop(char attr_sort)
{
    proc_t *processes;
    
    euid = geteuid();
    int memtotal = total_memory();

    int prev_y = 0, prev_x = 0;
    int curr_y = 0, curr_x = 0;
    int plineno = 0, prevplineno = 0;

    getmaxyx(stdscr, curr_y, curr_x);

    char *fstype = filesystem_type();

    if (!fstype)
        fstype = "Unavailable";

    bool running = true, update_sys = true;

    struct itimerspec *sys_timer = malloc(sizeof *sys_timer);
    if (sys_timer == NULL)
        return;

    int sys_timer_fd = set_sys_timer(sys_timer);

    while (running) {

        if ((processes = malloc(sizeof *processes)) == NULL)
            return;
        
        processes->prev = NULL; // set in current_proc call
        int nproc = current_procs(processes, memtotal);

        if (attr_sort)
            processes = sort_by_field(processes, attr_sort, nproc);
        
        getmaxyx(stdscr, curr_y, curr_x);

        // `xor` the current line positions against the previous
        // if any differ `clear` for a redraw.
        if ((prev_y ^ curr_y) | (prev_x ^ curr_x) | (plineno ^ prevplineno)) 
            clear();

        prev_y = curr_y;
        prev_x = curr_x;
        prevplineno = plineno;
        int max_y = curr_y - PROC_LINE_SIZE;

        if ((update_screen(processes, update_sys, fstype, plineno)) < 0)
            return;

        int key = wgetch(stdscr);

        switch (key) {

            case (KEY_UP):
                if (plineno > 0) 
                    plineno--;
                break;

            case (KEY_DOWN):
                if (plineno < (nproc - max_y)) 
                    plineno++; 
                break;

            case (KEY_C):
                if (attr_sort != KEY_C)
                    clear();
                attr_sort = KEY_C;
                break;

            case (KEY_D):
                if (attr_sort != KEY_D)
                    clear();
                attr_sort = KEY_D;
                break;

            case (KEY_E):
                if (attr_sort != KEY_E)
                    clear();
                attr_sort = KEY_E;
                break;

            case (KEY_I):
                if (attr_sort != KEY_I)
                    clear();
                attr_sort = KEY_I;
                break;

            case (KEY_M):
                if (attr_sort != KEY_M)
                    clear();
                attr_sort = KEY_M;
                break;

            case (KEY_N):
                if (attr_sort != KEY_N)
                    clear();
                attr_sort = KEY_N;
                break;

            case (KEY_O):
                if (attr_sort != KEY_O)
                    clear();
                attr_sort = KEY_O;
                break;

            case (KEY_P):
                if (attr_sort != KEY_P)
                    clear();
                attr_sort = KEY_P;
                break;

            case (KEY_R):
                if (attr_sort != KEY_R)
                    clear();
                attr_sort = KEY_R;
                break;

            case (KEY_S):
                if (attr_sort != KEY_S)
                    clear();
                attr_sort = KEY_S;
                break;

            case (KEY_T):
                if (attr_sort != KEY_T)
                    clear();
                attr_sort = KEY_T;
                break;

            case (KEY_V):
                if (attr_sort != KEY_V)
                    clear();
                attr_sort = KEY_V;
                break;

            case (KEY_ESCAPE):  
                running = false;
                break;

            default:
                break;
        }

        free_procs(processes); 
        delay_output(REFRESH_RATE);

        bool update_sys = sys_field_timer(sys_timer_fd); 

        if (update_sys) {
            close(sys_timer_fd);
            int sys_timer_fd = set_sys_timer(sys_timer);
        }

    }

    endwin();
    free(sys_timer);
}

int update_screen(proc_t *processes, bool sys_fields_refresh, 
                  char *fstype, int plineno)
{
    int cur_y = LINE_Y;
    int max_y, max_x;
    getmaxyx(stdscr, max_y, max_x);

    attron(A_BOLD);
    mvwprintw(stdscr, 1, (max_x / 2) - 4, DASHBOARD);
    attroff(A_BOLD);

    if (sys_fields_refresh)
        build_sys_info(fstype);

    attron(A_REVERSE);

    char *fieldbar = fieldbar_builder(); 
    if (fieldbar == NULL)
        return -ENOMEM;

    mvwprintw(stdscr, cur_y++, 1, fieldbar);

    attroff(A_REVERSE);

    while (processes && cur_y < max_y - 1) {
        if (plineno == 0) {
            mvwprintw(stdscr, cur_y, LINE_X, "%s  ", processes->name);
            mvwprintw(stdscr, cur_y, LINE_X + LPID, "%d   ", processes->pid);
            mvwprintw(stdscr, cur_y, LINE_X + LUSER, "%s   ", processes->user);
            mvwprintw(stdscr, cur_y, LINE_X + LCPU, "%d ", processes->cpuset);
            mvwprintw(stdscr, cur_y, LINE_X + LMEM, "%.2f%", 
                      processes->mempcent);
            if (processes->nice >= 0 && processes->nice < 10) 
                mvwprintw(stdscr, cur_y, LINE_X + LNNICE, "%d", 
                          processes->nice);
            else if (processes->nice >= 10) 
                mvwprintw(stdscr, cur_y, LINE_X + LMNICE, "%d", 
                          processes->nice);
            else 
                mvwprintw(stdscr, cur_y, LINE_X + LLNICE, "%d", 
                          processes->nice);
            mvwprintw(stdscr, cur_y, LINE_X + LPRIO, "%s", processes->ioprio);
            mvwprintw(stdscr, cur_y, LINE_X + LSTATE, "%s", processes->state);
            mvwprintw(stdscr, cur_y, LINE_X + LVMEM, "%d", processes->vmem);
            mvwprintw(stdscr, cur_y, LINE_X + LPTE, "%d", processes->pte);
            mvwprintw(stdscr, cur_y, LINE_X + LRSS, "%d", processes->rss); 
            mvwprintw(stdscr, cur_y, LINE_X + LREAD, "%llu", 
                      processes->io_read);
            mvwprintw(stdscr, cur_y, LINE_X + LWRITE, "%llu", 
                      processes->io_write);
            mvwprintw(stdscr, cur_y, LINE_X + LFDS, "%d", processes->open_fds);
            mvwprintw(stdscr, cur_y, LINE_X + LINVOL, "%d", 
                      processes->invol_sw);
            mvwprintw(stdscr, cur_y++, LINE_X + LTHRDS, "%d", 
                      processes->thrcnt);
        } else 
            plineno--;

        processes = processes->next;
    }

    box(stdscr, 0, 0);
    refresh();
    free(fieldbar);

    return 0;
}

bool sys_field_timer(int sys_timer_fd)
{
    uint64_t time_read = 0;
    int ret = read(sys_timer_fd, &time_read, sizeof(uint64_t));

    return ret > 0 ? true : false;
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

char *fieldbar_builder(void)
{
    unsigned int max_x = getmaxx(stdscr);
    char *fieldbar;

    if ((fieldbar = calloc(sizeof(char) * ALLOC_ALIGN(max_x), 
                                     sizeof(char))) == NULL)
        return NULL;

    unsigned int i, head; 
    // maintain a `head` of the string to avoid repeated calls to `strlen`
    for (i=0, head=0; i < fieldattr_size; i++) {
        add_space(fieldbar, fieldattrs[i], head, attrspace[i]);    
        head += (strlen(fieldattrs[i]) + attrspace[i]);
        // check one ahead if the next field attribute with space will go
        // beyond the width of the screen.  the array is padded with zeroes
        // @ the end to account for out of bounds references.
        if ((head + strlen(fieldattrs[i+1]) + attrspace[i+1]) >= max_x)
            break;
    }

    unsigned int spaceleft;

    if (max_x > head) {
        spaceleft = max_x - head; // check if blank bar space is needed to fill
        add_space(fieldbar, "", head, spaceleft);
    }

    return fieldbar;
}
    
void add_space(char *curbar, char const *field, int strterm, int spaces)
{
    int space;

    strcat(curbar + strterm, field);
    strterm += strlen(field);

    for (space=0; space < spaces; space++)
        curbar[strterm + space] = ' ';

    curbar[strterm + space] = '\0';
}
