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
#include "api.h"

#define flush() fflush(stdout)

char *ssp_host="0.0.0.0";
short int ssp_port=8083;
char *ssp_pidfile="/var/run/ssp.pid";

char *ssp_user="daemon";
int ssp_maxclients=1000;
int ssp_maxrecvs=2*1024*1024;

static bool listened;
static pthread_attr_t pth_recv_attr;
static pthread_t *tids;

int recv_data_len(node *ptr){
	unsigned char buf[4];
	int len=0,i,ret;

	ret=recv(ptr->sockfd,buf,sizeof(buf),MSG_WAITALL);

	if(ret!=sizeof(buf)){
		return ret>0?-1:0;
	}

	if(buf[0]){
		dprintf("Recieve From Client (sockfd:%d,host:\"%s\",port:%d) Data Package Header Error!\n",ptr->sockfd,ptr->host,ptr->port);
		return 0;
	}

	for(i=0;i<4;i++){
		len+=(buf[i]&0xff)<<((3-i)*8);
	}
	return len;
}

//接收来自客户端数据
//返回值:0(关闭连接),-1(接收到的数据长度与数据包长度不一致),>0(接收成功)
int socket_recv(node *ptr,char **data,int *data_len){
	int ret=recv_data_len(ptr);
	if(ret>0){
		*data_len=ret;
		if(*data_len>ssp_maxrecvs){
			dprintf("Recieve From Client (sockfd:%d,host:\"%s\",port:%d) Data Package Length (%d) Larger Than maxrecvs(%d)!\n",ptr->sockfd,ptr->host,ptr->port,*data_len,ssp_maxrecvs);
			return 0;
		}
		if(*data!=NULL){
			free(*data);
			*data=NULL;
		}
		*data=(char*)malloc(*data_len+1);
	}else{
		return ret;
	}
	ret=recv(ptr->sockfd,*data,*data_len,MSG_WAITALL);
	if(ret!=*data_len){
		free(*data);
		*data=NULL;
		if(ret>0){
			dprintf("Recieve From Client (sockfd:%d,host:\"%s\",port:%d) Package Length (%d),Recved Length(%d)!\n",ptr->sockfd,ptr->host,ptr->port,*data_len,ret);
			return -1;
		}else{
			return 0;
		}
	}else{
		*(*data+(*data_len))=0;//把最后一个字符设置为\0
	}
	return 1;
}

int socket_send(node *ptr,const char *data,int data_len){
	if(data_len<=0){
		return -1;
	}
	int plen=4+data_len;
	char *package;
	package=(char*)malloc(plen);

	int i;
	for(i=0;i<4;i++){
		package[i]=data_len>>((3-i)*8);
	}

	memcpy(package+4,data,data_len);//数据包内容

	int ret=send(ptr->sockfd,package,plen,0);
	free(package);
	if(ret>0 && ret!=plen){
		dprintf("Send Data Error! Package Length (%d),Sent Length(%d)!\n",plen,ret);
	}
	return ret;
}

void socket_break(int sid){
	shutdown(head->sockfd,2);
	unlink(ssp_pidfile);
}

static void node_clear(node *ptr){
	shutdown(ptr->sockfd,2);
	close(ptr->sockfd);
}

static void *socket_recv_thread(void *_ptr){
	node *ptr=(node*)_ptr;
	int data_len=0,ret;
	char *data=NULL;

	ret=socket_recv(ptr,&data,&data_len);

	THREAD_STARTUP();

	if(ret<=0){//关闭连接
		trigger(PHP_SSP_CLOSE,ptr);
		node_clear(ptr);
		remove_node(ptr);
	}else{//接收数据成功
		trigger(PHP_SSP_RECEIVE,ptr,&data,&data_len);
		if(data_len>0){
			trigger(PHP_SSP_SEND,ptr,&data,&data_len);
			socket_send(ptr,data,data_len);
		}
	}
	free(data);

	if(ret!=0){
		ptr->tid=0;
		ptr->reading=false;
	}

	THREAD_SHUTDOWN();

	pthread_detach(pthread_self());
	pthread_exit(NULL);
	return NULL;
}

static void *socket_accept_thread(void *_ptr){
	node *ptr=(node*)_ptr;

	insert_node(ptr);

	THREAD_STARTUP();

	if(ptr->index==0){
		trigger(PHP_SSP_CONNECT_DENIED,ptr);

		node_clear(ptr);
		free(ptr);
	}else{
		trigger(PHP_SSP_CONNECT,ptr);

		ptr->tid=0;
		ptr->reading=false;
	}

	THREAD_SHUTDOWN();

	pthread_detach(pthread_self());
	pthread_exit(NULL);
	return NULL;
}

