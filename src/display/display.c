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

int update_system_window(WINDOW *system_window, char *fstype)
{
    int max_x = getmaxx(system_window);

    werase(system_window);

    wattron(system_window, A_BOLD);
    mvwprintw(system_window, 1, (max_x / 2) - 4, DASHBOARD);
    wattroff(system_window, A_BOLD);

    build_sys_info(system_window, fstype);

    box(system_window, 0, 0);
    wrefresh(system_window);

    return 0;
}

int update_process_window(WINDOW *ps_window, proc_t *processes,
                          char *fieldbar, int process_line_num, int max_y)
{
    int cur_y = 1;

    wattron(ps_window, A_REVERSE);
    mvwprintw(ps_window, cur_y++, 1, fieldbar);
    wattroff(ps_window, A_REVERSE);

    while (processes && cur_y < max_y - 1) {
        if (process_line_num == 0) {

            mvwprintw(ps_window, cur_y, LINE_X, "%s  ", processes->name);
            mvwprintw(ps_window, cur_y, LINE_X + LPID, "%d   ", processes->pid);
            mvwprintw(ps_window, cur_y, LINE_X + LUSER, "%s   ", processes->user);
            mvwprintw(ps_window, cur_y, LINE_X + LCPU, "%d ", processes->cpuset);
            mvwprintw(ps_window, cur_y, LINE_X + LMEM, "%.2f%", processes->mempcent);

            if (processes->nice >= 0 && processes->nice < 10) 
                mvwprintw(ps_window, cur_y, LINE_X + LNNICE, "%d", processes->nice);
            else if (processes->nice >= 10) 
                mvwprintw(ps_window, cur_y, LINE_X + LMNICE, "%d", processes->nice);
            else 
                mvwprintw(ps_window, cur_y, LINE_X + LLNICE, "%d", processes->nice);

            mvwprintw(ps_window, cur_y, LINE_X + LPRIO, "%s", processes->ioprio);
            mvwprintw(ps_window, cur_y, LINE_X + LSTATE, "%s", processes->state);

            print_aligned_stat(ps_window, processes->vmem, cur_y, LINE_X + LVMEM);

            if (processes->pte != NULL)
                mvwprintw(ps_window, cur_y, LINE_X + LPTE, "%s", processes->pte);
            else
                mvwprintw(ps_window, cur_y, LINE_X + LPTE, "N/A");

            print_aligned_stat(ps_window, processes->rss, cur_y, LINE_X + LRSS);

            if (processes->io_read != NULL)
                print_aligned_stat(ps_window, processes->io_read, cur_y, LINE_X + LREAD);
            else
                mvwprintw(ps_window, cur_y, LINE_X + LREAD + 2, "N/A");

            if (processes->io_write != NULL)
                print_aligned_stat(ps_window, processes->io_write, cur_y, LINE_X + LWRITE);
            else
                mvwprintw(ps_window, cur_y, LINE_X + LWRITE + 2, "N/A");

            if (processes->open_fds != -1)
                mvwprintw(ps_window, cur_y, LINE_X + LFDS, "%d", processes->open_fds);
            else
                mvwprintw(ps_window, cur_y, LINE_X + LFDS, "N/A");

            if (processes->invol_sw != NULL)
                mvwprintw(ps_window, cur_y, LINE_X + LINVOL, "%s", processes->invol_sw);
            else
                mvwprintw(ps_window, cur_y, LINE_X + LINVOL, "N/A");
            
            mvwprintw(ps_window, cur_y++, LINE_X + LTHRDS, "%s", processes->thrcnt);
        } else 
            process_line_num--;

        processes = processes->next;
    }

    box(ps_window, 0, 0);
    wrefresh(ps_window);

    return 0;
}

void print_aligned_stat(WINDOW *ps_window, char *ps_stat, int y, int x)
{
    char *decimal_str = strchr(ps_stat, '.');
    int aligned_dec = strlen(ps_stat) - strlen(decimal_str) - 1;

    mvwprintw(ps_window, y, x - aligned_dec, "%s", ps_stat);
}

char *build_fieldbar(void)
{
    unsigned int max_x = getmaxx(stdscr);
    char *fieldbar;

    if ((fieldbar = calloc(sizeof(char) * ALLOC_ALIGN(max_x), 
                                     sizeof(char))) == NULL)
        return NULL;

    unsigned i, head; 
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
