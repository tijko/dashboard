#ifndef TASKSTATS_UTIL_H
#define TASKSTATS_UTIL_H


#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <linux/genetlink.h>


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

struct nl_session {
    int nl_conn;
    int nl_family_id;
};

#define GET_REQUEST_LENGTH(msg) (((struct nl_msg *) msg)->nlh.nlmsg_len)

struct nl_session *create_nl_session(void);

int get_family_id(int conn);

uint64_t taskstats_reply(struct nl_msg *reply, char field);

uint64_t task_req(int pid, struct nl_session *nls, char field);

#endif
