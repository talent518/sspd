#ifndef _SOCKET_H
#define _SOCKET_H

#include "node.h"

int recv_data_len(node *ptr);
int socket_recv(node *ptr,char **data,int *data_len);
int socket_send(node *ptr,const char *data,int data_len);

#endif
