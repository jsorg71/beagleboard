
// *((int*)(0x94000080 - 4)) = 0xAABBCCDD;
//sudo ./memdump 2483028092 8

#include <std.h>
#include <sys.h>
#include <sem.h>
#include <log.h>
#include <tsk.h>
#include <msgq.h>
#include <pool.h>

#include <dsplink.h>
#include <failure.h>
#include <sma_pool.h>
#include <zcpy_mqt.h>

#include <ti/sdo/edma3/drv/edma3_drv.h>

#include "dspmain_msg.h"

#define NUM_POOLS 1
#define NUM_MSG_QUEUES 1
#define MAX_PROCESSORS 2

#define DSP_MSGQNAME "DSPMSGQ0"
#define POOL_ID 0

static ZCPYMQT_Params mqtParams =
{
    POOL_ID
};

static MSGQ_Obj msgQueues[NUM_MSG_QUEUES];

static MSGQ_TransportObj transports[MAX_PROCESSORS] =
{
    MSGQ_NOTRANSPORT,
    {
        &ZCPYMQT_init,  /* Init Function                 */
        &ZCPYMQT_FXNS,  /* Transport interface functions */
        &mqtParams,     /* Transport params              */
        NULL,           /* Filled in by transport        */
        ID_GPP          /* Processor Id                  */
    }
};

/* can not be static */
MSGQ_Config MSGQ_config =
{
    msgQueues,
    transports,
    NUM_MSG_QUEUES,
    MAX_PROCESSORS,
    0,
    MSGQ_INVALIDMSGQ,
    POOL_INVALIDID
};

static SMAPOOL_Params PoolParams[NUM_POOLS] =
{
    {
        0,                 /* Pool ID */
        FALSE              /* Exact Match Requirement */
    }
};

static POOL_Obj pools[NUM_POOLS] =
{
    {
        &SMAPOOL_init,             /* Init Function                      */
        (POOL_Fxns*)&SMAPOOL_FXNS, /* Pool interface functions           */
        &PoolParams[0],            /* Pool params                        */
        NULL                       /* Pool object: Set within pool impl. */
    }
};

/* can not be static */
POOL_Config POOL_config =
{
    pools,
    NUM_POOLS
};

static MSGQ_Queue g_dsp_msgq = MSGQ_INVALIDMSGQ;
static MSGQ_Queue g_log_msgq = MSGQ_INVALIDMSGQ;

/* in bios_edma3_drv_sample_omap35xx_cfg.c */
extern EDMA3_DRV_GblConfigParams sampleEdma3GblCfgParams;

/*****************************************************************************/
static int
send_log_msg(const char* text_msg)
{
    MSGQ_Msg msg;
    struct my_msg_log_t* my_msg_log;
    int status;

    if (g_log_msgq == MSGQ_INVALIDMSGQ)
    {
        return SYS_OK;
    }
    status = MSGQ_alloc(POOL_ID, &msg, sizeof(struct my_msg_log_t));
    if (status == SYS_OK)
    {
        MSGQ_setMsgId(msg, MSGQ_MYMSGID);
        my_msg_log = (struct my_msg_log_t*)msg;
        my_msg_log->subid = LOGMSGSUBID;
        my_msg_log->sequence = 0;
        my_msg_log->reply_msgq = 0;
        SYS_sprintf(my_msg_log->log_msg, "%s", text_msg);
        status = MSGQ_put(g_log_msgq, msg);
        if (status != SYS_OK)
        {
            MSGQ_free(msg);
        }
    }
    return status;
}

/*****************************************************************************/
static void
process_MULTMSGSUBID(struct my_msg_t* my_msg)
{
    struct my_msg_mult_t* my_msg_mult;

    my_msg_mult = (struct my_msg_mult_t*)my_msg;
    my_msg_mult->z = my_msg_mult->x * my_msg_mult->y;
}


/*****************************************************************************/
static void
process_GETREPLYMSGQMSGSUBID(struct my_msg_t* my_msg)
{
    struct my_msg_get_reply_msgq_t* my_msg_get_reply_msgq;
    MSGQ_LocateAsyncAttrs asyncLocateAttrs;

    my_msg_get_reply_msgq = (struct my_msg_get_reply_msgq_t*)my_msg;
    asyncLocateAttrs = MSGQ_LOCATEASYNCATTRS;
    asyncLocateAttrs.poolId = POOL_ID;
    asyncLocateAttrs.arg = (Arg)my_msg;
    MSGQ_locateAsync(my_msg_get_reply_msgq->msgq_name, g_dsp_msgq,
                     &asyncLocateAttrs);
}

