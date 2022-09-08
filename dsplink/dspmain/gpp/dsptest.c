
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>

#include <dsplink.h>
#include <proc.h>
#include <msgq.h>
#include <pool.h>

#include "dspmain_msg.h"

struct dsptest_t
{
    MSGQ_Queue dsp_msgq;
    MSGQ_Queue gpp_msgq;
    int reply_msgq;
    int pool_id;
    int sequence;
    char gpp_msgq_name[64];
};

/*****************************************************************************/
static DSP_STATUS
send_mult_msg(struct dsptest_t* dsptest, int x, int y)
{
    DSP_STATUS status;
    MSGQ_Msg msg;
    struct my_msg_mult_t* my_msg_mult;

    status = MSGQ_alloc(dsptest->pool_id, sizeof(struct my_msg_mult_t), &msg);
    if (DSP_SUCCEEDED(status))
    {
        my_msg_mult = (struct my_msg_mult_t*)msg;
        my_msg_mult->subid = MULTMSGSUBID;
        my_msg_mult->sequence = dsptest->sequence++;
        my_msg_mult->reply_msgq = dsptest->reply_msgq;
        my_msg_mult->x = x;
        my_msg_mult->y = y;
        my_msg_mult->z = 0;
        MSGQ_setMsgId(msg, MSGQ_MYMSGID);
        status = MSGQ_put(dsptest->dsp_msgq, msg);
        if (DSP_FAILED(status))
        {
            MSGQ_free(msg);
        }
    }
    return status;
}


/*****************************************************************************/
static DSP_STATUS
send_get_reply_msgq_msg(struct dsptest_t* dsptest, const char* msgq_name)
{
    DSP_STATUS status;
    MSGQ_Msg msg;
    struct my_msg_get_reply_msgq_t* my_msg_get_reply_msgq;

    status = MSGQ_alloc(dsptest->pool_id,
                        sizeof(struct my_msg_get_reply_msgq_t), &msg);
    if (DSP_SUCCEEDED(status))
    {
        my_msg_get_reply_msgq = (struct my_msg_get_reply_msgq_t*)msg;
        my_msg_get_reply_msgq->subid = GETREPLYMSGQMSGSUBID;
        my_msg_get_reply_msgq->sequence = dsptest->sequence++;
        my_msg_get_reply_msgq->reply_msgq = 0;
        strcpy(my_msg_get_reply_msgq->msgq_name, msgq_name);
        MSGQ_setMsgId(msg, MSGQ_MYMSGID);
        status = MSGQ_put(dsptest->dsp_msgq, msg);
        if (DSP_FAILED(status))
        {
            MSGQ_free(msg);
        }
    }
    return status;
}

/*****************************************************************************/
static DSP_STATUS
process_mymsg(struct dsptest_t* dsptest, struct my_msg_t* my_msg)
{
    DSP_STATUS status;
    struct my_msg_mult_t* my_msg_mult;
    struct my_msg_get_reply_msgq_t* my_msg_get_reply_msgq;

    status = DSP_SOK;
    switch (my_msg->subid)
    {
        case MULTMSGSUBID:
            my_msg_mult = (struct my_msg_mult_t*)my_msg;
            printf("process_mymsg: MULTMSGSUBID x %d y %d z %d\n",
                   my_msg_mult->x, my_msg_mult->y, my_msg_mult->z);
            break;
        case GETREPLYMSGQMSGSUBID:
            my_msg_get_reply_msgq = (struct my_msg_get_reply_msgq_t*)my_msg;
            printf("process_mymsg: GETREPLYMSGQMSGSUBID reply_msgq 0x%x\n",
                   my_msg_get_reply_msgq->reply_msgq);
            dsptest->reply_msgq = my_msg_get_reply_msgq->reply_msgq;
            break;
    }
    return status;
}

