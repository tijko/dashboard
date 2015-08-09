#include <errno.h>
#include <string.h>
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
    proc_t *processes;
    
    euid = geteuid();
    int refresh_rate = REFRESH_RATE;
    int memtotal = total_memory();

    int prev_y = 0, prev_x = 0;
    int curr_y = 0, curr_x = 0;
    int plineno = 0, prevplineno = 0;
    getmaxyx(stdscr, curr_y, curr_x);

    char *fstype;
    fstype = filesystem_type();
    if (!fstype)
        fstype = "Unavailable";

    int running = 1;

    int nproc;
    int key;
    int max_y;

    while (running) {

        if ((processes = malloc(sizeof *processes)) == NULL)
            return;
        
        processes->prev = NULL;
        nproc = current_procs(processes, memtotal);

        if (attr_sort)
            processes = sort_by_field(processes, attr_sort, nproc);
        
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
                running = 0;
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
    int cur_y = 9; // set value as macro
    int max_y, max_x;
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
    unsigned int max_x = getmaxx(stdscr);
    char *fieldbar;

    if ((fieldbar = calloc(sizeof(char) * ALLOC_ALIGN(max_x), 
                                     sizeof(char))) == NULL)
        return NULL;

    int i, head; 
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
    
void add_space(char *curbar, char *field, int strterm, int spaces)
{
    int space;
    strcat(curbar + strterm, field);
    strterm += strlen(field);
    for (space=0; space < spaces; space++)
        curbar[strterm + space] = ' ';

    curbar[strterm + space] = '\0';
}
