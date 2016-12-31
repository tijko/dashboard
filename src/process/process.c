#define _POSIX_C_SOURCE 200890L

#include <pwd.h>
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdbool.h>

#include "process.h"
#include "../cpu/cpu.h"
#include "../ipc/ipc.h"
#include "../io/disk.h"
#include "../memory/memory.h"
#include "../util/file_utils.h"


static Tree *init_process_tree(void)
{
    Tree *tree = malloc(sizeof *tree);
    tree->root = NULL;
    tree->nil = malloc(sizeof *tree->nil);
    tree->nil->color = BLACK;
    tree->ps_number = 0;

    return tree;
}

Tree *build_process_tree(sysaux *system, struct nl_session *nls)
{
    Tree *tree = init_process_tree();
    proc_t **ps = readproctab(PROC_FILLSTAT | PROC_FILLMEM |
                              PROC_FILLENV  | PROC_FILLUSR |
                              PROC_FILLOOM  | PROC_FILLARG |
                              PROC_FILLSTATUS);

    int nproc = 0;
    for (; ps[nproc]; nproc++) {
        ps_node *node = init_proc_node();
        node->color = RED;
        node->ps = ps[nproc];
        get_process_stats(node, system, nls);
        node->left = tree->nil;
        node->right = tree->nil;
        insert_process(tree, tree->root, node);
    }

    tree->ps_number = nproc;

    return tree; 
}

void get_process_stats(ps_node *process, sysaux *system,
                       struct nl_session *nls)
{
    process->cpuset = current_cpus(process->ps->tid);
    process->ioprio = ioprio_class(process->ps->tid);
    char path[STAT_PATHMAX];
    //process->thrcnt = get_thread_count(process->pidstr);
    memset(path, 0, STAT_PATHMAX - 1); 
    snprintf(path, STAT_PATHMAX - 1, FD, process->ps->tid);
    process->open_fds = current_fds(path);
    
    if (system->euid == 0) {
        uint64_t io_read = get_process_taskstat_io(process->ps->tid, nls, 'o');
        process->io_read = malloc(sizeof(char) * MAXFIELD);
        snprintf(process->io_read, MAXFIELD - 1, "%lu", io_read);
        process->io_read = calculate_size(process->io_read, 0);
        uint64_t io_write = get_process_taskstat_io(process->ps->tid, nls, 'i');
        process->io_write = malloc(sizeof(char) * MAXFIELD);
        snprintf(process->io_write, MAXFIELD - 1, "%lu", io_write);
        process->io_write = calculate_size(process->io_write, 0);
        uint64_t invol_sw = get_process_ctxt_switches(process->ps->tid, nls);
        process->invol_sw = malloc(sizeof(char) * MAXFIELD);
        snprintf(process->invol_sw, MAXFIELD - 1, "%lu", invol_sw);
    } else {
        process->io_write = get_user_ps_write(process->ps->tid);
        process->io_read = get_user_ps_read(process->ps->tid);
        process->invol_sw = get_user_ps_ctxt_switches(process->ps->tid);
    }
}

ps_node *get_proc(Tree *tree, ps_node *proc, pid_t pid)
{
    if (tree == NULL || tree->root == NULL || proc == NULL || proc == tree->nil)
        return NULL;

    while (proc != tree->nil) {
        if (proc->ps->tid == pid)
            return proc;
        else if (proc->ps->tid < pid)
            proc = proc->right;
        else
            proc = proc->left;
    }
        
    return NULL;
}

static inline ps_node *get_tail(ps_node *process_list)
{
    while (process_list->next != NULL)
        process_list = process_list->next;
    return process_list;
}

ps_node *get_head(ps_node *process_list)
{
    while (process_list->prev != NULL)
        process_list = process_list->prev;
    return process_list;
}

bool ps_tree_member(Tree *ps_tree, pid_t pid)
{
    if (ps_tree == NULL || ps_tree->root == NULL || 
        ps_tree->root == ps_tree->nil)
        return false;

    ps_node *ps = ps_tree->root;
    while (ps != ps_tree->nil) {
        if (ps->ps->tid == pid)
            return true;
        else if (ps->ps->tid < pid)
            ps = ps->right;
        else
            ps = ps->left;
    }

    return false;
}

char *get_process_name(char *process)
{
    size_t path_length = strlen(process) + COMM_LEN;
    char *comm = malloc(sizeof(char) * path_length + 1);

    snprintf(comm, path_length, COMM, process);

    int comm_fd = open(comm, O_RDONLY);

    if (comm_fd == -1) {
        free(comm);
        return NULL;
    }

    char *process_name = malloc(sizeof(char) * PROCNAME_MAX);
    read(comm_fd, process_name, PROCNAME_MAX - 1);

    char *newline = strchr(process_name, '\n');
    if (newline == NULL) {
        free(comm);
        free(process_name);
        close(comm_fd);
        return NULL;
    }

    *newline = '\0';

    free(comm);
    close(comm_fd);

    return process_name;
} 

