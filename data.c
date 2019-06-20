#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>

#include "data.h"
#include "ssp.h"
#include "socket.h"

static int *indexs = NULL;
volatile int iindex = -1;
static conn_t *iconns = NULL;
volatile unsigned int readers=0;
volatile unsigned int conn_num=0;

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
	assert(indexs==NULL);
	assert(iconns==NULL);

	pthread_mutex_init(&mx_reader, NULL);
	pthread_mutex_init(&mx_writer, NULL);

	indexs = (int*)malloc(sizeof(int) * ssp_maxclients);
	memset(indexs, 0, sizeof(int) * ssp_maxclients);

	iconns=(conn_t*)malloc(sizeof(conn_t) * ssp_maxclients);
	memset(iconns, 0, sizeof(conn_t) * ssp_maxclients);
}

conn_t *index_conn(int i){
	conn_t *ptr=NULL;

	if(i < 0 || i >= ssp_maxclients) return NULL;

	BEGIN_READ_LOCK {
		ptr=&iconns[i];

		if(ptr && ptr->refable) {
			ref_conn(ptr);
		} else {
			ptr=NULL;
		}
	} END_READ_LOCK;

	return ptr;
}

conn_t *index_conn_signal(int i) {
	return &iconns[i];
}

void ref_conn(conn_t *ptr) {
    pthread_mutex_lock(&ptr->lock);
    ptr->ref_count++;
    pthread_cond_signal(&ptr->cond);
    pthread_mutex_unlock(&ptr->lock);
}

void unref_conn(conn_t *ptr) {
	pthread_mutex_lock(&ptr->lock);
	ptr->ref_count--;
	pthread_cond_signal(&ptr->cond);
	pthread_mutex_unlock(&ptr->lock);
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

void clean_conn(conn_t *ptr) {
	if(ptr->event.ev_base) event_del(&ptr->event);

	shutdown(ptr->sockfd, SHUT_RDWR);
	close(ptr->sockfd);

	if(ptr->thread) {
		queue_clean_ex(ptr->thread->write_queue, ptr, (queue_cmp_t) send_cmp);
		queue_clean(ptr->thread->close_queue, ptr);
	}

	if(ptr->rbuf) {
		free(ptr->rbuf);
		ptr->rbuf = NULL;
	}
	ptr->rbytes = 0;
	ptr->rsize = 0;

	ptr->refable = false;

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

			pthread_mutex_init(&ptr->lock, NULL);
			pthread_cond_init(&ptr->cond, NULL);

			ptr->refable = true;
		}
	} END_WRITE_LOCK;

	return ptr;
}

void remove_conn(conn_t *ptr){
	assert(indexs);
	assert(iconns);

	pthread_mutex_lock(&ptr->lock);
	while (ptr->ref_count > 0) {
		pthread_cond_wait(&ptr->cond, &ptr->lock);
	}
	pthread_mutex_unlock(&ptr->lock);

	BEGIN_WRITE_LOCK {
		conn_num--;

		indexs[++iindex] = ptr->index;

		pthread_mutex_destroy(&ptr->lock);
		pthread_cond_destroy(&ptr->cond);

		memset(ptr, 0, sizeof(conn_t));
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

	pthread_mutex_destroy(&mx_reader);
	pthread_mutex_destroy(&mx_writer);
}
