#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ncurses.h>
#include <pthread.h>

#include "display.h"
#include "../src/cpu.h"
#include "../src/disk.h"
#include "../src/memory.h"
#include "../src/process.h"
#include "../system/sys_stats.h"
#include "../src/util/sort_fields.h"


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
    dashboard_loop();
}

void dashboard_loop(void)
{
    int key;
    int max_y;
    int prev_y, prev_x;
    int curr_y, curr_x;
    int plineno;
    int prevplineno;
    char *fstype;
    int RUNNING;
    int nproc;
    int memtotal; 
    int sort;

    memtotal = total_memory();
    proc_t *processes = malloc(sizeof *processes);
    processes->prev = NULL;
    nproc = current_procs(processes, memtotal);

    sort = 0;
    plineno = 0;
    RUNNING = 1;

    getmaxyx(stdscr, curr_y, curr_x);

    fstype = filesystem_type();
    if (!fstype)
        fstype = "Unavailable";

    while (RUNNING) {

        getmaxyx(stdscr, curr_y, curr_x);

        if ((prev_y ^ curr_y) | (prev_x ^ curr_x) | (plineno ^ prevplineno)) 
            clear();

        prev_y = curr_y;
        prev_x = curr_x;
        prevplineno = plineno;
        max_y = curr_y - PROCLN;


        update_screen(processes, fstype, plineno);
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
                sort = KEY_C;
                clear();
                break;

            case (KEY_E):
                sort = KEY_E;
                clear();
                break;

            case (KEY_M):
                sort = KEY_M;
                clear();
                break;

            case (KEY_N):
                sort = KEY_N;
                clear();
                break;

            case (KEY_P):
                sort = KEY_P;
                clear();
                break;

            case (KEY_R):
                sort = KEY_R;
                clear();
                break;

            case (KEY_V):
                sort = KEY_V;
                clear();
                break;

            case (KEY_ESCAPE):
                RUNNING = 0;
                break;

            default:
                break;
        }

        free_procs(processes); 
        processes = malloc(sizeof *processes);
        nproc = current_procs(processes, memtotal);
        if (sort)
            processes = sort_by_field(processes, sort, nproc);
    }
    free_procs(processes);
    endwin();
}

void update_screen(proc_t *processes, char *fstype, int plineno)
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
    mvwprintw(stdscr, cur_y++, 1, fieldbar);
    attroff(A_REVERSE);

    while (processes->next && cur_y < max_y - 1) {
        if (plineno == 0) {
            mvwprintw(stdscr, cur_y, LINE_X, "%s  ", processes->name);
            mvwprintw(stdscr, cur_y, LINE_X + 19, "%d   ", processes->pid);
            mvwprintw(stdscr, cur_y, LINE_X + 26, "%s   ", processes->user);
            mvwprintw(stdscr, cur_y, LINE_X + 36, "%d ", processes->cpuset);
            mvwprintw(stdscr, cur_y, LINE_X + 40, "%.2f%", processes->mempcent);
            if (processes->nice >= 0 && processes->nice < 10) 
                mvwprintw(stdscr, cur_y, LINE_X + 50, "%d", processes->nice);
            else if (processes->nice >= 10) 
                mvwprintw(stdscr, cur_y, LINE_X + 49, "%d", processes->nice);
            else 
                mvwprintw(stdscr, cur_y, LINE_X + 48, "%d", processes->nice);
            mvwprintw(stdscr, cur_y, LINE_X + 55, "%s", processes->ioprio);
            mvwprintw(stdscr, cur_y, LINE_X + 63, "%s", processes->state);
            mvwprintw(stdscr, cur_y, LINE_X + 67, "%d", processes->vmem);
            mvwprintw(stdscr, cur_y, LINE_X + 75, "%d", processes->pte);
            mvwprintw(stdscr, cur_y++, LINE_X + 82, "%d", processes->rss); 
        } else {
            plineno--;
        }
        processes = processes->next;
    }
    box(stdscr, 0, 0);
    refresh();
    free(fieldbar);
}

char *fieldbar_builder(void)
{
    int spaceleft, i;
    int max_x = getmaxx(stdscr);
    char *fieldbar;
    char *fieldattrs[FIELDS] = {"PID", "USER", "CPU", "MEM%%", "NI", 
                           "PRIO", "ST", "VMEM", "PTE", "RES"};
    int attrspace[FIELDS] = {13, 5, 5, 2, 5, 4, 3, 3, 4, 4};
    fieldbar = add_space("", "NAME", 2, max_x);
    for (i=0; i < FIELDS; i++) 
        fieldbar = add_space(fieldbar, fieldattrs[i], 
                             attrspace[i], max_x);
    spaceleft = max_x - 61;
    fieldbar = add_space(fieldbar, " ", spaceleft, max_x);
    return fieldbar;
}
    
char *add_space(char *fieldbar, char *field, int spaces, size_t max)
{
    int space;
    char *tmp = malloc(sizeof(char) * max);
    for (space=0; space < spaces; ++space) {
        snprintf(tmp, max, "%s ", fieldbar); 
        fieldbar = strdup(tmp);
        free(tmp);
        tmp = malloc(sizeof(char) * max);
    }
    snprintf(tmp, max, "%s%s", fieldbar, field);
    fieldbar = strdup(tmp);
    free(tmp);
    return fieldbar;
}
