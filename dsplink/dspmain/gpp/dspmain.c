
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
#include <sys/stat.h>

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

#define UDS_SCK_FILE "/tmp/dspmain.sck"
#define LOG_FILENAME "/tmp/dspmain.log"
#define LAST_LOG_FILENAME "/tmp/dspmain_last.log"
#define MAX_LOG_PATH 256

#define LOG_FLAG_FILE   1
#define LOG_FLAG_STDOUT 2

static int g_term_pipe[2];

static int g_log_level = 4;

struct settings_info
{
    int daemonize;
    int forground;
};

static const char g_log_pre[][8] =
{
    "ERROR",
    "WARN",
    "INFO",
    "DEBUG"
};

static int g_log_fd = -1;
static int g_log_flags = LOG_FLAG_STDOUT;
static char g_log_filename[MAX_LOG_PATH] = LOG_FILENAME;
static char g_last_log_filename[MAX_LOG_PATH] = LAST_LOG_FILENAME;

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
    int mstime;
    int len;

    if (log_level < g_log_level)
    {
        log_line = xnew(char, 2048);
        if (log_line == NULL)
        {
            return 1;
        }
        va_start(ap, format);
        vsnprintf(log_line, 1024, format, ap);
        va_end(ap);
        mstime = get_mstime();
        len = snprintf(log_line + 1024, 1024, "[%10.10u][%s]%s\n",
                       mstime, g_log_pre[log_level % 4], log_line);
        if (g_log_flags & LOG_FLAG_FILE)
        {
            if (g_log_fd == -1)
            {
                free(log_line);
                return 1;
            }
            if (len != write(g_log_fd, log_line + 1024, len))
            {
                free(log_line);
                return 1;
            }
        }
        if (g_log_flags & LOG_FLAG_STDOUT)
        {
            printf("%s", log_line + 1024);
        }
        free(log_line);
    }
    return 0;
}

/*****************************************************************************/
int
log_init(int flags, int log_level)
{
    g_log_flags = flags;
    g_log_level = log_level;
    if (flags & LOG_FLAG_FILE)
    {
        g_log_fd = open(g_log_filename,
                        O_WRONLY | O_CREAT | O_TRUNC,
                        S_IRUSR | S_IWUSR);
        if (g_log_fd == -1)
        {
            return 1;
        }
        if (chmod(g_log_filename, 0666) != 0)
        {
            close(g_log_fd);
            g_log_fd = -1;
            return 1;
        }
    }
    return 0;
}

