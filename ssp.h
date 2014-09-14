#ifndef _SERVER_H
#define _SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "config.h"

#ifdef SSP_DEBUG_PRINTF
	#define dprintf(...) printf(__VA_ARGS__)
	#define conn_info(ptr) _conn_info_ex(stdout,ptr,"[ conn_info ] ")
#else
	#define dprintf(...)
	#define conn_info(ptr)
#endif

#define _conn_info_ex(fd,ptr,append) fprintf(fd,append" in %s on line %d: index(%d), sockfd(%d), host(%s), port(%d)!\n", __func__, __LINE__, ptr->index, ptr->sockfd, ptr->host, ptr->port)
#define conn_info_ex(ptr,append) _conn_info_ex(stderr,ptr,append)

extern unsigned int ssp_backlog;

extern char *ssp_host;
extern short int ssp_port;
extern char *ssp_pidfile;

extern char *ssp_user;
extern int ssp_nthreads;
extern int ssp_maxclients;
extern int ssp_maxrecvs;

int server_start();
int server_stop();
int server_status();

#endif
