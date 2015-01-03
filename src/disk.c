#include <stdio.h>
#include <fstab.h>
#include <sched.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/syscall.h>
#include <linux/taskstats.h>

#include "cpu.h"
#include "disk.h"


char *ioprio_classes[4] = {"", "Rt", "Be", "Id"};

char *filesystem_type(void)
{
    int ret;

    ret = setfsent();
    if (ret != 1)
        return NULL;

    struct fstab *file_system = getfsent();
    endfsent();
    if (!file_system)
        return NULL;
    return file_system->fs_vfstype;
}

int ioprio_get(int pid)
{
    long ioprio;
    ioprio = syscall(IOPRIO_GET, IOPRIO_WHO_PROCESS, pid);
    return ioprio;
}

char *ioprio_class(int pid)
{
    int ioprio;
    char *class;

    ioprio = ioprio_get(pid);
    ioprio >>= IOPRIO_SHIFT;
    if (ioprio != 0) { 
        class = malloc(sizeof(char) * PRIOLEN);
        snprintf(class, PRIOLEN, "%s", ioprio_classes[ioprio]);
    } else {
        class = ioprio_class_nice(pid);
    }

    return class;
}

char *ioprio_class_nice(int pid)
{
    int niceness, ioprio, ioprio_level;
    char *class;

    class = malloc(sizeof(char) * PRIOLEN);
    ioprio = sched_getscheduler(pid);
    niceness = nice(pid);
    ioprio_level = (niceness + 20) / 5;
    
    if (ioprio == SCHED_FIFO || ioprio == SCHED_RR)
        snprintf(class, PRIOLEN, "%s/%d", ioprio_classes[1], ioprio_level);
    else if (ioprio == SCHED_OTHER)
        snprintf(class, PRIOLEN, "%s/%d", ioprio_classes[2], ioprio_level);
    else
        snprintf(class, PRIOLEN, "%s", ioprio_classes[3]);
    return class;
}

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
    char *nlreq_msg, *name, *taskstats_genl_name;
    uint16_t name_len;
    struct nlattr *nla;
    struct nl_msg fam_msg;
   
    name_len = strlen(TASKSTATS_GENL_NAME) + 1;
    taskstats_genl_name = strndup(TASKSTATS_GENL_NAME, name_len);

    nlreq_msg = build_req(GENL_ID_CTRL, CTRL_CMD_GETFAMILY, 
                         CTRL_ATTR_FAMILY_NAME, taskstats_genl_name, name_len);

    req_len = GET_REQUEST_LENGTH(nlreq_msg);
    req = nl_req(conn, nlreq_msg, req_len);

    if (req == -1)
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
                    return family_id;
                break;
        }

        msgleft -= NLA_ALIGN(nla->nla_len);
        nla = NLA_ALIGNED_MSG(nla);
    }

    free(taskstats_genl_name);
    return family_id;
}

char *build_req(uint32_t nl_type, uint8_t gnl_cmd,
    uint16_t nla_type, void *nla_data, uint16_t nla_len)
{
    int pid;
    struct nl_msg req;
    struct nlattr *nla;

    pid = getpid();
    req.nlh.nlmsg_type = nl_type;
    req.nlh.nlmsg_flags = NLM_F_REQUEST;
    req.nlh.nlmsg_seq = 0;
    req.nlh.nlmsg_pid = pid;
    req.nlh.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN);

    req.genlh.cmd = gnl_cmd;
    req.genlh.version = 0x1;

    nla = (struct nlattr *) GENLMSG_DATA(&req);
    nla->nla_type = nla_type;
    nla->nla_len = nla_len + 1 + NLA_HDRLEN;
    memcpy(NLA_DATA(nla), nla_data, nla->nla_len);
    req.nlh.nlmsg_len += NLA_ALIGN(nla->nla_len);

    return (char *) &req;
}
            
int nl_req(int conn, char *buf, int buflen)
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
            return -1;
        }
    }

    return 0;
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

int taskstats_reply(struct nl_msg *reply, proc_t *procs)
{
    int msgleft;
    struct nlattr *nla;
    struct taskstats *io;

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
                io = (struct taskstats *) NLA_DATA(nla);
                procs->io_read = io->read_bytes;
                procs->io_write = io->write_bytes;
                break;
        }
    }
    return 0;
}

void proc_io(proc_t *procs)
{
    char *nlreq_msg;
    int req, pid, conn, family_id, req_len;
    struct nl_msg io_req;
    conn = create_conn();

    if (conn == -1) 
        goto error;

    family_id = get_family_id(conn);

    if (family_id == -1) 
        goto error;

    pid = procs->pid;
    nlreq_msg = build_req(family_id, TASKSTATS_CMD_GET, 
                          TASKSTATS_CMD_ATTR_PID, &pid, sizeof(uint32_t));

    req_len = GET_REQUEST_LENGTH(nlreq_msg);
    req = nl_req(conn, nlreq_msg, req_len);

    if (req == -1)
        goto error;

    memset(&io_req, 0, sizeof(io_req));
    req = nl_recv(conn, &io_req);

    if (!req || io_req.nlh.nlmsg_type == NLMSG_ERROR)
        goto error;

    req = taskstats_reply(&io_req, procs);

    if (req == -1)
        goto error;
    close(conn);

    return;

    error:
        procs->io_read = 0;
        procs->io_write = 0;
    close(conn);

    return;
}    