/*****************************************************************************/
static void
process_MSGQ_MYMSGID(MSGQ_Msg msg)
{
    struct my_msg_t* my_msg;
    int status;

    my_msg = (struct my_msg_t*)msg;
    switch (my_msg->subid)
    {
        case MULTMSGSUBID:
            process_MULTMSGSUBID(my_msg);
            break;
        case GETREPLYMSGQMSGSUBID:
            process_GETREPLYMSGQMSGSUBID(my_msg);
            return;
    }
    if (my_msg->reply_msgq != 0)
    {
        status = MSGQ_put(my_msg->reply_msgq, msg);
        if (status != SYS_OK)
        {
            MSGQ_free(msg);
        }
    }
    else
    {
        MSGQ_free(msg);
    }
}

/*****************************************************************************/
static void
process_MSGQ_ASYNCLOCATEMSGID(MSGQ_Msg msg)
{
    MSGQ_AsyncLocateMsg* async_locate_msg;
    struct my_msg_get_reply_msgq_t* my_msg_get_reply_msgq;
    char text[128];
    int status;

    async_locate_msg = (MSGQ_AsyncLocateMsg*)msg;
    if (g_log_msgq == MSGQ_INVALIDMSGQ)
    {
        g_log_msgq = async_locate_msg->msgqQueue;
    }
    if (async_locate_msg->arg != NULL)
    {
        my_msg_get_reply_msgq = (struct my_msg_get_reply_msgq_t*)
                                (async_locate_msg->arg);
        SYS_sprintf(text, "messageSWI: got MSGQ_ASYNCLOCATEMSGID "
                    "msgq 0x%x name %s",
                    async_locate_msg->msgqQueue,
                    my_msg_get_reply_msgq->msgq_name);
        send_log_msg(text);
        my_msg_get_reply_msgq->reply_msgq = async_locate_msg->msgqQueue;
        status = MSGQ_put(my_msg_get_reply_msgq->reply_msgq,
                          (MSGQ_Msg)my_msg_get_reply_msgq);
        if (status != SYS_OK)
        {
            MSGQ_free((MSGQ_Msg)my_msg_get_reply_msgq);
        }
    }
    MSGQ_free(msg);
 }

/*****************************************************************************/
static void
messageSWI(Arg arg0, Arg arg1)
{
    MSGQ_Msg msg;
    int status;

    (void)arg0;
    (void)arg1;

    status = MSGQ_get(g_dsp_msgq, &msg, 0);
    if (status == SYS_OK)
    {
        switch (MSGQ_getMsgId(msg))
        {
            case MSGQ_MYMSGID:
                process_MSGQ_MYMSGID(msg);
                break;
            case MSGQ_ASYNCLOCATEMSGID:
                process_MSGQ_ASYNCLOCATEMSGID(msg);
                break;
            default:
                MSGQ_free(msg);
                break;
        }
    }
}

/*****************************************************************************/
void
main(int argc, char** argv)
{
    SWI_Attrs swiAttrs = SWI_ATTRS;
    MSGQ_Attrs msgqAttrs = MSGQ_ATTRS;
    SWI_Handle swi;
    int status;

    DSPLINK_init();
    swiAttrs.fxn = messageSWI;
    swi = SWI_create(&swiAttrs);
    if (swi == NULL)
    {
        return;
    }
    msgqAttrs.notifyHandle = swi;
    msgqAttrs.post = (MSGQ_Post)SWI_post;
    msgqAttrs.pend = NULL;
    status = MSGQ_open(DSP_MSGQNAME, &g_dsp_msgq, &msgqAttrs);
    if (status != SYS_OK)
    {
        return;
    }
    MSGQ_setErrorHandler(g_dsp_msgq, POOL_ID);
    /* init edma3 */
    EDMA3_DRV_create(0, &sampleEdma3GblCfgParams, NULL);
    return;
}
