#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "file_utils.h"
#include "sort_fields.h"
#include "../process/process.h"


ps_node *sort_by_field(ps_node *procs, int field, int nproc)
{
    ps_node *ps_array[nproc + 1];
    uint64_t cmp_fields[2] = {0, 0};

    nproc = init_ps_array(ps_array, procs, nproc);
 
    for (int i=0; i < nproc; i++) {
        ps_node *cur = ps_array[i + 1];
        int j = i;
        cur_fields(ps_array, cur, j, field, cmp_fields);
        while (j >= 0 && cmp_fields[0] > cmp_fields[1]) {
            ps_array[j + 1] = ps_array[j];
            ps_array[j--] = cur;
            if (j >= 0)
                cur_fields(ps_array, cur, j, field, cmp_fields);
        }

        ps_array[j + 1] = cur;
    }

    return reorder(ps_array, nproc);
}

void cur_fields(ps_node *ps_array[], ps_node *cur, int ps_idx, 
                int field, uint64_t cmp_fields[])
{
    switch (field) {
        
        case (KEY_C):
            cmp_fields[0] = cur->cpuset;
            cmp_fields[1] = ps_array[ps_idx]->cpuset;
            break;

        case (KEY_D):
            cmp_fields[0] = cur->open_fds < 0 ? 0 : cur->open_fds;
            cmp_fields[1] = ps_array[ps_idx]->open_fds < 0 ? 0 : 
                            ps_array[ps_idx]->open_fds;
            break;

        case (KEY_I):
            cmp_fields[0] = cur->io_write == NULL ? 0 : value_from_string(cur->io_write);
            cmp_fields[1] = ps_array[ps_idx]->io_write == NULL ? 
                            0 : value_from_string(ps_array[ps_idx]->io_write);
            break;

        case (KEY_N):
            cmp_fields[0] = cur->ps->nice;
            cmp_fields[1] = ps_array[ps_idx]->ps->nice;
            break;

        case (KEY_O):
            cmp_fields[0] = cur->io_read == NULL ? 0 : value_from_string(cur->io_read);
            cmp_fields[1] = ps_array[ps_idx]->io_read == NULL ? 
                            0 : value_from_string(ps_array[ps_idx]->io_read);
            break;

        case (KEY_P):
            cmp_fields[0] = cur->ps->tid;
            cmp_fields[1] = ps_array[ps_idx]->ps->tid;
            break;

        case (KEY_R):
            cmp_fields[0] = cur->ps->rss;
            cmp_fields[1] = ps_array[ps_idx]->ps->rss;
            break;
        
        case (KEY_S):
            cmp_fields[0] = cur->invol_sw == NULL ? 0 : atoll(cur->invol_sw);
            cmp_fields[1] = ps_array[ps_idx]->invol_sw == NULL ?
                            0 : atoll(ps_array[ps_idx]->invol_sw);
            break;

    //    case (KEY_T):
    //        cmp_fields[0] = atoi(cur->ps->thrcnt);
    //        cmp_fields[1] = atoi(ps_array[ps_idx]->thrcnt);
    //        break;

        case (KEY_V):
            cmp_fields[0] = cur->ps->size;
            cmp_fields[1] = ps_array[ps_idx]->ps->size;
            break;
    }
}

ps_node *reorder(ps_node *ps_array[], int nproc)
{
    int i = 0;
    for (; i < nproc; i++) {
        ps_array[i]->next = ps_array[i + 1];
        ps_array[i + 1]->prev = ps_array[i];
    }

    ps_array[i]->next = NULL;
    ps_array[0]->prev = NULL;

    return ps_array[0];
}

int init_ps_array(ps_node *ps_array[], ps_node *procs, int nproc)
{
    for (int i=0; i < nproc + 1; i++) {
        ps_array[i] = procs;
        procs = procs->next;
    }

    return nproc;
}    