/*****************************************************************************/
static DSP_STATUS
dsptest_loop(struct dsptest_t* dsptest)
{
    DSP_STATUS status;
    MSGQ_Msg msg;
    struct my_msg_t* my_msg;
    Bool cont;

    status = send_get_reply_msgq_msg(dsptest, dsptest->gpp_msgq_name);
    printf("dsptest_loop: send_get_reply_msgq_msg status 0x%8.8lx\n", status);
    if (DSP_SUCCEEDED(status))
    {
        cont = TRUE;
        while (cont)
        {
            status = MSGQ_get(dsptest->gpp_msgq, 500, &msg);
            //printf("dsptest_loop: MSGQ_get status 0x%8.8lx\n", status);
            if (DSP_SUCCEEDED(status))
            {
                if (MSGQ_getMsgId(msg) == MSGQ_MYMSGID)
                {
                    my_msg = (struct my_msg_t*)msg;
                    status = process_mymsg(dsptest, my_msg);
                    if (DSP_FAILED(status))
                    {
                        cont = FALSE;
                    }
                }
                MSGQ_free(msg);
            }
            else if (status == DSP_ETIMEOUT)
            {
                send_mult_msg(dsptest, rand() % 100, rand() % 100);
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
dsptest_opened(struct dsptest_t* dsptest)
{
    DSP_STATUS status;
    ZCPYMQT_Attrs mqtAttrs;
    MSGQ_LocateAttrs syncLocateAttrs;

    memset(&mqtAttrs, 0, sizeof(mqtAttrs));
    mqtAttrs.poolId = dsptest->pool_id;
    status = MSGQ_transportOpen(0, &mqtAttrs);
    printf("dsptest_opened: MSGQ_transportOpen status 0x%8.8lx\n", status);
    if (DSP_SUCCEEDED(status))
    {
        memset(&syncLocateAttrs, 0, sizeof(syncLocateAttrs));
        syncLocateAttrs.timeout = WAIT_FOREVER;
        dsptest->dsp_msgq = MSGQ_INVALIDMSGQ;
        status = MSGQ_locate("DSPMSGQ0", &(dsptest->dsp_msgq), &syncLocateAttrs);
        printf("dsptest_opened: MSGQ_locate status 0x%8.8lx\n", status);
        if (DSP_SUCCEEDED(status))
        {
            status = dsptest_loop(dsptest);
            printf("dsptest_opened: dsptest_loop status 0x%8.8lx\n", status);
        }
        status = MSGQ_transportClose(0);
        printf("dsptest_opened: MSGQ_transportClose status 0x%8.8lx\n", status);
    }
    return status;
}

/*****************************************************************************/
static DSP_STATUS
dsptest_attached(struct dsptest_t* dsptest)
{
    DSP_STATUS status;

    sprintf(dsptest->gpp_msgq_name, "GPPMSGQ%d", getpid());
    dsptest->gpp_msgq = MSGQ_INVALIDMSGQ;
    status = MSGQ_open(dsptest->gpp_msgq_name, &(dsptest->gpp_msgq), NULL);
    printf("dsptest_attached: MSGQ_open status 0x%8.8lx\n", status);
    if (DSP_SUCCEEDED(status))
    {
        status = POOL_open(dsptest->pool_id, NULL);
        printf("dsptest_attached: POOL_open status 0x%8.8lx\n", status);
        if (DSP_SUCCEEDED(status))
        {
            status = dsptest_opened(dsptest);
            printf("dsptest_attached: dsptest_loaded status 0x%8.8lx\n", status);
        }
        status = MSGQ_close(dsptest->gpp_msgq);
        printf("dsptest_attached: MSGQ_close status 0x%8.8lx\n", status);
    }
    return status;
}

/*****************************************************************************/
int
main(int argc, char** argv)
{
    DSP_STATUS status;
    struct dsptest_t dsptest;

    srand(time(0));
    memset(&dsptest, 0, sizeof(dsptest));
    dsptest.pool_id = POOL_makePoolId(0, 0);
    status = PROC_setup(NULL);
    printf("main: PROC_setup status 0x%8.8lx\n", status);
    if (DSP_SUCCEEDED(status))
    {
        status = PROC_attach(0, NULL);
        printf("main: PROC_attach status 0x%8.8lx\n", status);
        if (DSP_SUCCEEDED(status))
        {
            status = dsptest_attached(&dsptest);
            printf("main: dsptest_attached status 0x%8.8lx\n", status);
            status = PROC_detach(0);
            printf("main: PROC_detach status 0x%8.8lx\n", status);
        }
        status = PROC_destroy();
        printf("main: PROC_destroy status 0x%8.8lx\n", status);
    }
    return 0;
}
