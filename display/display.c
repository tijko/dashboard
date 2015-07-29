#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ncurses.h>

#include "display.h"
#include "../src/cpu.h"
#include "../src/disk.h"
#include "../src/memory.h"
#include "../src/process.h"
#include "../system/sys_stats.h"
#include "../src/util/sort_fields.h"


void init_screen(int log_opt, char attr_sort)
{
    initscr();
    noecho();
    halfdelay(DELAY);
    keypad(stdscr, TRUE);
    curs_set(0);

    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    attron(COLOR_PAIR(1));
    dashboard_loop(log_opt, attr_sort);
}

void dashboard_loop(int log_opt, char attr_sort)
{
    int key;
    int max_y;
    int prev_y, prev_x;
    int curr_y, curr_x;
    int plineno, prevplineno;
    char *fstype;
    int RUNNING;
    int nproc;
    int memtotal; 
    int refresh_rate;
    int sort = attr_sort;
    proc_t *processes;
    
    euid = geteuid();
    refresh_rate = REFRESH_RATE;
    memtotal = total_memory();

    prev_y = 0, prev_x = 0;
    curr_y = 0, curr_x = 0;
    plineno = 0, prevplineno = 0;
    getmaxyx(stdscr, curr_y, curr_x);

    fstype = filesystem_type();
    if (!fstype)
        fstype = "Unavailable";

    RUNNING = 1;

    while (RUNNING) {

        if ((processes = malloc(sizeof *processes)) == NULL)
            return;
        
        processes->prev = NULL;
        nproc = current_procs(processes, memtotal);

        if (sort)
            processes = sort_by_field(processes, sort, nproc);
        
        getmaxyx(stdscr, curr_y, curr_x);

        if ((prev_y ^ curr_y) | (prev_x ^ curr_x) | (plineno ^ prevplineno)) 
            clear();

        prev_y = curr_y;
        prev_x = curr_x;
        prevplineno = plineno;
        max_y = curr_y - PROCLN;

        if ((update_screen(processes, fstype, plineno)) < 0)
            return;
        key = wgetch(stdscr);

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
                if (sort != KEY_C)
                    clear();
                sort = KEY_C;
                break;

            case (KEY_D):
                if (sort != KEY_D)
                    clear();
                sort = KEY_D;
                break;

            case (KEY_E):
                if (sort != KEY_E)
                    clear();
                sort = KEY_E;
                break;

            case (KEY_I):
                if (sort != KEY_I)
                    clear();
                sort = KEY_I;
                break;

            case (KEY_M):
                if (sort != KEY_M)
                    clear();
                sort = KEY_M;
                break;

            case (KEY_N):
                if (sort != KEY_N)
                    clear();
                sort = KEY_N;
                break;

            case (KEY_O):
                if (sort != KEY_O)
                    clear();
                sort = KEY_O;
                break;

            case (KEY_P):
                if (sort != KEY_P)
                    clear();
                sort = KEY_P;
                break;

            case (KEY_R):
                if (sort != KEY_R)
                    clear();
                sort = KEY_R;
                break;

            case (KEY_S):
                if (sort != KEY_S)
                    clear();
                sort = KEY_S;
                break;

            case (KEY_T):
                if (sort != KEY_T)
                    clear();
                sort = KEY_T;
                break;

            case (KEY_V):
                if (sort != KEY_V)
                    clear();
                sort = KEY_V;
                break;

            case (KEY_ESCAPE):  
                RUNNING = 0;
                break;

            default:
                break;
        }

        free_procs(processes); 

        if (refresh_rate == REFRESH_RATE) {
            clear();
            --refresh_rate;
        } else if (refresh_rate == 0)
            refresh_rate = REFRESH_RATE;
        else
            --refresh_rate;
    }

    endwin();
}

