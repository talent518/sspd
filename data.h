#ifndef _NODE_H
#define _NODE_H

#include <stdbool.h>
#include <inttypes.h>
#include <pthread.h>

#include <event.h>

#include "ssp_event.h"
#include "queue.h"

typedef struct _conn_t {
	int index;

	int sockfd;
	char host[16];
	int port;

	bool refable;
	int ref_count;

	char *rbuf;
	int rbytes;
	int rsize;

#if ASYNC_SEND
	char *wbuf;
	int wbytes;
	int wsize;
	short evflags;
#endif // ASYNC_SEND

	worker_thread_t *thread;

	pthread_mutex_t lock;
	pthread_cond_t cond;

	struct event event;

	int type;
	int requests;
	char username[64];
	char sendKey[256];
	char receiveKey[256];
} conn_t;

#define BEGIN_READ_LOCK		begin_read_lock();
#define END_READ_LOCK		end_read_lock();
#define BEGIN_WRITE_LOCK	begin_write_lock();
#define END_WRITE_LOCK		end_write_lock();

#define CONN_NUM _conn_num()

//读写锁
void begin_read_lock();
void end_read_lock();
void begin_write_lock();
void end_write_lock();

//连接数
unsigned int _conn_num();

//接点读
conn_t* index_conn(int index);
#ifdef SSP_DATA_SOCKFD_PORT
conn_t* sockfd_conn(int sockfd);
conn_t* port_conn(int port);
#else
#define sockfd_conn(a) (NULL)
#define port_conn(a) (NULL)
#endif

void ref_conn(conn_t *ptr);
void unref_conn(conn_t *ptr);

//接点写
void attach_conn();
conn_t *insert_conn();
void remove_conn(conn_t *ptr);
void clean_conn(conn_t *ptr);
void detach_conn();

#endif