static void *socket_daemon_thread(void *_indx){
	int i,n=1,indx=*(int*)_indx;
	bool flag=false;
	struct timeval tv;
	fd_set fds;
	int maxsock;
	int ret;
	node *ptr;
	int idxs[col_num];
	while(listened){
		FD_ZERO(&fds);
		n=0;
		maxsock=0;
		BEGIN_READ_NODE{
			for(i=0;i<col_num;i++){
					ptr=gnodes[indx][i];
					if(ptr!=NULL && ptr->reading==false){
						FD_SET(ptr->sockfd,&fds);
						if(ptr->sockfd>maxsock){
							maxsock=ptr->sockfd;
						}
						idxs[n++]=i;
						flag=true;
					}
			}
		}END_READ_NODE;

		if(n==0){
			usleep(100);
			continue;
		}

		tv.tv_sec=0;
		tv.tv_usec=10;

		ret=select(maxsock+1,&fds,NULL,NULL,&tv);
		if(ret<=0){
			continue;
		}
		BEGIN_READ_NODE{
			for(i=0;i<n;i++){
				ptr=gnodes[indx][idxs[i]];
				if(ptr!=NULL && FD_ISSET(ptr->sockfd,&fds)){
					ptr->reading=true;
					while(pthread_create(&ptr->tid,&pth_recv_attr,socket_recv_thread,ptr)!=0){
						dprintf("Create socket(%d:%d) recv thread error!\n",indx,idxs[i]);
						END_READ_NODE{
							usleep(10);
						}BEGIN_READ_NODE;
					}
				}
			}
		}END_READ_NODE;
	}

	free(_indx);
	tids[indx]=0;

	pthread_detach(pthread_self());
	pthread_exit(NULL);
	return NULL;
}

