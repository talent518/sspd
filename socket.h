#ifndef _SOCKET_H
#define _SOCKET_H

#include "data.h"

typedef struct _send_t {
	conn_t *ptr;
	char *str;
	int len;
} send_t;

int recv_data_len(conn_t *ptr);
int socket_recv(conn_t *ptr, char **data, int *data_len);
int socket_send(conn_t *ptr, char *data, int data_len);

void socket_close(conn_t *ptr);

void socket_set_listen(int s);
void socket_set_accept(int s, int send_timeout, int recv_timeout, int send_buffer, int recv_buffer);
#define socket_set_connect(s,rt,st,rb,sb) socket_set_accept(s,rt,st,rb,sb)

void _socket_setnonblock(int fd);
void _socket_setblocking(int fd);

#define socket_setnonblock(ptr) _socket_setnonblock(ptr->sockfd)
#define socket_setblocking(ptr) _socket_setblocking(ptr->sockfd)

#endif
