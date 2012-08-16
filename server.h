#ifndef _SERVER_H
#define _SERVER_H

#include <stdbool.h>

extern bool debug;

char *ssp_user;
char *ssp_pidfile;
char *ssp_host;
short int ssp_port;
int ssp_maxclients;
int ssp_maxrecvs;
char **ssp_bind;
pthread_mutex_t *ssp_mutex;

int socket_send(int sockfd,const char *data,int data_len);
int socket_start();
int socket_stop();
int socket_status();

#endif
