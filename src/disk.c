#include <fstab.h>
#include <sched.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
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

    class = malloc(sizeof(char) * PRIOLEN);
    
    ioprio = ioprio_get(pid);
    ioprio >>= IOPRIO_SHIFT;
    if (ioprio != 0) 
        snprintf(class, PRIOLEN, "%s", ioprio_classes[ioprio]);
    else 
        class = ioprio_class_nice(pid);
    return class;
}

char *ioprio_class_nice(int pid)
{
    int niceness;
    int ioprio;
    int ioprio_level;
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
    int req;
    int family_id;
    char *taskstats_genl_name;
    uint16_t name_len;
    struct nlattr *nla;
    struct nl_msg fam_msg;
   
    name_len = strlen(TASKSTATS_GENL_NAME) + 1;
    taskstats_genl_name = strndup(TASKSTATS_GENL_NAME, name_len);

    req = nl_req(conn, GENL_ID_CTRL, CTRL_CMD_GETFAMILY,
                 CTRL_ATTR_FAMILY_NAME, taskstats_genl_name, name_len);

    if (req == -1)
        return -1;

    memset(&fam_msg, 0, sizeof(fam_msg));
    req = nl_recv(conn, &fam_msg);
    if (req == -1)
        return -1;

    nla = (struct nlattr *) GENLMSG_DATA(&fam_msg);
    family_id = *(int *) NLA_DATA(nla);
    return family_id;
}

int nl_req(int conn, uint32_t nl_type, uint32_t gnl_cmd,
    uint16_t nla_type, void *nla_data, uint16_t nla_len)
{
    int pid;
    struct nl_msg req;
    struct nlattr *nla;
    struct sockaddr_nl nladdr;
    
    char *buf;
    ssize_t buflen;
    socklen_t nladdr_len;
    int bytes_sent;

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

    nladdr_len = sizeof(struct sockaddr_nl);
    memset(&nladdr, 0, nladdr_len);
    nladdr.nl_family = AF_NETLINK;

    buf = (char *) &req;
    buflen = req.nlh.nlmsg_len;

    while ((bytes_sent = sendto(conn, buf, buflen, 0, 
           (struct sockaddr *) &nladdr, nladdr_len)) < buflen) {
        if (bytes_sent > 0) {
            buf += bytes_sent;
            buflen -= bytes_sent;
        } else if (bytes_sent == -1) {
            return -1;
        }
    }

    return 1;
}

int nl_recv(int conn, struct nl_msg *req)
{
    int bytes_recv;
    size_t msg_size = sizeof(*req);

    bytes_recv = recv(conn, req, msg_size, 0);
    if (bytes_recv == -1)
        return -1;

    return 1;
}

void proc_io(proc_t *procs)
{
    int req;
    int pid;
    int conn;
    int family_id;
    struct nl_msg io_req;
    conn = create_conn();
    if (conn == -1) 
        goto error;

    family_id = get_family_id(conn);
    if (family_id == -1) 
        goto error;

    pid = procs->pid;
    req = nl_req(conn, family_id, TASKSTATS_CMD_GET, 
      TASKSTATS_CMD_ATTR_PID, &pid, sizeof(uint32_t));

    if (req == -1)
        goto error;

    memset(&io_req, 0, sizeof(io_req));
    req = nl_recv(conn, &io_req);
    if (req == -1)
        goto error;

    procs->io_read = 1;    
    procs->io_write = 1;

    return;

    error:
        procs->io_read = 0;
        procs->io_write = 0;
    return;
}    
    
