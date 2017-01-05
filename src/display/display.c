#define _POSIX_C_SOURCE 199310L

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>
#include <ncurses.h>

#include "display.h"
#include "../process/process.h"
#include "../system/sys_stats.h"


void init_windows(WINDOW **display_windows)
{

    initscr();
    noecho();
    halfdelay(DELAY);
    curs_set(0);
    keypad(stdscr, TRUE);

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);

    display_windows[0] = newwin(LINE_Y, 0, 0, 0);
    display_windows[1] = newwin(0, 0, LINE_Y, 0);

    wattron(display_windows[0], COLOR_PAIR(1));
    wattron(display_windows[1], COLOR_PAIR(1));
}

void update_system_window(WINDOW *system_window, sysaux *sys)
{
    int max_x = getmaxx(system_window);

    werase(system_window);

    wattron(system_window, A_BOLD);
    mvwprintw(system_window, 1, (max_x / 2) - 4, DASHBOARD);
    wattroff(system_window, A_BOLD);

    build_sys_info(system_window, sys->fstype);

    box(system_window, 0, 0);
}
/*
static void print_aligned_stat(WINDOW *ps_window, char *ps_stat, int y, int x)
{
    char *decimal_str = strchr(ps_stat, '.');
    size_t aligned_dec = strlen(ps_stat) - strlen(decimal_str) - 1;

    mvwprintw(ps_window, y, x - aligned_dec, "%s", ps_stat);
}
*/
static inline void add_space(char *curbar, char const *field, 
                             int strterm, unsigned int spaces)
{
    unsigned int space;

    strcat(curbar + strterm, field);
    strterm += strlen(field);

    for (space=0; space < spaces; space++)
        curbar[strterm + space] = ' ';

    curbar[strterm + space] = '\0';
}

void update_default_window(WINDOW *ps_window, ps_node const *ps_list,
                           char const *fieldbar, int process_line_num, int max_y)
{
    werase(ps_window);

    int cur_y = 1;

    wattron(ps_window, A_REVERSE);
    mvwprintw(ps_window, cur_y++, 1, fieldbar);
    wattroff(ps_window, A_REVERSE);

    long clk_tcks = sysconf(_SC_CLK_TCK);

    while (ps_list && cur_y < max_y) {
        proc_t *ps = ps_list->ps;
        if (process_line_num == 0) {
            mvwprintw(ps_window, cur_y, LCMD, "%s", ps->cmd);
            mvwprintw(ps_window, cur_y, LPID, "%d", ps->tid);
            mvwprintw(ps_window, cur_y, LPPID, "%d", ps->ppid);
            mvwprintw(ps_window, cur_y, LUSER, "%s", ps->ruser);
            mvwprintw(ps_window, cur_y, LTTY, "%d", ps->tty);
            mvwprintw(ps_window, cur_y, LUTM, "%lld", ps->utime / clk_tcks);
            mvwprintw(ps_window, cur_y, LSTM, "%lld", ps->stime / clk_tcks);
            if (ps_list->open_fds == -1)
                mvwprintw(ps_window, cur_y, LFDS, "N/A");
            else
                mvwprintw(ps_window, cur_y, LFDS, "%d", ps_list->open_fds);
            mvwprintw(ps_window, cur_y++, LNLWP, "%d", ps->nlwp);
        } else 
            process_line_num--;

        ps_list = ps_list->next;
    }

    box(ps_window, 0, 0);
    wrefresh(ps_window);
}

void update_memory_window(WINDOW *ps_window, ps_node const *ps_list,
                          char const *fieldbar, int process_line_num, int max_y)
{
    werase(ps_window);

    int cur_y = 1;

    wattron(ps_window, A_REVERSE);
    mvwprintw(ps_window, cur_y++, 1, fieldbar);
    wattroff(ps_window, A_REVERSE);

    while (ps_list && cur_y < max_y) {
        proc_t *ps = ps_list->ps;
        if (process_line_num == 0) {
            mvwprintw(ps_window, cur_y, LCMD, "%s", ps->cmd);
            mvwprintw(ps_window, cur_y, LPID, "%d", ps->tid);
            mvwprintw(ps_window, cur_y, LPPID, "%d", ps->ppid);
            mvwprintw(ps_window, cur_y, LUSER, "%s", ps->ruser);

            mvwprintw(ps_window, cur_y, LSIZE, "%lu", ps->size);
            mvwprintw(ps_window, cur_y, LRSS, "%lu", ps->resident);
            mvwprintw(ps_window, cur_y, LSHR, "%lu", ps->share);
            mvwprintw(ps_window, cur_y, LLCK, "%lu", ps->vm_lock);
            mvwprintw(ps_window, cur_y, LDATA, "%lu", ps->vm_data);
            mvwprintw(ps_window, cur_y, LSWAP, "%lu", ps->vm_swap);
            mvwprintw(ps_window, cur_y++, LLIB, "%lu", ps->vm_lib);


        } else 
            process_line_num--;

        ps_list = ps_list->next;
    }

    box(ps_window, 0, 0);
    wrefresh(ps_window);
}

char *build_default_fieldbar(void)
{
    unsigned int default_attrsize = ATTRSIZE(default_attrs);
    unsigned int max_x = getmaxx(stdscr);
    char *fieldbar;

    if ((fieldbar = calloc(sizeof(char) * (ALLOC_ALIGN(max_x)) + 1, 
                                           sizeof(char))) == NULL)
        return NULL;

    unsigned i, head; 
    // maintain a `head` of the string to avoid repeated calls to `strlen`
    for (i=0, head=0; i < default_attrsize; i++) {
        add_space(fieldbar, default_attrs[i], head, default_attrspace[i]);
        head += (strlen(default_attrs[i]) + default_attrspace[i]);
        // check one ahead if the next field attribute with space will go
        // beyond the width of the screen.  the array is padded with zeroes
        // @ the end to account for out of bounds references.
        if ((head + strlen(default_attrs[i+1]) + 
                    default_attrspace[i+1]) >= max_x)
            break;
    }

    unsigned int spaceleft;

    if (max_x > head) {
        spaceleft = max_x - head; // check if blank bar space is needed to fill
        add_space(fieldbar, "", head, spaceleft);
    }

    return fieldbar;
}
    
char *build_memory_fieldbar(void)
{
    unsigned int memory_attrsize = ATTRSIZE(memory_attrs);
    unsigned int max_x = getmaxx(stdscr);
    char *fieldbar;

    if ((fieldbar = calloc(sizeof(char) * (ALLOC_ALIGN(max_x)) + 1, 
                                           sizeof(char))) == NULL)
        return NULL;

    unsigned i, head; 
    for (i=0, head=0; i < memory_attrsize; i++) {
        add_space(fieldbar, memory_attrs[i], head, memory_attrspace[i]);
        head += (strlen(memory_attrs[i]) + memory_attrspace[i]);
        if ((head + strlen(memory_attrs[i+1]) + 
                    memory_attrspace[i+1]) >= max_x)
            break;
    }

    unsigned int spaceleft;

    if (max_x > head) {
        spaceleft = max_x - head;
        add_space(fieldbar, "", head, spaceleft);
    }

    return fieldbar;
}
    
