
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

#define MAX_PROCESSORS 2
#define NUM_MSG_QUEUES 1
#define NUM_POOLS 1

#define DSP_MSGQNAME "DSPMSGQ0"
#define GPP_MSGQNAME "GPPMSGQ1"
#define POOL_ID 0

#define MQT_init ZCPYMQT_init
#define MQT_FXNS ZCPYMQT_FXNS

#define POOL_init SMAPOOL_init
#define POOL_FXNS SMAPOOL_FXNS
#define POOL_PARAMS &PoolParams

SMAPOOL_Params PoolParams =
{
    0, /* Pool ID */
    TRUE
};

ZCPYMQT_Params mqtParams =
{
    POOL_ID
};

static MSGQ_Obj msgQueues[NUM_MSG_QUEUES];

static MSGQ_TransportObj transports[MAX_PROCESSORS] =
{
    MSGQ_NOTRANSPORT,
    {
        &MQT_init,      /* Init Function                 */
        &MQT_FXNS,      /* Transport interface functions */
        &mqtParams,     /* Transport params              */
        NULL,           /* Filled in by transport        */
        ID_GPP          /* Processor Id                  */
     }
};

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

static POOL_Obj pools[NUM_POOLS] =
{
    {
        &POOL_init,             /* Init Function                      */
        (POOL_Fxns*)&POOL_FXNS, /* Pool interface functions           */
        POOL_PARAMS,            /* Pool params                        */
        NULL                    /* Pool object: Set within pool impl. */
    }
};

POOL_Config POOL_config =
{
    pools,
    NUM_POOLS
};

static MSGQ_Queue g_dsp_msgq = MSGQ_INVALIDMSGQ;
static MSGQ_Queue g_gpp_msgq = MSGQ_INVALIDMSGQ;

/* in bios_edma3_drv_sample_omap35xx_cfg.c */
extern EDMA3_DRV_GblConfigParams sampleEdma3GblCfgParams;

/*****************************************************************************/
static int
send_log_msg(const char* text_msg)
{
    MSGQ_Msg msg;
    struct my_msg_log_t* my_msg_log;
    int status;

    if (g_gpp_msgq == MSGQ_INVALIDMSGQ)
    {
        return SYS_OK;
    }
    status = MSGQ_alloc(POOL_ID, &msg, sizeof(struct my_msg_log_t));
    if (status == SYS_OK)
    {
        MSGQ_setMsgId(msg, MSGQ_MYMSGID);
        my_msg_log = (struct my_msg_log_t*)msg;
        my_msg_log->subid = LOGMSGSUBID;
        SYS_sprintf(my_msg_log->log_msg, "%s", text_msg);
        status = MSGQ_put(g_gpp_msgq, msg);
        if (status != SYS_OK)
        {
            MSGQ_free(msg);
        }
    }
    return status;
}

/*****************************************************************************/
static void
process_mymsg(struct my_msg_t* my_msg)
{
    struct my_msg_mult_t* my_msg_mult;

    switch (my_msg->subid)
    {
        case MULTMSGSUBID:
            send_log_msg("process_mymsg: got MULTMSGSUBID");
            my_msg_mult = (struct my_msg_mult_t*)my_msg;
            my_msg_mult->z = my_msg_mult->x * my_msg_mult->y;
            break;
    }
}

/*****************************************************************************/
static void
messageSWI(Arg arg0, Arg arg1)
{
    static Bool firstTime = TRUE;
    MSGQ_LocateAsyncAttrs asyncLocateAttrs = MSGQ_LOCATEASYNCATTRS;
    MSGQ_Msg msg;
    int status;
    struct my_msg_t* my_msg;
    EDMA3_DRV_Result result;
    char text[128];
    MSGQ_AsyncLocateMsg* async_locate_msg;

    (void)arg0;
    (void)arg1;

    if (firstTime)
    {
        firstTime = FALSE;
        asyncLocateAttrs.poolId = POOL_ID;
        MSGQ_locateAsync(GPP_MSGQNAME, g_dsp_msgq, &asyncLocateAttrs);
    }
    else
    {
        status = MSGQ_get(g_dsp_msgq, &msg, 0);
        if (status == SYS_OK)
        {
            switch (MSGQ_getMsgId(msg))
            {
                case MSGQ_MYMSGID:
                    if (g_gpp_msgq != MSGQ_INVALIDMSGQ)
                    {
                        my_msg = (struct my_msg_t*)msg;
                        process_mymsg(my_msg);
                        status = MSGQ_put(g_gpp_msgq, msg);
                        if (status != SYS_OK)
                        {
                            MSGQ_free(msg);
                        }
                    }
                    else
                    {
                        MSGQ_free(msg);
                    }
                    break;
                case MSGQ_ASYNCLOCATEMSGID:
                    async_locate_msg = (MSGQ_AsyncLocateMsg*)msg;
                    g_gpp_msgq = async_locate_msg->msgqQueue;
                    MSGQ_free(msg);
                    send_log_msg("messageSWI: got MSGQ_ASYNCLOCATEMSGID");
                    /* init edma3 */
                    result = EDMA3_DRV_create(0, &sampleEdma3GblCfgParams, NULL);
                    SYS_sprintf(text, "messageSWI: EDMA3_DRV_create result %d", result);
                    send_log_msg(text);
                    break;
                default:
                    MSGQ_free(msg);
                    break;
            }
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
    msgqAttrs.post = (MSGQ_Post) SWI_post;
    msgqAttrs.pend = NULL;
    status = MSGQ_open(DSP_MSGQNAME, &g_dsp_msgq, &msgqAttrs);
    if (status != SYS_OK)
    {
        return;
    }
    MSGQ_setErrorHandler(g_dsp_msgq, POOL_ID);
    SWI_post(swi);
    return;
}