int socket_start(){
	struct sockaddr_in sin;
	struct sockaddr_in pin;
	socklen_t len=sizeof(pin);
	int listen_fd;
	int conn_fd;
	int ret;
	node *ptr;

	int pid,i=19,cols=tput_cols();

	printf("Starting SSP server");
	strnprint(".",cols-i-9);
	flush();

	signal(SIGHUP,socket_break);
	signal(SIGTERM,socket_break);
	signal(SIGINT,socket_break);
	signal(SIGTSTP,socket_break);

	listen_fd=socket(AF_INET,SOCK_STREAM,0);
	if(listen_fd<0){
		system("echo -e \"\\E[31m\".[Failed]");
		system("tput sgr0");
		printf("Not on the host %s bind port %d\n",ssp_host,ssp_port);
		return 0;
	}
	int opt=1;
	setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(int));

	int send_timeout=5000,recv_timeout=5000;
	setsockopt(listen_fd,SOL_SOCKET,SO_SNDTIMEO,&send_timeout,sizeof(int));//发送超时
	setsockopt(listen_fd,SOL_SOCKET,SO_RCVTIMEO,&recv_timeout,sizeof(int));//接收超时

	typedef struct {
		u_short l_onoff;
		u_short l_linger;
	} linger;
	linger m_sLinger;
	m_sLinger.l_onoff=1;//(在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
	// 如果m_sLinger.l_onoff=0;则功能和2.)作用相同;
	m_sLinger.l_linger=5;//(容许逗留的时间为5秒)
	setsockopt(listen_fd,SOL_SOCKET,SO_LINGER,(const char*)&m_sLinger,sizeof(linger));

	int send_buffer=0,recv_buffer=0;
	setsockopt(listen_fd,SOL_SOCKET,SO_SNDBUF,(char *)&send_buffer,sizeof(int));//发送缓冲区大小
	setsockopt(listen_fd,SOL_SOCKET,SO_RCVBUF,(char *)&recv_buffer,sizeof(int));//接收缓冲区大小

	bzero(&sin,sizeof(sin));
	sin.sin_family=AF_INET;
	sin.sin_addr.s_addr=inet_addr(ssp_host);
	sin.sin_port=htons(ssp_port);

	ret=bind(listen_fd,(struct sockaddr *)&sin,sizeof(sin));
	if(ret<0){
		system("echo -e \"\\E[31m\".[Failed]");
		system("tput sgr0");
		printf("Not on the host %s bind port %d\n",ssp_host,ssp_port);
		return 0;
	}

	ret=listen(listen_fd,128);
	if(ret<0){
		system("echo -e \"\\E[31m\".[Failed]");
		system("tput sgr0");
		return 0;
	}

	struct passwd *pwnam;
	pwnam = getpwnam(ssp_user);
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
		return 0;
	}
	if(pid>0){
		FILE *fp;
		fp=fopen(ssp_pidfile,"w+");
		if(fp==NULL){
			printf("file '%s' open fail.\n",ssp_pidfile);
		}else{
			fprintf(fp,"%d",pid);
			fclose(fp);
		}
		sleep(1);
		return 1;
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
	}else{
		system("echo -e \"\\E[32m\"[Succeed]");
		system("tput sgr0");
	}

	pthread_attr_init(&pth_recv_attr);
	pthread_attr_setguardsize(&pth_recv_attr,0);
	pthread_attr_setstacksize(&pth_recv_attr,32*1024);//set stack size 1M
	pthread_attr_setdetachstate(&pth_recv_attr,PTHREAD_CREATE_DETACHED);
	pthread_attr_setscope(&pth_recv_attr,PTHREAD_SCOPE_SYSTEM);

	attach_node();
	head->sockfd=listen_fd;
	inet_ntop(AF_INET, &sin.sin_addr, head->host, sizeof(head->host));
	head->port=ntohs(sin.sin_port);

	listened=true;

	SERVICE_STARTUP();
	trigger(PHP_SSP_START);

	//创建接收守护线程
	int *indx;
	tids=(pthread_t *)malloc(sizeof(pthread_t)*row_num);

	pthread_attr_t pth_daemon_attr;
	pthread_attr_init(&pth_daemon_attr);
	pthread_attr_setguardsize(&pth_daemon_attr,0);
	pthread_attr_setstacksize(&pth_daemon_attr,16*1024);//set stack size 1M
	pthread_attr_setscope(&pth_daemon_attr,PTHREAD_SCOPE_SYSTEM);

	for(i=0;i<row_num;i++){
		indx=(int*)malloc(sizeof(int));
		*indx=i;
		if(pthread_create(&tids[i],&pth_daemon_attr,socket_daemon_thread,indx)!=0){
			dprintf("Create daemon thread(%d) error!\n",i);
		}
	}

	while(listened){
		conn_fd=accept(listen_fd,(struct sockaddr *)&pin,&len);
		if(conn_fd<=0){
			break;
		}

		ptr=(node *)malloc(sizeof(node));

		bzero(ptr,sizeof(node));

		ptr->sockfd=conn_fd;
		inet_ntop(AF_INET,&pin.sin_addr,ptr->host,sizeof(ptr->host));
		ptr->port=ntohs(pin.sin_port);

		ptr->reading=true;

		while(pthread_create(&ptr->tid,&pth_recv_attr,socket_accept_thread,ptr)!=0){
			dprintf("Create socket(%d) accept thread error!\n",conn_fd);
			usleep(100);
		}
	}
	close(listen_fd);

	listened=false;
	usleep(100);
	//等待接收守护线程结束
	for(i=0;i<row_num;i++){
		if(tids[i]){
			pthread_join(tids[i],NULL);
		}
	}
	pthread_attr_destroy(&pth_daemon_attr);
	
	BEGIN_READ_NODE{
		pthread_t tid;
		node *p=head;
		while(p->next!=head){
			p=p->next;

			if(p->tid>0){
				pthread_join(p->tid,NULL);
			}

			p->reading=false;

			trigger(PHP_SSP_CLOSE,p);

			node_clear(p);
		}
	}END_READ_NODE;

	trigger(PHP_SSP_STOP);
	SERVICE_SHUTDOWN();

	detach_node();

	pthread_attr_destroy(&pth_recv_attr);

	exit(0);
}

int socket_status(){
	FILE *fp;
	int pid,i=17,cols=tput_cols();

	printf("SSP server status");
	flush();
	strnprint(".",cols-i-9);

	fp=fopen(ssp_pidfile,"r+");
	if(fp!=NULL){
		fscanf(fp,"%d",&pid);
		fclose(fp);
		if(pid==getsid(pid)){
			system("echo -e \"\\E[32m\"[Running]");
			system("tput sgr0");
			return 1;
		}
		unlink(ssp_pidfile);
	}
	system("echo -e \"\\E[31m\"[stopped]");
	system("tput sgr0");
	return 0;
}

int socket_stop(){
	FILE *fp;
	int pid,i=19,cols=tput_cols();

	printf("Stopping SSP server");
	flush();

	fp=fopen(ssp_pidfile,"r+");
	if(fp!=NULL){
		fscanf(fp,"%d",&pid);
		fclose(fp);
		unlink(ssp_pidfile);
		if(pid==getsid(pid)){
			kill(pid,SIGTERM);
			while(pid==getsid(pid)){
				printf(".");
				flush();
				i++;
				sleep(1);
				if(cols-i-9==0){
					kill(pid,SIGKILL);break;
				}
			}
			strnprint(".",cols-i-9);
			flush();
			system("echo -e \"\\E[32m\"[Succeed]");
			system("tput sgr0");
			return 1;
		}
	}
	strnprint(".",cols-i-8);
	flush();
	system("echo -e \"\\E[31m\"[Failed]");
	system("tput sgr0");
	return 0;
}
