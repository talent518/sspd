#ifndef _SERVER_H
#define _SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "node.h"

#if 0
	#define dprintf(...) printf(__VA_ARGS__)
#else
	#define dprintf(...)
#endif

extern char *ssp_host;
extern short int ssp_port;
extern char *ssp_pidfile;

extern char *ssp_user;
extern int ssp_maxclients;
extern int ssp_maxrecvs;

int socket_send(node *ptr,const char *data,int data_len);

int socket_start();
int socket_stop();
int socket_status();

#endif
