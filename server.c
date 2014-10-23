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

#include "config.h"
#include "php_ext.h"
#include "php_func.h"
#include "ssp.h"
#include "data.h"
#include "api.h"
#include "event.h"

#define flush() fflush(stdout)

unsigned int ssp_backlog=1024;

char *ssp_host="0.0.0.0";
short int ssp_port=8083;
char *ssp_pidfile="/var/run/ssp.pid";

char *ssp_user="daemon";

int ssp_maxclients=1000;
int ssp_maxrecvs=2*1024*1024;

int server_start()
{
	struct sockaddr_in sin;
	int listen_fd;
	int ret;

	int pid,i=19,cols=tput_cols();

	printf("Starting SSP server");
	strnprint(".",cols-i-9);
	flush();

	listen_fd=socket(AF_INET,SOCK_STREAM,0);
	if (listen_fd<0)
	{
		system("echo -e \"\\E[31m\".[Failed]");
		system("tput sgr0");
		printf("Not on the host %s bind port %d\n",ssp_host,ssp_port);
		return 0;
	}
	int opt=1;
	setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(int));
    setsockopt(listen_fd, SOL_SOCKET, SO_KEEPALIVE,&opt, sizeof(int));

	int send_timeout=1000,recv_timeout=1000;
	setsockopt(listen_fd,SOL_SOCKET,SO_SNDTIMEO,&send_timeout,sizeof(int));//发送超时
	setsockopt(listen_fd,SOL_SOCKET,SO_RCVTIMEO,&recv_timeout,sizeof(int));//接收超时

	typedef struct
	{
		u_short l_onoff;
		u_short l_linger;
	} linger;
	linger m_sLinger;
	m_sLinger.l_onoff=1;//(在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
	// 如果m_sLinger.l_onoff=0;则功能和2.)作用相同;
	m_sLinger.l_linger=5;//(容许逗留的时间为5秒)
	setsockopt(listen_fd,SOL_SOCKET,SO_LINGER,&m_sLinger,sizeof(linger));

	int send_buffer=0,recv_buffer=0;
	setsockopt(listen_fd,SOL_SOCKET,SO_SNDBUF,&send_buffer,sizeof(int));//发送缓冲区大小
	setsockopt(listen_fd,SOL_SOCKET,SO_RCVBUF,&recv_buffer,sizeof(int));//接收缓冲区大小

	bzero(&sin,sizeof(sin));
	sin.sin_family=AF_INET;
	sin.sin_addr.s_addr=inet_addr(ssp_host);
	sin.sin_port=htons(ssp_port);

	ret=bind(listen_fd,(struct sockaddr *)&sin,sizeof(sin));
	if (ret<0)
	{
		system("echo -e \"\\E[31m\".[Failed]");
		system("tput sgr0");
		printf("Not on the host %s bind port %d\n",ssp_host,ssp_port);
		return 0;
	}

	ret=listen(listen_fd, ssp_backlog);
	if (ret<0)
	{
		system("echo -e \"\\E[31m\".[Failed]");
		system("tput sgr0");
		return 0;
	}

	pid=fork();

	if (pid==-1)
	{
		system("echo -e \"\\E[31m\".[Failed]");
		system("tput sgr0");
		printf("fork failure!\n");
		return 0;
	}
	if (pid>0)
	{
		FILE *fp;
		fp=fopen(ssp_pidfile,"w+");
		if (fp==NULL)
		{
			printf("file '%s' open fail.\n",ssp_pidfile);
		}
		else
		{
			fprintf(fp,"%d",pid);
			fclose(fp);
		}
		sleep(1);
		return 1;
	}

	struct passwd *pwnam;
	pwnam = getpwnam(ssp_user);

	setuid(pwnam->pw_uid);
	setgid(pwnam->pw_gid);

	ret=setsid();
	if (ret<1)
	{
		system("echo -e \"\\E[31m\".[Failed]");
		system("tput sgr0");
	}
	else
	{
		system("echo -e \"\\E[32m\"[Succeed]");
		system("tput sgr0");
	}

	attach_conn();

	THREAD_STARTUP();

	trigger(PHP_SSP_START);

	loop_event(listen_fd);

	trigger(PHP_SSP_STOP);

	THREAD_SHUTDOWN();

	shutdown(listen_fd, 2);
	close(listen_fd);

	detach_conn();

	exit(0);
}

int server_stop()
{
	FILE *fp;
	int pid,i=19,cols=tput_cols();

	printf("Stopping SSP server");
	flush();

	fp=fopen(ssp_pidfile,"r+");
	if (fp!=NULL)
	{
		fscanf(fp,"%d",&pid);
		fclose(fp);
		unlink(ssp_pidfile);
		if (pid==getsid(pid))
		{
			kill(pid,SIGINT);
			while (pid==getsid(pid))
			{
				printf(".");
				flush();
				i++;
				sleep(1);
				if (cols-i-9==0)
				{
					kill(pid,SIGKILL);
					break;
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

int server_status()
{
	FILE *fp;
	int pid,i=17,cols=tput_cols();

	printf("SSP server status");
	strnprint(".",cols-i-9);
	flush();

	fp=fopen(ssp_pidfile,"r+");
	if (fp!=NULL)
	{
		fscanf(fp,"%d",&pid);
		fclose(fp);
		if (pid==getsid(pid))
		{
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
