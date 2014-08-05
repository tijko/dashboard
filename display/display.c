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
#include "../system/header_stats.h"


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
    int memtotal = total_memory();
    proc_t *processes = malloc(sizeof *processes);
    current_procs(processes, memtotal);

    int key;
    int RUNNING = 1;
    int y = 3;

    char *fstype;
    fstype = filesystem_type();
    if (!fstype)
        fstype = "Unavailable";

    while (RUNNING) {
        
        update_screen(processes, fstype);
        key = wgetch(stdscr);
        switch (key) {
            case (KEY_UP):
            //    scrollok(mywin, TRUE); // TRUE
            //    idlok(mywin, FALSE);
            //XXX have some kind of print for next in-line with linked list
                scrl(-10);
                y = y - 10;
                break;
            case (KEY_DOWN):
            //    scrollok(mywin, TRUE); // TRUE
            //    idlok(mywin, FALSE);
                scrl(10);
                y = y + 10;
                break;
            case (113):
                RUNNING = 0;
                break;
            default:
                break;
        }
        refresh();
        clear();
        free_procs(processes); 
        processes = malloc(sizeof *processes);
        current_procs(processes, memtotal);
    }
    endwin();
}

// XXX: only scrollok(stdscr, TRUE) set when input-key ("arrows") are received then
//      reset back.
//
//      keep a count for how many lines are in screen(get screen size func)
//      when printing loop for that many lines so "bunches" do not happen
//

void update_screen(proc_t *processes, char *fstype)
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
        mvwprintw(stdscr, cur_y, LINE_X, "%s  ", processes->name);
        mvwprintw(stdscr, cur_y, LINE_X + 20, "%d   ", processes->pid);
        mvwprintw(stdscr, cur_y, LINE_X + 30, "%d   ", processes->cpuset);
        mvwprintw(stdscr, cur_y, LINE_X + 40, "%s ", processes->user);
        mvwprintw(stdscr, cur_y++, LINE_X + 50, "%.2f%", processes->mempcent);
//        mvwprintw(stdscr, cur_y++, LINE_X + 50, "%.2f", 0.0);
        processes = processes->next;
    }
    box(stdscr, 0, 0);
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
    spaceleft = max_x - 42;
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
