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

node *find(int sockfd,bool is_port){
	node *p,*ptr=NULL;
	p=head;
	while(p->next!=NULL){
		p=p->next;
		if(is_port && p->port==sockfd){
			ptr=p;
			break;
		}
		if(!is_port && p->sockfd==sockfd){
			ptr=p;
			break;
		}
	}
	return ptr;
}

int insert(node *head,node *ptr){
	pthread_mutex_lock(&node_mutex);
	node *p;
	p=head;
	//printf("\ninsert node:\n");
	while(p->next!=NULL){
		p=p->next;
		//printf("sockfd:%d,host:%s,port:%d,flag:%s\n",p->sockfd,p->host,p->port,p->flag?"true":"false");
	}
	p->next=ptr;
	//printf("new node sockfd:%d,host:%s,port:%d,flag:%s\n",ptr->sockfd,ptr->host,ptr->port,ptr->flag?"true":"false");
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
	while(p->next!=NULL){
		q=p->next;
		if(q->sockfd==socket)
		{
			p->next=q->next;
			free(q);
			close(socket);
			node_num--;
			break;
		}else
			p=p->next;
	}
	pthread_mutex_unlock(&node_mutex);
	return 0;
}

int recv_int(int sockfd){
	char *buf;
	int len=0,ret,i;
	buf=(char*)malloc(sizeof(int));
	bzero(buf,sizeof(int));
	i=0;
	do{
		ret=recv(sockfd,buf+i,sizeof(int)-i,0);
		if(ret<1){
			return ret;
		}
		i+=ret;
	}while(i!=sizeof(int));

	for(i=0;i<4;i++){
		len+=(buf[i]&0xff)<<((3-i)*8);
	}

	if(len>0){
		return len;
	}else{
		php_printf("Server Recieve Package Header Error:%d\n",len);
		return len;
	}
}

void thread(node *ptr){
	int recv_len=0,recved_len=0,len,i;
	char *package;

	if(debug){
		php_printf("\nAccept new connections (%d) for the host %s, port %d.\n",ptr->sockfd,ptr->host,ptr->port);
	}

	pthread_mutex_lock(&node_mutex);
	trigger(PHP_SSP_CONNECT,ptr);
	if(node_num>SSP_G(maxclients)){
		trigger(PHP_SSP_CONNECT_DENIED,ptr);
		ptr->flag=false;
	}
	pthread_mutex_unlock(&node_mutex);

	while(ptr->flag){
		if(recved_len==0){
			recv_len=recv_int(ptr->sockfd);
			if(recv_len>0){
				if(recv_len>SSP_G(maxrecvs)){
					php_printf("Server Recieve Package Length Must %d<=%d!\n",recv_len,SSP_G(maxrecvs));
					break;
				}
				package=(char*)malloc(sizeof(char*)*recv_len);
			}else if(recv_len==0x47455420){
				recv_len=0;
				continue;
			}else{
				break;
			}
		}

		len=recv(ptr->sockfd,package+recved_len,recv_len-recved_len,0);
		if(len<0 && debug){
			php_printf("Server Recieve Package Data Failed!\n");
			break;
		}
		if(len==0)
			break;
/*
		printf("\npackage len:%d,package data:%s\n",recv_len-recved_len,strndup(package+recved_len,recv_len-recved_len));
		if(!(strncmp(package,"<?xml ",6)==0 || strncmp(package,"<request ",9)==0)){
			free(package);
			recved_len=0;
			continue;
		}
*/
		recved_len+=len;

		if(recved_len==recv_len && recv_len>0){
		zend_first_try{
			trigger(PHP_SSP_RECEIVE,ptr,&package,&recv_len);
			if(recv_len>0){
				trigger(PHP_SSP_SEND,ptr,&package,&recv_len);
				socket_send(ptr->sockfd,package,recv_len);
			}
		}zend_end_try();
			free(package);
			recved_len=0;
		}
	}
	trigger(PHP_SSP_CLOSE,ptr);
	del(head,ptr->sockfd);
	pthread_exit(NULL);
}

