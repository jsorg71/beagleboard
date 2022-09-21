
#include <std.h>
#include <sma_pool.h>
#include <zcpy_mqt.h>

#include "dspmain.h"

/*
 * The purpose of this file is to provide POOL_config and MSGQ_config.
 * They are used to setup shared memory and message queue.
*/

static ZCPYMQT_Params g_mqtParams =
{
    POOL_ID                 /* Uint16 poolId */
};

static MSGQ_Obj g_msgQueues[NUM_MSG_QUEUES] =
{
    {                       /* MSGQ_Obj */
        NULL,               /* String name */
        {                   /* QUE_Obj queue */
            NULL,           /* QUE_Obj* next */
            NULL            /* QUE_Obj* prev */
        },
        NULL,               /* Ptr notifyHandle */
        NULL,               /* MSGQ_Pend pend */
        NULL,               /* MSGQ_Post post */
        0                   /* Uns status */
    }
};

/*****************************************************************************/
static Int
my_MSGQ_MqtOpen(MSGQ_TransportHandle mqtHandle)
{
    return ZCPYMQT_FXNS.open(mqtHandle);
}

/*****************************************************************************/
static Int
my_MSGQ_MqtClose(MSGQ_TransportHandle mqtHandle)
{
    return ZCPYMQT_FXNS.close(mqtHandle);
}

/*****************************************************************************/
static Int
my_MSGQ_MqtLocate(MSGQ_TransportHandle mqtHandle, String queueName,
                  Bool sync, MSGQ_Queue* msgqQueue, Ptr locateAttrs)
{
    return ZCPYMQT_FXNS.locate(mqtHandle, queueName, sync,
                               msgqQueue, locateAttrs);
}

/*****************************************************************************/
static Int
my_MSGQ_MqtRelease(MSGQ_TransportHandle mqtHandle, MSGQ_Queue msgqQueue)
{
    return ZCPYMQT_FXNS.release(mqtHandle, msgqQueue);
}

/*****************************************************************************/
static Int
my_MSGQ_MqtPut(MSGQ_TransportHandle mqtHandle, MSGQ_Msg msg)
{
    return ZCPYMQT_FXNS.put(mqtHandle, msg);
}

/* our own MSGQ_TransportFxns for fun */
static MSGQ_TransportFxns g_ZCPYMQT_FXNS =
{
    my_MSGQ_MqtOpen,        /* MSGQ_MqtOpen open */
    my_MSGQ_MqtClose,       /* MSGQ_MqtClose close */
    my_MSGQ_MqtLocate,      /* MSGQ_MqtLocate locate */
    my_MSGQ_MqtRelease,     /* MSGQ_MqtRelease release */
    my_MSGQ_MqtPut          /* MSGQ_MqtPut put */
};

static MSGQ_TransportObj g_transports[MAX_PROCESSORS] =
{
    MSGQ_NOTRANSPORT,       /* MSGQ_TransportObj */
    {                       /* MSGQ_TransportObj */
        &ZCPYMQT_init,      /* MSGQ_MqtInit initFxn */
        &g_ZCPYMQT_FXNS,    /* MSGQ_TransportFxns* fxns */
        &g_mqtParams,       /* Ptr params */
        NULL,               /* Ptr object */
        ID_GPP              /* Uint16 procId */
    }
};

/* can not be static */
MSGQ_Config MSGQ_config =
{
    g_msgQueues,            /* MSGQ_Obj* msgqQueues */
    g_transports,           /* MSGQ_TransportObj* transports */
    NUM_MSG_QUEUES,         /* Uint16 numMsgqQueues */
    MAX_PROCESSORS,         /* Uint16 numProcessors */
    0,                      /* Uint16 startUninitialized */
    MSGQ_INVALIDMSGQ,       /* MSGQ_Queue errorQueue */
    POOL_INVALIDID          /* Uint16 errorPoolId */
};

static SMAPOOL_Params g_PoolParams[NUM_POOLS] =
{
    {                       /* SMAPOOL_Params */
        POOL_ID,            /* Uint16 poolId */
        FALSE               /* Bool exactMatchReq */
    }
};

/*****************************************************************************/
static Int
my_POOL_Open(Ptr* object, Ptr params)
{
    return SMAPOOL_FXNS.open(object, params);
}

/*****************************************************************************/
static Void
my_POOL_Close(Ptr object)
{
    SMAPOOL_FXNS.close(object);
}

/*****************************************************************************/
static Int
my_POOL_Alloc(Ptr object, Ptr* buf, size_t size)
{
    return SMAPOOL_FXNS.alloc(object, buf, size);
}

/*****************************************************************************/
static Void
my_POOL_Free(Ptr object, Ptr buf, size_t size)
{
    SMAPOOL_FXNS.free(object, buf, size);
}

/* our own POOL_Fxns because SMAPOOL_FXNS is const */
static POOL_Fxns g_SMAPOOL_FXNS =
{
    my_POOL_Open,           /* POOL_Open open */
    my_POOL_Close,          /* POOL_Close close */
    my_POOL_Alloc,          /* POOL_Alloc alloc */
    my_POOL_Free            /* POOL_Free free */
};

static POOL_Obj g_pools[NUM_POOLS] =
{
    {                       /* POOL_Obj */
        &SMAPOOL_init,      /* POOL_Init initFxn */
        &g_SMAPOOL_FXNS,    /* POOL_Fxns* fxns */
        g_PoolParams + 0,   /* Ptr params */
        NULL                /* Ptr object */
    }
};

/* can not be static */
POOL_Config POOL_config =
{
    g_pools,                /* POOL_Obj* allocators */
    NUM_POOLS               /* Uint16 numAllocators */
};
