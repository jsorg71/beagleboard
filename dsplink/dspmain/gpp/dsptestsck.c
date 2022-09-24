
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include <ti/cmem.h>

#include "libdspmain.h"

#define minmax(_val, _lo, _hi) (_val) < (_lo) ? (_lo) : (_val) > (_hi) ? (_hi) : (_val)

/*****************************************************************************/
static int
get_mstime(void)
{
    struct timeval tp;

    gettimeofday(&tp, 0);
    return (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}

static unsigned int g_tab_ptr[256];

/*****************************************************************************/
static unsigned int
crc_update(unsigned int crc, unsigned char* data, size_t bytes)
{
    unsigned char* end;

    end = data + bytes;
    while (data < end)
    {
        crc = g_tab_ptr[*(data++) ^ (crc & 0xff)] ^ (crc >> 8);
    }
    return crc;
}

/*****************************************************************************/
static unsigned int
crc_update16(unsigned int crc, unsigned char* adata, size_t bytes)
{
    unsigned short* data;
    unsigned short* end;
    unsigned short pixel;

    data = (unsigned short*)adata;
    end = (unsigned short*)(adata + bytes);
    while (data < end)
    {
        pixel = *(data++);
        crc = g_tab_ptr[(pixel ^ crc) & 0xff] ^ (crc >> 8);
        crc = g_tab_ptr[((pixel >> 8) ^ crc) & 0xff] ^ (crc >> 8);
    }
    return crc;
}

/*****************************************************************************/
static unsigned int
crc_update24(unsigned int crc, unsigned char* adata, size_t bytes)
{
    unsigned int* data;
    unsigned int* end;
    unsigned int pixel;

    data = (unsigned int*)adata;
    end = (unsigned int*)(adata + bytes);
    while (data < end)
    {
        pixel = *(data++);
        crc = g_tab_ptr[(pixel ^ crc) & 0xff] ^ (crc >> 8);
        crc = g_tab_ptr[((pixel >> 8) ^ crc) & 0xff] ^ (crc >> 8);
        crc = g_tab_ptr[((pixel >> 16) ^ crc) & 0xff] ^ (crc >> 8);
    }
    return crc;
}

/*****************************************************************************/
static unsigned int
crc_update32(unsigned int crc, unsigned char* adata, size_t bytes)
{
    unsigned int* data;
    unsigned int* end;
    unsigned int pixel;

    data = (unsigned int*)adata;
    end = (unsigned int*)(adata + bytes);
    while (data < end)
    {
        pixel = *(data++);
        crc = g_tab_ptr[(pixel ^ crc) & 0xff] ^ (crc >> 8);
        crc = g_tab_ptr[((pixel >> 8) ^ crc) & 0xff] ^ (crc >> 8);
        crc = g_tab_ptr[((pixel >> 16) ^ crc) & 0xff] ^ (crc >> 8);
        crc = g_tab_ptr[((pixel >> 24) ^ crc) & 0xff] ^ (crc >> 8);
    }
    return crc;
}

/*****************************************************************************/
static void
gen_table(void)
{
    unsigned int remainder;
    unsigned char b;
    unsigned int bit;

    b = 0;
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
        g_tab_ptr[b] = remainder;
    } while (0 != ++b);
}

#define MAX_CRC_WIDTH 1024
#define MAX_CRC_HEIGHT 1024

