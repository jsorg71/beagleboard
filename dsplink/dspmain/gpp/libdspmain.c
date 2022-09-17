
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/un.h>
#include <sys/socket.h>

#include "arch.h"
#include "parse.h"

#include "libdspmain.h"

#define xnew(type_, count_) (type_ *) malloc(count_ * sizeof(type_))
#define xnew0(type_, count_) (type_ *) calloc(count_, sizeof(type_))

struct libdspmain_t
{
    int sck;
    int sequence;
    struct stream in_s;
    struct stream out_s;
};

/*****************************************************************************/
int
dspmain_init(void** obj)
{
    struct libdspmain_t* ldsp;
    struct sockaddr_un s;

    ldsp = xnew0(struct libdspmain_t, 1);
    if (ldsp == NULL)
    {
        return 1;
    }
    ldsp->in_s.size = 1024;
    ldsp->in_s.p = ldsp->in_s.data = xnew(char, ldsp->in_s.size);
    if (ldsp->in_s.data == NULL)
    {
        free(ldsp);
        return 1;
    }
    ldsp->out_s.size = 1024;
    ldsp->out_s.p = ldsp->out_s.data = xnew(char, ldsp->out_s.size);
    if (ldsp->out_s.data == NULL)
    {
        free(ldsp->in_s.data);
        free(ldsp);
        return 1;
    }
    ldsp->sck = socket(PF_LOCAL, SOCK_STREAM, 0);
    if (ldsp->sck == -1)
    {
        free(ldsp->out_s.data);
        free(ldsp->in_s.data);
        free(ldsp);
        return 1;
    }
    memset(&s, 0, sizeof(struct sockaddr_un));
    s.sun_family = AF_UNIX;
    strncpy(s.sun_path, "/tmp/dspmain.sck", sizeof(s.sun_path));
    s.sun_path[sizeof(s.sun_path) - 1] = 0;
    if (connect(ldsp->sck, (struct sockaddr*)&s, sizeof(struct sockaddr_un)) != 0)
    {
        close(ldsp->sck);
        free(ldsp->out_s.data);
        free(ldsp->in_s.data);
        free(ldsp);
        return 1;
    }
    *obj = ldsp;
    return 0;
}

/*****************************************************************************/
int
dspmain_deinit(void* obj)
{
    struct libdspmain_t* ldsp;

    ldsp = (struct libdspmain_t*)obj;
    close(ldsp->sck);
    free(ldsp);
    return 0;
}

/*****************************************************************************/
int
dspmain_mult(void* obj, int x, int y, int* cookie)
{
    int error;
    struct libdspmain_t* ldsp;
    struct stream* out_s;

    ldsp = (struct libdspmain_t*)obj;
    out_s = &(ldsp->out_s);
    if (!s_check_rem_out(out_s, 24))
    {
        error = dspmain_flush(obj);
        if (error != 0)
        {
            return error;
        }
    }
    out_uint32_le(out_s, 1); /* MULTMSGSUBID */
    out_uint32_le(out_s, 24);
    out_uint32_le(out_s, ldsp->sequence);
    out_uint32_le(out_s, x);
    out_uint32_le(out_s, y);
    out_uint32_le(out_s, 0);
    *cookie = ldsp->sequence++;
    return 0;
}

/*****************************************************************************/
int
dspmain_mult_result(void* obj, int cookie, int* z)
{
    struct libdspmain_t* ldsp;
    struct stream* in_s;
    int error;
    int subid;
    int size;
    int sequence;

    error = dspmain_flush(obj);
    if (error != 0)
    {
        return error;
    }
    ldsp = (struct libdspmain_t*)obj;
    in_s = &(ldsp->in_s);
    in_s->p = in_s->data;
    error = recv(ldsp->sck, in_s->p, 8, 0);
    if (error != 8)
    {
        return 1;
    }
    in_uint32_le(in_s, subid);
    if (subid != 1) /* MULTMSGSUBID */
    {
        return 1;
    }
    in_uint32_le(in_s, size);
    if (size != 24)
    {
        return 1;
    }
    error = recv(ldsp->sck, in_s->p, size - 8, 0);
    if (error != size - 8)
    {
        return 1;
    }
    in_uint32_le(in_s, sequence);
    if (sequence > cookie)
    {
        return 1;
    }
    if (sequence < cookie)
    {
        return dspmain_mult_result(obj, cookie, z);
    }
    in_uint8s(in_s, 8);
    in_uint32_le(in_s, *z);
    return 0;
}

/*****************************************************************************/
int
dspmain_crc32(void* obj, int addr, int format, int width, int height,
              int stride_bytes, int* cookie)
{
    int error;
    struct libdspmain_t* ldsp;
    struct stream* out_s;

    ldsp = (struct libdspmain_t*)obj;
    out_s = &(ldsp->out_s);
    if (!s_check_rem_out(out_s, 36))
    {
        error = dspmain_flush(obj);
        if (error != 0)
        {
            return error;
        }
    }
    out_uint32_le(out_s, 2); /* CRC32MSGSUBID */
    out_uint32_le(out_s, 36);
    out_uint32_le(out_s, ldsp->sequence);
    out_uint32_le(out_s, addr);
    out_uint32_le(out_s, format);
    out_uint32_le(out_s, width);
    out_uint32_le(out_s, height);
    out_uint32_le(out_s, stride_bytes);
    out_uint32_le(out_s, 0);
    *cookie = ldsp->sequence++;
    return 0;
}

/*****************************************************************************/
int
dspmain_crc32_result(void* obj, int cookie, int* crc32)
{
    struct libdspmain_t* ldsp;
    struct stream* in_s;
    int error;
    int subid;
    int size;
    int sequence;

    error = dspmain_flush(obj);
    if (error != 0)
    {
        return error;
    }
    ldsp = (struct libdspmain_t*)obj;
    in_s = &(ldsp->in_s);
    in_s->p = in_s->data;
    error = recv(ldsp->sck, in_s->p, 8, 0);
    if (error != 8)
    {
        return 1;
    }
    in_uint32_le(in_s, subid);
    if (subid != 2) /* CRC32MSGSUBID */
    {
        return 1;
    }
    in_uint32_le(in_s, size);
    if (size != 36)
    {
        return 1;
    }
    error = recv(ldsp->sck, in_s->p, size - 8, 0);
    if (error != size - 8)
    {
        return 1;
    }
    in_uint32_le(in_s, sequence);
    if (sequence > cookie)
    {
        return 1;
    }
    if (sequence < cookie)
    {
        return dspmain_mult_result(obj, cookie, crc32);
    }
    in_uint8s(in_s, 20);
    in_uint32_le(in_s, *crc32);
    return 0;
}

/*****************************************************************************/
int
dspmain_flush(void* obj)
{
    struct libdspmain_t* ldsp;
    int bytes;
    int error;
    struct stream* out_s;

    ldsp = (struct libdspmain_t*)obj;
    out_s = &(ldsp->out_s);
    out_s->end = out_s->p;
    out_s->p = out_s->data;
    bytes = (int)(out_s->end - out_s->data);
    if (bytes == 0)
    {
        return 0;
    }
    error = send(ldsp->sck, out_s->data, bytes, 0);
    if (error != bytes)
    {
        return 1;
    }
    return 0;
}
