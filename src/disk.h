#define _GNU_SOURCE

#ifdef __i386__
#define IOPRIO_GET 290
#elif __x86_64__
#define IOPRIO_GET 252
#endif

#define IOPRIO_WHO_PROCESS 1

char *filesystem_type(void);

int ioprio_get(int pid);
