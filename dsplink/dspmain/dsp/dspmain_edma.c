
#include <std.h>
#include <sem.h>

#include <ti/sdo/edma3/drv/sample/bios_edma3_drv_sample.h>
#include <ti/sdo/edma3/drv/edma3_drv.h>

#include "dspmain_edma.h"

extern EDMA3_DRV_Handle hEdma; /* in bios_edma3_drv_sample_init.c */

static SEM_Handle g_edmaSemHandle = NULL;
static volatile short g_irqRaised1 = 0;

/* OPT Field specific defines */
#define OPT_SYNCDIM_SHIFT   0x00000002u
#define OPT_TCC_MASK        0x0003F000u
#define OPT_TCC_SHIFT       0x0000000Cu
#define OPT_ITCINTEN_SHIFT  0x00000015u
#define OPT_TCINTEN_SHIFT   0x00000014u

#define MINMAX(_val, _lo, _hi) \
    (_val) < (_lo) ? (_lo) : (_val) > (_hi) ? (_hi) : (_val)

#define MY_SET_PARAM_SRC(_src) \
    param_handle->SRC = _src

#define MY_SET_PARAM_AB(_acnt, _bcnt) \
    param_handle->A_B_CNT = ((_bcnt) << 16) | (_acnt)

#define DMA_START(_chid) \
    EDMA3_DRV_enableTransfer(hEdma, _chid, EDMA3_DRV_TRIG_MODE_MANUAL)

