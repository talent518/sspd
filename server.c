#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <error.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <signal.h>
#include <pthread.h>
#include "server.h"

int node_num=0;
pthread_mutex_t node_mutex;

int insert(node *head,node *ptr){
	pthread_mutex_lock(&node_mutex);
	node *p;
	p=head;
	while(p->next!=NULL){
		p=p->next;
	}
	p->next=ptr;
	node_num++;
	pthread_mutex_unlock(&node_mutex);
	return 0;
}

int del(node *head,int socket){
	pthread_mutex_lock(&node_mutex);
	node *p,*q;
	p=head;
	if(p->next==NULL)
	{
		perror("del");
		pthread_mutex_unlock(&node_mutex);
		return (-1);
	}
	while(p->next!=NULL)
	{
		q=p->next;
		if(q->sockfd==socket)
		{
			p->next=q->next;
			free(q);
			close(socket);
			break;
		}else
			p=p->next;
	}
	node_num--;
	pthread_mutex_unlock(&node_mutex);
	return 0;
}

void thread(node *p){
	int i,len,socket;
	char *q;
	char buff[BUFFER_MAX];
	socket=p->sockfd;
	while(1)
	{
		memset(buff,0,sizeof(buff));
		len=read(socket,buff,BUFFER_MAX);
		if(len<=0)
			break;
		if((q=strchr(buff,'\n'))!=NULL)
			*q='\0';
		printf("received from client :%s\n",buff);
	}
	del(head,socket);
	pthread_exit(NULL);
}

int socket_listen(short int port,char *pidfile){
	struct sockaddr_in sin;
	struct sockaddr_in pin;
	socklen_t len=sizeof(pin);
	int listen_fd;
	int conn_fd;
	int ret;
	node *ptr;

	signal(SIGPIPE,SIG_IGN);
	signal(SIGCHLD,SIG_IGN);

	head=(node *)malloc(sizeof(node));
	head->sockfd=0;
	head->next=NULL;
	bzero(&sin,sizeof(sin));
	sin.sin_family=AF_INET;
	sin.sin_addr.s_addr=INADDR_ANY;
	sin.sin_port=htons(port);
	listen_fd=socket(AF_INET,SOCK_STREAM,0);
	{
		int opt=1;
		setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	}
	ret=bind(listen_fd,(struct sockaddr *)&sin,sizeof(sin));
	if(ret<0){
		perror("bind");
		exit(1);
	}
	listen(listen_fd,T_MAX);
	printf("accepting connections......\n");

    pthread_mutex_init(&node_mutex, NULL);

    pthread_attr_t child_thread_attr;
    pthread_attr_init(&child_thread_attr);
    pthread_attr_setdetachstate(&child_thread_attr,PTHREAD_CREATE_DETACHED);

	while(1){
		conn_fd=accept(listen_fd,(struct sockaddr *)&pin,&len);
		ptr=(node *)malloc(sizeof(node));
		ptr->sockfd=conn_fd;

		inet_ntop(AF_INET, &pin.sin_addr, ptr->host, sizeof(ptr->host));
		ptr->port=ntohs(pin.sin_port);

		printf("\naccept connection: %s:%d\n",ptr->host,ptr->port);

		ptr->next=NULL;
		insert(head,ptr);
		pthread_create(&ptr->tid,&child_thread_attr,(void*)thread,ptr);
	}
    close(listen_fd);
    pthread_attr_destroy(&child_thread_attr);
    pthread_mutex_destroy(&node_mutex);
    pthread_exit (NULL);
}
