#ifndef PROC_H
#define PROC_H


#include <stdint.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>
#include <proc/readproc.h>

#include "../util/taskstats.h"
#include "../system/sys_stats.h"


#define PROCNAME_MAX 1024
#define STAT_PATHMAX 32
#define MAXPROCPATH 2048
#define MAXFIELD 32

typedef int Color;

enum Color {
    BLACK = 0,
    RED   = 1
};

typedef struct process_node {
    proc_t *ps;
    int cpuset;
    int open_fds;
    char *ioprio;
    //char *thrcnt; ? nlwp 0
    char *io_read;
    char *io_write;
    char *invol_sw;
    struct process_node *prev;
    struct process_node *next;
    struct process_node *left;
    struct process_node *right;
    struct process_node *parent;
    Color color;
} ps_node;

typedef struct process_tree {
    ps_node *root;
    ps_node *nil;
    int ps_number;
} Tree;

typedef struct ps_removal {
    struct ps_removal *head;
    ps_node *ps;
    struct ps_removal *next;
} ps_unlink;

ps_node *ps_list;

Tree *build_process_tree(sysaux *system, struct nl_session *nls);

void update_ps_tree(Tree *ps_tree, sysaux *system, struct nl_session *nls);

bool ps_tree_member(Tree *ps_tree, pid_t pid);

ps_node *init_proc(void);

ps_node *create_proc(sysaux *system, struct nl_session *nls);

void get_process_stats(ps_node *process, sysaux *sys, struct nl_session *nls);

void free_process_list(ps_node *process_list);

ps_node *get_head(ps_node *process_list);

char *proc_user(char *path);

int get_current_pids(char **pid_list);

char *get_process_name(char *process);

void free_ps_fields(ps_node *ps);

void insert_process(Tree *tree, ps_node *process, ps_node *new_process);

void delete_process(Tree *tree, ps_node *process, pid_t pid);

void insert_fixup(Tree *tree, ps_node *process);

void delete_fixup(Tree *tree, ps_node *process);

void left_rotate(Tree *tree, ps_node *process);

void right_rotate(Tree *tree, ps_node *process);

ps_node *min_tree(Tree *tree, ps_node *process);

void transplant_process(Tree *tree, ps_node *process_out, 
                                           ps_node *process_in);

void free_ps_tree(Tree *ps_tree);

void free_ps_tree_nodes(Tree *ps_tree, ps_node *ps);

void free_ps(ps_node *ps);

void tree_to_list(Tree *tree, ps_node *ps);

ps_node *get_proc(Tree *tree, ps_node *proc, pid_t pid);

void inorder(Tree *tree, ps_node *ps);

#endif
