#define _GNU_SOURCE

#include <stdbool.h>
#include <linux/genetlink.h>

#ifdef __i386__
#define IOPRIO_GET 290
#elif __x86_64__
#define IOPRIO_GET 252
#endif

#define GENLMSG_DATA(nlh) ((void *) (NLMSG_DATA(nlh) + GENL_HDRLEN))
#define NLA_ALIGNED_MSG(nla) ((struct nlattr *) (((char *) nla) + \
                                                   NLA_ALIGN(nla->nla_len)))
#define GENLMSG_PAYLOAD(nlh) (NLMSG_PAYLOAD(nlh, 0) - GENL_HDRLEN)
#define NLA_PAYLOAD(nlh) (GENLMSG_PAYLOAD(nlh))
#define NLA_DATA(nla) ((void *) ((char *) nla + NLA_HDRLEN))

#define IOPRIO_WHO_PROCESS 1

#define MAX_MSG_SZ 1024

#define IOPRIO_SHIFT 13

#define PRIOLEN 8

struct nl_msg {
    struct nlmsghdr nlh;
    struct genlmsghdr genlh;
    char buf[MAX_MSG_SZ];
};

#define GET_REQUEST_LENGTH(msg) (((struct nl_msg *) msg)->nlh.nlmsg_len)

char *filesystem_type(void);

int ioprio_get(int pid);

char *ioprio_class(int pid);

char *ioprio_class_nice(int pid);

int create_conn(void);

int get_family_id(int conn);

char *build_req(uint32_t nl_type, uint8_t gnl_cmd, uint16_t nla_type, 
                                    void *nla_data, uint16_t nla_len);

bool nl_req(int conn, char *buf, int buflen);

bool nl_recv(int conn, struct nl_msg *req);

int taskstats_reply(struct nl_msg *reply, proc_t *procs);

void proc_io(proc_t *procs);
