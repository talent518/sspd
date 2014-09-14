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

int recv_data_len(conn_t *ptr){
	unsigned char buf[4];
	int len=0,i,ret;

	ret=recv(ptr->sockfd,buf,sizeof(buf), MSG_WAITALL);

	if(ret!=sizeof(buf)){
		conn_info_ex(ptr, "[ Data packet is legitimate or has closed the connection ] ");
		return ret>0?-1:0;
	}

	if(buf[0]){
		conn_info_ex(ptr, "[ Data packet is legitimate or has closed the connection ] ");
		return 0;
	}

	for(i=0;i<4;i++){
		len+=(buf[i]&0xff)<<((3-i)*8);
	}
	return len;
}

//接收来自客户端数据
//返回值:0(关闭连接),-1(接收到的数据长度与数据包长度不一致),>0(接收成功)
int socket_recv(conn_t *ptr,char **data,int *data_len){
	int ret=recv_data_len(ptr);
	if(ret>0){
		*data_len=ret;
		if(*data_len>ssp_maxrecvs){
			conn_info_ex(ptr, "[ The received data beyond the limit ] ");
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
			conn_info_ex(ptr,"[ Data packets are not complete ] ");
			return -1;
		}else{
			conn_info_ex(ptr,"[ Has closed the connection ] ");
			return 0;
		}
	}else{
		*(*data+(*data_len))=0;//把最后一个字符设置为\0
	}
	return 1;
}

int socket_send(conn_t *ptr,const char *data,int data_len){
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
		conn_info_ex(ptr, "[ Failed sending data ] ");
	}
	return ret;
}

void socket_close(conn_t *ptr) {
	BEGIN_READ_LOCK {
		if(ptr->refable) {
			ptr->refable=false;

			//shutdown(ptr->sockfd,SHUT_RD);

			conn_info(ptr);

			queue_push(ptr->thread->close_queue, ptr);

			char buf[1];
			buf[0] = 'x';
			write(ptr->thread->write_fd, buf, 1);
		}
	} END_READ_LOCK;
}
