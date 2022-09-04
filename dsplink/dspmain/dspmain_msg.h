
#define MSGQ_MYMSGID 10

#define MYMSGHEADER MSGQ_MsgHeader msg_header; int subid; int sequence;


struct my_msg_t
{
    MYMSGHEADER
};

#define MULTMSGSUBID 0
struct my_msg_mult_t
{
    MYMSGHEADER
    int x;
    int y;
    int z;
};

#define LOGMSGSUBID 1
struct my_msg_log_t
{
    MYMSGHEADER
    char log_msg[256];
};
