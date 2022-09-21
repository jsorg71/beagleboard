
#include <std.h>
#include <msgq.h>
#include <sem.h>

#include <ti/sdo/edma3/drv/edma3_drv.h>

#include "dspmain_msg.h"

extern EDMA3_DRV_Handle hEdma; /* in bios_edma3_drv_sample_init.c */

extern SEM_Obj g_edmaSemObj; /* in dspmain.c */

static volatile short g_irqRaised1 = 0;

/* OPT Field specific defines */
#define OPT_SYNCDIM_SHIFT                   (0x00000002u)
#define OPT_TCC_MASK                        (0x0003F000u)
#define OPT_TCC_SHIFT                       (0x0000000Cu)
#define OPT_ITCINTEN_SHIFT                  (0x00000015u)
#define OPT_TCINTEN_SHIFT                   (0x00000014u)

#define L1DBUFSIZE (15 * 1024)
#define L1DBUF1 (0x10f10000 + 1 * 1024)
#define L1DBUF2 (0x10f10000 + 16 * 1024)

/*****************************************************************************/
/* Callback function 1 */
static void
callback1(unsigned int tcc, EDMA3_RM_TccStatus status, void* appData)
{
    (void)tcc;
    (void)appData;

    switch (status)
    {
        case EDMA3_RM_XFER_COMPLETE:
            /* Transfer completed successfully */
            g_irqRaised1 = 1;
            SEM_post(&g_edmaSemObj);
            break;
        case EDMA3_RM_E_CC_DMA_EVT_MISS:
            /* Transfer resulted in DMA event miss error. */
            g_irqRaised1 = -1;
            SEM_post(&g_edmaSemObj);
            break;
        case EDMA3_RM_E_CC_QDMA_EVT_MISS:
            /* Transfer resulted in QDMA event miss error. */
            g_irqRaised1 = -2;
            SEM_post(&g_edmaSemObj);
            break;
        default:
            break;
    }
}

/*****************************************************************************/
static EDMA3_DRV_Result
request_channels(unsigned int* chids, unsigned int* tcc)
{
    EDMA3_DRV_Result result;

    chids[0] = EDMA3_DRV_DMA_CHANNEL_ANY;
    tcc[0] = EDMA3_DRV_TCC_ANY;
    result = EDMA3_DRV_requestChannel(hEdma, chids + 0, tcc + 0,
                                      (EDMA3_RM_EventQueue)0,
                                      &callback1, NULL);
    if (result != EDMA3_DRV_SOK)
    {
        return result;
    }
    chids[1] = EDMA3_DRV_LINK_CHANNEL;
    result = EDMA3_DRV_requestChannel(hEdma, chids + 1, NULL,
                                      (EDMA3_RM_EventQueue)0,
                                      &callback1, NULL);
    if (result != EDMA3_DRV_SOK)
    {
        EDMA3_DRV_freeChannel(hEdma, chids[0]);
        return result;
    }
    chids[2] = EDMA3_DRV_LINK_CHANNEL;
    result = EDMA3_DRV_requestChannel(hEdma, chids + 2, NULL,
                                      (EDMA3_RM_EventQueue)0,
                                      &callback1, NULL);
    if (result != EDMA3_DRV_SOK)
    {
        EDMA3_DRV_freeChannel(hEdma, chids[1]);
        EDMA3_DRV_freeChannel(hEdma, chids[0]);
        return result;
    }
    return EDMA3_DRV_SOK;
}

/*****************************************************************************/
static EDMA3_DRV_Result
set_param_link(unsigned int src_addr, unsigned int width,
               unsigned int* chids, unsigned int tcc,
               EDMA3_DRV_ParamentryRegs **param_handle)
{
    EDMA3_DRV_Result result;
    EDMA3_DRV_PaRAMRegs paramSet;
    unsigned int phyaddress;

    paramSet.srcAddr    = src_addr;
    paramSet.destAddr   = L1DBUF1;
    paramSet.srcBIdx    = 0;
    paramSet.destBIdx   = 0;
    paramSet.srcCIdx    = 0;
    paramSet.destCIdx   = 0;
    paramSet.aCnt       = width;
    paramSet.bCnt       = 1;
    paramSet.cCnt       = 1;
    paramSet.bCntReload = 0;
    paramSet.linkAddr   = 0xFFFFu;
    paramSet.opt        = 0;
    paramSet.opt       &= 0xFFFFFFFCu;
    paramSet.opt       |= ((tcc << OPT_TCC_SHIFT) & OPT_TCC_MASK);
    paramSet.opt       |= (1 << OPT_ITCINTEN_SHIFT);
    paramSet.opt       |= (1 << OPT_TCINTEN_SHIFT);
    paramSet.opt       &= 0xFFFFFFFBu;
    result = EDMA3_DRV_setPaRAM(hEdma, chids[0], &paramSet);
    if (result != EDMA3_DRV_SOK)
    {
        return result;
    }
    result = EDMA3_DRV_setPaRAM(hEdma, chids[1], &paramSet);
    if (result != EDMA3_DRV_SOK)
    {
        return result;
    }
    paramSet.destAddr   = L1DBUF2;
    result = EDMA3_DRV_setPaRAM(hEdma, chids[2], &paramSet);
    if (result != EDMA3_DRV_SOK)
    {
        return result;
    }
    result = EDMA3_DRV_linkChannel(hEdma, chids[0], chids[2]);
    if (result != EDMA3_DRV_SOK)
    {
        return result;
    }
    result = EDMA3_DRV_linkChannel(hEdma, chids[2], chids[1]);
    if (result != EDMA3_DRV_SOK)
    {
        return result;
    }
    result = EDMA3_DRV_linkChannel(hEdma, chids[1], chids[2]);
    if (result != EDMA3_DRV_SOK)
    {
        return result;
    }
    phyaddress = 0;
    result = EDMA3_DRV_getPaRAMPhyAddr(hEdma, chids[0], &phyaddress);
    if (result != EDMA3_DRV_SOK)
    {
        return result;
    }
    *param_handle = (EDMA3_DRV_ParamentryRegs*)phyaddress;
    return EDMA3_DRV_SOK;
}

