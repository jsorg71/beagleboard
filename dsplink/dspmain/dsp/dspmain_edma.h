
#ifndef _DSPMAIN_EDMA_H_
#define _DSPMAIN_EDMA_H_

// 0x10f08000 0x10F17FFF = L1D 64K
#define L1DADDR             0x10F10000

struct dma_in_info
{
    void (*setup)(struct dma_in_info* dii);
    void (*process)(struct dma_in_info* dii, unsigned int addr, int bytes);
    unsigned int src_addr;
    unsigned int width;
    unsigned int height;
    int stride_bytes;
    unsigned int bytes_per_pixel;
    unsigned int ping_addr;
    unsigned int pong_addr;
    unsigned int ping_pong_bytes;
    unsigned int user[4];
};

void
process_dma_in(struct dma_in_info* dii);
void
dma_init(void);

#endif
