
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dsplink.h>
#include <proc.h>
#include <msgq.h>
#include <pool.h>

#include "dspmain_msg.h"

struct dspmain_t
{
    MSGQ_Queue dsp_msgq;
    MSGQ_Queue gpp_msgq;
    int reply_msgq;
    int pool_id;
    int sequence;
};

/*****************************************************************************/
static DSP_STATUS
send_mult_msg(struct dspmain_t* dspmain, int x, int y)
{
    DSP_STATUS status;
    MSGQ_Msg msg;
    struct my_msg_mult_t* my_msg_mult;

    status = MSGQ_alloc(dspmain->pool_id, sizeof(struct my_msg_mult_t), &msg);
    if (DSP_SUCCEEDED(status))
    {
        my_msg_mult = (struct my_msg_mult_t*)msg;
        my_msg_mult->subid = MULTMSGSUBID;
        my_msg_mult->sequence = dspmain->sequence++;
        my_msg_mult->reply_msgq = dspmain->reply_msgq;
        my_msg_mult->x = x;
        my_msg_mult->y = y;
        my_msg_mult->z = 0;
        MSGQ_setMsgId(msg, MSGQ_MYMSGID);
        status = MSGQ_put(dspmain->dsp_msgq, msg);
        if (DSP_FAILED(status))
        {
            MSGQ_free(msg);
        }
    }
    return status;
}

