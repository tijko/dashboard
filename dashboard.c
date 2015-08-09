#include <getopt.h>
#include <string.h>
#include "display/display.h"


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

    int num_opts = sizeof(opts) / sizeof( typeof(opts[0])) ;

    int i;
    for (i=0; i < num_opts; i++)
        if (!strcmp(opt, opts[i]))
            return opts_char[i];
    return 0;
}

int main(int argc, char *argv[])
{
    int lopt, log_opt;
    char attr_sort;

    struct option lopts[] = {
        {"help", 0, NULL, 'h'},
        {"log", 0, NULL, 'l'},
        {"sort", 1, NULL, 's'},
        {NULL, 0, NULL, 0}
    };

    const char *sopts = "hls:";

    log_opt = 0;
    attr_sort = 0;

    while ((lopt = getopt_long_only(argc, argv, sopts, lopts, NULL)) != -1) {
        switch (lopt) {
            case('h'):
                print_usage();
                return 0;
            case('l'):
                log_opt = 1;
                break;
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

    init_screen(log_opt, attr_sort);

    return 0;
}
