
#include <std.h>
#include <msgq.h>

#include <ti/sdo/edma3/drv/edma3_drv.h>
#include <ti/sdo/edma3/drv/sample/bios_edma3_drv_sample.h>

#include "dspmain_msg.h"

extern EDMA3_DRV_Handle hEdma; /* in bios_edma3_drv_sample_init.c */

volatile short irqRaised1 = 0;

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
            irqRaised1 = 1;
            break;
        case EDMA3_RM_E_CC_DMA_EVT_MISS:
            /* Transfer resulted in DMA event miss error. */
            irqRaised1 = -1;
            break;
        case EDMA3_RM_E_CC_QDMA_EVT_MISS:
            /* Transfer resulted in QDMA event miss error. */
            irqRaised1 = -2;
            break;
        default:
            break;
    }
}

/*****************************************************************************/
void
process_CRC32MSGSUBID(struct my_msg_t* my_msg)
{
    struct my_msg_crc32_t* my_msg_crc32;
    int stride_bytes;
    int index;
    int height;
    int jndex;
    int width;
    unsigned char* data;
    unsigned char* data_org;

    //int i;
    unsigned int t = 0;
    //unsigned int r = ~0;
#if 0
    EDMA3_DRV_Result result;
    unsigned int chId = 0;
    unsigned int tcc = 0;

    tcc = EDMA3_DRV_TCC_ANY;
    chId = EDMA3_DRV_DMA_CHANNEL_ANY;
    //chId = EDMA3_DRV_QDMA_CHANNEL_ANY;
    result = EDMA3_DRV_requestChannel(hEdma, &chId, &tcc,
                                      (EDMA3_RM_EventQueue)0,
                                      &callback1, NULL);
    //*((int*)(0x94000080 - 4)) = (int)hEdma;
    *((int*)(0x94000080 - 4)) = result;
    //EDMA3_DRV_E_SEMAPHORE
    if (result == EDMA3_DRV_SOK)
    {
        EDMA3_DRV_freeChannel(hEdma, chId);
        *((int*)(0x94000080 - 4)) = 0xEE00EE00;
    }
#endif

    my_msg_crc32 = (struct my_msg_crc32_t*)my_msg;
    stride_bytes = my_msg_crc32->stride_bytes;
    data_org = (unsigned char*)(my_msg_crc32->addr);
    width = my_msg_crc32->width;
    height = my_msg_crc32->height;
    for (index = 0; index < height; index++)
    {
        data = data_org;
        data += index * stride_bytes;
        for (jndex = 0; jndex < width; jndex++)
        {
            t += *(data++);
#if 0
            r ^= *(data++);
            for (i = 0; i < 8; i++)
            {
                t = ~((r & 1) - 1);
                r = (r >> 1) ^ (0xEDB88320 & t);
            }
#endif
        }
    }
    //my_msg_crc32->crc32 = ~r;
    my_msg_crc32->crc32 = t;
}
