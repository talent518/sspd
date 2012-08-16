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
#include <signal.h>
#include <pthread.h>
#include <pwd.h>
#include <sys/types.h>

#include "php_ext.h"
#include "php_func.h"
#include "server.h"
#include "node.h"

bool debug=false;

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

int recv_int(int sockfd){
	char *buf;
	int len=0,ret,i;
	buf=(char*)malloc(sizeof(int));
	bzero(buf,sizeof(int));
	i=0;
	do{
		ret=recv(sockfd,buf+i,1,MSG_WAITALL);
		if(ret!=1){
			return ret;
		}
		if(*buf==0){
			i++;
		}
	}while(i!=sizeof(int));

	for(i=0;i<4;i++){
		len+=(buf[i]&0xff)<<((3-i)*8);
	}

	free(buf);

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

#ifdef ZTS
	ssp_request_startup(php_self);
#endif

	trigger(PHP_SSP_CONNECT,ptr);

	TSRMLS_FETCH();
	if(node_num>SSP_G(maxclients)){
		trigger(PHP_SSP_CONNECT_DENIED,ptr);
		ptr->flag=false;
	}

	while(ptr->flag){
		if(recved_len==0){
			recv_len=recv_int(ptr->sockfd);
			if(recv_len>0){
				if(recv_len>SSP_G(maxrecvs)){
					php_printf("Server Recieve Package Length Must %d<=%d!\n",recv_len,SSP_G(maxrecvs));
					break;
				}
				package=(char*)malloc(sizeof(char*)*recv_len);
			}else{
				break;
			}
		}

		len=recv(ptr->sockfd,package+recved_len,recv_len-recved_len,MSG_WAITALL);
		if(len<0 && debug){
			free(package);
			php_printf("Server Recieve Package Data Failed!\n");
			break;
		}
		if(len==0){
			free(package);
			break;
		}

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

	shutdown(ptr->sockfd,2);
	close(ptr->sockfd);

	delete(ptr);

	php_request_shutdown((void *)0);

	pthread_exit(NULL);
}

int socket_send(int sockfd,const char *data,int data_len){
	if(data_len<=0){
		return -1;
	}
	int plen=sizeof(int)+data_len;
	char *package;
	package=(char*)malloc(plen);
	int i;
	for(i=0;i<4;i++){
		package[i]=data_len>>((3-i)*8);
	}

	memcpy(package+sizeof(int),data,data_len);

	int ret=send(sockfd,package,plen,0);
	if(ret!=sizeof(data_len)+data_len && debug){
		php_printf("Send Data Error! Length:%d,Package Length:%d\n",data_len,plen);
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

	TSRMLS_FETCH();

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

	TSRMLS_FETCH();
	fp=fopen(SSP_G(pidfile),"r+");
	if(fp!=NULL){
		fscanf(fp,"%d",&pid);
		fclose(fp);
		unlink(SSP_G(pidfile));
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
	//node *p;
	//pthread_mutex_lock(&node_mutex);
	head->flag=false;
	shutdown(head->sockfd,2);
	//close(head->sockfd);
	/*p=head;
	while(p->next!=head){
		p=p->next;
		p->flag=false;
		if(shutdown(p->sockfd,2)!=0){
			//node_num--;
			printf("shutdown node(%d) error(%d)",node_num,errno);
		}
		//close(p->sockfd);
		//trigger(PHP_SSP_CLOSE,p);
		//usleep(100);
	}
	pthread_mutex_unlock(&node_mutex);
*/
	TSRMLS_FETCH();
	unlink(SSP_G(pidfile));

	//sleep(1);
	//trigger(PHP_SSP_STOP);

	//php_request_shutdown((void *) 0);
	//php_end();

	//exit(EG(exit_status));
}

int socket_start(){
	TSRMLS_FETCH();
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

	ret=listen(listen_fd,SSP_G(maxclients));
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

	construct();
	head->sockfd=listen_fd;
	inet_ntop(AF_INET, &sin.sin_addr, head->host, sizeof(head->host));
	head->port=ntohs(sin.sin_port);
	head->flag=true;

	if(debug){
		php_printf("\nListen host for the %s, port %d.\n",head->host,head->port);
	}

	trigger(PHP_SSP_START);

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

		insert(ptr);

		pthread_create(&ptr->tid,NULL,(void*)thread,ptr);
	}

	close(listen_fd);

	pthread_t tid;
	void *tret;
	ptr=head->next;
	while(ptr!=head){
		tid=ptr->tid;
		ptr->flag=false;
		if(shutdown(ptr->sockfd,2)!=0){
			printf("shutdown node(%d) error(%d)",node_num,errno);
		}
		ptr=ptr->next;
		pthread_join(tid,&tret);
	}

	if(node_num>0){
		printf("There are %d nodes not successfully deleted.",node_num);
	}

	destruct();

	trigger(PHP_SSP_STOP);

	php_end();

	exit(EG(exit_status));
}
