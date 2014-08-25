#include "../process.h"
#include "../../display/display.h"


proc_t *sort_by_field(proc_t *procs, int field, int nproc);

void cur_fields(proc_t *proc_arr[], proc_t *cur, int proc_index,
                int field, int cmp_fields[]);