#define DMA_WAIT(_chid) \
    SEM_pend(g_edmaSemHandle, SYS_FOREVER); \
    if (g_irqRaised1 < 0) \
    { \
        EDMA3_DRV_clearErrorBits(hEdma, _chid); \
    } \
    g_irqRaised1 = 0

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
            SEM_post(g_edmaSemHandle);
            break;
        case EDMA3_RM_E_CC_DMA_EVT_MISS:
            /* Transfer resulted in DMA event miss error. */
            g_irqRaised1 = -1;
            SEM_post(g_edmaSemHandle);
            break;
        case EDMA3_RM_E_CC_QDMA_EVT_MISS:
            /* Transfer resulted in QDMA event miss error. */
            g_irqRaised1 = -2;
            SEM_post(g_edmaSemHandle);
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
set_param_link(unsigned int ping_addr, unsigned int pong_addr,
               int src_stride, int dst_stride,
               unsigned int* chids, unsigned int tcc,
               EDMA3_DRV_ParamentryRegs **param_handle)
{
    EDMA3_DRV_Result result;
    EDMA3_DRV_PaRAMRegs paramSet;
    unsigned int phyaddress;

    paramSet.srcAddr    = 0; /* set later */
    paramSet.destAddr   = ping_addr;
    paramSet.srcBIdx    = src_stride;
    paramSet.destBIdx   = dst_stride;
    paramSet.srcCIdx    = 0;
    paramSet.destCIdx   = 0;
    paramSet.aCnt       = 0; /* set later */
    paramSet.bCnt       = 0; /* set later */
    paramSet.cCnt       = 1;
    paramSet.bCntReload = 0;
    paramSet.linkAddr   = 0xFFFFu;
    paramSet.opt        = 0;
    paramSet.opt        |= ((tcc << OPT_TCC_SHIFT) & OPT_TCC_MASK);
    paramSet.opt        |= (1 << OPT_TCINTEN_SHIFT);
    paramSet.opt        |= (1 << OPT_SYNCDIM_SHIFT);
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
    paramSet.destAddr = pong_addr;
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
void
process_dma_in(struct dma_in_info* dii)
{
    EDMA3_DRV_ParamentryRegs* param_handle;
    EDMA3_DRV_Result result;
    unsigned int chids[3];
    unsigned int tcc;
    unsigned int src;
    int height;
    int stride_bytes;
    int rows_per_dma;
    int bytes_in_ping;
    int bytes_in_pong;
    int row_bytes;

    result = request_channels(chids, &tcc);
    if (result != EDMA3_DRV_SOK)
    {
        return;
    }
    stride_bytes = dii->stride_bytes;
    height = dii->height;
    src = dii->src_addr;
    row_bytes = dii->width * dii->bytes_per_pixel;
    rows_per_dma = dii->ping_pong_bytes / row_bytes;
    rows_per_dma = MINMAX(rows_per_dma, 1, height);
    param_handle = NULL;
    result = set_param_link(dii->ping_addr, dii->pong_addr,
                            stride_bytes, row_bytes, chids, tcc,
                            &param_handle);
    if (result == EDMA3_DRV_SOK)
    {
        bytes_in_ping = row_bytes * rows_per_dma;
        MY_SET_PARAM_SRC(src);
        MY_SET_PARAM_AB(row_bytes, rows_per_dma);
        /* start ping */
        DMA_START(chids[0]);
        /* use this free time to setup */
        dii->setup(dii);
        /* wait for ping */
        DMA_WAIT(chids[0]);
        src += stride_bytes * rows_per_dma;
        height -= rows_per_dma;
        if (height < 1)
        {
            /* process ping */
            dii->process(dii, dii->ping_addr, bytes_in_ping);
        }
        else
        {
            while (height >= rows_per_dma * 2)
            {
                bytes_in_pong = row_bytes * rows_per_dma;
                MY_SET_PARAM_SRC(src);
                MY_SET_PARAM_AB(row_bytes, rows_per_dma);
                /* start pong */
                DMA_START(chids[0]);
                /* process ping */
                dii->process(dii, dii->ping_addr, bytes_in_ping);
                /* wait for pong */
                DMA_WAIT(chids[0]);
                src += stride_bytes * rows_per_dma;
                height -= rows_per_dma;
                bytes_in_ping = row_bytes * rows_per_dma;
                MY_SET_PARAM_SRC(src);
                MY_SET_PARAM_AB(row_bytes, rows_per_dma);
                /* start ping */
                DMA_START(chids[0]);
                /* process pong */
                dii->process(dii, dii->pong_addr, bytes_in_pong);
                /* wait for ping */
                DMA_WAIT(chids[0]);
                src += stride_bytes * rows_per_dma;
                height -= rows_per_dma;
            }
            if (height > rows_per_dma)
            {
                rows_per_dma = height / 2;
                bytes_in_pong = row_bytes * rows_per_dma;
                MY_SET_PARAM_SRC(src);
                MY_SET_PARAM_AB(row_bytes, rows_per_dma);
                /* start pong */
                DMA_START(chids[0]);
                /* process ping */
                dii->process(dii, dii->ping_addr, bytes_in_ping);
                /* wait for pong */
                DMA_WAIT(chids[0]);
                src += stride_bytes * rows_per_dma;
                height -= rows_per_dma;
                rows_per_dma = height;
                bytes_in_ping = row_bytes * rows_per_dma;
                MY_SET_PARAM_SRC(src);
                MY_SET_PARAM_AB(row_bytes, rows_per_dma);
                /* start ping */
                DMA_START(chids[0]);
                /* process pong */
                dii->process(dii, dii->pong_addr, bytes_in_pong);
                /* wait for ping */
                DMA_WAIT(chids[0]);
                src += stride_bytes * rows_per_dma;
                height -= rows_per_dma;
            }
            if (height > 0)
            {
                rows_per_dma = height;
                bytes_in_pong = row_bytes * rows_per_dma;
                MY_SET_PARAM_SRC(src);
                MY_SET_PARAM_AB(row_bytes, rows_per_dma);
                /* start pong */
                DMA_START(chids[0]);
                /* process ping */
                dii->process(dii, dii->ping_addr, bytes_in_ping);
                /* wait for pong */
                DMA_WAIT(chids[0]);
                src += stride_bytes * rows_per_dma;
                height -= rows_per_dma;
                /* process pong */
                dii->process(dii, dii->pong_addr, bytes_in_pong);
            }
            else
            {
                /* process ping */
                dii->process(dii, dii->ping_addr, bytes_in_ping);
            }
        }
    }
    EDMA3_DRV_freeChannel(hEdma, chids[2]);
    EDMA3_DRV_freeChannel(hEdma, chids[1]);
    EDMA3_DRV_freeChannel(hEdma, chids[0]);
}

/*****************************************************************************/
void
process_dma_pro(struct dma_pro_info* dpi)
{
    EDMA3_DRV_ParamentryRegs* param_handle;
    EDMA3_DRV_ParamentryRegs* param_handle1;
    EDMA3_DRV_Result result;
    unsigned int chids[3];
    unsigned int chids1[3];
    unsigned int tcc;
    unsigned int tcc1;
    int num_tiles;

    result = request_channels(chids, &tcc);
    if (result != EDMA3_DRV_SOK)
    {
        return;
    }
    result = request_channels(chids1, &tcc1);
    if (result != EDMA3_DRV_SOK)
    {
        EDMA3_DRV_freeChannel(hEdma, chids[2]);
        EDMA3_DRV_freeChannel(hEdma, chids[1]);
        EDMA3_DRV_freeChannel(hEdma, chids[0]);
        return;
    }
    result = set_param_link(dpi->ping_addr, dpi->pong_addr,
                            0, 0, chids, tcc,
                            &param_handle);
    if (result == EDMA3_DRV_SOK)
    {
        result = set_param_link(dpi->ping_addr, dpi->pong_addr,
                                0, 0, chids1, tcc1,
                                &param_handle1);
    }
    if (result == EDMA3_DRV_SOK)
    {
        num_tiles = 10;

        /* start dma in ping rlgr */
        DMA_START(chids[0]);
        /* use this free time to setup */
        dpi->setup(dpi);
        /* wait dma in ping rlgr */
        DMA_WAIT(chids[0]);
        num_tiles--;
        if (num_tiles < 1)
        {
            /* process ping rlgr */
            dpi->ping_rlgr(dpi);
            /* start dma in ping diff */
            DMA_START(chids[0]);
            /* wait dma in ping diff */
            DMA_WAIT(chids[0]);
            /* process ping diff */
            dpi->ping_diff(dpi);
            /* start dma out ping diff */
            DMA_START(chids1[0]);
            /* wait dma out ping diff */
            DMA_WAIT(chids1[0]);
            /* process ping diff */
            dpi->ping_dwt(dpi);
            /* start dma out ping dwt */
            DMA_START(chids1[0]);
            /* wait dma out ping dwt */
            DMA_WAIT(chids1[0]);
        }
        else
        {
            /* start dma in pong rlgr */
            DMA_START(chids[0]);
            /* process ping rlgr */
            dpi->ping_rlgr(dpi);
            /* wait dma in pong rlgr */
            DMA_WAIT(chids[0]);
            num_tiles--;
            while (num_tiles > 1)
            {
                /* start dma in ping diff */
                DMA_START(chids[0]);
                /* process pong rlgr */
                dpi->pong_rlgr(dpi);
                /* wait dma in ping diff */
                DMA_WAIT(chids[0]);
                /* start dma in pong diff */
                DMA_START(chids[0]);
                /* process ping diff */
                dpi->ping_diff(dpi);
                /* wait dma in pong diff */
                DMA_WAIT(chids[0]);
                /* start dma out ping diff */
                DMA_START(chids1[0]);
                /* process pong diff */
                dpi->pong_diff(dpi);
                /* wait dma out ping diff */
                DMA_WAIT(chids1[0]);
                /* start dma out pong diff */
                DMA_START(chids1[0]);
                /* process ping dwt */
                dpi->ping_dwt(dpi);
                /* wait dma out pong diff */
                DMA_WAIT(chids1[0]);
                /* start dma out ping dwt, dma in ping rlgr */
                DMA_START(chids1[0]);
                /* process pong dwt */
                dpi->pong_dwt(dpi);
                /* wait dma out ping dwt, dma in ping rlgr */
                DMA_WAIT(chids1[0]);
                num_tiles--;
                /* start dma out pong dwt, dma in pong rlgr */
                DMA_START(chids1[0]);
                /* process ping rlgr */
                dpi->ping_rlgr(dpi);
                /* wait dma out pong dwt, dma in pong rlgr */
                DMA_WAIT(chids1[0]);
                num_tiles--;
            }
            if (num_tiles > 0)
            {
                /* start dma in ping diff */
                DMA_START(chids[0]);
                /* process pong rlgr */
                dpi->pong_rlgr(dpi);
                /* wait dma in ping diff */
                DMA_WAIT(chids[0]);
                /* start dma in pong diff */
                DMA_START(chids[0]);
                /* process ping diff */
                dpi->ping_diff(dpi);
                /* wait dma in pong diff */
                DMA_WAIT(chids[0]);
                /* start dma out ping diff */
                DMA_START(chids1[0]);
                /* process pong diff */
                dpi->pong_diff(dpi);
                /* wait dma out ping diff */
                DMA_WAIT(chids1[0]);
                /* start dma out pong diff */
                DMA_START(chids1[0]);
                /* process ping dwt */
                dpi->ping_dwt(dpi);
                /* wait dma out pong diff */
                DMA_WAIT(chids1[0]);
                /* start dma out ping dwt, dma in ping rlgr */
                DMA_START(chids1[0]);
                /* process pong dwt */
                dpi->pong_dwt(dpi);
                /* wait dma out ping dwt, dma in ping rlgr */
                DMA_WAIT(chids1[0]);
                num_tiles--;
                /* start dma out pong dwt */
                DMA_START(chids1[0]);
                /* process ping rlgr */
                dpi->ping_rlgr(dpi);
                /* wait dma out pong dwt */
                DMA_WAIT(chids1[0]);
                /* start dma in ping diff */
                DMA_START(chids[0]);
                /* wait dma in ping diff */
                DMA_WAIT(chids[0]);
                /* process ping diff */
                dpi->ping_diff(dpi);
                /* start dma out ping diff */
                DMA_START(chids1[0]);
                /* wait dma out ping diff */
                DMA_WAIT(chids1[0]);
                /* process ping dwt */
                dpi->ping_dwt(dpi);
                /* start dma out ping dwt */
                DMA_START(chids1[0]);
                /* wait dma out ping dwt */
                DMA_WAIT(chids1[0]);
            }
            else
            {
                /* start dma in ping diff */
                DMA_START(chids[0]);
                /* process pong rlgr */
                dpi->pong_rlgr(dpi);
                /* wait dma in ping diff */
                DMA_WAIT(chids[0]);
                /* start dma in pong diff */
                DMA_START(chids[0]);
                /* process ping diff */
                dpi->ping_diff(dpi);
                /* wait dma in pong diff */
                DMA_WAIT(chids[0]);
                /* start dma out ping diff */
                DMA_START(chids1[0]);
                /* process pong diff */
                dpi->pong_diff(dpi);
                /* wait dma out ping diff */
                DMA_WAIT(chids1[0]);
                /* start dma out pong diff */
                DMA_START(chids1[0]);
                /* process ping dwt */
                dpi->ping_dwt(dpi);
                /* wait dma out pong diff */
                DMA_WAIT(chids1[0]);
                /* start dma out ping dwt */
                DMA_START(chids1[0]);
                /* process pong dwt */
                dpi->pong_dwt(dpi);
                /* wait dma out ping dwt */
                DMA_WAIT(chids1[0]);
                /* start dma out pong dwt */
                DMA_START(chids1[0]);
                /* wait dma out pong dwt */
                DMA_WAIT(chids1[0]);
            }
        }
    }
    EDMA3_DRV_freeChannel(hEdma, chids[2]);
    EDMA3_DRV_freeChannel(hEdma, chids[1]);
    EDMA3_DRV_freeChannel(hEdma, chids[0]);
    EDMA3_DRV_freeChannel(hEdma, chids1[2]);
    EDMA3_DRV_freeChannel(hEdma, chids1[1]);
    EDMA3_DRV_freeChannel(hEdma, chids1[0]);
}

/*****************************************************************************/
void
dma_init(void)
{
    SEM_Attrs semAttrs = SEM_ATTRS;

    g_edmaSemHandle = SEM_create(0, &semAttrs);
    edma3init();
}

/*****************************************************************************/
void
dma_deinit(void)
{
    edma3deinit();
    SEM_delete(g_edmaSemHandle);
}
