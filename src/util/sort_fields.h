#ifndef SORT_H
#define SORT_H

#include <stdint.h>

#include "../process/process.h"


#define KEY_C 99
#define KEY_D 100
#define KEY_E 101
#define KEY_I 105
#define KEY_M 109
#define KEY_N 110
#define KEY_O 111
#define KEY_P 112
#define KEY_R 114
#define KEY_S 115
#define KEY_T 116
#define KEY_V 118
#define KEY_ESCAPE 113

proc_t *sort_by_field(proc_t *procs, int field, int nproc);

void cur_fields(proc_t *proc_arr[], proc_t *cur, int proc_index,
                int field, uint64_t cmp_fields[]);

proc_t *reorder(proc_t *proc_arr[], proc_t *head, int nproc);

void init_process_array(proc_t *proc_arr[], proc_t *procs, int nproc);

#endif
