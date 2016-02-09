#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "sort_fields.h"
#include "../process/process.h"


proc_t *sort_by_field(proc_t *procs, int field, int nproc)
{
    proc_t *ps_array[nproc + 1];
    uint64_t cmp_fields[2] = {0, 0};

    init_ps_array(ps_array, procs, nproc);
 
    for (int i=0; i < nproc; i++) {
        proc_t *cur = ps_array[i + 1];
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

void cur_fields(proc_t *ps_array[], proc_t *cur, int ps_idx, 
                int field, uint64_t cmp_fields[])
{
        switch (field) {
            
            case (KEY_C):
                cmp_fields[0] = cur->cpuset;
                cmp_fields[1] = ps_array[ps_idx]->cpuset;
                break;

            case (KEY_D):
                cmp_fields[0] = cur->open_fds;
                cmp_fields[1] = ps_array[ps_idx]->open_fds;
                break;

            case (KEY_E):
                cmp_fields[0] = cur->pte == NULL ? 0 : atoi(cur->pte);
                cmp_fields[1] = ps_array[ps_idx]->pte == NULL ? 
                                0 : atoi(ps_array[ps_idx]->pte);
                break;
            
            case (KEY_I):
                cmp_fields[0] = cur->io_write == NULL ? 0 : atoll(cur->io_write);
                cmp_fields[1] = ps_array[ps_idx]->io_write == NULL ? 
                                0 : atoll(ps_array[ps_idx]->io_write);
                break;

            /*
             * When ordering based on memory percentage used, multiple the
             * current value by 100.  The memory percentage values  are only 
             * set to the hundreths place and when casted to `uint64_t` the 
             * decimal value is cut out.  On reordering, the original values
             * are maintained and ordered correctly.
             */

            case (KEY_M):   
                cmp_fields[0] = cur->mempcent * 100;
                cmp_fields[1] = ps_array[ps_idx]->mempcent * 100;
                break;

            case (KEY_N):
                cmp_fields[0] = cur->nice;
                cmp_fields[1] = ps_array[ps_idx]->nice;
                break;

            case (KEY_O):
                cmp_fields[0] = cur->io_read == NULL ? 0 : atoll(cur->io_read);
                cmp_fields[1] = ps_array[ps_idx]->io_read == NULL ? 
                                0 : atoll(ps_array[ps_idx]->io_read);
                break;

            case (KEY_P):
                cmp_fields[0] = cur->pid;
                cmp_fields[1] = ps_array[ps_idx]->pid;
                break;

            case (KEY_R):
                cmp_fields[0] = atoi(cur->rss);
                cmp_fields[1] = atoi(ps_array[ps_idx]->rss);
                break;
            
            case (KEY_S):
                cmp_fields[0] = cur->invol_sw == NULL ? 0 : atoll(cur->invol_sw);
                cmp_fields[1] = ps_array[ps_idx]->invol_sw == NULL ?
                                0 : atoll(ps_array[ps_idx]->invol_sw);
                break;

            case (KEY_T):
                cmp_fields[0] = atoi(cur->thrcnt);
                cmp_fields[1] = atoi(ps_array[ps_idx]->thrcnt);
                break;

            case (KEY_V):
                cmp_fields[0] = atoi(cur->vmem);
                cmp_fields[1] = atoi(ps_array[ps_idx]->vmem);
                break;
        }
}

proc_t *reorder(proc_t *ps_array[], int nproc)
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

void init_ps_array(proc_t *ps_array[], proc_t *procs, int nproc)
{
    for (int i=0; i < nproc + 1; i++) {
        ps_array[i] = procs;
        procs = procs->next;
    }
}    