char *proc_user(char *path)
{
    char *uid = parse_proc(path, UID);
    if (uid == NULL) 
        return NULL;
    struct passwd *getuser = getpwuid(atoi(uid));
    if (getuser == NULL)
        return NULL;
    free(uid);
    return strdup(getuser->pw_name);
}

void insert_process(Tree *tree, ps_node *process, ps_node *new_process)
{
    if (!tree->root || tree->root == tree->nil) {
        tree->root = new_process;
        tree->root->color = BLACK;
        tree->root->parent = tree->nil;
        return;
    } else if (process->ps->tid < new_process->ps->tid) {
        if (process->right == tree->nil) {
            process->right = new_process;
            new_process->parent = process;
        } else {
            insert_process(tree, process->right, new_process);
            return;
        }
    } else {
        if (process->left == tree->nil) {
            process->left = new_process;
            new_process->parent = process;
        } else {
            insert_process(tree, process->left, new_process);
            return;
        }
    }

    insert_fixup(tree, new_process);
}

void insert_fixup(Tree *tree, ps_node *process)
{
    while (process->parent->color == RED) {
        if (process->parent == process->parent->parent->left) {
            ps_node *uncle = process->parent->parent->right;
            if (uncle->color == RED) {
                uncle->color = BLACK;
                process->parent->color = BLACK;
                process->parent->parent->color = RED;
                process = process->parent->parent;
            } else {
                if (process == process->parent->right) {
                    process = process->parent;
                    left_rotate(tree, process);
                }

                process->parent->color = BLACK;
                process->parent->parent->color = RED;
                right_rotate(tree, process->parent->parent);
            }
        } else {
            ps_node *uncle = process->parent->parent->left;
            if (uncle->color == RED) {
                uncle->color = BLACK;
                process->parent->color = BLACK;
                process->parent->parent->color = RED;
                process = process->parent->parent;
            } else {
                if (process == process->parent->left) {
                    process = process->parent;
                    right_rotate(tree, process);
                }
                process->parent->color = BLACK;
                process->parent->parent->color = RED;
                left_rotate(tree, process->parent->parent);
            }
        }
    }

    tree->root->color = BLACK;
}

void delete_process(Tree *tree, ps_node *process, pid_t pid)
{
    if (!tree || !tree->root || !tree->nil || !process || 
        tree->root == tree->nil || process == tree->nil)
        return;

    int color;
    ps_node *replace;
    if (process->ps->tid < pid) {
        delete_process(tree, process->left, pid);
        return;
    } else if (process->ps->tid > pid) {
        delete_process(tree, process->right, pid);
        return;
    } else {
        color = process->color;
        if (process->left == tree->nil) {
            replace = process->right;
            transplant_process(tree, process, process->right);
        } else if (process->right == tree->nil) {
            replace = process->left;
            transplant_process(tree, process, process->left);
        } else {
            ps_node *successor = min_tree(tree, process->right);
            color = successor->color;
            replace = successor->right;
            if (process->right == successor)
                replace->parent = successor;
            else {
                transplant_process(tree, successor, successor->right);
                successor->right = process->right;
                successor->right->parent = successor;
            }

            transplant_process(tree, process, successor);
            successor->left = process->left;
            successor->left->parent = successor;
            successor->color = process->color;
        }
    }

    if (color == BLACK)
        delete_fixup(tree, replace);
}

void delete_fixup(Tree *tree, ps_node *process)
{
    while (process != tree->root && process->color == BLACK)
    {
        if (process == process->parent->left) {
            ps_node *sibling = process->parent->right;
            if (sibling->color == RED) {
                sibling->color = BLACK;
                process->parent->color = RED;
                left_rotate(tree, process->parent);
                sibling = process->parent->right;
            }

            if (sibling->left->color == BLACK && 
                sibling->right->color == BLACK) {
                sibling->color = RED;
                process = process->parent;
            } else {
                if (sibling->right->color == BLACK) {
                    sibling->left->color = BLACK;
                    sibling->color = RED;
                    right_rotate(tree, sibling);
                    sibling = process->parent->right;
                }

                sibling->color = process->parent->color;
                process->parent->color = BLACK;
                sibling->right->color = BLACK;
                left_rotate(tree, process->parent);
                process = tree->root;
            }
        } else {
            ps_node *sibling = process->parent->left;
            if (sibling->color == RED) {
                sibling->color = BLACK;
                process->parent->color = RED;
                right_rotate(tree, process->parent);
                sibling = process->parent->left;
            }

            if (sibling->right->color == BLACK &&
                sibling->left->color == BLACK) {
                sibling->color = RED;
                process = process->parent;
            } else {

                if (sibling->left->color == BLACK) {
                    sibling->right->color = BLACK;
                    sibling->color = RED;
                    left_rotate(tree, sibling);
                    sibling = process->parent->left;
                }

                sibling->color = process->parent->color;
                process->parent->color = BLACK;
                sibling->left->color = BLACK;
                right_rotate(tree, process->parent);
                process = tree->root;
            }
        }
    }

    process->color = BLACK;
}

