#ifndef _SERVER_H
#define _SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "config.h"
#include "api.h"

#define BEGIN_RUNTIME() double runtime=microtime()
#define END_RUNTIME() printf("[ run time ] %20s: %.3fs\n", __func__, microtime()-runtime)

#ifdef SSP_DEBUG_PRINTF
#define dprintf(...) fprintf(stdout,__VA_ARGS__)
#define conn_info(ptr) _conn_info_ex(stdout,ptr,"[ conn_info ] ")
#else
#define dprintf(...)
#define conn_info(ptr)
#endif

#define _conn_info_ex(fd,ptr,append) fprintf(fd,append" in %20s on line (%3d): ref_count(%5d), index(%5d), sockfd(%5d), host(%15s), port(%5d)!\n", __func__, __LINE__, ptr->ref_count, ptr->index, ptr->sockfd, ptr->host, ptr->port)
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