int socket_send(int sockfd,char *data,int data_len){
	char *package;
	package=(char*)malloc(sizeof(data_len)+data_len);
	int i;
	for(i=0;i<4;i++){
		package[i]=data_len>>((3-i)*8);
	}

	memcpy(package+sizeof(data_len),data,data_len);

	int ret=send(sockfd,package,sizeof(data_len)+data_len,0);
	if(ret!=sizeof(data_len)+data_len && debug){
		php_printf("Send DAta Error! Length:%d,Package Length:%d\n",data_len,sizeof(data_len)+data_len);
	}
	return ret;
}

int socket_status(){
	FILE *fp;
	int pid,ret,i=17,cols=COLS;

	printf("SSP server status");
	flush();

	while(cols-9>i){
		printf(".");
		flush();
		i++;
	}

	fp=fopen(SSP_G(pidfile),"r+");
	if(fp!=NULL){
		fscanf(fp,"%d",&pid);
		fclose(fp);
		if(pid==getsid(pid)){
			system("echo -e \"\\E[32m\"[Running]");
			system("tput sgr0");
			return 1;
		}
		unlink(SSP_G(pidfile));
	}
	system("echo -e \"\\E[31m\"[stopped]");
	system("tput sgr0");
	return 0;
}

int socket_stop(){
	FILE *fp;
	int pid,i=19,cols=COLS;

	printf("Stopping SSP server");
	flush();

	fp=fopen(SSP_G(pidfile),"r+");
	if(fp!=NULL){
		fscanf(fp,"%d",&pid);
		fclose(fp);
		if(pid==getsid(pid)){
			kill(pid,SIGTERM);
			while(pid==getsid(pid)){
				printf(".");
				flush();
				i++;
				sleep(1);
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
	pthread_mutex_lock(&node_mutex);
	while(p!=NULL){
		p->flag=false;
		close(p->sockfd);
		shutdown(p->sockfd,2);
		trigger(PHP_SSP_CLOSE,p);
		usleep(100);
		p=p->next;
	}
	pthread_mutex_unlock(&node_mutex);

	trigger(PHP_SSP_STOP);

	unlink(SSP_G(pidfile));

	php_request_shutdown((void *) 0);
	php_end();

	exit(EG(exit_status));
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
		printf("Not on the host %s bind port %d\n",SSP_G(host),SSP_G(port));
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

#ifdef HAVE_SIGNAL_H
#if defined(SIGPIPE) && defined(SIG_IGN)
	signal(SIGPIPE, SIG_IGN);
#endif
#if defined(SIGCHLD) && defined(SIG_IGN)
	signal(SIGCHLD,SIG_IGN);
#endif
#endif

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

	if(debug){
		php_printf("\nListen host for the %s, port %d.\n",head->host,head->port);
	}

	trigger(PHP_SSP_START);

    pthread_mutex_init(&node_mutex, NULL);

    pthread_attr_t child_thread_attr;
    pthread_attr_init(&child_thread_attr);
    pthread_attr_setdetachstate(&child_thread_attr,PTHREAD_CREATE_DETACHED);

	while(head->flag){
		conn_fd=accept(listen_fd,(struct sockaddr *)&pin,&len);
		if(conn_fd<=0){
			break;
		}
		ptr=(node *)malloc(sizeof(node));
		ptr->sockfd=conn_fd;
		inet_ntop(AF_INET, &pin.sin_addr, ptr->host, sizeof(ptr->host));
		ptr->port=ntohs(pin.sin_port);
		ptr->flag=true;
		ptr->next=NULL;

		insert(head,ptr);

		pthread_mutex_lock(&node_mutex);
		pthread_create(&ptr->tid,&child_thread_attr,(void*)thread,ptr);
		pthread_mutex_unlock(&node_mutex);
	}

	node_num--;

	close(listen_fd);
    pthread_attr_destroy(&child_thread_attr);
    pthread_mutex_destroy(&node_mutex);
	pthread_exit(NULL);
}
