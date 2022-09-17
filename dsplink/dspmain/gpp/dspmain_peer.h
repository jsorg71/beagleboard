
#ifndef _DSPMAIN_PEER_H_
#define _DSPMAIN_PEER_H_

int
dspmain_queue_mult(struct dspmain_t* dspmain, struct dspmain_peer_t* peer,
                   struct my_msg_mult_t* my_msg_mult);
int
dspmain_queue_crc32(struct dspmain_t* dspmain, struct dspmain_peer_t* peer,
                    struct my_msg_crc32_t* my_msg_crc32);
int
dspmain_peer_get_fds(struct dspmain_t* dspmain, int* max_fd,
                     fd_set* rfds, fd_set* wfds);
int
dspmain_peer_check_fds(struct dspmain_t* dspmain, fd_set* rfds, fd_set* wfds);
int
dspmain_peer_add_fd(struct dspmain_t* dspmain, int sck);
int
dspmain_peer_queue(struct dspmain_peer_t* peer, struct stream* out_s);
int
dspmain_peer_queue_copy(struct dspmain_peer_t* peer, struct stream* out_s);

#endif
