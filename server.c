#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <sys/socket.h>
#include <error.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <signal.h>
#include <pthread.h>
#include <pwd.h>
#include <sys/types.h>

#include "php_ext.h"
#include "server.h"

bool debug=false;
int node_num=0;
pthread_mutex_t node_mutex;

#define COLS shl_cols()
#define flush() fflush(stdout)

int shl_cols(){
	FILE *fp;
	int ret=0;
	fp=popen("tput cols","r");
	if(fp!=NULL){
		fscanf(fp,"%d",&ret);
		fclose(fp);
	}
	return ret;
}

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
		perror("delete node error.");
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

void thread(node *ptr){
	int i,len,socket;
	char *q;
	char buff[BUFFER_MAX];
	socket=ptr->sockfd;
	while(ptr->flag)
	{
		memset(buff,0,sizeof(buff));
		len=read(socket,buff,BUFFER_MAX);
		if(len<0){
			printf("Server Recieve Data Failed!\n");
			break;
		}
		if(len==0)
			break;
		if((q=strchr(buff,'\n'))!=NULL)
			*q='\0';
		if(debug)
			printf("Received from client :%s\n\n",buff);
	}
	del(head,socket);
	if(debug)
		printf("Close connections (%d) for the host %s, port %d.\n",socket,ptr->host,ptr->port);
	pthread_exit(NULL);
}

int socket_stop(){
	FILE *fp;
	int pid,ret,i=19,cols=COLS;

	printf("Stopping SSP server");
	flush();

	fp=fopen(SSP_G(pidfile),"r+");
	if(fp!=NULL){
		fscanf(fp,"%d",&pid);
		fclose(fp);
		if(pid==getsid(pid)){
			ret=kill(pid,SIGKILL);
			while(ret && getsid(pid)){
				printf(".");
				flush();
				sleep(1);
				i++;
			}
			while(cols-9>i){
				printf(".");
				flush();
				i++;
			}
			system("echo -e \"\\E[32m\"[Succeed]");
			system("tput sgr0");
			return 1;
		}
		unlink(SSP_G(pidfile));
	}
	while(cols-8>i){
		printf(".");
		flush();
		i++;
	}
	system("echo -e \"\\E[31m\"[Failed]");
	system("tput sgr0");
	return 0;
}

void socket_exit(int sid){
	node *p;
	p=head;
	do{
		p->flag=false;
		close(p->sockfd);
		p->sockfd=0;
		p=p->next;
	}while(p!=NULL);
	exit(0);
}

int socket_start(){
	struct sockaddr_in sin;
	struct sockaddr_in pin;
	socklen_t len=sizeof(pin);
	int listen_fd;
	int conn_fd;
	int ret;
	node *ptr;

	int pid,i=19,cols=COLS;

	printf("Starting SSP server");
	while(cols-9>i){
		printf(".");
		i++;
	}
	flush();

	signal(SIGPIPE,SIG_IGN);
	signal(SIGCHLD,SIG_IGN);

	signal(SIGHUP,socket_exit);
	signal(SIGTERM,socket_exit);
	signal(SIGINT,socket_exit);
	signal(SIGKILL,socket_exit);
	signal(SIGSTOP,socket_exit);
	signal(SIGTSTP,socket_exit);

	listen_fd=socket(AF_INET,SOCK_STREAM,0);
	{
		int opt=1;
		setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
	}

	bzero(&sin,sizeof(sin));
	sin.sin_family=AF_INET;
	sin.sin_addr.s_addr=inet_addr(SSP_G(host));
	sin.sin_port=htons(SSP_G(port));

	ret=bind(listen_fd,(struct sockaddr *)&sin,sizeof(sin));
	if(ret<0){
		system("echo -e \"\\E[31m\".[Failed]");
		system("tput sgr0");
		printf("Not on the host %s bind port %d",head->host,head->port);
		return 1;
	}

	ret=listen(listen_fd,T_MAX);
	if(ret<0){
		system("echo -e \"\\E[31m\".[Failed]");
		system("tput sgr0");
		return 1;
	}

	struct passwd *pwnam;
	pwnam = getpwnam(SSP_G(user));
/*
	printf("name:%s\n",pwnam->pw_name);
	printf("passwd:%s\n",pwnam->pw_passwd);
	printf("uid:%d\n",pwnam->pw_uid);
	printf("gid:%d\n",pwnam->pw_gid);
	printf("home:%s\n",pwnam->pw_dir);
	printf("gecos:%s\n",pwnam->pw_gecos);
	printf("shell:%s\n",pwnam->pw_shell);
*/

	pid=fork();

	if(pid==-1){
		system("echo -e \"\\E[31m\".[Failed]");
		system("tput sgr0");
		printf("fork failure!\n");
		return 1;
	}
	if(pid>0){
		FILE *fp;
		fp=fopen(SSP_G(pidfile),"w+");
		if(fp==NULL){
			printf("file '%s' open fail.\n",SSP_G(pidfile));
		}else{
			fprintf(fp,"%d",pid);
			fclose(fp);
		}
		sleep(1);
		return 0;
	}

	setuid(pwnam->pw_uid);
	setgid(pwnam->pw_gid);

	ret=setsid();
	if(ret<1){
		system("echo -e \"\\E[31m\".[Failed]");
		system("tput sgr0");
		printf("sid:%d\n",ret);
		return 1;
	}

	system("echo -e \"\\E[32m\"[Succeed]");
	system("tput sgr0");

	head=(node *)malloc(sizeof(node));
	head->sockfd=listen_fd;
	inet_ntop(AF_INET, &sin.sin_addr, head->host, sizeof(head->host));
	head->port=ntohs(sin.sin_port);
	head->flag=true;
	head->next=NULL;

	if(debug)
		printf("\nListen host for the %s, port %d.\n\n",head->host,head->port);

    pthread_mutex_init(&node_mutex, NULL);

    pthread_attr_t child_thread_attr;
    pthread_attr_init(&child_thread_attr);
    pthread_attr_setdetachstate(&child_thread_attr,PTHREAD_CREATE_DETACHED);

	while(head->flag){
		conn_fd=accept(listen_fd,(struct sockaddr *)&pin,&len);
		ptr=(node *)malloc(sizeof(node));
		ptr->sockfd=conn_fd;

		inet_ntop(AF_INET, &pin.sin_addr, ptr->host, sizeof(ptr->host));
		ptr->port=ntohs(pin.sin_port);

		if(debug)
			printf("\n\nAccept new connections (%d) for the host %s, port %d.\n\n",conn_fd,ptr->host,ptr->port);

		ptr->flag=true;

		ptr->next=NULL;
		insert(head,ptr);
		pthread_create(&ptr->tid,&child_thread_attr,(void*)thread,ptr);
	}

	node_num--;

	close(listen_fd);
    pthread_attr_destroy(&child_thread_attr);
    pthread_mutex_destroy(&node_mutex);
    pthread_exit(NULL);

	return 0;
}
