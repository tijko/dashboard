#include <stdio.h>
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


void init_screen(void)
{
    initscr();
    noecho();
    halfdelay(5);
    keypad(stdscr, TRUE);
    curs_set(0);
    setscrreg(8, 50);    

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

    memtotal = total_memory();
    proc_t *processes = malloc(sizeof *processes);
    processes->prev = NULL;
    nproc = current_procs(processes, memtotal);

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
                if (plineno > 0) {
                    plineno--;
                }
                break;
            case (KEY_DOWN):
                if (plineno < (nproc - max_y)) {
                    plineno++; 
                }
                break;
            case (113):
                RUNNING = 0;
                break;
            default:
                break;
        }

        free_procs(processes); 
        processes = malloc(sizeof *processes);
        nproc = current_procs(processes, memtotal);
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
            mvwprintw(stdscr, cur_y, LINE_X + 20, "%d   ", processes->pid);
            mvwprintw(stdscr, cur_y, LINE_X + 30, "%d   ", processes->cpuset);
            mvwprintw(stdscr, cur_y, LINE_X + 40, "%s ", processes->user);
            mvwprintw(stdscr, cur_y, LINE_X + 50, "%.2f%", processes->mempcent);
            if (processes->nice >= 0 && processes->nice < 10) 
                mvwprintw(stdscr, cur_y, LINE_X + 60, "%d", processes->nice);
            else if (processes->nice >= 10) 
                mvwprintw(stdscr, cur_y, LINE_X + 59, "%d", processes->nice);
            else 
                mvwprintw(stdscr, cur_y, LINE_X + 58, "%d", processes->nice);
            mvwprintw(stdscr, cur_y, LINE_X + 65, "%s", processes->ioprio);
            mvwprintw(stdscr, cur_y, LINE_X + 73, "%s", processes->state);
            mvwprintw(stdscr, cur_y++, LINE_X + 77, "%d", processes->vmem);
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
    int spaceleft;
    int max_x = getmaxx(stdscr);
    char *fieldbar;
    fieldbar = add_space("", "NAME", 2, max_x);
    fieldbar = add_space(fieldbar, "PID", 13, max_x);
    fieldbar = add_space(fieldbar, "CPU", 7, max_x);
    fieldbar = add_space(fieldbar, "USER", 9, max_x);
    fieldbar = add_space(fieldbar, "MEM%%", 6, max_x);
    fieldbar = add_space(fieldbar, "NI", 5, max_x);
    fieldbar = add_space(fieldbar, "PRIO", 4, max_x);
    fieldbar = add_space(fieldbar, "ST", 3, max_x);
    fieldbar = add_space(fieldbar, "VMEM", 3, max_x);
    spaceleft = max_x - 79;
    fieldbar = add_space(fieldbar, " ", spaceleft, max_x);
    return fieldbar;
}
    
char *add_space(char *fieldbar, char *field, int spaces, size_t max)
{
    int space;
    char *tmp = malloc(sizeof(char) * max);
    for (space=0; space < spaces; ++space) {
        snprintf(tmp, max, "%s ", fieldbar); 
        fieldbar = tmp;
        tmp = malloc(sizeof(char) * max);
    }
    snprintf(tmp, max, "%s%s", fieldbar, field);
    fieldbar = tmp;
    return fieldbar;
}
