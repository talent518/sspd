#ifndef _SERVER_H
#define _SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#if 0
	#define dprintf(...) printf(__VA_ARGS__)
#else
	#define dprintf(...)
#endif

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
