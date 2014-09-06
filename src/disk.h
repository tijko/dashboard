#define _GNU_SOURCE

#include <linux/genetlink.h>

#ifdef __i386__
#define IOPRIO_GET 290
#elif __x86_64__
#define IOPRIO_GET 252
#endif

#define GENLMSG_DATA(nlh) (void *) (NLMSG_DATA(nlh) + GENL_HDRLEN)
#define NLA_DATA(nla) (void *) ((char *) nla + NLA_HDRLEN)
#define IOPRIO_WHO_PROCESS 1

#define MAX_MSG_SZ 1024

#define IOPRIO_SHIFT 13

#define PRIOLEN 8

struct nl_msg {
    struct nlmsghdr nlh;
    struct genlmsghdr genlh;
    char buf[MAX_MSG_SZ];
};

char *filesystem_type(void);

int ioprio_get(int pid);

char *ioprio_class(int pid);

int create_conn(void);

int get_family_id(int conn);

int nl_req(int conn, uint32_t nl_type, uint32_t gnl_cmd,
    uint16_t nla_type, void *nla_data, uint16_t nla_len);

int nl_recv(int conn, struct nl_msg *req);

void proc_io(proc_t *procs);
