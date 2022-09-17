
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
dspmain_crc32(void* obj, int addr, int format, int width, int height,
              int stride_bytes, int* cookie);
int
dspmain_crc32_result(void* obj, int cookie, int* crc32);
int
dspmain_flush(void* obj);

#endif
