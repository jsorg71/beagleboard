
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/socket.h>

#include <dsplink.h>
#include <proc.h>
#include <msgq.h>
#include <pool.h>

#include "arch.h"
#include "parse.h"

#include "dspmain.h"
#include "dspmain_msg.h"
#include "dspmain_peer.h"
#include "dspmain_error.h"

static int g_term_pipe[2];

static int g_log_level = 4;

static const char g_log_pre[][8] =
{
    "ERROR",
    "WARN",
    "INFO",
    "DEBUG"
};

/*****************************************************************************/
static int
get_mstime(void)
{
    struct timeval tp;

    gettimeofday(&tp, 0);
    return (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}

/*****************************************************************************/
int
logln(int log_level, const char* format, ...)
{
    va_list ap;
    char* log_line;

    if (log_level < g_log_level)
    {
        log_line = (char*)malloc(2048);
        va_start(ap, format);
        vsnprintf(log_line, 1024, format, ap);
        va_end(ap);
        snprintf(log_line + 1024, 1024, "[%10.10u][%s]%s",
                 get_mstime(), g_log_pre[log_level % 4], log_line);
        printf("%s\n", log_line + 1024);
        free(log_line);
    }
    return 0;
}

/*****************************************************************************/
static DSP_STATUS
send_get_reply_msgq_msg(struct dspmain_t* dspmain, const char* msgq_name)
{
    DSP_STATUS status;
    MSGQ_Msg msg;
    struct my_msg_get_reply_msgq_t* my_msg_get_reply_msgq;
    int index;

    status = MSGQ_alloc(dspmain->pool_id,
                        sizeof(struct my_msg_get_reply_msgq_t), &msg);
    if (DSP_SUCCEEDED(status))
    {
        my_msg_get_reply_msgq = (struct my_msg_get_reply_msgq_t*)msg;
        my_msg_get_reply_msgq->subid = GETREPLYMSGQMSGSUBID;
        my_msg_get_reply_msgq->sequence = dspmain->sequence++;
        my_msg_get_reply_msgq->reply_msgq = 0;
        for (index = 0; index < MSGQ_MYMSGID_NUM_USER; index++)
        {
            my_msg_get_reply_msgq->user[index] = 0;
        }
        strcpy(my_msg_get_reply_msgq->msgq_name, msgq_name);
        MSGQ_setMsgId(msg, MSGQ_MYMSGID);
        MSGQ_setSrcQueue(msg, dspmain->gpp_msgq);
        status = MSGQ_put(dspmain->dsp_msgq, msg);
        if (DSP_SUCCEEDED(status))
        {
            dspmain->processing_count++;
        }
        else
        {
            MSGQ_free(msg);
        }
    }
    return status;
}

#if 0
/*****************************************************************************/
/* produce a hex dump */
static void
hexdump(const void* p, int len)
{
    unsigned char *line;
    int i;
    int thisline;
    int offset;

    line = (unsigned char*)p;
    offset = 0;

    while (offset < len)
    {
        printf("%04x ", offset);
        thisline = len - offset;

        if (thisline > 16)
        {
            thisline = 16;
        }

        for (i = 0; i < thisline; i++)
        {
            printf("%02x ", line[i]);
        }

        for (; i < 16; i++)
        {
            printf("   ");
        }

        for (i = 0; i < thisline; i++)
        {
            printf("%c", (line[i] >= 0x20 && line[i] < 0x7f) ? line[i] : '.');
        }

        printf("\n");
        offset += thisline;
        line += thisline;
    }
}
#endif

/*****************************************************************************/
DSP_STATUS
send_mult_msg(struct dspmain_t* dspmain, int* user, int num_user,
              int x, int y, int z)
{
    DSP_STATUS status;
    MSGQ_Msg msg;
    struct my_msg_mult_t* my_msg_mult;
    int index;

    status = MSGQ_alloc(dspmain->pool_id, sizeof(struct my_msg_mult_t), &msg);
    if (DSP_SUCCEEDED(status))
    {
        my_msg_mult = (struct my_msg_mult_t*)msg;
        my_msg_mult->subid = MULTMSGSUBID;
        my_msg_mult->sequence = dspmain->sequence++;
        my_msg_mult->reply_msgq = dspmain->reply_msgq;
        for (index = 0; index < MSGQ_MYMSGID_NUM_USER; index++)
        {
            my_msg_mult->user[index] = index < num_user ? user[index] : 0;
        }
        my_msg_mult->x = x;
        my_msg_mult->y = y;
        my_msg_mult->z = z;
        MSGQ_setMsgId(msg, MSGQ_MYMSGID);
        MSGQ_setSrcQueue(msg, dspmain->gpp_msgq);
        status = MSGQ_put(dspmain->dsp_msgq, msg);
        if (DSP_SUCCEEDED(status))
        {
            //hexdump(my_msg_mult, sizeof(struct my_msg_mult_t));
            dspmain->processing_count++;
        }
        else
        {
            MSGQ_free(msg);
        }
    }
    return status;
}

/*****************************************************************************/
static DSP_STATUS
process_mymsg(struct dspmain_t* dspmain, struct my_msg_t* my_msg)
{
    DSP_STATUS status;
    struct my_msg_get_reply_msgq_t* my_msg_get_reply_msgq;
    struct my_msg_mult_t* my_msg_mult;
    struct dspmain_peer_t* peer;

    peer = dspmain->peer_head;
    while (peer != NULL)
    {
        if (peer->id == my_msg->user[0])
        {
            break;
        }
        peer = peer->next;
    }
    status = DSP_SOK;
    switch (my_msg->subid)
    {
        case GETREPLYMSGQMSGSUBID:
            my_msg_get_reply_msgq = (struct my_msg_get_reply_msgq_t*)my_msg;
            LOGLND((LOG_INFO, LOGS "GETREPLYMSGQMSGSUBID reply_msgq 0x%x",
                    LOGP, my_msg_get_reply_msgq->reply_msgq));
            dspmain->reply_msgq = my_msg_get_reply_msgq->reply_msgq;
            break;
        case MULTMSGSUBID:
            my_msg_mult = (struct my_msg_mult_t*)my_msg;
            LOGLND((LOG_INFO, LOGS "MULTMSGSUBID z %d", LOGP, my_msg_mult->z));
            if (peer != NULL)
            {
                dspmain_queue_mult(dspmain, peer, my_msg_mult);
            }
            break;
    }
    return status;
}

/*****************************************************************************/
static DSP_STATUS
dspmain_process_one_msgq(struct dspmain_t* dspmain, int timeout)
{
    DSP_STATUS status;
    MSGQ_Msg msg;
    struct my_msg_t* my_msg;

    status = MSGQ_get(dspmain->gpp_msgq, timeout, &msg);
    if (DSP_SUCCEEDED(status))
    {
        dspmain->processing_count--;
        if (MSGQ_getMsgId(msg) == MSGQ_MYMSGID)
        {
            my_msg = (struct my_msg_t*)msg;
            status = process_mymsg(dspmain, my_msg);
            MSGQ_free(msg);
        }
    }
    else if (status == DSP_ETIMEOUT)
    {
    }
    else if (status == DSP_ENOTCOMPLETE)
    {
    }
    return status;
}

/*****************************************************************************/
static int
dspmain_get_select_info(struct dspmain_t* dspmain, int* max_fd,
                        fd_set* rfds, fd_set* wfds)
{
    int lmax_fd;

    FD_ZERO(rfds);
    FD_ZERO(wfds);
    FD_SET(dspmain->listen_sck, rfds);
    lmax_fd = dspmain->listen_sck;
    FD_SET(g_term_pipe[0], rfds);
    if (g_term_pipe[0] > lmax_fd)
    {
        lmax_fd = g_term_pipe[0];
    }
    if (dspmain_peer_get_fds(dspmain, &lmax_fd, rfds, wfds) != 0)
    {
        LOGLN((LOG_ERROR, LOGS "dspmain_peer_get_fds failed", LOGP));
    }
    *max_fd = lmax_fd;
    return 0;
}

/*****************************************************************************/
static int
dspmain_wait_fds(struct dspmain_t* dspmain,
                 fd_set* rfds, fd_set* wfds, int timeout)
{
    int error;
    struct timeval time;
    struct timeval* ptime;
    int max_fd;

    dspmain_get_select_info(dspmain, &max_fd, rfds, wfds);
    if (timeout == -1)
    {
        ptime = NULL;
    }
    else
    {
        time.tv_sec = timeout / 1000;
        time.tv_usec = (timeout * 1000) % 1000000;
        ptime = &time;
    }
    error = select(max_fd + 1, rfds, wfds, 0, ptime);
    return error;
}

/*****************************************************************************/
static int
dspmain_check_fds(struct dspmain_t* dspmain, fd_set* rfds, fd_set* wfds)
{
    int error;
    int sck;
    socklen_t sock_len;
    struct sockaddr_un s;

    if (FD_ISSET(g_term_pipe[0], rfds))
    {
        LOGLN((LOG_INFO, LOGS "term set", LOGP));
        dspmain->term = 1;
    }
    if (FD_ISSET(dspmain->listen_sck, rfds))
    {
        sock_len = sizeof(struct sockaddr_un);
        sck = accept(dspmain->listen_sck, (struct sockaddr*)&s, &sock_len);
        if (sck != -1)
        {
            if (dspmain_peer_add_fd(dspmain, sck) != 0)
            {
                close(sck);
            }
        }
    }
    error = dspmain_peer_check_fds(dspmain, rfds, wfds);
    return error;
}

/*****************************************************************************/
static DSP_STATUS
dspmain_loop(struct dspmain_t* dspmain)
{
    DSP_STATUS status;
    fd_set rfds;
    fd_set wfds;

    status = send_get_reply_msgq_msg(dspmain, dspmain->gpp_msgq_name);
    LOGLN((LOG_INFO, LOGS "send_get_reply_msgq_msg status 0x%8.8lx",
           LOGP, status));
    if (DSP_SUCCEEDED(status))
    {
        while (dspmain->reply_msgq == 0)
        {
            dspmain_process_one_msgq(dspmain, WAIT_FOREVER);
        }
        LOGLN((LOG_INFO, LOGS "got reply_msgq 0x%x",
               LOGP, dspmain->reply_msgq));
        while (dspmain->term == 0)
        {
            if (dspmain_wait_fds(dspmain, &rfds, &wfds, 0) > 0)
            {
                LOGLND((LOG_INFO, LOGS "calling dspmain_check_fds", LOGP));
                dspmain_check_fds(dspmain, &rfds, &wfds);
            }
            else if (dspmain->processing_count > 0)
            {
                LOGLND((LOG_INFO, LOGS "calling dspmain_process_one_msgq",
                        LOGP));
                dspmain_process_one_msgq(dspmain, WAIT_FOREVER);
            }
            else
            {
                LOGLND((LOG_INFO, LOGS "calling dspmain_wait_fds", LOGP));
                dspmain_wait_fds(dspmain, &rfds, &wfds, -1);
            }
        }
    }
    return status;
}

/*****************************************************************************/
static DSP_STATUS
dspmain_started(struct dspmain_t* dspmain)
{
    DSP_STATUS status;
    ZCPYMQT_Attrs mqtAttrs;
    MSGQ_LocateAttrs syncLocateAttrs;

    memset(&mqtAttrs, 0, sizeof(mqtAttrs));
    mqtAttrs.poolId = dspmain->pool_id;
    status = MSGQ_transportOpen(0, &mqtAttrs);
    LOGLN((LOG_INFO, LOGS "MSGQ_transportOpen status 0x%8.8lx", LOGP, status));
    if (DSP_SUCCEEDED(status))
    {
        memset(&syncLocateAttrs, 0, sizeof(syncLocateAttrs));
        syncLocateAttrs.timeout = WAIT_FOREVER;
        dspmain->dsp_msgq = MSGQ_INVALIDMSGQ;
        status = MSGQ_locate("DSPMSGQ0", &(dspmain->dsp_msgq),
                             &syncLocateAttrs);
        LOGLN((LOG_INFO, LOGS "MSGQ_locate status 0x%8.8lx name DSPMSGQ0",
               LOGP, status));
        if (DSP_SUCCEEDED(status))
        {
            status = dspmain_loop(dspmain);
            LOGLN((LOG_INFO, LOGS "dspmain_loop status 0x%8.8lx",
                   LOGP, status));
        }
        status = MSGQ_transportClose(0);
        LOGLN((LOG_INFO, LOGS "MSGQ_transportClose status 0x%8.8lx",
               LOGP, status));
    }
    return status;
}

/*****************************************************************************/
static DSP_STATUS
dspmain_loaded(struct dspmain_t* dspmain)
{
    DSP_STATUS status;
    SMAPOOL_Attrs poolAttrs;
    Uint32 size[1];
    Uint32 numBufs[1];

    memset(&poolAttrs, 0, sizeof(poolAttrs));
    size[0] = 512;
    numBufs[0] = 0xD0000 / size[0];
    poolAttrs.bufSizes = size;
    poolAttrs.numBuffers = numBufs;
    poolAttrs.numBufPools = 1;
    poolAttrs.exactMatchReq = FALSE;
    status = POOL_open(dspmain->pool_id, &poolAttrs);
    LOGLN((LOG_INFO, LOGS "POOL_open status 0x%8.8lx", LOGP, status));
    if (DSP_SUCCEEDED(status))
    {
        status = PROC_start(0);
        LOGLN((LOG_INFO, LOGS "PROC_start status 0x%8.8lx", LOGP, status));
        if (DSP_SUCCEEDED(status))
        {
            status = dspmain_started(dspmain);
            LOGLN((LOG_INFO, LOGS "dspmain_started status 0x%8.8lx",
                   LOGP, status));
            status = PROC_stop(0);
            LOGLN((LOG_INFO, LOGS "PROC_stop status 0x%8.8lx", LOGP, status));
        }
        status = POOL_close(dspmain->pool_id);
        LOGLN((LOG_INFO, LOGS "POOL_close status 0x%8.8lx", LOGP, status));
    }
    return status;
}

/*****************************************************************************/
static DSP_STATUS
dspmain_attached(struct dspmain_t* dspmain)
{
    DSP_STATUS status;

    sprintf(dspmain->gpp_msgq_name, "GPPMSGQ%d", getpid());
    dspmain->gpp_msgq = MSGQ_INVALIDMSGQ;
    status = MSGQ_open(dspmain->gpp_msgq_name, &(dspmain->gpp_msgq), NULL);
    LOGLN((LOG_INFO, LOGS "MSGQ_open status 0x%8.8lx name %s", LOGP,
           status, dspmain->gpp_msgq_name));
    if (DSP_SUCCEEDED(status))
    {
        status = PROC_load(0, "dspmain.out", 0, 0);
        LOGLN((LOG_INFO, LOGS "PROC_load status 0x%8.8lx", LOGP, status));
        if (DSP_SUCCEEDED(status))
        {
            status = dspmain_loaded(dspmain);
            LOGLN((LOG_INFO, LOGS "dspmain_loaded status 0x%8.8lx",
                   LOGP, status));
        }
        status = MSGQ_close(dspmain->gpp_msgq);
        LOGLN((LOG_INFO, LOGS "MSGQ_close status 0x%8.8lx", LOGP, status));
    }
    return status;
}

/*****************************************************************************/
static void
sig_int(int sig)
{
    (void)sig;
    if (write(g_term_pipe[1], "sig", 4) != 4)
    {
    }
}

/*****************************************************************************/
static void
sig_pipe(int sig)
{
    (void)sig;
}

/*****************************************************************************/
int
main(int argc, char** argv)
{
    DSP_STATUS status;
    struct dspmain_t dspmain;
    struct sockaddr_un s;
    int sck;

    pipe(g_term_pipe);
    memset(&dspmain, 0, sizeof(dspmain));
    sck = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (sck == -1)
    {
        LOGLN((LOG_ERROR, LOGS "sck error", LOGP));
        return 1;
    }
    unlink("/tmp/dspmain.sck");
    memset(&s, 0, sizeof(struct sockaddr_un));
    s.sun_family = AF_UNIX;
    strncpy(s.sun_path, "/tmp/dspmain.sck", sizeof(s.sun_path));
    s.sun_path[sizeof(s.sun_path) - 1] = 0;
    if (bind(sck, (struct sockaddr*)&s, sizeof(struct sockaddr_un)) != 0)
    {
        LOGLN((LOG_ERROR, LOGS "bind failed", LOGP));
        close(sck);
        return 1;
    }
    if (listen(sck, 2) != 0)
    {
        LOGLN((LOG_ERROR, LOGS "listen failed", LOGP));
        close(sck);
        return 1;
    }
    dspmain.listen_sck = sck;
    dspmain.pool_id = POOL_makePoolId(0, 0);
    status = PROC_setup(NULL);
    LOGLN((LOG_INFO, LOGS "PROC_setup status 0x%8.8lx", LOGP, status));
    if (DSP_SUCCEEDED(status))
    {
        status = PROC_attach(0, NULL);
        LOGLN((LOG_INFO, LOGS "PROC_attach status 0x%8.8lx", LOGP, status));
        if (DSP_SUCCEEDED(status))
        {
            signal(SIGINT, sig_int);
            signal(SIGTERM, sig_int);
            signal(SIGPIPE, sig_pipe);
            status = dspmain_attached(&dspmain);
            LOGLN((LOG_INFO, LOGS "dspmain_attached status 0x%8.8lx",
                   LOGP, status));
            status = PROC_detach(0);
            LOGLN((LOG_INFO, LOGS "PROC_detach status 0x%8.8lx", LOGP, status));
        }
        status = PROC_destroy();
        LOGLN((LOG_INFO, LOGS "PROC_destroy status 0x%8.8lx", LOGP, status));
    }
    return 0;
}
