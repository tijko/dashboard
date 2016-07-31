#define _POSIX_C_SOURCE 200810L

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/taskstats.h>

#include "taskstats.h"


static inline void build_req(struct nl_msg *req, uint32_t nl_type, 
                             uint8_t gnl_cmd, uint16_t nla_type, 
                             void *nla_data, uint16_t nla_len)
{
    struct nlattr *nla;

    req->nlh.nlmsg_type = nl_type;
    req->nlh.nlmsg_flags = NLM_F_REQUEST;
    req->nlh.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);

    req->genlh.cmd = gnl_cmd;
    req->genlh.version = 0x1;

    nla = (struct nlattr *) GENLMSG_DATA(req);
    nla->nla_type = nla_type;
    nla->nla_len = nla_len + NLA_HDRLEN;
    memcpy(NLA_DATA(nla), nla_data, nla_len);
    req->nlh.nlmsg_len += NLA_ALIGN(nla->nla_len);
}

static bool nl_req(int conn, char *buf, int buflen)
{
    int bytes_sent;
    struct sockaddr_nl nladdr;
    
    socklen_t nladdr_len = sizeof(struct sockaddr_nl);
    memset(&nladdr, 0, nladdr_len);
    nladdr.nl_family = AF_NETLINK;

    while ((bytes_sent = sendto(conn, buf, buflen, 0, 
           (struct sockaddr *) &nladdr, nladdr_len)) < buflen) {

        if (bytes_sent == -1)
            return false;

        buf += bytes_sent;
        buflen -= bytes_sent;
    }

    return true;
}

static bool nl_recv(int conn, struct nl_msg *req)
{
    int bytes_recv;
    size_t msg_size = sizeof(*req);

    bytes_recv = recv(conn, req, msg_size, 0);
    if (bytes_recv == -1)
        return false;

    return true;
}

struct nl_session *create_nl_session(void)
{
    struct sockaddr_nl nladdr;
    struct nl_session *nls = malloc(sizeof *nls);

    socklen_t nladdr_len = sizeof(struct sockaddr_nl);
    memset(&nladdr, 0, nladdr_len);
    nladdr.nl_family = AF_NETLINK;

    int conn = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (conn == -1)
        return NULL;

    if (bind(conn, (struct sockaddr *) &nladdr, nladdr_len))
        return NULL;

    nls->nl_conn = conn;
    nls->nl_family_id = get_family_id(conn);

    return nls;
}

int get_family_id(int conn)
{
    char *name;
   
    uint16_t name_len = strlen(TASKSTATS_GENL_NAME) + 1;
    char *taskstats_genl_name = strndup(TASKSTATS_GENL_NAME, name_len);

    struct nl_msg *nlreq_msg = calloc(1, sizeof *nlreq_msg);
    build_req(nlreq_msg, GENL_ID_CTRL, CTRL_CMD_GETFAMILY, 
              CTRL_ATTR_FAMILY_NAME, taskstats_genl_name, name_len);

    int req_len = GET_REQUEST_LENGTH(nlreq_msg);
    int req = nl_req(conn, (char *) nlreq_msg, req_len);

    if (!req)
        return -1;

    struct nl_msg fam_msg;
    memset(&fam_msg, 0, sizeof(fam_msg));
    req = nl_recv(conn, &fam_msg);
    if (!req)
        return -1;

    int msgleft = NLA_PAYLOAD((struct nlmsghdr *) &fam_msg);
    int family_id = -1;
    struct nlattr *nla = (struct nlattr *) GENLMSG_DATA(&fam_msg);

    while (msgleft) {
        switch (nla->nla_type) {
            case (CTRL_ATTR_FAMILY_ID):
                family_id = *((int *) NLA_DATA(nla));                
                break;

            case (CTRL_ATTR_FAMILY_NAME):
                name = NLA_DATA(nla);
                if (strcmp(TASKSTATS_GENL_NAME, name))
                    msgleft = 0;
                break;
        }

        msgleft -= NLA_ALIGN(nla->nla_len);
        nla = NLA_ALIGNED_MSG(nla);
    }

    free(taskstats_genl_name);
    free(nlreq_msg);

    return family_id;
}

uint64_t taskstats_reply(struct nl_msg *reply, char field)
{
    int msgleft;
    struct nlattr *nla;
    struct taskstats *tsk;

    nla = (struct nlattr *) GENLMSG_DATA(reply);
    msgleft = NLA_PAYLOAD((struct nlmsghdr *) reply);

    while (msgleft) {

        switch (nla->nla_type) {

            case (TASKSTATS_TYPE_AGGR_PID):
                nla = (struct nlattr *) NLA_DATA(nla);
                msgleft -= NLA_HDRLEN;
                break;

            case (TASKSTATS_TYPE_PID):
                msgleft -= nla->nla_len;
                nla = NLA_ALIGNED_MSG(nla);
                break;

            case (TASKSTATS_TYPE_STATS):
                msgleft -= nla->nla_len;
                tsk = (struct taskstats *) NLA_DATA(nla);

                switch (field) {

                    case ('i'): 
                        return tsk->read_bytes;

                    case ('o'):
                        return tsk->write_bytes;

                    case ('s'):
                        return tsk->nivcsw;

                    default:
                        break;
                }

                break;
        }
    }

    return 0;
}

uint64_t task_req(int pid, struct nl_session *nls, char field)
{
    struct nl_msg req;

    struct nl_msg *nlreq_msg = calloc(1, sizeof *nlreq_msg);
    uint64_t tstat_reply_value = 0;

    build_req(nlreq_msg, nls->nl_family_id, TASKSTATS_CMD_GET, 
              TASKSTATS_CMD_ATTR_PID, &pid, sizeof(uint32_t));

    int req_len = GET_REQUEST_LENGTH(nlreq_msg);
    if (!nl_req(nls->nl_conn, (char *) nlreq_msg, req_len))
        goto free_msg;

    memset(&req, 0, sizeof(req));

    if (!nl_recv(nls->nl_conn, &req) || req.nlh.nlmsg_type == NLMSG_ERROR)
        goto free_msg;

    tstat_reply_value = taskstats_reply(&req, field);

free_msg:
        free(nlreq_msg);

    return tstat_reply_value; 
}
