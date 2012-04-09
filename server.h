#ifndef _SERVER_H
#define _SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <error.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <pthread.h>

#define BUFFER_MAX 1024
#define T_MAX 20

extern int node_num;

typedef struct user{
	pthread_t tid;
	int sockfd;
	char host[15];
	uint16_t port;
	struct user *next;
} node;

node *head;

int socket_listen(short int port,char *pidfile);

#endif
