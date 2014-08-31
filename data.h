#ifndef _NODE_H
#define _NODE_H

#include <stdbool.h>
#include <inttypes.h>
#include <pthread.h>

#include <glib.h>
#include <event.h>

#include "event.h"

typedef struct _conn_t{
	int index;
	
	int sockfd;
	char host[15];
	int port;

	event_thread_t *thread;

	struct event event;
} conn_t;

typedef struct _index_queue_t
{
	unsigned int index;

	struct _index_queue_t *prev;
	struct _index_queue_t *next;
} index_queue_t;

extern index_queue_t *indexQueueHead;

extern GHashTable *fconns;
extern GHashTable *iconns;
extern GHashTable *pconns;

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
conn_t* sockfd_conn(int sockfd);
conn_t* port_conn(int port);

//接点写
void attach_conn();
void insert_conn(conn_t *ptr);
void remove_conn(conn_t *ptr);
void clean_conn(conn_t *ptr);
void detach_conn();

#endif