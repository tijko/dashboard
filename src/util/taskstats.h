#ifndef TASKSTATS_UTIL_H
#define TASKSTATS_UTIL_H


#include <stdint.h>
#include <linux/genetlink.h>

#include "../process/process.h"


#define MAX_MSG_SZ 1024

#define GENLMSG_DATA(nlh) ((void *) (NLMSG_DATA(nlh) + GENL_HDRLEN))
#define NLA_ALIGNED_MSG(nla) ((struct nlattr *) (((char *) nla) + \
                                                   NLA_ALIGN(nla->nla_len)))
#define GENLMSG_PAYLOAD(nlh) (NLMSG_PAYLOAD(nlh, 0) - GENL_HDRLEN)
#define NLA_PAYLOAD(nlh) (GENLMSG_PAYLOAD(nlh))
#define NLA_DATA(nla) ((void *) ((char *) nla + NLA_HDRLEN))

struct nl_msg {
    struct nlmsghdr nlh;
    struct genlmsghdr genlh;
    char buf[MAX_MSG_SZ];
};

#define GET_REQUEST_LENGTH(msg) (((struct nl_msg *) msg)->nlh.nlmsg_len)

int create_conn(void);

int get_family_id(int conn);

void build_req(struct nl_msg *req, uint32_t nl_type, uint8_t gnl_cmd, 
               uint16_t nla_type, void *nla_data, uint16_t nla_len);

bool nl_req(int conn, char *buf, int buflen);

bool nl_recv(int conn, struct nl_msg *req);

int taskstats_reply(struct nl_msg *reply, proc_t *procs, char field);

void task_req(proc_t *procs, char field);

#endif
