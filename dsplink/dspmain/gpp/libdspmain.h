
#ifndef _LIBDSPMAIN_H_
#define _LIBDSPMAIN_H_

int
dspmain_init(void** obj);
int
dspmain_deinit(void* obj);
int
dspmain_mult(void* obj, int x, int y, int* cookie);
int
dspmain_mult_result(void* obj, int cookie, int* z);
int
dspmain_flush(void* obj);

#endif