/*****************************************************************************/
static DSP_STATUS
send_get_reply_msgq_msg(struct dspmain_t* dspmain, const char* msgq_name)
{
    DSP_STATUS status;
    MSGQ_Msg msg;
    struct my_msg_get_reply_msgq_t* my_msg_get_reply_msgq;

    status = MSGQ_alloc(dspmain->pool_id,
                        sizeof(struct my_msg_get_reply_msgq_t), &msg);
    if (DSP_SUCCEEDED(status))
    {
        my_msg_get_reply_msgq = (struct my_msg_get_reply_msgq_t*)msg;
        my_msg_get_reply_msgq->subid = GETREPLYMSGQMSGSUBID;
        my_msg_get_reply_msgq->sequence = dspmain->sequence++;
        my_msg_get_reply_msgq->reply_msgq = 0;
        strcpy(my_msg_get_reply_msgq->msgq_name, msgq_name);
        MSGQ_setMsgId(msg, MSGQ_MYMSGID);
        status = MSGQ_put(dspmain->dsp_msgq, msg);
        if (DSP_FAILED(status))
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
    struct my_msg_mult_t* my_msg_mult;
    struct my_msg_log_t* my_msg_log;
    struct my_msg_get_reply_msgq_t* my_msg_get_reply_msgq;

    status = DSP_SOK;
    switch (my_msg->subid)
    {
        case MULTMSGSUBID:
            my_msg_mult = (struct my_msg_mult_t*)my_msg;
            printf("process_mymsg: MULTMSGSUBID z %d\n", my_msg_mult->z);
            break;
        case LOGMSGSUBID:
            my_msg_log = (struct my_msg_log_t*)my_msg;
            printf("process_mymsg: LOGMSGSUBID msg from dsp [%s]\n",
                   my_msg_log->log_msg);
            break;
        case GETREPLYMSGQMSGSUBID:
            my_msg_get_reply_msgq = (struct my_msg_get_reply_msgq_t*)my_msg;
            printf("process_mymsg: GETREPLYMSGQMSGSUBID reply_msgq 0x%x\n",
                   my_msg_get_reply_msgq->reply_msgq);
            dspmain->reply_msgq = my_msg_get_reply_msgq->reply_msgq;

            send_mult_msg(dspmain, 4, 8);

            break;
    }
    return status;
}

/*****************************************************************************/
static DSP_STATUS
dspmain_loop(struct dspmain_t* dspmain)
{
    DSP_STATUS status;
    MSGQ_Msg msg;
    struct my_msg_t* my_msg;
    Bool cont;

    status = send_get_reply_msgq_msg(dspmain, "GPPMSGQ1");
    printf("dspmain_loop: send_get_reply_msgq_msg status 0x%8.8lx\n", status);
    if (DSP_SUCCEEDED(status))
    {
        cont = TRUE;
        while (cont)
        {
            status = MSGQ_get(dspmain->gpp_msgq, WAIT_FOREVER, &msg);
            //status = MSGQ_get(*gpp_msgq, 1000, &msg);
            //printf("dspmain_started: MSGQ_get status 0x%8.8lx\n", status);
            if (DSP_SUCCEEDED(status))
            {
                if (MSGQ_getMsgId(msg) == MSGQ_MYMSGID)
                {
                    my_msg = (struct my_msg_t*)msg;
                    status = process_mymsg(dspmain, my_msg);
                    if (DSP_FAILED(status))
                    {
                        cont = FALSE;
                    }
                }
                MSGQ_free(msg);
            }
            else
            {
                cont = FALSE;
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
    printf("dspmain_started: MSGQ_transportOpen status 0x%8.8lx\n", status);
    if (DSP_SUCCEEDED(status))
    {
        memset(&syncLocateAttrs, 0, sizeof(syncLocateAttrs));
        syncLocateAttrs.timeout = WAIT_FOREVER;
        dspmain->dsp_msgq = MSGQ_INVALIDMSGQ;
        status = MSGQ_locate("DSPMSGQ0", &(dspmain->dsp_msgq),
                             &syncLocateAttrs);
        printf("dspmain_started: MSGQ_locate status 0x%8.8lx\n", status);
        if (DSP_SUCCEEDED(status))
        {
            status = dspmain_loop(dspmain);
            printf("dspmain_started: dspmain_loop status 0x%8.8lx\n",
                   status);
        }
        status = MSGQ_transportClose(0);
        printf("dspmain_started: MSGQ_transportClose status 0x%8.8lx\n",
               status);
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
    printf("dspmain_loaded: POOL_open status 0x%8.8lx\n", status);
    if (DSP_SUCCEEDED(status))
    {
        status = PROC_start(0);
        printf("dspmain_loaded: PROC_start status 0x%8.8lx\n", status);
        if (DSP_SUCCEEDED(status))
        {
            status = dspmain_started(dspmain);
            printf("dspmain_loaded: dspmain_started status 0x%8.8lx\n",
                   status);
            status = PROC_stop(0);
            printf("dspmain_loaded: PROC_stop status 0x%8.8lx\n", status);
        }
        status = POOL_close(dspmain->pool_id);
        printf("dspmain_loaded: POOL_close status 0x%8.8lx\n", status);
    }
    return status;
}

/*****************************************************************************/
static DSP_STATUS
dspmain_attached(struct dspmain_t* dspmain)
{
    DSP_STATUS status;

    dspmain->gpp_msgq = MSGQ_INVALIDMSGQ;
    status = MSGQ_open("GPPMSGQ1", &(dspmain->gpp_msgq), NULL);
    printf("dspmain_attached: MSGQ_open status 0x%8.8lx\n", status);
    if (DSP_SUCCEEDED(status))
    {
        status = PROC_load(0, "dspmain.out", 0, 0);
        printf("dspmain_attached: PROC_load status 0x%8.8lx\n", status);
        if (DSP_SUCCEEDED(status))
        {
            status = dspmain_loaded(dspmain);
            printf("dspmain_attached: dspmain_loaded status 0x%8.8lx\n",
                   status);
        }
        status = MSGQ_close(dspmain->gpp_msgq);
        printf("dspmain_attached: MSGQ_close status 0x%8.8lx\n", status);
    }
    return status;
}

/*****************************************************************************/
int
main(int argc, char** argv)
{
    DSP_STATUS status;
    struct dspmain_t dspmain;

    memset(&dspmain, 0, sizeof(dspmain));
    dspmain.pool_id = POOL_makePoolId(0, 0);
    status = PROC_setup(NULL);
    printf("main: PROC_setup status 0x%8.8lx\n", status);
    if (DSP_SUCCEEDED(status))
    {
        status = PROC_attach(0, NULL);
        printf("main: PROC_attach status 0x%8.8lx\n", status);
        if (DSP_SUCCEEDED(status))
        {
            status = dspmain_attached(&dspmain);
            printf("main: dspmain_attached status 0x%8.8lx\n", status);
            status = PROC_detach(0);
            printf("main: PROC_detach status 0x%8.8lx\n", status);
        }
        status = PROC_destroy();
        printf("main: PROC_destroy status 0x%8.8lx\n", status);
    }
    return 0;
}
