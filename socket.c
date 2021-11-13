#include <sys/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include "ssp.h"
#include "socket.h"
#include "ssp_event.h"

int recv_data_len(conn_t *ptr) {
	unsigned char buf[4];
	register int len = 0, i, ret;

	ret = recv(ptr->sockfd, buf, sizeof(buf), MSG_WAITALL);

	if (ret <= 0) return 0;

	if (ret != sizeof(buf)) {
		conn_info_ex(ptr, "[ The data packet is not complete ] ");
		return 0;
	}

	if (buf[0]) {
		conn_info_ex(ptr, "[ The data packet head is not legitimate ] ");
		return 0;
	}

	for (i = 0; i < 4; i++) {
		len += (buf[i] & 0xff) << ((3 - i) * 8);
	}

	return len;
}

//接收来自客户端数据
//返回值:0(关闭连接),-1(接收到的数据长度与数据包长度不一致),>0(接收成功)
int socket_recv(conn_t *ptr, char **data, int *data_len) {
	register int ret;
	if (ptr->rbuf == NULL) {
		ret = recv_data_len(ptr);
		if (ret > 0) {
			if (ret > ssp_maxrecvs) {
				conn_info_ex(ptr, "[ The received data beyond the limit ] ");
				return 0;
			}
			ptr->rbuf = (char*) malloc(ret + 1);
			ptr->rbuf[ret] = '\0';
			ptr->rsize = ret;
			ptr->rbytes = 0;

			return -1;
		} else {
			return ret;
		}
	} else {
		ret = recv(ptr->sockfd, ptr->rbuf + ptr->rbytes, ptr->rsize - ptr->rbytes, MSG_DONTWAIT);

		if (ret <= 0) return 0;

		ptr->rbytes += ret;

		if (ptr->rbytes != ptr->rsize) return -1;

		if (*data) {
			free(*data);
		}

		*data = ptr->rbuf;
		*data_len = ptr->rsize;

		ptr->rbuf = NULL;
		ptr->rsize = ptr->rbytes = 0;
	
		return 1;
	}
}

#if ASYNC_SEND
void socket_send_buf(conn_t *ptr, char *package, int plen);
#endif

int socket_send(conn_t *ptr, char *data, int data_len) {
	register int i, plen;
	char *package;

	if (data_len <= 0) {
		return -1;
	}

	plen = 4 + data_len;
	package = (char*) malloc(plen);

	for (i = 0; i < 4; i++) {
		package[i] = data_len >> ((3 - i) * 8);
	}

	memcpy(package + 4, data, data_len); //数据包内容

#if ASYNC_SEND
	if(!ptr->thread) {
		i = send(ptr->sockfd, package, plen, MSG_WAITALL);
		free(package);
		return i;
	} else if(ptr->thread->tid == pthread_self()) {
		socket_send_buf(ptr, package, plen);
		return plen;
	}
	send_t *s = (send_t*)malloc(sizeof(send_t));
	s->ptr = ptr;
	s->str = package;
	s->len = plen;

	queue_push(ptr->thread->write_queue, s);

	write(ptr->thread->write_fd, "w", 1);

	return plen;
#else
	i = send(ptr->sockfd, package, plen, MSG_WAITALL);
	free(package);
	return i;
#endif // ASYNC_SEND
}

void socket_close(conn_t *ptr) {
	queue_push(ptr->thread->close_queue, ptr);

	write(ptr->thread->write_fd, "x", 1);
}

void socket_set_listen(int s) {
	int opt = 1;
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
}

void socket_set_accept(int s, int send_timeout, int recv_timeout, int send_buffer, int recv_buffer) {
	setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &send_timeout, sizeof(int)); // 发送超时
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &recv_timeout, sizeof(int)); // 接收超时

	setsockopt(s, SOL_SOCKET, SO_SNDBUF, &send_buffer, sizeof(int));//发送缓冲区大小
	setsockopt(s, SOL_SOCKET, SO_RCVBUF, &recv_buffer, sizeof(int));//接收缓冲区大小

	typedef struct {
		u_short l_onoff;
		u_short l_linger;
	} linger;
	linger m_sLinger;
	m_sLinger.l_onoff = 0;//(在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
	// 如果m_sLinger.l_onoff=0;则功能和2.)作用相同;
	m_sLinger.l_linger = 0;//(容许逗留的时间为5秒)
	setsockopt(s, SOL_SOCKET, SO_LINGER, &m_sLinger, sizeof(linger));
}

void _socket_setnonblock(int fd) {
	int opt = fcntl(fd, F_GETFL);
	opt |= O_NONBLOCK;
	fcntl(fd, F_SETFL, opt);
}

void _socket_setblocking(int fd) {
	int opt = fcntl(fd, F_GETFL);
	opt ^= O_NONBLOCK;
	fcntl(fd, F_SETFL, opt);
}

