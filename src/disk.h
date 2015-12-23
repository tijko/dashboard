#ifndef DISK_H
#define DISK_H


#ifdef __i386__
#define IOPRIO_GET 290
#elif __x86_64__
#define IOPRIO_GET 252
#endif

#define IOPRIO_WHO_PROCESS 1

#define IOPRIO_CLASS_SHIFT (13)
#define IOPRIO_PRIO_MASK ((1UL << IOPRIO_CLASS_SHIFT) - 1)

#define PRIOLEN 8

char *filesystem_type(void);

int ioprio_get(int pid);

char *ioprio_class(int pid);

char *ioprio_class_nice(int pid);

void proc_io(proc_t *procs);

#endif
