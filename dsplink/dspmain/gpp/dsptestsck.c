
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>

#include "libdspmain.h"

/*****************************************************************************/
static int
get_mstime(void)
{
    struct timeval tp;

    gettimeofday(&tp, 0);
    return (tp.tv_sec * 1000) + (tp.tv_usec / 1000);
}

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

        error = dspmain_crc32(obj, 0x10f10000, 0, 128, 128, 128, &cookie[0]);
        //error = dspmain_crc32(obj, 0x94200000, 0, 128, 128, 128, &cookie[0]);
        printf("[%10.10u] main: dspmain_crc32 %d\n", get_mstime(), error);

        error = dspmain_crc32_result(obj, cookie[0], &crc32);
        printf("[%10.10u] main: dspmain_mult_result %d crc32 0x%8.8x\n", get_mstime(), error, crc32);

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
