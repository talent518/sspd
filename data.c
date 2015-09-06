#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>

#include "data.h"
#include "ssp.h"

queue_t *iqueue=NULL;

GHashTable *fconns; // sockfd table
GHashTable *iconns; // index table
GHashTable *pconns; // port table

static unsigned int readers=0;
static unsigned int conn_num=0;

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
	assert(iqueue==NULL);

	iqueue=queue_init();

	pthread_mutex_init(&mx_reader, NULL);
	pthread_mutex_init(&mx_writer, NULL);

	fconns=g_hash_table_new(g_int_hash, g_int_equal);
	iconns=g_hash_table_new(g_int_hash, g_int_equal);
	pconns=g_hash_table_new(g_int_hash, g_int_equal);
}

conn_t *index_conn(int index){
	conn_t *ptr=NULL;

	BEGIN_READ_LOCK {
		ptr=g_hash_table_lookup(iconns, &index);

		if(ptr && ptr->refable) {
			ref_conn(ptr);
		} else {
			ptr=NULL;
		}
	} END_READ_LOCK;

	return ptr;
}

conn_t *sockfd_conn(int sockfd){
	conn_t *ptr=NULL;

	BEGIN_READ_LOCK {
		ptr=g_hash_table_lookup(fconns, &sockfd);

		if(ptr && ptr->refable) {
			ref_conn(ptr);
		} else {
			ptr=NULL;
		}
	} END_READ_LOCK;

	return ptr;
}

conn_t *port_conn(int port){
	conn_t *ptr=NULL;

	BEGIN_READ_LOCK {
		ptr=g_hash_table_lookup(pconns, &port);

		if(ptr && ptr->refable) {
			ref_conn(ptr);
		} else {
			ptr=NULL;
		}
	} END_READ_LOCK;

	return ptr;
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

void clean_conn(conn_t *ptr){
	if(ptr->event.ev_base) {
		event_del(&ptr->event);
	}

	shutdown(ptr->sockfd,SHUT_RDWR);
	close(ptr->sockfd);
	if(ptr->rbuf) {
		free(ptr->rbuf);
		ptr->rbuf = NULL;
	}
	ptr->rbytes = 0;
	ptr->rsize = 0;
}

unsigned int _conn_num(){
	unsigned int ret;
	BEGIN_READ_LOCK {
		ret=conn_num;
	} END_READ_LOCK;
	return ret;
}

void insert_conn(conn_t *ptr){
	assert(iqueue);
	BEGIN_WRITE_LOCK {
		conn_num++;

		int *index=(int*)queue_pop(iqueue);

		if(index){
			ptr->index=*index;
			free(index);
		} else {
			ptr->index = conn_num;
		}

		g_hash_table_insert(iconns, &ptr->index, ptr);
		g_hash_table_insert(fconns, &ptr->sockfd, ptr);
		g_hash_table_insert(pconns, &ptr->port, ptr);

		pthread_mutex_init(&ptr->lock, NULL);
		pthread_cond_init(&ptr->cond, NULL);

		ptr->refable=true;

	} END_WRITE_LOCK;
}

void remove_conn(conn_t *ptr){
	assert(iqueue);

	BEGIN_WRITE_LOCK {
		conn_num--;

		g_hash_table_remove(iconns, &ptr->index);
		g_hash_table_remove(fconns, &ptr->sockfd);
		g_hash_table_remove(pconns, &ptr->port);

		int *index=(int *)malloc(sizeof(int));
		*index=ptr->index;

		queue_push(iqueue,(void *)index);

		pthread_mutex_lock(&ptr->lock);
		while (ptr->ref_count > 0) {
			pthread_cond_wait(&ptr->cond, &ptr->lock);
		}
		pthread_mutex_unlock(&ptr->lock);

		pthread_mutex_destroy(&ptr->lock);
		pthread_cond_destroy(&ptr->cond);

		if(ptr->rbuf) {
			free(ptr->rbuf);
		}

		free(ptr);
	} END_WRITE_LOCK;
}

void detach_conn(){
	assert(iqueue);
	BEGIN_WRITE_LOCK {
		queue_free(iqueue);
		iqueue=NULL;

		g_hash_table_destroy(iconns);
		g_hash_table_destroy(fconns);
		g_hash_table_destroy(pconns);
	} END_WRITE_LOCK;

	pthread_mutex_destroy(&mx_reader);
	pthread_mutex_destroy(&mx_writer);
}
