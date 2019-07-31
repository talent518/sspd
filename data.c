#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>

#include "data.h"
#include "ssp.h"
#include "socket.h"

typedef struct _conn_refcount_t {
	int ref_count;

	pthread_mutex_t lock;
	pthread_cond_t cond;
} conn_refcount_t;

static int *indexs = NULL;
volatile int iindex = -1;
static conn_t *iconns = NULL;
volatile unsigned int readers=0;
volatile unsigned int conn_num=0;
static conn_refcount_t *refcounts = NULL;

pthread_mutex_t mx_reader,mx_writer;

void begin_read_lock(){
	pthread_mutex_lock(&mx_reader);
	if ((++(readers)) == 1) {
		pthread_mutex_lock(&mx_writer);
	}
	pthread_mutex_unlock(&mx_reader);
}
void end_read_lock(){
	pthread_mutex_lock(&mx_reader);
	if ((--(readers)) == 0) {
		pthread_mutex_unlock(&mx_writer);
	}
	pthread_mutex_unlock(&mx_reader);
}
void begin_write_lock(){
	pthread_mutex_lock(&mx_writer);
}
void end_write_lock(){
	pthread_mutex_unlock(&mx_writer);
}

void attach_conn(){
	assert(indexs == NULL);
	assert(iconns == NULL);
	assert(refcounts == NULL);

	pthread_mutex_init(&mx_reader, NULL);
	pthread_mutex_init(&mx_writer, NULL);

	indexs = (int*)malloc(sizeof(int) * ssp_maxclients);
	memset(indexs, 0, sizeof(int) * ssp_maxclients);

	iconns = (conn_t*)malloc(sizeof(conn_t) * ssp_maxclients);
	memset(iconns, 0, sizeof(conn_t) * ssp_maxclients);

	refcounts = (conn_refcount_t*)malloc(sizeof(conn_refcount_t) * ssp_maxclients);

	register int i;
	for(i=0; i<ssp_maxclients; i++) {
		iconns[i].sockfd = -1;
		refcounts[i].ref_count = 0;
		pthread_mutex_init(&refcounts[i].lock, NULL);
		pthread_cond_init(&refcounts[i].cond, NULL);
	}
}

conn_t *index_conn(int i){
	conn_t *ptr = NULL;

	if(i < 0 || i >= ssp_maxclients) return NULL;

	BEGIN_READ_LOCK {
		ptr = &iconns[i];

		if(ptr->index == i && ptr->refable) {
			ref_conn(ptr);
		} else {
			ptr = NULL;
		}
	} END_READ_LOCK;

	return ptr;
}

conn_t *index_conn_signal(int i) {
	return &iconns[i];
}

void ref_conn(conn_t *ptr) {
	assert(refcounts);

	int i = ptr->index;
	//printf("%s: %d lock\n", __func__, i);
	pthread_mutex_lock(&refcounts[i].lock);
	refcounts[i].ref_count++;
	pthread_cond_signal(&refcounts[i].cond);
	pthread_mutex_unlock(&refcounts[i].lock);
	//printf("%s: %d unlock\n", __func__, i);
}

void unref_conn(conn_t *ptr) {
	assert(refcounts);

	int i = ptr->index;
	//printf("%s: %d lock\n", __func__, i);
	pthread_mutex_lock(&refcounts[i].lock);
	assert(refcounts[i].ref_count > 0);
	refcounts[i].ref_count--;
	pthread_cond_signal(&refcounts[i].cond);
	pthread_mutex_unlock(&refcounts[i].lock);
	//printf("%s: %d unlock\n", __func__, i);
}

void clean_conn(conn_t *ptr) {
	ptr->refable = false;

	if(ptr->sockfd >= 0) {
		event_del(&ptr->event);

		shutdown(ptr->sockfd, SHUT_RDWR);
		close(ptr->sockfd);

		ptr->sockfd = -1;
	}

	if(ptr->rbuf) {
		free(ptr->rbuf);
		ptr->rbuf = NULL;
	}
	ptr->rbytes = 0;
	ptr->rsize = 0;

#if ASYNC_SEND
	if(ptr->wbuf) {
		free(ptr->wbuf);
		ptr->wbuf = NULL;
	}
	ptr->wbytes = 0;
	ptr->wsize = 0;
#endif // ASYNC_SEND
}

unsigned int _conn_num(){
	unsigned int ret;
	BEGIN_READ_LOCK {
		ret=conn_num;
	} END_READ_LOCK;
	return ret;
}

conn_t *insert_conn(){
	conn_t *ptr = NULL;
	assert(indexs);
	assert(iconns);

	BEGIN_WRITE_LOCK {
		if(conn_num < ssp_maxclients) {
			if(iindex >= 0){
				int i = indexs[iindex--];
				ptr = &iconns[i];
				ptr->index = i;
			} else {
				ptr = &iconns[conn_num];
				ptr->index = conn_num;
			}

			conn_num++;

			ptr->refable = true;
		}
	} END_WRITE_LOCK;

	return ptr;
}

static bool send_cmp(send_t *s, conn_t *ptr) {
	if(s->ptr == ptr) {
		free(s->str);
		free(s);
		return true;
	} else {
		return false;
	}
}

void remove_conn(conn_t *ptr){
	assert(indexs);
	assert(iconns);
	assert(refcounts);

	int i = ptr->index;

	//printf("lock: %d, %d\n", i, ptr->thread->id);
	pthread_mutex_lock(&refcounts[i].lock);
	while (refcounts[i].ref_count > 0) {
		pthread_cond_wait(&refcounts[i].cond, &refcounts[i].lock);
	}
	pthread_mutex_unlock(&refcounts[i].lock);
	//printf("unlock: %d, %d\n", i, ptr->thread->id);

	BEGIN_WRITE_LOCK {
		conn_num--;

		indexs[++iindex] = i;

		if(ptr->thread) {
			queue_clean_ex(ptr->thread->write_queue, ptr, (queue_cmp_t) send_cmp);
			queue_clean(ptr->thread->close_queue, ptr);
		}

		memset(ptr, 0, sizeof(conn_t));

		ptr->sockfd = -1;
	} END_WRITE_LOCK;
}

void detach_conn(){
	assert(indexs);
	assert(iconns);

	BEGIN_WRITE_LOCK {
		free(indexs);
		free(iconns);

		indexs = NULL;
		iconns = NULL;
	} END_WRITE_LOCK;

	register int i;
	for(i=0; i<ssp_maxclients; i++) {
		pthread_mutex_destroy(&refcounts[i].lock);
		pthread_cond_destroy(&refcounts[i].cond);
	}

	pthread_mutex_destroy(&mx_reader);
	pthread_mutex_destroy(&mx_writer);
}