int update_screen(proc_t *processes, char *fstype, int plineno)
{
    int max_y, max_x;
    int cur_y = 9;
    getmaxyx(stdscr, max_y, max_x);

    attron(A_BOLD);
    mvwprintw(stdscr, 1, (max_x / 2) - 4, "DASHBOARD");
    attroff(A_BOLD);

    build_info(fstype);

    attron(A_REVERSE);
    char *fieldbar = fieldbar_builder(); 
    if (fieldbar == NULL)
        return -ENOMEM;
    mvwprintw(stdscr, cur_y++, 1, fieldbar);
    attroff(A_REVERSE);

    while (processes && cur_y < max_y - 1) {
        if (plineno == 0) {
            mvwprintw(stdscr, cur_y, LINE_X, "%s  ", processes->name);
            mvwprintw(stdscr, cur_y, LINE_X + 18, "%d   ", processes->pid);
            mvwprintw(stdscr, cur_y, LINE_X + 26, "%s   ", processes->user);
            mvwprintw(stdscr, cur_y, LINE_X + 36, "%d ", processes->cpuset);
            mvwprintw(stdscr, cur_y, LINE_X + 40, "%.2f%", processes->mempcent);
            if (processes->nice >= 0 && processes->nice < 10) 
                mvwprintw(stdscr, cur_y, LINE_X + 50, "%d", processes->nice);
            else if (processes->nice >= 10) 
                mvwprintw(stdscr, cur_y, LINE_X + 49, "%d", processes->nice);
            else 
                mvwprintw(stdscr, cur_y, LINE_X + 48, "%d", processes->nice);
            mvwprintw(stdscr, cur_y, LINE_X + 54, "%s", processes->ioprio);
            mvwprintw(stdscr, cur_y, LINE_X + 63, "%s", processes->state);
            mvwprintw(stdscr, cur_y, LINE_X + 69, "%d", processes->vmem);
            mvwprintw(stdscr, cur_y, LINE_X + 79, "%d", processes->pte);
            mvwprintw(stdscr, cur_y, LINE_X + 88, "%d", processes->rss); 
            mvwprintw(stdscr, cur_y, LINE_X + 97, "%llu", processes->io_read);
            mvwprintw(stdscr, cur_y, LINE_X + 111, "%llu", processes->io_write);
            mvwprintw(stdscr, cur_y, LINE_X + 124, "%d", processes->open_fds);
            mvwprintw(stdscr, cur_y, LINE_X + 134, "%d", processes->invol_sw);
            mvwprintw(stdscr, cur_y++, LINE_X + 144, "%d", processes->thrcnt);
        } else {
            plineno--;
        }
        processes = processes->next;
    }

    box(stdscr, 0, 0);
    refresh();
    free(fieldbar);

    return 0;
}

char *fieldbar_builder(void)
{
    unsigned int spaceleft, max_x = getmaxx(stdscr);
    char *fieldbar, *tmpbar;

    int fieldsize = ALLOC_ALIGN(strlen(fieldattrs[0]) + attrspace[0] + 1);

    fieldbar = calloc(sizeof(char) * fieldsize, sizeof(char));

    int i, current_size;
    for (i=0, current_size=fieldsize; current_size < max_x &&
                                      i < fieldattr_size - 1; i++) {
        if (i) {
            fieldsize = ALLOC_ALIGN(strlen(fieldbar) + strlen(fieldattrs[i]) + 
                                    attrspace[i] + 1);
            tmpbar = realloc(fieldbar, sizeof(char) * fieldsize);
            if (tmpbar == NULL)
                return NULL;

            fieldbar = tmpbar;
        }

        add_space(fieldbar, fieldattrs[i], attrspace[i]);
        current_size = current_size + ALLOC_ALIGN(strlen(fieldattrs[i+1] + 
                                                         attrspace[i] + 1));
    }

    if (max_x > current_size) {
        spaceleft = max_x - current_size;
        fieldsize = ALLOC_ALIGN(spaceleft + attrspace[i] + 1);
        tmpbar = realloc(fieldbar, sizeof(char) * fieldsize);
        if (tmpbar == NULL)
            return NULL;
        fieldbar = tmpbar;
        add_space(fieldbar, fieldattrs[i], spaceleft);
    } 

    return fieldbar;
}
    
void add_space(char *curbar, char *field, int spaces)
{
    int space, endbar;
    endbar = strlen(curbar);
    memcpy(curbar + endbar, field, strlen(field) + 1);
    endbar = strlen(curbar);
    for (space=0; space < spaces; space++)
        curbar[endbar + space] = ' ';

    curbar[endbar + space] = '\0';
}
