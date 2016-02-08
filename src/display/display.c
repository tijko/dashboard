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

int update_process_window(WINDOW *process_window, proc_t *processes,
                          char *fieldbar, int process_line_num, int max_y)
{
    int cur_y = 1;

    wattron(process_window, A_REVERSE);
    mvwprintw(process_window, cur_y++, 1, fieldbar);
    wattroff(process_window, A_REVERSE);

    while (processes && cur_y < max_y - 1) {
        if (process_line_num == 0) {
            mvwprintw(process_window, cur_y, LINE_X, "%s  ", processes->name);
            mvwprintw(process_window, cur_y, LINE_X + LPID, 
                      "%d   ", processes->pid);
            mvwprintw(process_window, cur_y, LINE_X + LUSER, 
                      "%s   ", processes->user);
            mvwprintw(process_window, cur_y, LINE_X + LCPU, 
                      "%d ", processes->cpuset);
            mvwprintw(process_window, cur_y, LINE_X + LMEM, "%.2f%", 
                      processes->mempcent);
            if (processes->nice >= 0 && processes->nice < 10) 
                mvwprintw(process_window, cur_y, LINE_X + LNNICE, "%d", 
                          processes->nice);
            else if (processes->nice >= 10) 
                mvwprintw(process_window, cur_y, LINE_X + LMNICE, "%d", 
                          processes->nice);
            else 
                mvwprintw(process_window, cur_y, LINE_X + LLNICE, "%d", 
                          processes->nice);
            mvwprintw(process_window, cur_y, LINE_X + LPRIO, 
                      "%s", processes->ioprio);
            mvwprintw(process_window, cur_y, LINE_X + LSTATE, 
                      "%s", processes->state);
            mvwprintw(process_window, cur_y, LINE_X + LVMEM, 
                      "%s", processes->vmem);
            mvwprintw(process_window, cur_y, LINE_X + LPTE, 
                      "%d", processes->pte);
            mvwprintw(process_window, cur_y, LINE_X + LRSS, 
                      "%s", processes->rss);
            if (processes->io_read != NULL)
                mvwprintw(process_window, cur_y, LINE_X + LREAD, "%s",
                          processes->io_read);
            else
                mvwprintw(process_window, cur_y, LINE_X + LREAD, "N/A");
            if (processes->io_write != NULL)
                mvwprintw(process_window, cur_y, LINE_X + LWRITE, "%s",
                          processes->io_write);
            else
                mvwprintw(process_window, cur_y, LINE_X + LWRITE, "N/A");
            if (processes->open_fds != -1)
                mvwprintw(process_window, cur_y, LINE_X + LFDS, "%d", 
                          processes->open_fds);
            else
                mvwprintw(process_window, cur_y, LINE_X + LFDS, "N/A");
            if (processes->invol_sw != NULL)
                mvwprintw(process_window, cur_y, LINE_X + LINVOL, "%s", 
                          processes->invol_sw);
            else
                mvwprintw(process_window, cur_y, LINE_X + LINVOL, "N/A");
            
            mvwprintw(process_window, cur_y++, LINE_X + LTHRDS, "%s", 
                      processes->thrcnt);
        } else 
            process_line_num--;

        processes = processes->next;
    }

    box(process_window, 0, 0);
    wrefresh(process_window);

    return 0;
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
