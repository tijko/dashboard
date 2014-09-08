#include <stdio.h>

#include "sort_fields.h"


proc_t *sort_by_field(proc_t *procs, int field, int nproc)
{
    int i, j;
    proc_t *head, *cur, *prev;
    proc_t *proc_arr[nproc + 1];
   
    int cmp_fields[2];
 
    for (i=0; i < nproc; i++) {
        proc_arr[i] = procs;
        procs = procs->next;
    }
    
    for (i=0; i < nproc - 1; i++) {
        cur = proc_arr[i + 1];
        j = i;
        cur_fields(proc_arr, cur, j, field, cmp_fields);
        while (j >= 0 && cmp_fields[0] > cmp_fields[1]) {
            proc_arr[j + 1] = proc_arr[j];
            proc_arr[j--] = cur;
            if (j >= 0)
                cur_fields(proc_arr, cur, j, field, cmp_fields);
        }
        proc_arr[j + 1] = cur;
    }

    head = proc_arr[0];
    head->prev = NULL;
    cur = head;
    for (i=1; i < nproc; i++) {
        cur->next = proc_arr[i];
        prev = cur;
        cur = cur->next;
        cur->prev = prev;
    }
    cur->next = NULL;

    return head; 
}

void cur_fields(proc_t *proc_arr[], proc_t *cur, int proc_index, 
                int field, int cmp_fields[])
{
        switch (field) {
            
            case (KEY_C):
                cmp_fields[0] = cur->cpuset;
                cmp_fields[1] = proc_arr[proc_index]->cpuset;
                break;

            case (KEY_E):
                cmp_fields[0] = cur->pte;
                cmp_fields[1] = proc_arr[proc_index]->pte;
                break;

            case (KEY_M):
                cmp_fields[0] = cur->mempcent;
                cmp_fields[1] = proc_arr[proc_index]->mempcent;
                break;

            case (KEY_N):
                cmp_fields[0] = cur->nice;
                cmp_fields[1] = proc_arr[proc_index]->nice;
                break;

            case (KEY_P):
                cmp_fields[0] = cur->pid;
                cmp_fields[1] = proc_arr[proc_index]->pid;
                break;

            case (KEY_R):
                cmp_fields[0] = cur->rss;
                cmp_fields[1] = proc_arr[proc_index]->rss;
                break;

            case (KEY_V):
                cmp_fields[0] = cur->vmem;
                cmp_fields[1] = proc_arr[proc_index]->vmem;
                break;
        }
}
