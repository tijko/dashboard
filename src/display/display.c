#define _POSIX_C_SOURCE 199310L

#include <time.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <ncurses.h>

#include "display.h"
#include "../process/process.h"
#include "../system/sys_stats.h" // XXX


void init_screen(void)
{
    initscr();
    noecho();
    halfdelay(DELAY);
    keypad(stdscr, TRUE);
    curs_set(0);

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    attron(COLOR_PAIR(1));
}

int update_screen(proc_t *processes, bool sys_fields_refresh, char *fstype, 
                  char *fieldbar, int process_line_num, int max_x, int max_y)
{
    int cur_y = LINE_Y;

    attron(A_BOLD);
    mvwprintw(stdscr, 1, (max_x / 2) - 4, DASHBOARD);
    attroff(A_BOLD);

    if (sys_fields_refresh) // mv to dashboard.c
        build_sys_info(fstype);

    attron(A_REVERSE);
    mvwprintw(stdscr, cur_y++, 1, fieldbar);
    attroff(A_REVERSE);

    while (processes && cur_y < max_y - 1) {
        if (process_line_num == 0) {
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
            process_line_num--;

        processes = processes->next;
    }

    box(stdscr, 0, 0);
    refresh();

    return 0;
}

char *build_fieldbar(void)
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