int main(int argc, char** argv)
{
    int error;
    int cookie[3];
    int x;
    int y;
    int z;
    int crc32;
    int sleep_time;
    void* obj;
    CMEM_AllocParams ap;
    unsigned char* uint8ptr;
    unsigned int uint8addr;
    unsigned int crc;
    int width;
    int height;
    unsigned int format;
    unsigned int bytes_per_pixel;

    srand(time(0));
    CMEM_init();
    ap.type = CMEM_HEAP;
    ap.flags = CMEM_CACHED;
    ap.alignment = 32;
    uint8ptr = (unsigned char*)CMEM_alloc(MAX_CRC_WIDTH * MAX_CRC_HEIGHT * 4, &ap);
    uint8addr = CMEM_getPhys(uint8ptr);
    printf("main: uint8ptr %p uint8addr 0x%8.8x\n", uint8ptr, uint8addr);

    gen_table();

    error = dspmain_init(&obj);
    printf("main: dspmain_init %d\n", error);

    sleep_time = 500;
    if (argc > 1)
    {
        sleep_time = atoi(argv[1]);
    }
    srand(time(0));
    for (;;)
    {
        x = rand() % 100;
        y = rand() % 100;
        error = dspmain_mult(obj, x, y, &cookie[0]);
        printf("[%10.10u] main: dspmain_mult %d x %d y %d z(calc) %d\n", get_mstime(), error, x, y, x * y);

        x = rand() % 100;
        y = rand() % 100;
        error = dspmain_mult(obj, x, y, &cookie[1]);
        printf("[%10.10u] main: dspmain_mult %d x %d y %d z(calc) %d\n", get_mstime(), error, x, y, x * y);

        x = rand() % 100;
        y = rand() % 100;
        error = dspmain_mult(obj, x, y, &cookie[2]);
        printf("[%10.10u] main: dspmain_mult %d x %d y %d z(calc) %d\n", get_mstime(), error, x, y, x * y);

        error = dspmain_flush(obj);
        printf("[%10.10u] main: dspmain_flush %d\n", get_mstime(), error);

        error = dspmain_mult_result(obj, cookie[0], &z);
        printf("[%10.10u] main: dspmain_mult_result %d z %d\n", get_mstime(), error, z);

        error = dspmain_mult_result(obj, cookie[1], &z);
        printf("[%10.10u] main: dspmain_mult_result %d z %d\n", get_mstime(), error, z);

        error = dspmain_mult_result(obj, cookie[2], &z);
        printf("[%10.10u] main: dspmain_mult_result %d z %d\n", get_mstime(), error, z);

        format = rand() % 4 + 1;
        bytes_per_pixel = format == 3 ? 4 : format;
        width = rand() % MAX_CRC_WIDTH;
        width = minmax(width, 1, 1024);
        height = rand() % MAX_CRC_HEIGHT;
        height = minmax(height, 1, 1024);
        printf("[%10.10u] main: crc image width %d height %d format %d\n", get_mstime(), width, height, format);
        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width * bytes_per_pixel; x++)
            {
                uint8ptr[y * 1024 + x] = rand();
            }
        }
        x = get_mstime();
        crc = ~0;
        if (format == 1)
        {
            for (z = 0; z < height; z++)
            {
                crc = crc_update(crc, uint8ptr + z * MAX_CRC_WIDTH, width);
            }
        }
        else if (format == 2)
        {
            for (z = 0; z < height; z++)
            {
                crc = crc_update16(crc, uint8ptr + z * MAX_CRC_WIDTH * 2, width * 2);
            }
        }
        else if (format == 3)
        {
            for (z = 0; z < height; z++)
            {
                crc = crc_update24(crc, uint8ptr + z * MAX_CRC_WIDTH * 4, width * 4);
            }
        }
        else if (format == 4)
        {
            for (z = 0; z < height; z++)
            {
                crc = crc_update32(crc, uint8ptr + z * MAX_CRC_WIDTH * 4, width * 4);
            }
        }
        crc = ~crc;
        y = get_mstime();
        printf("[%10.10u] main: cpu crc calc time ms %d\n", get_mstime(), y - x);
        CMEM_cacheWb(uint8ptr, MAX_CRC_WIDTH * height * bytes_per_pixel);
        x = get_mstime();
        error = dspmain_crc32(obj, uint8addr, format, width, height, MAX_CRC_WIDTH * bytes_per_pixel, &cookie[0]);
        printf("[%10.10u] main: dspmain_crc32 %d crc 0x%8.8x\n", get_mstime(), error, crc);

        error = dspmain_crc32_result(obj, cookie[0], &crc32);
        y = get_mstime();
        printf("[%10.10u] main: dspmain_mult_result %d crc32 0x%8.8x match %d dsp crc calc time %d\n", get_mstime(), error, crc32, crc32 == crc, y - x);

        if (sleep_time > 0)
        {
            //usleep(500 * 1000);
            usleep(sleep_time * 1000);
        }
    }

    error = dspmain_deinit(obj);
    printf("main: dspmain_deinit %d\n", error);

    return 0;
}