/*****************************************************************************/
static EDMA3_DRV_Result
start_edma(unsigned int chid)
{
    return EDMA3_DRV_enableTransfer(hEdma, chid, EDMA3_DRV_TRIG_MODE_MANUAL);
}

/*****************************************************************************/
static void
wait_edma(unsigned int chid)
{
    SEM_pend(&g_edmaSemObj, SYS_FOREVER);
    if (g_irqRaised1 < 0)
    {
        EDMA3_DRV_clearErrorBits(hEdma, chid);
    }
    g_irqRaised1 = 0;
}

/*****************************************************************************/
static void
gen_table(void)
{
    unsigned int* tab_ptr;
    unsigned int remainder;
    unsigned char b;
    unsigned int bit;

    b = 0;
    tab_ptr = (unsigned int*)0x10f10000;
    do
    {
        remainder = b;
        for (bit = 8; bit > 0; --bit)
        {
            if (remainder & 1)
            {
                remainder = (remainder >> 1) ^ 0xEDB88320;
            }
            else
            {
                remainder = (remainder >> 1);
            }
        }
        tab_ptr[b] = remainder;
    } while (0 != ++b);
}

/*****************************************************************************/
static unsigned int
crc_update(unsigned int crc, unsigned int addr, size_t bytes)
{
    unsigned char* data;
    unsigned char* end;
    unsigned int* tab_ptr;

    tab_ptr = (unsigned int*)0x10f10000;
    data = (unsigned char*)addr;
    end = data + bytes;
    while (data < end)
    {
        crc = tab_ptr[*(data++) ^ (crc & 0xff)] ^ (crc >> 8);
    }
    return crc;
}

/*****************************************************************************/
void
process_CRC32MSGSUBID(struct my_msg_t* my_msg)
{
    struct my_msg_crc32_t* my_msg_crc32;
    EDMA3_DRV_ParamentryRegs *param_handle;
    EDMA3_DRV_Result result;
    unsigned int chids[3];
    unsigned int tcc;
    unsigned int src;
    int width;
    int height;
    int stride_bytes;
    Uns crc;

    my_msg_crc32 = (struct my_msg_crc32_t*)my_msg;
    crc = ~0;
    if ((my_msg_crc32->width < 1) || (my_msg_crc32->height < 1) ||
        (my_msg_crc32->width > L1DBUFSIZE))
    {
        my_msg_crc32->crc32 = crc;
        return;
    }
    result = request_channels(chids, &tcc);
    if (result == EDMA3_DRV_SOK)
    {
        stride_bytes = my_msg_crc32->stride_bytes;
        width = my_msg_crc32->width;
        height = my_msg_crc32->height;
        src = my_msg_crc32->addr;
        param_handle = NULL;
        result = set_param_link(src, width, chids, tcc, &param_handle);
        if (result == EDMA3_DRV_SOK)
        {
            /* start ping */
            result = start_edma(chids[0]);
            if (result == EDMA3_DRV_SOK)
            {
                /* use this free time to generate crc table */
                gen_table();
                /* wait for ping */
                wait_edma(chids[0]);
            }
            src += stride_bytes;
            height--;
            if (height < 1)
            {
                /* process ping */
                crc = crc_update(crc, L1DBUF1, width);
            }
            else
            {
                while (height > 1)
                {
                    /* start pong */
                    param_handle->SRC = src;
                    result = start_edma(chids[0]);
                    if (result == EDMA3_DRV_SOK)
                    {
                        /* process ping */
                        crc = crc_update(crc, L1DBUF1, width);
                        /* wait for pong */
                        wait_edma(chids[0]);
                    }
                    src += stride_bytes;
                    height--;
                    /* start ping */
                    param_handle->SRC = src;
                    result = start_edma(chids[0]);
                    if (result == EDMA3_DRV_SOK)
                    {
                        /* process pong */
                        crc = crc_update(crc, L1DBUF2, width);
                        /* wait for ping */
                        wait_edma(chids[0]);
                    }
                    src += stride_bytes;
                    height--;
                }
                if (height > 0)
                {
                    /* start pong */
                    param_handle->SRC = src;
                    result = start_edma(chids[0]);
                    if (result == EDMA3_DRV_SOK)
                    {
                        /* process ping */
                        crc = crc_update(crc, L1DBUF1, width);
                        /* wait for pong */
                        wait_edma(chids[0]);
                    }
                    src += stride_bytes;
                    height--;
                    /* process pong */
                    crc = crc_update(crc, L1DBUF2, width);
                }
                else
                {
                    /* process ping */
                    crc = crc_update(crc, L1DBUF1, width);
                }
            }
        }
        EDMA3_DRV_freeChannel(hEdma, chids[2]);
        EDMA3_DRV_freeChannel(hEdma, chids[1]);
        EDMA3_DRV_freeChannel(hEdma, chids[0]);
    }
    my_msg_crc32->crc32 = ~crc;
}
