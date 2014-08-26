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
#include <pwd.h>
#include <sys/types.h>
#include <signal.h>

#include "php_ext.h"
#include "php_func.h"
#include "ssp.h"
#include "node.h"
#include "api.h"

#define flush() fflush(stdout)

#define	BACKLOG 16

char *ssp_host="0.0.0.0";
short int ssp_port=8083;
char *ssp_pidfile="/var/run/ssp.pid";

char *ssp_user="daemon";

int ssp_maxclients=1000;
int ssp_maxrecvs=2*1024*1024;

// event.c中定义
void event_daemon (int sockfd);

int server_start(){
	struct sockaddr_in sin;
	int listen_fd;
	int ret;

	int pid,i=19,cols=tput_cols();

	printf("Starting SSP server");
	strnprint(".",cols-i-9);
	flush();

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

	ret=listen(listen_fd, BACKLOG);
	if(ret<0){
		system("echo -e \"\\E[31m\".[Failed]");
		system("tput sgr0");
		return 0;
	}

	struct passwd *pwnam;
	pwnam = getpwnam(ssp_user);

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

	attach_node();
	head->sockfd=listen_fd;
	inet_ntop(AF_INET, &sin.sin_addr, head->host, sizeof(head->host));
	head->port=ntohs(sin.sin_port);

	trigger(PHP_SSP_START);

	event_daemon(listen_fd);

	close(listen_fd);
	
	BEGIN_READ_NODE{
		node *p=head;
		while(p->next!=head){
			p=p->next;

			p->reading=false;

			trigger(PHP_SSP_CLOSE,p);

			clean_node(p);
		}
	}END_READ_NODE;

	trigger(PHP_SSP_STOP);

	detach_node();

	exit(0);
}

int server_stop(){
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

int server_status(){
	FILE *fp;
	int pid,i=17,cols=tput_cols();

	printf("SSP server status");
	strnprint(".",cols-i-9);
	flush();

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
