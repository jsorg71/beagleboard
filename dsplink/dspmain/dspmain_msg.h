
#define MSGQ_MAX_BYTES 512
#define MSGQ_POOL_BYTES 0xD0000
#define MSGQ_MYMSGID 10
#define MSGQ_MYMSGID_NUM_USER 4

#define MYMSGHEADER \
    MSGQ_MsgHeader msg_header; \
    int subid; \
    int sequence; \
    int reply_msgq; \
    int user[MSGQ_MYMSGID_NUM_USER];

struct my_msg_t
{
    MYMSGHEADER
};

#define GETREPLYMSGQMSGSUBID 0
struct my_msg_get_reply_msgq_t
{
    MYMSGHEADER
    char msgq_name[32];
};

#define MULTMSGSUBID 1
struct my_msg_mult_t
{
    MYMSGHEADER
    int x;
    int y;
    int z;
};

#define CRC32MSGSUBID 2
struct my_msg_crc32_t
{
    MYMSGHEADER
    int addr;
    int format;
    int width;
    int height;
    int stride_bytes;
    int crc32;
};

#define SHUTDOWNMSGSUBID 3
struct my_msg_shutdown_t
{
    MYMSGHEADER
};