/*****************************************************************************/
int
log_deinit(void)
{
    if (g_log_fd != -1)
    {
        close(g_log_fd);
        unlink(g_last_log_filename);
        rename(g_log_filename, g_last_log_filename);
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

/*****************************************************************************/
static DSP_STATUS
send_shutdown_msg(struct dspmain_t* dspmain, const char* msgq_name)
{
    DSP_STATUS status;
    MSGQ_Msg msg;
    struct my_msg_shutdown_t* my_msg_shutdown;
    int index;

    status = MSGQ_alloc(dspmain->pool_id,
                        sizeof(struct my_msg_shutdown_t), &msg);
    if (DSP_SUCCEEDED(status))
    {
        my_msg_shutdown = (struct my_msg_shutdown_t*)msg;
        my_msg_shutdown->subid = SHUTDOWNMSGSUBID;
        my_msg_shutdown->sequence = dspmain->sequence++;
        /* set reply_msgq not for reply but so dsp can call MSGQ_release */
        my_msg_shutdown->reply_msgq = dspmain->reply_msgq;
        for (index = 0; index < MSGQ_MYMSGID_NUM_USER; index++)
        {
            my_msg_shutdown->user[index] = 0;
        }
        MSGQ_setMsgId(msg, MSGQ_MYMSGID);
        MSGQ_setSrcQueue(msg, dspmain->gpp_msgq);
        status = MSGQ_put(dspmain->dsp_msgq, msg);
        if (DSP_SUCCEEDED(status))
        {
        }
        else
        {
            MSGQ_free(msg);
        }
    }
    return status;
}

/*****************************************************************************/
DSP_STATUS
send_mult_msg(struct dspmain_t* dspmain, int* user, int num_user,
              int x, int y)
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
        my_msg_mult->z = 0;
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

/*****************************************************************************/
DSP_STATUS
send_crc32_msg(struct dspmain_t* dspmain, int* user, int num_user,
               int addr, int format, int width, int height,
               int stride_bytes)
{
    DSP_STATUS status;
    MSGQ_Msg msg;
    struct my_msg_crc32_t* my_msg_crc32;
    int index;

    status = MSGQ_alloc(dspmain->pool_id, sizeof(struct my_msg_crc32_t), &msg);
    if (DSP_SUCCEEDED(status))
    {
        my_msg_crc32 = (struct my_msg_crc32_t*)msg;
        my_msg_crc32->subid = CRC32MSGSUBID;
        my_msg_crc32->sequence = dspmain->sequence++;
        my_msg_crc32->reply_msgq = dspmain->reply_msgq;
        for (index = 0; index < MSGQ_MYMSGID_NUM_USER; index++)
        {
            my_msg_crc32->user[index] = index < num_user ? user[index] : 0;
        }
        my_msg_crc32->addr = addr;
        my_msg_crc32->format = format;
        my_msg_crc32->width = width;
        my_msg_crc32->height = height;
        my_msg_crc32->stride_bytes = stride_bytes;
        my_msg_crc32->crc32 = 0;
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

/*****************************************************************************/
static DSP_STATUS
process_mymsg(struct dspmain_t* dspmain, struct my_msg_t* my_msg)
{
    DSP_STATUS status;
    struct my_msg_get_reply_msgq_t* my_msg_get_reply_msgq;
    struct my_msg_mult_t* my_msg_mult;
    struct my_msg_crc32_t* my_msg_crc32;
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
        case CRC32MSGSUBID:
            my_msg_crc32 = (struct my_msg_crc32_t*)my_msg;
            LOGLND((LOG_INFO, LOGS "CRC32MSGSUBID z %d", LOGP,
                    my_msg_crc32->crc32));
            if (peer != NULL)
            {
                dspmain_queue_crc32(dspmain, peer, my_msg_crc32);
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
dspmain_wait_fds(struct dspmain_t* dspmain,
                 fd_set* rfds, fd_set* wfds, int timeout)
{
    int error;
    struct timeval time;
    struct timeval* ptime;
    int max_fd;

    FD_ZERO(rfds);
    FD_ZERO(wfds);
    FD_SET(dspmain->listen_sck, rfds);
    max_fd = dspmain->listen_sck;
    FD_SET(g_term_pipe[0], rfds);
    if (g_term_pipe[0] > max_fd)
    {
        max_fd = g_term_pipe[0];
    }
    error = dspmain_peer_get_fds(dspmain, &max_fd, rfds, wfds);
    if (error != DSP_ERROR_NONE)
    {
        LOGLN((LOG_ERROR, LOGS "dspmain_peer_get_fds error %d", LOGP, error));
    }
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
static void
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
        return;
    }
    if (FD_ISSET(dspmain->listen_sck, rfds))
    {
        sock_len = sizeof(s);
        sck = accept(dspmain->listen_sck, (struct sockaddr*)&s, &sock_len);
        if (sck != -1)
        {
            if (dspmain_peer_add_fd(dspmain, sck) != DSP_ERROR_NONE)
            {
                LOGLN((LOG_ERROR, LOGS "dspmain_peer_add_fd failed", LOGP));
                close(sck);
            }
        }
    }
    error = dspmain_peer_check_fds(dspmain, rfds, wfds);
    if ((error != DSP_ERROR_NONE) && (error != DSP_ERROR_PEER_REMOVED))
    {
        LOGLN((LOG_ERROR, LOGS "dspmain_peer_check_fds error %d", LOGP, error));
    }
}

/*****************************************************************************/
static DSP_STATUS
dspmain_loop(struct dspmain_t* dspmain)
{
    DSP_STATUS status;
    fd_set rfds;
    fd_set wfds;

    status = send_get_reply_msgq_msg(dspmain, dspmain->gpp_msgq_name);
    LOGLN((LOG_INFO, LOGS "send_get_reply_msgq_msg status 0x%8.8lx name %s",
           LOGP, status, dspmain->gpp_msgq_name));
    if (DSP_SUCCEEDED(status))
    {
        while (dspmain->reply_msgq == 0)
        {
            status = dspmain_process_one_msgq(dspmain, WAIT_FOREVER);
            if (DSP_FAILED(status))
            {
                return status;
            }
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
        status = send_shutdown_msg(dspmain, dspmain->gpp_msgq_name);
        if (DSP_SUCCEEDED(status))
        {
            LOGLN((LOG_INFO, LOGS "send_shutdown_msg succeeded", LOGP));
            usleep(500 * 1000);
        }
        else
        {
            LOGLN((LOG_INFO, LOGS "send_shutdown_msg failed", LOGP));
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
        LOGLN((LOG_INFO, LOGS "MSGQ_locate status 0x%8.8lx timeout 0x%8.8x "
               "name DSPMSGQ0", LOGP, status, syncLocateAttrs.timeout));
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
    size[0] = MSGQ_MAX_BYTES;
    numBufs[0] = MSGQ_POOL_BYTES / size[0];
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
            usleep(50 * 1000);
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

    snprintf(dspmain->gpp_msgq_name, sizeof(dspmain->gpp_msgq_name),
             "GPPMSGQ%d", getpid());
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
static DSP_STATUS
dspmain_listening(struct dspmain_t* dspmain)
{
    DSP_STATUS status;

    dspmain->pool_id = POOL_makePoolId(0, 0);
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
            status = dspmain_attached(dspmain);
            LOGLN((LOG_INFO, LOGS "dspmain_attached status 0x%8.8lx",
                   LOGP, status));
            status = PROC_detach(0);
            LOGLN((LOG_INFO, LOGS "PROC_detach status 0x%8.8lx", LOGP, status));
        }
        status = PROC_destroy();
        LOGLN((LOG_INFO, LOGS "PROC_destroy status 0x%8.8lx", LOGP, status));
    }
    return status;
}

/*****************************************************************************/
static int
process_args(int argc, char** argv, struct settings_info* settings)
{
    int index;

    if (argc < 1)
    {
        return DSP_ERROR_PARAM;
    }
    for (index = 1; index < argc; index++)
    {
        if (strcmp("-D", argv[index]) == 0)
        {
            settings->daemonize = 1;
        }
        else if (strcmp("-F", argv[index]) == 0)
        {
            settings->forground = 1;
        }
        else
        {
            return DSP_ERROR_PARAM;
        }
    }
    return DSP_ERROR_NONE;
}

/*****************************************************************************/
static int
printf_help(int argc, char** argv)
{
    if (argc < 1)
    {
        return DSP_ERROR_NONE;
    }
    printf("%s: command line options\n", argv[0]);
    printf("    -D      run daemon and log to file, example -D\n");
    printf("    -F      run forground and log to stdout, example -F\n");
    return DSP_ERROR_NONE;
}

/*****************************************************************************/
int
main(int argc, char** argv)
{
    struct settings_info* settings;
    struct dspmain_t* dspmain;
    struct sockaddr_un s;
    int error;
    int sck;
    size_t sz;

    settings = xnew0(struct settings_info, 1);
    if (settings == NULL)
    {
        printf("malloc error\n");
        return 0;
    }
    if (process_args(argc, argv, settings) != DSP_ERROR_NONE)
    {
        printf_help(argc, argv);
        free(settings);
        return 0;
    }
    if (settings->daemonize)
    {
        error = fork();
        if (error == 0)
        {
            close(0);
            close(1);
            close(2);
            open("/dev/null", O_RDONLY);
            open("/dev/null", O_WRONLY);
            open("/dev/null", O_WRONLY);
            log_init(LOG_FLAG_FILE, 4);
        }
        else if (error > 0)
        {
            printf("start daemon with pid %d\n", error);
            free(settings);
            return 0;
        }
        else
        {
            printf("fork failed\n");
            free(settings);
            return 1;
        }
    }
    else if (settings->forground)
    {
        log_init(LOG_FLAG_STDOUT, 4);
    }
    else
    {
        printf_help(argc, argv);
        free(settings);
        return 0;
    }
    dspmain = xnew0(struct dspmain_t, 1);
    if (dspmain != NULL)
    {
        if (pipe(g_term_pipe) == 0)
        {
            sck = socket(PF_LOCAL, SOCK_STREAM, 0);
            if (sck > -1)
            {
                unlink(UDS_SCK_FILE);
                sz = sizeof(s);
                memset(&s, 0, sz);
                s.sun_family = AF_UNIX;
                sz = sizeof(s.sun_path);
                strncpy(s.sun_path, UDS_SCK_FILE, sz);
                s.sun_path[sz - 1] = 0;
                sz = sizeof(s);
                if (bind(sck, (struct sockaddr*)&s, sz) == 0)
                {
                    if (listen(sck, 2) == 0)
                    {
                        dspmain->listen_sck = sck;
                        dspmain_listening(dspmain);
                        dspmain_peer_cleanup(dspmain);
                    }
                    else
                    {
                        LOGLN((LOG_ERROR, LOGS "listen error", LOGP));
                    }
                }
                else
                {
                    LOGLN((LOG_ERROR, LOGS "bind error", LOGP));
                }
                close(sck);
            }
            else
            {
                LOGLN((LOG_ERROR, LOGS "socket error", LOGP));
            }
            close(g_term_pipe[0]);
            close(g_term_pipe[1]);
        }
        else
        {
            LOGLN((LOG_ERROR, LOGS "pipe error", LOGP));
        }
        free(dspmain);
    }
    else
    {
        LOGLN((LOG_ERROR, LOGS "calloc error", LOGP));
    }
    log_deinit();
    free(settings);
    return 0;
}
