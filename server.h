#ifndef _SERVER_H
#define _SERVER_H

#include <stdbool.h>

extern bool debug;

int socket_send(int sockfd,const char *data,int data_len);
int socket_start();
int socket_stop();
int socket_status();

#endif
