
#ifndef _DSPMAIN_H_
#define _DSPMAIN_H_

#define xnew(type_, count_) (type_ *) malloc(count_ * sizeof(type_))
#define xnew0(type_, count_) (type_ *) calloc(count_, sizeof(type_))

struct dspmain_peer_t
{
    int sck;
    int id;
    struct stream* in_s;
    struct stream* out_s_head;
    struct stream* out_s_tail;
    struct dspmain_peer_t* next;
};

struct dspmain_t
{
    MSGQ_Queue dsp_msgq;
    MSGQ_Queue gpp_msgq;
    int reply_msgq;
    int pool_id;
    int sequence;
    int processing_count;
    int term;
    int unique_id;
    char gpp_msgq_name[64];
    int listen_sck;
    int last_peer_id;
    struct dspmain_peer_t* peer_head;
    struct dspmain_peer_t* peer_tail;
};

int
logln(int log_level, const char* format, ...);

DSP_STATUS
send_mult_msg(struct dspmain_t* dspmain, int* user, int num_user,
              int x, int y, int z);
DSP_STATUS
send_crc32_msg(struct dspmain_t* dspmain, int* user, int num_user,
               int addr, int format, int width, int height,
               int stride_bytes, int crc32);

#define LOG_ERROR 0
#define LOG_WARN  1
#define LOG_INFO  2
#define LOG_DEBUG 3

#define LOGS "[%s][%d][%s]:"
#define LOGP __FILE__, __LINE__, __FUNCTION__

#if !defined(__FUNCTION__) && defined(__FUNC__)
#define LOG_PRE const char* __FUNCTION__ = __FUNC__; (void)__FUNCTION__;
#else
#define LOG_PRE
#endif

#if !defined(LOG_LEVEL)
#define LOG_LEVEL 1
#endif
#if LOG_LEVEL > 0
#define LOGLN(_args) do { LOG_PRE logln _args ; } while (0)
#else
#define LOGLN(_args)
#endif
#if LOG_LEVEL > 10
#define LOGLND(_args) do { LOG_PRE logln _args ; } while (0)
#else
#define LOGLND(_args)
#endif

#endif
