
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "libdspmain.h"

int main(int argc, char** argv)
{
    int error;
    int cookie[3];
    int x;
    int y;
    int z;
    void* obj;
    
    error = dspmain_init(&obj);
    printf("main: dspmain_init %d\n", error);

    srand(time(0));
    for (;;)
    {
        x = rand() % 100;
        y = rand() % 100;
        error = dspmain_mult(obj, x, y, &cookie[0]);
        printf("main: dspmain_mult %d x %d y %d z(calc) %d\n", error, x, y, x * y);

        x = rand() % 100;
        y = rand() % 100;
        error = dspmain_mult(obj, x, y, &cookie[1]);
        printf("main: dspmain_mult %d x %d y %d z(calc) %d\n", error, x, y, x * y);

        x = rand() % 100;
        y = rand() % 100;
        error = dspmain_mult(obj, x, y, &cookie[2]);
        printf("main: dspmain_mult %d x %d y %d z(calc) %d\n", error, x, y, x * y);

        error = dspmain_flush(obj);
        printf("main: dspmain_flush %d\n", error);

        error = dspmain_mult_result(obj, cookie[0], &z);
        printf("main: dspmain_mult_result %d z %d\n", error, z);

        error = dspmain_mult_result(obj, cookie[1], &z);
        printf("main: dspmain_mult_result %d z %d\n", error, z);

        error = dspmain_mult_result(obj, cookie[2], &z);
        printf("main: dspmain_mult_result %d z %d\n", error, z);

        usleep(500 * 1000);
    }

    error = dspmain_deinit(obj);
    printf("main: dspmain_deinit %d\n", error);

    return 0;
}
