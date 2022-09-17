
// *((int*)(0x94000080 - 4)) = 0xAABBCCDD;
//sudo ./memdump 2483028092 8
// *((int*)0x10f10000) = 0xAAAAAACB;
// *((int*)0x94200000) = 0xAAAAAACB;
//sudo ./memdump 2485125120 4

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
#include <ti/sdo/edma3/drv/sample/bios_edma3_drv_sample.h>

#include "dspmain_msg.h"
#include "dspmain_edma.h"

#define NUM_POOLS 1
#define NUM_MSG_QUEUES 1
#define MAX_PROCESSORS 2

#define DSP_MSGQNAME "DSPMSGQ0"
#define POOL_ID 0

static ZCPYMQT_Params mqtParams =
{
    POOL_ID
};

static MSGQ_Obj msgQueues[NUM_MSG_QUEUES] = { 0 };

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
        PoolParams + 0,            /* Pool params                        */
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

static SEM_Obj g_notifySemObj = { 0 };

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
process_MULTMSGSUBID(struct my_msg_t* my_msg)
{
    struct my_msg_mult_t* my_msg_mult;

    my_msg_mult = (struct my_msg_mult_t*)my_msg;
    my_msg_mult->z = my_msg_mult->x * my_msg_mult->y;
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
        case GETREPLYMSGQMSGSUBID:
            process_GETREPLYMSGQMSGSUBID(my_msg);
            return;
        case MULTMSGSUBID:
            process_MULTMSGSUBID(my_msg);
            break;
        case CRC32MSGSUBID:
            process_CRC32MSGSUBID(my_msg);
            break;
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
    int status;

    async_locate_msg = (MSGQ_AsyncLocateMsg*)msg;
    if (async_locate_msg->arg != NULL)
    {
        my_msg_get_reply_msgq = (struct my_msg_get_reply_msgq_t*)
                                (async_locate_msg->arg);
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
static int
my_tsk(void)
{
    MSGQ_Msg msg;
    MSGQ_Attrs msgqAttrs;
    int status;

    SEM_new(&g_notifySemObj, 0);
    msgqAttrs = MSGQ_ATTRS;
    msgqAttrs.notifyHandle = &g_notifySemObj;
    msgqAttrs.pend = (MSGQ_Pend)SEM_pendBinary;
    msgqAttrs.post = (MSGQ_Post)SEM_postBinary;
    status = MSGQ_open(DSP_MSGQNAME, &g_dsp_msgq, &msgqAttrs);
    if (status != SYS_OK)
    {
        return 1;
    }
    MSGQ_setErrorHandler(g_dsp_msgq, POOL_ID);
    status = SYS_OK;
    while ((status == SYS_OK) || (status == SYS_ETIMEOUT))
    {
        status = MSGQ_get(g_dsp_msgq, &msg, SYS_FOREVER);
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
    return 0;
}

/*****************************************************************************/
void
main(int argc, char** argv)
{
    DSPLINK_init();
    TSK_create(my_tsk, NULL, 0);
}
