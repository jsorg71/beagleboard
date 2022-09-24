
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/socket.h>

#include <dsplink.h>
#include <proc.h>
#include <msgq.h>
#include <pool.h>

#include "arch.h"
#include "parse.h"

#include "dspmain.h"
#include "dspmain_msg.h"
#include "dspmain_peer.h"
#include "dspmain_error.h"

#define SCK_HEADER_BYTES 8
#define SCK_MAX_MSG_BYTES 512

/*****************************************************************************/
static int
dspmain_peer_delete_one(struct dspmain_peer_t* peer)
{
    struct stream* out_s;
    struct stream* lout_s;

    close(peer->sck);
    out_s = peer->out_s_head;
    while (out_s != NULL)
    {
        lout_s = out_s;
        out_s = out_s->next;
        free(lout_s->data);
        free(lout_s);
    }
    if (peer->in_s != NULL)
    {
        free(peer->in_s->data);
        free(peer->in_s);
    }
    free(peer);
    return DSP_ERROR_NONE;
}

/*****************************************************************************/
static int
dspmain_peer_remove_one(struct dspmain_t* dspmain,
                        struct dspmain_peer_t** apeer,
                        struct dspmain_peer_t* last_peer)
{
    struct dspmain_peer_t* peer;
    struct dspmain_peer_t* next_peer;

    peer = *apeer;
    LOGLN((LOG_INFO, LOGS "sck %d", LOGP, peer->sck));
    if (dspmain->peer_head == peer)
    {
        if (dspmain->peer_tail == peer)
        {
            /* remove only item */
            dspmain->peer_head = NULL;
            dspmain->peer_tail = NULL;
            dspmain_peer_delete_one(peer);
            peer = NULL;
        }
        else
        {
            /* remove first item */
            dspmain->peer_head = peer->next;
            next_peer = peer->next;
            dspmain_peer_delete_one(peer);
            peer = next_peer;
        }
    }
    else if (dspmain->peer_tail == peer)
    {
        /* remove last item */
        dspmain->peer_tail = last_peer;
        last_peer->next = NULL;
        dspmain_peer_delete_one(peer);
        peer = NULL;
    }
    else
    {
        /* remove middle item */
        last_peer->next = peer->next;
        next_peer = peer->next;
        dspmain_peer_delete_one(peer);
        peer = next_peer;
    }
    *apeer = peer;
    return DSP_ERROR_NONE;
}

/*****************************************************************************/
int
dspmain_queue_mult(struct dspmain_t* dspmain, struct dspmain_peer_t* peer,
                   struct my_msg_mult_t* my_msg_mult)
{
    struct stream* out_s;
    int rv;

    (void)dspmain;
    out_s = xnew0(struct stream, 1);
    if (out_s == NULL)
    {
        return DSP_ERROR_MEMORY;
    }
    out_s->data = xnew(char, 16);
    if (out_s->data == NULL)
    {
        free(out_s);
        return DSP_ERROR_MEMORY;
    }
    out_s->p = out_s->data;
    out_uint32_le(out_s, MULTMSGSUBID);
    out_uint32_le(out_s, 16);
    out_uint32_le(out_s, my_msg_mult->user[1]); /* sequence */
    out_uint32_le(out_s, my_msg_mult->z);
    out_s->end = out_s->p;
    out_s->p = out_s->data;
    rv = dspmain_peer_queue(peer, out_s);
    return rv;
}

/*****************************************************************************/
int
dspmain_queue_crc32(struct dspmain_t* dspmain, struct dspmain_peer_t* peer,
                    struct my_msg_crc32_t* my_msg_crc32)
{
    struct stream* out_s;
    int rv;

    (void)dspmain;
    out_s = xnew0(struct stream, 1);
    if (out_s == NULL)
    {
        return DSP_ERROR_MEMORY;
    }
    out_s->data = xnew(char, 16);
    if (out_s->data == NULL)
    {
        free(out_s);
        return DSP_ERROR_MEMORY;
    }
    out_s->p = out_s->data;
    out_uint32_le(out_s, CRC32MSGSUBID);
    out_uint32_le(out_s, 16);
    out_uint32_le(out_s, my_msg_crc32->user[1]); /* sequence */
    out_uint32_le(out_s, my_msg_crc32->crc32);
    out_s->end = out_s->p;
    out_s->p = out_s->data;
    rv = dspmain_peer_queue(peer, out_s);
    return rv;
}

/*****************************************************************************/
static int
dspmain_peer_process_msg_mult(struct dspmain_t* dspmain,
                              struct dspmain_peer_t* peer,
                              struct stream* in_s)
{
    int x;
    int y;
    int user[2];
    DSP_STATUS status;

