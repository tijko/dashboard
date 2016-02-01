#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "sort_fields.h"
#include "../process/process.h"


proc_t *sort_by_field(proc_t *procs, int field, int nproc)
{
    proc_t *proc_arr[nproc + 1];
    uint64_t cmp_fields[2] = {0, 0};

    init_process_array(proc_arr, procs, nproc);
 
    for (int i=0; i < nproc; i++) {
        proc_t *cur = proc_arr[i + 1];
        int j = i;
        cur_fields(proc_arr, cur, j, field, cmp_fields);
        while (j >= 0 && cmp_fields[0] > cmp_fields[1]) {
            proc_arr[j + 1] = proc_arr[j];
            proc_arr[j--] = cur;
            if (j >= 0)
                cur_fields(proc_arr, cur, j, field, cmp_fields);
        }

        proc_arr[j + 1] = cur;
    }

    return reorder(proc_arr, nproc);
}

void cur_fields(proc_t *proc_arr[], proc_t *cur, int proc_index, 
                int field, uint64_t cmp_fields[])
{
        switch (field) {
            
            case (KEY_C):
                cmp_fields[0] = cur->cpuset;
                cmp_fields[1] = proc_arr[proc_index]->cpuset;
                break;

            case (KEY_D):
                cmp_fields[0] = cur->open_fds;
                cmp_fields[1] = proc_arr[proc_index]->open_fds;
                break;

            case (KEY_E):
                cmp_fields[0] = cur->pte;
                cmp_fields[1] = proc_arr[proc_index]->pte;
                break;
            
            case (KEY_I):
                cmp_fields[0] = cur->io_write;
                cmp_fields[1] = proc_arr[proc_index]->io_write;
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
                cmp_fields[1] = proc_arr[proc_index]->mempcent * 100;
                break;

            case (KEY_N):
                cmp_fields[0] = cur->nice;
                cmp_fields[1] = proc_arr[proc_index]->nice;
                break;

            case (KEY_O):
                cmp_fields[0] = cur->io_read;
                cmp_fields[1] = proc_arr[proc_index]->io_read;
                break;

            case (KEY_P):
                cmp_fields[0] = cur->pid;
                cmp_fields[1] = proc_arr[proc_index]->pid;
                break;

            case (KEY_R):
                cmp_fields[0] = atoi(cur->rss);
                cmp_fields[1] = atoi(proc_arr[proc_index]->rss);
                break;
            
            case (KEY_S):
                cmp_fields[0] = cur->invol_sw;
                cmp_fields[1] = proc_arr[proc_index]->invol_sw;
                break;

            case (KEY_T):
                cmp_fields[0] = atoi(cur->thrcnt);
                cmp_fields[1] = atoi(proc_arr[proc_index]->thrcnt);
                break;

            case (KEY_V):
                cmp_fields[0] = atoi(cur->vmem);
                cmp_fields[1] = atoi(proc_arr[proc_index]->vmem);
                break;
        }
}

proc_t *reorder(proc_t *proc_arr[], int nproc)
{
    int i = 0;
    for (; i < nproc; i++) {
        proc_arr[i]->next = proc_arr[i + 1];
        proc_arr[i + 1]->prev = proc_arr[i];
    }

    proc_arr[i]->next = NULL;
    proc_arr[0]->prev = NULL;

    return proc_arr[0];
}

void init_process_array(proc_t *proc_arr[], proc_t *procs, int nproc)
{
    for (int i=0; i < nproc + 1; i++) {
        proc_arr[i] = procs;
        procs = procs->next;
    }
}    