void transplant_process(Tree *tree, ps_node *process_out,
                                    ps_node *process_in)
{
    if (process_out == tree->root)
        tree->root = process_in;
    else if (process_out == process_out->parent->left)
        process_out->parent->left = process_in;
    else
        process_out->parent->right = process_in;
    process_in->parent = process_out->parent;
}

void left_rotate(Tree *tree, ps_node *process)
{
    ps_node *process_up = process->right;
    process->right = process_up->left;
    if (process->right != tree->nil)
        process->right->parent = process;

    if (process == tree->root)
        tree->root = process_up;
    else if (process == process->parent->left)
        process->parent->left = process_up;
    else
        process->parent->right = process_up;

    process_up->parent = process->parent;
    process_up->left = process;
    process->parent = process_up;
}

void right_rotate(Tree *tree, ps_node *process)
{
    ps_node *process_up = process->left;
    process->left = process_up->right;
    if (process->left != tree->nil)
        process->left->parent = process;

    if (process == tree->root)
        tree->root = process_up;
    else if (process == process->parent->left)
        process->parent->left = process_up;
    else
        process->parent->right = process_up;

    process_up->parent = process->parent;
    process_up->right = process;
    process->parent = process_up;
}

ps_node *min_tree(Tree *tree, ps_node *process)
{
    while (process->left != tree->nil)
        process = process->left;
    return process;
}

void tree_to_list(Tree *tree, ps_node *ps)
{
    if (ps == tree->nil)
        return;

    tree_to_list(tree, ps->left);

    if (ps_list == NULL) {
        ps_list = ps;
        ps_list->prev = NULL;
        ps_list->next = NULL;
    } else {
        ps_list->next = ps;
        ps->prev = ps_list;
        ps_list = ps_list->next;
        ps_list->next = NULL;
    }

    tree_to_list(tree, ps->right);
}

ps_node *init_proc_node(void)
{
    ps_node *node = malloc(sizeof *node);
    node->cpuset = 0;
    node->open_fds = 0;
    node->ioprio = NULL;
    //node->thrcnt = NULL;
    node->io_read = NULL;
    node->io_write = NULL;
    node->invol_sw = NULL;
    node->prev = NULL;
    node->next = NULL;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    return node;
}

void free_ps_tree(Tree *ps_tree)
{
    free_ps_tree_nodes(ps_tree, ps_tree->root);
    free(ps_tree->nil);
    free(ps_tree);
}

void free_ps_tree_nodes(Tree *ps_tree, ps_node *ps)
{

    if (ps_tree == NULL || ps == NULL || ps == ps_tree->nil)
        return;
    free_ps_tree_nodes(ps_tree, ps->left);
    free_ps_tree_nodes(ps_tree, ps->right);
    free_ps(ps);
}

void free_ps(ps_node *ps)
{
    ps->next = NULL;
    ps->prev = NULL;
    ps->parent = NULL;
    ps->left = NULL;
    ps->right = NULL;
    free(ps);
}
    
void free_ps_fields(ps_node *ps)
{
    if (ps->ioprio != NULL)
        free(ps->ioprio);
    //if (ps->thrcnt != NULL)
    //    free(ps->thrcnt);
    if (ps->invol_sw != NULL)
        free(ps->invol_sw);
    if (ps->io_read != NULL)
        free(ps->io_read);
    if (ps->io_write != NULL)
        free(ps->io_write);
}

void free_process_list(ps_node *process_list)
{
    for (ps_node *tmp=process_list; process_list != NULL; tmp=process_list) {
        process_list = process_list->next;
        if (tmp->ioprio)
            free(tmp->ioprio);
        //if (tmp->thrcnt)
        //    free(tmp->thrcnt);
        if (tmp->io_read)
            free(tmp->io_read);
        if (tmp->io_write)
            free(tmp->io_write);
        if (tmp->invol_sw)
            free(tmp->invol_sw);
        free(tmp);
    }
}

void inorder(Tree *tree, ps_node *ps)
{
    if (ps == tree->nil)
        return;

    inorder(tree, ps->left);
    printf("%d ", ps->ps->tid);
    inorder(tree, ps->right);
}