    if (!s_check_rem(in_s, 12))
    {
        return DSP_ERROR_RANGE;
    }
    user[0] = peer->id;
    in_uint32_le(in_s, user[1]); /* sequence */
    in_uint32_le(in_s, x);
    in_uint32_le(in_s, y);
    status = send_mult_msg(dspmain, user, 2, x, y);
    if (DSP_FAILED(status))
    {
        return DSP_ERROR_PARAM;
    }
    return DSP_ERROR_NONE;
}

/*****************************************************************************/
static int
dspmain_peer_process_msg_crc32(struct dspmain_t* dspmain,
                               struct dspmain_peer_t* peer,
                               struct stream* in_s)
{
    int addr;
    int format;
    int width;
    int height;
    int stride_bytes;
    int user[2];
    DSP_STATUS status;

    if (!s_check_rem(in_s, 24))
    {
        return DSP_ERROR_RANGE;
    }
    user[0] = peer->id;
    in_uint32_le(in_s, user[1]); /* sequence */
    in_uint32_le(in_s, addr);
    in_uint32_le(in_s, format);
    in_uint32_le(in_s, width);
    in_uint32_le(in_s, height);
    in_uint32_le(in_s, stride_bytes);
    status = send_crc32_msg(dspmain, user, 2, addr, format, width, height,
                            stride_bytes);
    if (DSP_FAILED(status))
    {
        return DSP_ERROR_PARAM;
    }
    return DSP_ERROR_NONE;
}

/*****************************************************************************/
static int
dspmain_peer_process_msg(struct dspmain_t* dspmain, struct dspmain_peer_t* peer)
{
    int pdu_code;
    int pdu_bytes;
    int rv;
    struct stream* in_s;

    rv = DSP_ERROR_NONE;
    in_s = peer->in_s;
    in_uint32_le(in_s, pdu_code);
    in_uint32_le(in_s, pdu_bytes);
    if ((pdu_bytes < SCK_HEADER_BYTES) ||
        !s_check_rem(in_s, pdu_bytes - SCK_HEADER_BYTES))
    {
        return DSP_ERROR_RANGE;
    }
    switch (pdu_code)
    {
        case MULTMSGSUBID:
            rv = dspmain_peer_process_msg_mult(dspmain, peer, in_s);
            break;
        case CRC32MSGSUBID:
            rv = dspmain_peer_process_msg_crc32(dspmain, peer, in_s);
            break;
    }
    return rv;
}

/*****************************************************************************/
int
dspmain_peer_get_fds(struct dspmain_t* dspmain, int* max_fd,
                     fd_set* rfds, fd_set* wfds)
{
    struct dspmain_peer_t* peer;
    int lmax_fd;

    lmax_fd = *max_fd;
    peer = dspmain->peer_head;
    while (peer != NULL)
    {
        if (peer->sck > lmax_fd)
        {
            lmax_fd = peer->sck;
        }
        FD_SET(peer->sck, rfds);
        if (peer->out_s_head != NULL)
        {
            FD_SET(peer->sck, wfds);
        }
        peer = peer->next;
    }
    *max_fd = lmax_fd;
    return DSP_ERROR_NONE;
}

