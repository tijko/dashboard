#ifndef SORT_H
#define SORT_H

#include <stdint.h>

#include "../process/process.h"


#define KEY_C 99
#define KEY_D 100
#define KEY_E 101
#define KEY_H 104
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

ps_node *sort_by_field(ps_node *procs, int field, int nproc);

void cur_fields(ps_node *ps_array[], ps_node *cur, int proc_index,
                int field, uint64_t cmp_fields[]);

ps_node *reorder(ps_node *ps_array[], int nproc);

int init_ps_array(ps_node *ps_array[], ps_node *procs, int nproc);

#endif
