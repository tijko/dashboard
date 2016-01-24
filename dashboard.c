#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/timerfd.h>

#include "dashboard.h"
#include "src/cpu/cpu.h"
#include "src/io/disk.h"
#include "src/ipc/ipc.h"
#include "src/memory/memory.h"
#include "src/process/process.h"
#include "src/display/display.h"
#include "src/util/file_utils.h"
#include "src/system/sys_stats.h"
#include "src/util/sort_fields.h"


void print_usage(void)
{
    printf("Usage:\n\
            dashboard [options]\n\
            dashboard [options] [--sort] <attr>\n\n\
            Dashboard command line systems utility for logging/monitoring\n\
            system processes.  It is an emulation of the old version of the \n\
            linux utility top.  Process attributes and data associated with \n\
            said process can be sorted and logged at the users descretion.\n\n\
            -h, --help              Prints this usage message.\n\
            -l, --log               Logs the data collected to database\n\
                                    under the current users HOME\n\
                                    directory path as database.db.\n\
            -s=ATTR, --sort=ATTR    Sorts the processes based on the ATTR\n\
                                    parameter provided.\n\n\
            ATTR is one of the process attributes being listed by dashboard\n\
            c, cpu                  The number of cores the process is\n\
                                    currently allowed to run on.\n\
            d, fds                  The number of open file descriptors.\n\
            e, pte                  The number of page table entries.\n\
            m, mem                  The amount of memory the process using.\n\
            n, nice                 The niceness of the process.\n\
            p, pid                  The numeric id of the process.\n\
            r, rss                  The resident set size of the process.\n\
            s, invol                The number of involuntary ctxt-swtchs.\n\
            v, vmem                 The amount of virtual memory being\n\
                                    consumed by the process.\n\
            i, write                The total input written by process.\n\
            o, output               The total output done by the process.\n\n");
}

char set_sort_option(char *opt)
{

    char *opts[] = {"cpu", "fds", "pte", "mem", "nice", "pid",
                    "rss", "invol", "vmem", "write", "output"}; 

    char opts_char[] = {'c', 'd', 'e', 'm', 'n', 'p', 
                        'r', 's', 'v', 'i', 'o'};

    int num_opts = sizeof(opts) / sizeof(__typeof__(opts[0]));

    int i;
    for (i=0; i < num_opts; i++)
        if (!strcmp(opt, opts[i]))
            return opts_char[i];

    return 0;
}


void dashboard_mainloop(char attr_sort)
{
    init_screen();

    bool running = true, update_sys = true;
    int process_line_num = 0, prev_process_line_num = 0;

    char *fstype = filesystem_type();

    if (!fstype)
        fstype = "Unavailable";

    board_t *dashboard = init_board();

    dashboard->process_list = build_process_list(dashboard->memtotal, 
                                                 dashboard->euid);

    if (dashboard == NULL)
        return;

    struct itimerspec *sys_timer = malloc(sizeof *sys_timer);
    if (sys_timer == NULL)
        return;

    int sys_timer_fd = set_sys_timer(sys_timer);
    int redraw = 1;

    while (running) {

        int number_of_processes = get_numberof_processes(dashboard->process_list);


        if (attr_sort) // XXX return void from sort --
            dashboard->process_list = sort_by_field(dashboard->process_list, 
                                                    attr_sort,
                                                    number_of_processes);

        getmaxyx(stdscr, dashboard->max_y, dashboard->max_x);

        if (dashboard->max_x ^ dashboard->prev_x) {
            free(dashboard->fieldbar);
            dashboard->fieldbar = build_fieldbar();
        }

        // `xor` the current line positions against the previous
        // if any differ `clear` for a redraw.
        if ((dashboard->prev_y ^ dashboard->max_y) | 
            (dashboard->prev_x ^ dashboard->max_x) |
            (process_line_num ^ prev_process_line_num) || redraw == 1) { 
            clear();
            redraw = 0;
        }

        dashboard->prev_y = dashboard->max_y;
        dashboard->prev_x = dashboard->max_x;
        prev_process_line_num = process_line_num;

        if ((update_screen(dashboard->process_list, update_sys, fstype, 
                           dashboard->fieldbar, process_line_num, 
                           dashboard->max_x, dashboard->max_y)) < 0)
            return;

        int key = wgetch(stdscr);

        switch (key) {

            case (KEY_UP):
                if (process_line_num > 0) 
                    process_line_num--;
                break;

            case (KEY_DOWN):
                if (process_line_num < (number_of_processes - 
                                       (dashboard->max_y - PROC_LINE_SIZE))) 
                    process_line_num++; 
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
                running = false;
                break;

            default:
                break;
        }

        update_process_stats(dashboard);
        dashboard->process_list = update_process_list(dashboard->process_list,
                                                      dashboard->memtotal,
                                                      dashboard->euid, &redraw);
        delay_output(REFRESH_RATE);

        bool update_sys = is_sysfield_timer_expired(sys_timer_fd); 

        if (update_sys) {
            close(sys_timer_fd);
            sys_timer_fd = set_sys_timer(sys_timer);
        }

    }

    endwin();
    free(sys_timer);
    free_board(dashboard);
}

board_t *init_board(void)
{
    board_t *board = malloc(sizeof *board);
    if (board == NULL)
        return NULL;

    board->euid = geteuid();
    board->memtotal = total_memory();

    getmaxyx(stdscr, board->max_y, board->max_x);
    board->prev_x = 0;
    board->prev_y = 0;
    board->fieldbar = build_fieldbar();

    return board;
}

void free_board(board_t *board)
{
    free(board->fieldbar);
    free_process_list(board->process_list);
    free(board);
}

void update_process_stats(board_t *dashboard)
{
    proc_t *process_list = dashboard->process_list;

    long memory = dashboard->memtotal;
    uid_t user = dashboard->euid;

    for (; process_list != NULL; process_list=process_list->next)
        get_process_stats(process_list, memory, user);
}


int main(int argc, char *argv[])
{
    int lopt;

    struct option lopts[] = {
                             {"help", 0, NULL, 'h'},
                             {"sort", 1, NULL, 's'},
                             {NULL,   0, NULL,  0 }
    };

    const char *sopts = "hs:";

    char attr_sort = 0;

    while ((lopt = getopt_long_only(argc, argv, sopts, lopts, NULL)) != -1) {

        switch (lopt) {

            case('h'):
                print_usage();
                return 0;

            case('s'):
                if (optarg)
                    attr_sort = set_sort_option(optarg);
                break;

            case('?'):
                print_usage();
                return 0;

            default:
                break;
        }
    }

    dashboard_mainloop(attr_sort);

    return 0;
}
