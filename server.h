#ifndef _SERVER_H
#define _SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <error.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>

extern int node_num;
extern bool debug;

typedef struct user{
	int sockfd;
	char host[15];
	uint16_t port;
	int flag;
	struct user *next;
} node;

node *head;
node *find(int sockfd,bool is_port);

int socket_send(int sockfd,char *data,int data_len);
int socket_start();
int socket_stop();
int socket_status();

#endif
