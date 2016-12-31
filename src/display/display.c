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

static void print_aligned_stat(WINDOW *ps_window, char *ps_stat, int y, int x)
{
    char *decimal_str = strchr(ps_stat, '.');
    size_t aligned_dec = strlen(ps_stat) - strlen(decimal_str) - 1;

    mvwprintw(ps_window, y, x - aligned_dec, "%s", ps_stat);
}

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

void update_process_window(WINDOW *ps_window, ps_node const *ps_list,
                           char const *fieldbar, int process_line_num, int max_y)
{
    int cur_y = 1;

    wattron(ps_window, A_REVERSE);
    mvwprintw(ps_window, cur_y++, 1, fieldbar);
    wattroff(ps_window, A_REVERSE);

    while (ps_list && cur_y < max_y - 1) {
        if (process_line_num == 0) {

            if (ps_list->ps->cmd != NULL)
                mvwprintw(ps_window, cur_y, LINE_X, "%s  ", ps_list->ps->cmd);
            else
                mvwprintw(ps_window, cur_y, LINE_X, "N/A");
            mvwprintw(ps_window, cur_y, LINE_X + LPID, "%d   ", ps_list->ps->tid);
            if (ps_list->ps->euser != NULL)
                mvwprintw(ps_window, cur_y, LINE_X + LUSER, "%s   ", ps_list->ps->euser);
            else
                mvwprintw(ps_window, cur_y, LINE_X + LUSER, "N/A");
                
            mvwprintw(ps_window, cur_y, LINE_X + LCPU, "%d ", ps_list->cpuset);

            if (ps_list->ps->nice >= 0 && ps_list->ps->nice < 10) 
                mvwprintw(ps_window, cur_y, LINE_X + LNNICE, "%d", ps_list->ps->nice);
            else if (ps_list->ps->nice >= 10) 
                mvwprintw(ps_window, cur_y, LINE_X + LMNICE, "%d", ps_list->ps->nice);
            else 
                mvwprintw(ps_window, cur_y, LINE_X + LLNICE, "%d", ps_list->ps->nice);

            if (ps_list->ioprio != NULL)
                mvwprintw(ps_window, cur_y, LINE_X + LPRIO, "%s", ps_list->ioprio);
            else
                mvwprintw(ps_window, cur_y, LINE_X + LPRIO, "N/A");
            mvwprintw(ps_window, cur_y, LINE_X + LSTATE, "%c", ps_list->ps->state);
            mvwprintw(ps_window, cur_y, LINE_X + LVMEM, "%ld", ps_list->ps->size);

            mvwprintw(ps_window, cur_y, LINE_X + LRSS, "%ld", ps_list->ps->rss);

            if (ps_list->io_read != NULL)
                print_aligned_stat(ps_window, ps_list->io_read, cur_y, LINE_X + LREAD);
            else
                mvwprintw(ps_window, cur_y, LINE_X + LREAD + 2, "N/A");

            if (ps_list->io_write != NULL)
                print_aligned_stat(ps_window, ps_list->io_write, cur_y, LINE_X + LWRITE);
            else
                mvwprintw(ps_window, cur_y, LINE_X + LWRITE + 2, "N/A");

            if (ps_list->open_fds != -1)
                mvwprintw(ps_window, cur_y, LINE_X + LFDS, "%d", ps_list->open_fds);
            else
                mvwprintw(ps_window, cur_y, LINE_X + LFDS, "N/A");

            if (ps_list->invol_sw != NULL)
                mvwprintw(ps_window, cur_y++, LINE_X + LINVOL, "%s", ps_list->invol_sw);
            else
                mvwprintw(ps_window, cur_y++, LINE_X + LINVOL, "N/A");
            
//            if (ps_list->ps->thrcnt != NULL)
//                mvwprintw(ps_window, cur_y++, LINE_X + LTHRDS, "%s", ps_list->ps->thrcnt);
 //           else
//                mvwprintw(ps_window, cur_y++, LINE_X + LTHRDS, "N/A");
        } else 
            process_line_num--;

        ps_list = ps_list->next;
    }

    box(ps_window, 0, 0);
    wrefresh(ps_window);
}

char *build_fieldbar(void)
{
    unsigned int max_x = getmaxx(stdscr);
    char *fieldbar;

    if ((fieldbar = calloc(sizeof(char) * (ALLOC_ALIGN(max_x)) + 1, 
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
    