/*****************************************************************************/
int
dspmain_peer_check_fds(struct dspmain_t* dspmain, fd_set* rfds, fd_set* wfds)
{
    struct dspmain_peer_t* peer;
    struct dspmain_peer_t* last_peer;
    struct stream* out_s;
    struct stream* in_s;
    int out_bytes;
    int in_bytes;
    int sent;
    int reed;
    int pdu_bytes;
    int rv;
    int error;

    rv = DSP_ERROR_NONE;
    last_peer = NULL;
    peer = dspmain->peer_head;
    while (peer != NULL)
    {
        if (FD_ISSET(peer->sck, rfds))
        {
            in_s = peer->in_s;
            if (in_s == NULL)
            {
                in_s = xnew0(struct stream, 1);
                if (in_s == NULL)
                {
                    return DSP_ERROR_MEMORY;
                }
                in_s->size = SCK_MAX_MSG_BYTES;
                in_s->data = xnew(char, in_s->size);
                if (in_s->data == NULL)
                {
                    free(in_s);
                    return DSP_ERROR_MEMORY;
                }
                in_s->p = in_s->data;
                in_s->end = in_s->data;
                peer->in_s = in_s;
            }
            if (in_s->p == in_s->data)
            {
                in_s->end = in_s->data + SCK_HEADER_BYTES;
            }
            in_bytes = (int)(in_s->end - in_s->p);
            reed = recv(peer->sck, in_s->p, in_bytes, 0);
            if (reed < 1)
            {
                /* error */
                error = dspmain_peer_remove_one(dspmain, &peer, last_peer);
                if (error != DSP_ERROR_NONE)
                {
                    return error;
                }
                rv = DSP_ERROR_PEER_REMOVED;
                continue;
            }
            else
            {
                in_s->p += reed;
                if (in_s->p >= in_s->end)
                {
                    if (in_s->p == in_s->data + SCK_HEADER_BYTES)
                    {
                        /* finished reading in header */
                        in_s->p = in_s->data;
                        in_uint8s(in_s, 4); /* pdu_code */
                        in_uint32_le(in_s, pdu_bytes);
                        if ((pdu_bytes < SCK_HEADER_BYTES) || (pdu_bytes > in_s->size))
                        {
                            error = dspmain_peer_remove_one(dspmain, &peer, last_peer);
                            if (error != DSP_ERROR_NONE)
                            {
                                return error;
                            }
                            rv = DSP_ERROR_PEER_REMOVED;
                            continue;
                        }
                        in_s->end = in_s->data + pdu_bytes;
                    }
                    if (in_s->p >= in_s->end)
                    {
                        /* finished reading in header and payload */
                        in_s->p = in_s->data;
                        rv = dspmain_peer_process_msg(dspmain, peer);
                        if (rv != 0)
                        {
                            error = dspmain_peer_remove_one(dspmain, &peer, last_peer);
                            if (error != DSP_ERROR_NONE)
                            {
                                return error;
                            }
                            rv = DSP_ERROR_PEER_REMOVED;
                            continue;
                        }
                        in_s->p = in_s->data;
                    }
                }
            }
        }
        if (FD_ISSET(peer->sck, wfds))
        {
            out_s = peer->out_s_head;
            if (out_s != NULL)
            {
                out_bytes = (int)(out_s->end - out_s->p);
                sent = send(peer->sck, out_s->p, out_bytes, 0);
                if (sent < 1)
                {
                    /* error */
                    error = dspmain_peer_remove_one(dspmain, &peer, last_peer);
                    if (error != DSP_ERROR_NONE)
                    {
                        return error;
                    }
                    rv = DSP_ERROR_PEER_REMOVED;
                    continue;
                }
                out_s->p += sent;
                if (out_s->p >= out_s->end)
                {
                    if (out_s->next == NULL)
                    {
                        peer->out_s_head = NULL;
                        peer->out_s_tail = NULL;
                    }
                    else
                    {
                        peer->out_s_head = out_s->next;
                    }
                    free(out_s->data);
                    free(out_s);
                }
            }
        }
        last_peer = peer;
        peer = peer->next;
    }
    return rv;
}

/*****************************************************************************/
int
dspmain_peer_add_fd(struct dspmain_t* dspmain, int sck)
{
    struct dspmain_peer_t* peer;

    peer = xnew0(struct dspmain_peer_t, 1);
    if (peer == NULL)
    {
        return DSP_ERROR_MEMORY;
    }
    dspmain->unique_id++;
    peer->id = dspmain->unique_id;
    LOGLN((LOG_INFO, LOGS "sck %d peer id %d", LOGP, sck, peer->id));
    peer->sck = sck;
    if (dspmain->peer_head == NULL)
    {
        dspmain->peer_head = peer;
        dspmain->peer_tail = peer;
    }
    else
    {
        dspmain->peer_tail->next = peer;
        dspmain->peer_tail = peer;
    }
    return DSP_ERROR_NONE;
}

/*****************************************************************************/
int
dspmain_peer_queue(struct dspmain_peer_t* peer, struct stream* out_s)
{
    if (peer->out_s_tail == NULL)
    {
        peer->out_s_head = out_s;
        peer->out_s_tail = out_s;
    }
    else
    {
        peer->out_s_tail->next = out_s;
        peer->out_s_tail = out_s;
    }
    return DSP_ERROR_NONE;
}

/*****************************************************************************/
int
dspmain_peer_queue_copy(struct dspmain_peer_t* peer, struct stream* out_s)
{
    struct stream* lout_s;
    int bytes;

    lout_s = xnew0(struct stream, 1);
    if (lout_s == NULL)
    {
        return DSP_ERROR_MEMORY;
    }
    bytes = (int)(out_s->end - out_s->data);
    if ((bytes < SCK_HEADER_BYTES) || (bytes > SCK_MAX_MSG_BYTES))
    {
        free(lout_s);
        return DSP_ERROR_PARAM;
    }
    lout_s->size = bytes;
    lout_s->data = xnew(char, lout_s->size);
    if (lout_s->data == NULL)
    {
        free(lout_s);
        return DSP_ERROR_MEMORY;
    }
    lout_s->p = lout_s->data;
    out_uint8p(lout_s, out_s->data, bytes);
    lout_s->end = lout_s->p;
    lout_s->p = lout_s->data;
    return dspmain_peer_queue(peer, lout_s);
}
