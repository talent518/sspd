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

#include "ssp.h"
#include "socket.h"

int recv_data_len(node *ptr){
	unsigned char buf[4];
	int len=0,i,ret;

	ret=recv(ptr->sockfd,buf,sizeof(buf), MSG_WAITALL);

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
