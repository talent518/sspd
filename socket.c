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
#include <sys/types.h>
#include <assert.h>

#include "ssp.h"
#include "socket.h"
#include "event.h"

int recv_data_len(conn_t *ptr)
{
	unsigned char buf[4];
	int len=0,i,ret;

	ret=recv(ptr->sockfd,buf,sizeof(buf), MSG_WAITALL);

	if(ret<0) {
		return -1;
	} else if(ret==0) {
		return 0;
	}

	if (ret!=sizeof(buf))
	{
		conn_info_ex(ptr, "[ The data packet is not complete ] ");
		return 0;
	}

	if (buf[0])
	{
		conn_info_ex(ptr, "[ The data packet head is not legitimate ] ");
		return 0;
	}

	for (i=0;i<4;i++)
	{
		len+=(buf[i]&0xff)<<((3-i)*8);
	}

	return len;
}

//接收来自客户端数据
//返回值:0(关闭连接),-1(接收到的数据长度与数据包长度不一致),>0(接收成功)
int socket_recv(conn_t *ptr,char **data,int *data_len)
{
	int ret;
	if(ptr->rbuf==NULL) {
		ret=recv_data_len(ptr);
		if (ret>0)
		{
			if (ret>ssp_maxrecvs)
			{
				conn_info_ex(ptr, "[ The received data beyond the limit ] ");
				return 0;
			}
			ptr->rbuf=(char*)malloc(ret+1);
			bzero(ptr->rbuf,ret+1);
			ptr->rsize=ret;
			ptr->rbytes=0;
		}
		else
		{
			return ret;
		}
	}
	
	ret=recv(ptr->sockfd,ptr->rbuf+ptr->rbytes,ptr->rsize-ptr->rbytes,MSG_DONTWAIT);

	if(ret<0) {
		return -1;
	} else if(ret==0) {
		return 0;
	}

	ptr->rbytes+=ret;

	if (ptr->rsize>0 && ptr->rbytes==ptr->rsize)
	{
		if (*data)
		{
			free(*data);
		}

		*data=ptr->rbuf;
		*data_len=ptr->rsize;

		ptr->rbuf=NULL;
		ptr->rsize=ptr->rbytes=0;

		return 1;
	}
	return -1;
}

int socket_send(conn_t *ptr,const char *data,int data_len)
{
	int i,ret,plen;
	char *package;

	if (data_len<=0)
	{
		return -1;
	}

	plen=4+data_len;
	package=(char*)malloc(plen);

	for (i=0;i<4;i++)
	{
		package[i]=data_len>>((3-i)*8);
	}

	memcpy(package+4,data,data_len);//数据包内容

	ret=send(ptr->sockfd,package,plen,MSG_WAITALL);
	free(package);
	
	if (ret>0 && ret!=plen)
	{
		conn_info_ex(ptr, "[ Failed sending data ] ");
	}

	return ret;
}

void socket_close(conn_t *ptr)
{
	BEGIN_RUNTIME();
	char buf[1];
	buf[0] = '\0';

	BEGIN_READ_LOCK
	{
		if (ptr->refable)
		{
			ptr->refable=false;

			conn_info(ptr);

			queue_push(ptr->thread->close_queue, ptr);

			buf[0] = 'x';
		}
	} END_READ_LOCK;

	if(buf[0]) {
		write(ptr->thread->write_fd, buf, 1);
	}
}
