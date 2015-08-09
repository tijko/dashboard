#define _POSIX_C_SOURCE 200810L

#include <sys/socket.h>
#include <linux/taskstats.h>

#include "taskstats.h"


int create_conn(void)
{
    int conn;
    struct sockaddr_nl nladdr;
    socklen_t nladdr_len;
    
    nladdr_len = sizeof(struct sockaddr_nl);
    memset(&nladdr, 0, nladdr_len);
    nladdr.nl_family = AF_NETLINK;

    conn = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
    if (conn == -1)
        return -1;

    if (bind(conn, (struct sockaddr *) &nladdr, nladdr_len))
        return -1;

    return conn;
}

int get_family_id(int conn)
{
    int req, family_id, msgleft, req_len;
    char *name, *taskstats_genl_name;
    uint16_t name_len;
    struct nlattr *nla;
    struct nl_msg fam_msg, *nlreq_msg;
   
    name_len = strlen(TASKSTATS_GENL_NAME) + 1;
    taskstats_genl_name = strndup(TASKSTATS_GENL_NAME, name_len);

    nlreq_msg = malloc(sizeof *nlreq_msg);
    build_req(nlreq_msg, GENL_ID_CTRL, CTRL_CMD_GETFAMILY, 
              CTRL_ATTR_FAMILY_NAME, taskstats_genl_name, name_len);

    req_len = GET_REQUEST_LENGTH(nlreq_msg);
    req = nl_req(conn, (char *) nlreq_msg, req_len);

    if (!req)
        return -1;

    memset(&fam_msg, 0, sizeof(fam_msg));
    req = nl_recv(conn, &fam_msg);
    if (!req)
        return -1;

    msgleft = NLA_PAYLOAD((struct nlmsghdr *) &fam_msg);
    family_id = -1;
    nla = (struct nlattr *) GENLMSG_DATA(&fam_msg);

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

void build_req(struct nl_msg *req, uint32_t nl_type, uint8_t gnl_cmd,
               uint16_t nla_type, void *nla_data, uint16_t nla_len)
{
    int pid;
    struct nl_msg nlreq;
    struct nlattr *nla;

    pid = getpid();
    nlreq.nlh.nlmsg_type = nl_type;
    nlreq.nlh.nlmsg_flags = NLM_F_REQUEST;
    nlreq.nlh.nlmsg_seq = 0;
    nlreq.nlh.nlmsg_pid = pid;
    nlreq.nlh.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);

    nlreq.genlh.cmd = gnl_cmd;
    nlreq.genlh.version = 0x1;

    nla = (struct nlattr *) GENLMSG_DATA(&nlreq);
    nla->nla_type = nla_type;
    nla->nla_len = nla_len + 1 + NLA_HDRLEN;
    memcpy(NLA_DATA(nla), nla_data, nla->nla_len);
    nlreq.nlh.nlmsg_len += NLA_ALIGN(nla->nla_len);

    memcpy(req, &nlreq, sizeof(nlreq));
}
            
bool nl_req(int conn, char *buf, int buflen)
{
    int bytes_sent;
    struct sockaddr_nl nladdr;
    
    socklen_t nladdr_len;

    nladdr_len = sizeof(struct sockaddr_nl);
    memset(&nladdr, 0, nladdr_len);
    nladdr.nl_family = AF_NETLINK;

    while ((bytes_sent = sendto(conn, buf, buflen, 0, 
           (struct sockaddr *) &nladdr, nladdr_len)) < buflen) {
        if (bytes_sent > 0) {
            buf += bytes_sent;
            buflen -= bytes_sent;
        } else if (bytes_sent == -1) {
            return false;
        }
    }

    return true;
}

bool nl_recv(int conn, struct nl_msg *req)
{
    int bytes_recv;
    size_t msg_size = sizeof(*req);

    bytes_recv = recv(conn, req, msg_size, 0);
    if (bytes_recv == -1)
        return false;

    return true;
}

int taskstats_reply(struct nl_msg *reply, proc_t *procs, char field)
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
                    case ('d'):                        
                        procs->io_read = tsk->read_bytes;
                        procs->io_write = tsk->write_bytes;
                        break;
                    case ('s'):
                        procs->invol_sw = tsk->nivcsw;
                        break;
                }

                break;
        }
    }
    return 0;
}

void task_req(proc_t *procs, char field)
{
    int pid, conn, family_id, req_len;
    struct nl_msg req, *nlreq_msg;
    conn = create_conn();
    nlreq_msg = malloc(sizeof *nlreq_msg);

    if (conn == -1) 
        goto close_conn;

    family_id = get_family_id(conn);

    if (family_id == -1) 
        goto close_conn;

    pid = procs->pid;
    build_req(nlreq_msg, family_id, TASKSTATS_CMD_GET, 
              TASKSTATS_CMD_ATTR_PID, &pid, sizeof(uint32_t));

    req_len = GET_REQUEST_LENGTH(nlreq_msg);
    if (!nl_req(conn, (char *) nlreq_msg, req_len))
        goto close_conn;

    memset(&req, 0, sizeof(req));
    if (!nl_recv(conn, &req) || req.nlh.nlmsg_type == NLMSG_ERROR)
        goto close_conn;

    taskstats_reply(&req, procs, field);

    free(nlreq_msg);
    close(conn);

    return; 

    close_conn:
        free(nlreq_msg);
        close(conn);
        
        switch (field) {
            case ('d'):
                procs->io_read = 0;
                procs->io_write = 0;
                break;
            case ('s'):
                procs->invol_sw = 0;
                break;
        }

    return;
}
