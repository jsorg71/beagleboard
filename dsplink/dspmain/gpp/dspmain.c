
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dsplink.h>
#include <proc.h>
#include <msgq.h>
#include <pool.h>

#include "dspmain_msg.h"

static int g_sequence = 0;

/*****************************************************************************/
static DSP_STATUS
send_mult_msg(MSGQ_Queue* dsp_msgq, int x, int y)
{
    DSP_STATUS status;
    MSGQ_Msg msg;
    struct my_msg_mult_t* my_msg_mult;

    status = MSGQ_alloc(POOL_makePoolId(0, 0), sizeof(struct my_msg_mult_t), &msg);
    if (DSP_SUCCEEDED(status))
    {
        my_msg_mult = (struct my_msg_mult_t*)msg;
        my_msg_mult->subid = MULTMSGSUBID;
        my_msg_mult->sequence = g_sequence++;
        my_msg_mult->x = x;
        my_msg_mult->y = y;
        my_msg_mult->z = 0;
        MSGQ_setMsgId(msg, MSGQ_MYMSGID);
        status = MSGQ_put(*dsp_msgq, msg);
        if (!DSP_SUCCEEDED(status))
        {
            MSGQ_free(msg);
        }
    }
    return status;
}

/*****************************************************************************/
static DSP_STATUS
process_mymsg(struct my_msg_t* my_msg)
{
    DSP_STATUS status;
    struct my_msg_mult_t* my_msg_mult;
    struct my_msg_log_t* my_msg_log;

    status = DSP_SOK;
    switch (my_msg->subid)
    {
        case MULTMSGSUBID:
            my_msg_mult = (struct my_msg_mult_t*)my_msg;
            printf("process_mymsg: z %d\n", my_msg_mult->z);
            break;
        case LOGMSGSUBID:
            my_msg_log = (struct my_msg_log_t*)my_msg;
            printf("process_mymsg: msg from dsp [%s]\n", my_msg_log->log_msg);
            break;
    }
    return status;
}

/*****************************************************************************/
static DSP_STATUS
dspmain_loop(MSGQ_Queue* gpp_msgq, MSGQ_Queue* dsp_msgq)
{
    DSP_STATUS status;
    MSGQ_Msg msg;
    struct my_msg_t* my_msg;
    Bool cont;

    status = send_mult_msg(dsp_msgq, 9, 7);
    printf("dspmain_started: send_mult_msg status 0x%8.8lx\n", status);
    if (DSP_SUCCEEDED(status))
    {
        cont = TRUE;
        while (cont)
        {
            status = MSGQ_get(*gpp_msgq, WAIT_FOREVER, &msg);
            printf("dspmain_started: MSGQ_get status 0x%8.8lx\n", status);
            if (DSP_SUCCEEDED(status))
            {
                if (MSGQ_getMsgId(msg) == MSGQ_MYMSGID)
                {
                    my_msg = (struct my_msg_t*)msg;
                    status = process_mymsg(my_msg);
                    if (!DSP_SUCCEEDED(status))
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
dspmain_started(MSGQ_Queue* gpp_msgq)
{
    DSP_STATUS status;
    ZCPYMQT_Attrs mqtAttrs;
    MSGQ_LocateAttrs syncLocateAttrs;
    MSGQ_Queue dsp_msgq;

    memset(&mqtAttrs, 0, sizeof(mqtAttrs));
    mqtAttrs.poolId = POOL_makePoolId(0, 0);
    status = MSGQ_transportOpen(0, &mqtAttrs);
    printf("dspmain_started: MSGQ_transportOpen status 0x%8.8lx\n", status);
    if (DSP_SUCCEEDED(status))
    {
        memset(&syncLocateAttrs, 0, sizeof(syncLocateAttrs));
        syncLocateAttrs.timeout = WAIT_FOREVER;
        dsp_msgq = MSGQ_INVALIDMSGQ;
        status = MSGQ_locate("DSPMSGQ0", &dsp_msgq, &syncLocateAttrs);
        printf("dspmain_started: MSGQ_locate status 0x%8.8lx\n", status);
        if (DSP_SUCCEEDED(status))
        {
            status = dspmain_loop(gpp_msgq, &dsp_msgq);
            printf("dspmain_started: dspmain_loop status 0x%8.8lx\n", status);
        }
        status = MSGQ_transportClose(0);
        printf("dspmain_started: MSGQ_transportClose status 0x%8.8lx\n", status);
    }
    return status;
}

/*****************************************************************************/
static DSP_STATUS
dspmain_loaded(MSGQ_Queue* gpp_msgq)
{
    DSP_STATUS status;
    SMAPOOL_Attrs poolAttrs;
    Uint32 size[1];
    Uint32 numBufs[1];

    memset(&poolAttrs, 0, sizeof(poolAttrs));
    size[0] = 1024;
    numBufs[0] = 4;
    poolAttrs.bufSizes = size;
    poolAttrs.numBuffers = numBufs;
    poolAttrs.numBufPools = 1;
    //poolAttrs.exactMatchReq = TRUE;
    status = POOL_open(POOL_makePoolId(0, 0), &poolAttrs);
    printf("dspmain_loaded: POOL_open status 0x%8.8lx\n", status);
    if (DSP_SUCCEEDED(status))
    {
        status = PROC_start(0);
        printf("dspmain_loaded: PROC_start status 0x%8.8lx\n", status);
        if (DSP_SUCCEEDED(status))
        {
            status = dspmain_started(gpp_msgq);
            printf("dspmain_loaded: dspmain_started status 0x%8.8lx\n", status);
            status = PROC_stop(0);
            printf("dspmain_loaded: PROC_stop status 0x%8.8lx\n", status);
        }
        status = POOL_close(POOL_makePoolId(0, 0));
        printf("dspmain_loaded: POOL_close status 0x%8.8lx\n", status);
    }
    return status;
}

/*****************************************************************************/
static DSP_STATUS
dspmain_attached(void)
{
    DSP_STATUS status;
    MSGQ_Queue gpp_msgq;

    gpp_msgq = MSGQ_INVALIDMSGQ;
    status = MSGQ_open("GPPMSGQ1", &gpp_msgq, NULL);
    printf("dspmain_attached: MSGQ_open status 0x%8.8lx\n", status);
    if (DSP_SUCCEEDED(status))
    {
        status = PROC_load(0, "dspmain.out", 0, 0);
        printf("dspmain_attached: PROC_load status 0x%8.8lx\n", status);
        if (DSP_SUCCEEDED(status))
        {
            status = dspmain_loaded(&gpp_msgq);
            printf("dspmain_attached: dspmain_loaded status 0x%8.8lx\n", status);
        }
        status = MSGQ_close(gpp_msgq);
        printf("dspmain_attached: MSGQ_close status 0x%8.8lx\n", status);
    }
    return status;
}

/*****************************************************************************/
int
main(int argc, char** argv)
{
    DSP_STATUS status;

    status = PROC_setup(NULL);
    printf("main: PROC_setup status 0x%8.8lx\n", status);
    if (DSP_SUCCEEDED(status))
    {
        status = PROC_attach(0, NULL);
        printf("main: PROC_attach status 0x%8.8lx\n", status);
        if (DSP_SUCCEEDED(status))
        {
            status = dspmain_attached();
            printf("main: dspmain_attached status 0x%8.8lx\n", status);
            status = PROC_detach(0);
            printf("main: PROC_detach status 0x%8.8lx\n", status);
        }
        status = PROC_destroy();
        printf("main: PROC_destroy status 0x%8.8lx\n", status);
    }
    return 0;
}
