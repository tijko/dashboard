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

    for (int i=0; i < num_opts; i++)
        if (!strcmp(opt, opts[i]))
            return opts_char[i];

    return 0;
}


void dashboard_mainloop(char attr_sort)
{
    WINDOW *display_windows[2];
    init_windows(display_windows);

    WINDOW *system_window = display_windows[0];
    WINDOW *process_window = display_windows[1];

    bool running = true;
    int ps_ln_number = 0, prev_ps_ln_number = 0;

    char *fstype = filesystem_type();

    if (!fstype)
        fstype = "Unavailable";
    
    // XXX init in init_board...
    ps_list = NULL;
    board_t *dashboard = init_board();
    int prev_ps_number = dashboard->process_tree->ps_number;

    if (dashboard == NULL)
        return;

    struct itimerspec *sys_timer = malloc(sizeof *sys_timer);
    if (sys_timer == NULL)
        return;

    int sys_timer_fd = set_sys_timer(sys_timer);
    bool sys_scr_init = false;

        
    while (running) {

        if (attr_sort)
            dashboard->process_list = sort_by_field(dashboard->process_list, 
                                                    attr_sort,
                                                    dashboard->process_tree->ps_number);

        getmaxyx(process_window, dashboard->max_y, dashboard->max_x);

        if (dashboard->max_x ^ dashboard->prev_x) {
            free(dashboard->fieldbar);
            dashboard->fieldbar = build_fieldbar();
        }

        // `xor` the current line positions against the previous
        // if any differ `clear` for a redraw.
        if ((dashboard->prev_y ^ dashboard->max_y) | 
            (dashboard->prev_x ^ dashboard->max_x) |
            (ps_ln_number ^ prev_ps_ln_number)) { 
            werase(process_window);
        }

        dashboard->prev_y = dashboard->max_y;
        dashboard->prev_x = dashboard->max_x;
        prev_ps_ln_number = ps_ln_number;

        if ((update_process_window(process_window, dashboard->process_list,
                                   dashboard->fieldbar, ps_ln_number, 
                                   dashboard->max_y)) < 0)
            break;

        int key = wgetch(stdscr);

        switch (key) {

            case (KEY_UP):
                if (ps_ln_number > 0) 
                    ps_ln_number--;
                flushinp();
                break;

            case (KEY_DOWN):
                if (ps_ln_number < (dashboard->process_tree->ps_number - 
                                       (dashboard->max_y - PROC_LINE_SIZE))) 
                    ps_ln_number++;
                flushinp();
                break;

            case (KEY_C):
                if (attr_sort != KEY_C)
                    wclear(process_window);
                attr_sort = KEY_C;
                break;

            case (KEY_D):
                if (attr_sort != KEY_D)
                    wclear(process_window);
                attr_sort = KEY_D;
                break;

            case (KEY_E):
                if (attr_sort != KEY_E)
                    wclear(process_window);
                attr_sort = KEY_E;
                break;

            case (KEY_I):
                if (attr_sort != KEY_I)
                    wclear(process_window);
                attr_sort = KEY_I;
                break;

            case (KEY_M):
                if (attr_sort != KEY_M)
                    wclear(process_window);
                attr_sort = KEY_M;
                break;

            case (KEY_N):
                if (attr_sort != KEY_N)
                    wclear(process_window);
                attr_sort = KEY_N;
                break;

            case (KEY_O):
                if (attr_sort != KEY_O)
                    wclear(process_window);
                attr_sort = KEY_O;
                break;

            case (KEY_P):
                if (attr_sort != KEY_P)
                    wclear(process_window);
                attr_sort = KEY_P;
                break;

            case (KEY_R):
                if (attr_sort != KEY_R)
                    wclear(process_window);
                attr_sort = KEY_R;
                break;

            case (KEY_S):
                if (attr_sort != KEY_S)
                    wclear(process_window);
                attr_sort = KEY_S;
                break;

            case (KEY_T):
                if (attr_sort != KEY_T)
                    wclear(process_window);
                attr_sort = KEY_T;
                break;

            case (KEY_V):
                if (attr_sort != KEY_V)
                    wclear(process_window);
                attr_sort = KEY_V;
                break;

            case (KEY_ESCAPE):  
                running = false;
                break;

            default:
                break;
        }
        
        update_process_stats(dashboard->process_tree, 
                             dashboard->process_tree->root, dashboard->system);

        update_ps_tree(dashboard->process_tree, dashboard->system);

        if (prev_ps_number > dashboard->process_tree->ps_number) {
            int diff = prev_ps_number - dashboard->process_tree->ps_number;
            if (ps_ln_number == (prev_ps_number - (dashboard->max_y - 
                                                   PROC_LINE_SIZE)))
                ps_ln_number -= diff;
            else if ((prev_ps_number - (dashboard->max_y - PROC_LINE_SIZE)) - 
                                                       ps_ln_number < diff)
                ps_ln_number -= (diff - ((prev_ps_number - (dashboard->max_y - 
                                          PROC_LINE_SIZE)) - ps_ln_number));
        }

        prev_ps_number = dashboard->process_tree->ps_number;

        ps_list = NULL;
        tree_to_list(dashboard->process_tree, dashboard->process_tree->root);
        dashboard->process_list = get_head(ps_list);

        delay_output(REFRESH_RATE);

        if (!sys_scr_init) {
            update_system_window(system_window, fstype);
            sys_scr_init = true;
        }

        if (is_sysfield_timer_expired(sys_timer_fd)) {
            close(sys_timer_fd);
            sys_timer_fd = set_sys_timer(sys_timer);
            update_system_window(system_window, fstype);
        }
    }

    endwin();
    free(sys_timer);
    free_board(dashboard);

}

board_t *init_board(void)
{
    board_t *board = malloc(sizeof *board);
    board->system = malloc(sizeof *(board->system));
    if (board == NULL)
        return NULL;

    board->system->euid = geteuid();
    board->system->memtotal = total_memory();
    int max_pid_count = max_pids();
    board->system->max_pids = max_pid_count;
    board->system->current_pids = malloc(sizeof(char *) * max_pid_count);
    getmaxyx(stdscr, board->max_y, board->max_x);
    board->prev_x = 0;
    board->prev_y = 0;
    board->fieldbar = build_fieldbar();
    board->process_tree = build_process_tree(board->system); 
    tree_to_list(board->process_tree, board->process_tree->root);
    board->process_list = get_head(ps_list);

    return board;
}

void free_board(board_t *board)
{
    free(board->system->current_pids);
    free(board->system);
    free(board->fieldbar);
    free_ps_tree(board->process_tree);
    free(board);
}

void update_process_stats(proc_tree_t *ps_tree, proc_t *ps, sysaux_t *sys)
{
    if (ps_tree == NULL || ps_tree->root == NULL || 
        ps_tree->root == ps_tree->nil || ps == NULL || ps == ps_tree->nil)
        return;

    update_process_stats(ps_tree, ps->left, sys);
    get_process_stats(ps, sys);
    update_process_stats(ps_tree, ps->right, sys);
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
