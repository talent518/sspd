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

int recv_int(int sockfd){
	char *buf;
	int len=0,ret,i;
	buf=(char*)malloc(sizeof(int));
	bzero(buf,sizeof(int));
	ret=recv(sockfd,buf,sizeof(int),0);
	if(ret!=sizeof(int)){
		return 0;
	}
	for(i=0;i<4;i++){
		len+=(buf[i]&0xff)<<((3-i)*8);
	}
	return len;
}

void thread(node *ptr){
	int recv_len=0,recved_len=0,len,i;
	char *package;

	trigger(PHP_SSP_CONNECT,ptr);

	while(ptr->flag){
		if(recved_len==0){
			recv_len=recv_int(ptr->sockfd);
			if(recv_len>0){
				package=(char*)malloc(sizeof(char*)*recv_len);
			}
		}
		if(recv_len<1){
			len=-1;
		}else{
			len=recv(ptr->sockfd,package+recved_len,recv_len-recved_len,0);
		}
		if(len<0 && debug){
			php_printf("Server Recieve Data Failed!\n");
			break;
		}
		if(len==0)
			break;
		if(debug){
			php_printf("Received from client :%s\n\n",package);
		}

		recved_len+=len;

		if(recved_len==recv_len){
			trigger(PHP_SSP_RECEIVE,ptr,&package,&recv_len);
			if(recv_len>0){
				trigger(PHP_SSP_SEND,ptr,&package,&recv_len);
			}
			socket_send(ptr->sockfd,package,recv_len);
			free(package);
			recved_len=0;
		}
	}
	if(debug){
		php_printf("Close connections (%d) for the host %s, port %d.\n",ptr->sockfd,ptr->host,ptr->port);
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
		php_printf("data_len:%d,package_len:%d\n",data_len,sizeof(data_len)+data_len);
	}
	return ret;
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
			ret=kill(pid,SIGTERM);
			int status,wait_pid;
			wait_pid = waitpid(pid, &status, WNOHANG|WUNTRACED);
#ifdef WIFEXITED
			if(!WIFEXITED(status)){
				kill(pid,SIGKILL);
			}
#endif
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
	while(p->next!=NULL){
		p=p->next;
		p->flag=false;
		p->sockfd=0;
		pthread_exit(&p->tid);
		trigger(PHP_SSP_CLOSE,p);
		close(p->sockfd);
	};
	close(head->sockfd);
	head->flag=false;
	head->sockfd=0;
	pthread_mutex_unlock(&node_mutex);
	trigger(PHP_SSP_STOP);
	unlink(SSP_G(pidfile));
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
		ptr=(node *)malloc(sizeof(node));
		ptr->sockfd=conn_fd;

		inet_ntop(AF_INET, &pin.sin_addr, ptr->host, sizeof(ptr->host));
		ptr->port=ntohs(pin.sin_port);

		if(debug){
			php_printf("\nAccept new connections (%d) for the host %s, port %d.\n",conn_fd,ptr->host,ptr->port);
		}

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
