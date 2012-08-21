#ifndef _SERVER_H
#define _SERVER_H

#include <stdbool.h>
#include <pthread.h>

#define SSP_OPT_HOST 2
#define SSP_OPT_PORT 3
#define SSP_OPT_PIDFILE 1

#define SSP_OPT_USER 0
#define SSP_OPT_MAX_CLIENTS 4
#define SSP_OPT_MAX_RECVS 5

extern bool debug;

extern char *ssp_host;
extern short int ssp_port;
extern char *ssp_pidfile;

extern char *ssp_user;
extern int ssp_maxclients;
extern int ssp_maxrecvs;

#ifndef ZTS
	extern pthread_mutex_t *ssp_mutex;
#endif

int socket_send(int sockfd,const char *data,int data_len);
int socket_start();
int socket_stop();
int socket_status();

#endif
