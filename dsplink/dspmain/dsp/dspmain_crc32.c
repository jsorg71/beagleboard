
#include <std.h>
#include <msgq.h>

#include "dspmain_msg.h"
#include "dspmain_edma.h"

#define L1DBUFBYTES         (15 * 1024 + 512)
#define L1DBUF1             (L1DADDR + 1 * 1024)
#define L1DBUF2             (L1DADDR + 16 * 1024 + 512)

/*****************************************************************************/
static void
gen_table(struct dma_in_info* dii)
{
    unsigned int* tab_ptr;
    unsigned int remainder;
    unsigned char b;
    unsigned int bit;

    (void)dii;

    b = 0;
    tab_ptr = (unsigned int*)L1DADDR;
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
static void
crc_update8(struct dma_in_info* dii, unsigned int addr, int bytes)
{
    unsigned char* data;
    unsigned char* end;
    unsigned int* tab_ptr;
    unsigned int crc;
    unsigned char pixel;

    crc = dii->user[0];
    tab_ptr = (unsigned int*)L1DADDR;
    data = (unsigned char*)addr;
    end = (unsigned char*)(addr + bytes);
    while (data < end)
    {
        pixel = *(data++);
        crc = tab_ptr[(pixel ^ crc) & 0xff] ^ (crc >> 8);
    }
    dii->user[0] = crc;
}

/*****************************************************************************/
static void
crc_update16(struct dma_in_info* dii, unsigned int addr, int bytes)
{
    unsigned short* data;
    unsigned short* end;
    unsigned int* tab_ptr;
    unsigned int crc;
    unsigned short pixel;

    crc = dii->user[0];
    tab_ptr = (unsigned int*)L1DADDR;
    data = (unsigned short*)addr;
    end = (unsigned short*)(addr + bytes);
    while (data < end)
    {
        pixel = *(data++);
        crc = tab_ptr[(pixel ^ crc) & 0xff] ^ (crc >> 8);
        crc = tab_ptr[((pixel >> 8) ^ crc) & 0xff] ^ (crc >> 8);
    }
    dii->user[0] = crc;
}

/*****************************************************************************/
static void
crc_update24(struct dma_in_info* dii, unsigned int addr, int bytes)
{
    unsigned int* data;
    unsigned int* end;
    unsigned int* tab_ptr;
    unsigned int crc;
    unsigned int pixel;

    crc = dii->user[0];
    tab_ptr = (unsigned int*)L1DADDR;
    data = (unsigned int*)addr;
    end = (unsigned int*)(addr + bytes);
    while (data < end)
    {
        pixel = *(data++);
        crc = tab_ptr[(pixel ^ crc) & 0xff] ^ (crc >> 8);
        crc = tab_ptr[((pixel >> 8) ^ crc) & 0xff] ^ (crc >> 8);
        crc = tab_ptr[((pixel >> 16) ^ crc) & 0xff] ^ (crc >> 8);
    }
    dii->user[0] = crc;
}

/*****************************************************************************/
static void
crc_update32(struct dma_in_info* dii, unsigned int addr, int bytes)
{
    unsigned int* data;
    unsigned int* end;
    unsigned int* tab_ptr;
    unsigned int crc;
    unsigned int pixel;

    crc = dii->user[0];
    tab_ptr = (unsigned int*)L1DADDR;
    data = (unsigned int*)addr;
    end = (unsigned int*)(addr + bytes);
    while (data < end)
    {
        pixel = *(data++);
        crc = tab_ptr[(pixel ^ crc) & 0xff] ^ (crc >> 8);
        crc = tab_ptr[((pixel >> 8) ^ crc) & 0xff] ^ (crc >> 8);
        crc = tab_ptr[((pixel >> 16) ^ crc) & 0xff] ^ (crc >> 8);
        crc = tab_ptr[((pixel >> 24) ^ crc) & 0xff] ^ (crc >> 8);
    }
    dii->user[0] = crc;
}

/*****************************************************************************/
void
process_CRC32MSGSUBID(struct my_msg_t* my_msg)
{
    struct my_msg_crc32_t* my_msg_crc32;
    struct dma_in_info dii;

    my_msg_crc32 = (struct my_msg_crc32_t*)my_msg;
    dii.setup = gen_table;
    switch (my_msg_crc32->format)
    {
        case 4:
            dii.process = crc_update32;
            dii.bytes_per_pixel = 4;
            break;
        case 3: /* depth 24 bpp 32 */
            dii.process = crc_update24;
            dii.bytes_per_pixel = 4;
            break;
        case 2:
            dii.process = crc_update16;
            dii.bytes_per_pixel = 2;
            break;
        case 1:
            /* fallthrough */
        default:
            dii.process = crc_update8;
            dii.bytes_per_pixel = 1;
            break;
    }
    dii.src_addr = my_msg_crc32->addr;
    dii.width = my_msg_crc32->width;
    dii.height = my_msg_crc32->height;
    dii.stride_bytes = my_msg_crc32->stride_bytes;
    dii.ping_addr = L1DBUF1;
    dii.pong_addr = L1DBUF2;
    dii.ping_pong_bytes = L1DBUFBYTES;
    dii.user[0] = ~0; /* crc32 */
    if ((dii.width < 1) || (dii.height < 1) ||
        (dii.width * dii.bytes_per_pixel > dii.ping_pong_bytes))
    {
        return;
    }
    process_dma_in(&dii);
    my_msg_crc32->crc32 = ~dii.user[0];
}
