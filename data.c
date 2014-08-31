#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>

#include "data.h"
#include "ssp.h"

index_queue_t *indexQueueHead=NULL;

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
	assert(indexQueueHead==NULL);

	indexQueueHead=(index_queue_t *)malloc(sizeof(index_queue_t));
	indexQueueHead->index=0;
	indexQueueHead->prev=indexQueueHead->next=indexQueueHead;

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
	} END_READ_LOCK;

	dprintf("%s: index(%d), sockfd(%d), host(%s), port(%d)\n\n", __func__, ptr->index, ptr->sockfd, ptr->host, ptr->port);

	return ptr;
}

conn_t *sockfd_conn(int sockfd){
	conn_t *ptr;

	BEGIN_READ_LOCK {
		ptr=g_hash_table_lookup(fconns, &sockfd);
	} END_READ_LOCK;

	return ptr;
}

conn_t *port_conn(int port){
	conn_t *ptr=NULL;

	BEGIN_READ_LOCK {
		ptr=g_hash_table_lookup(pconns, &port);
	} END_READ_LOCK;

	return ptr;
}

unsigned int _conn_num(){
	unsigned int ret;
	BEGIN_READ_LOCK {
		ret=conn_num;
	} END_READ_LOCK;
	return ret;
}

void insert_conn(conn_t *ptr){
	assert(indexQueueHead);
	BEGIN_WRITE_LOCK {
		conn_num++;

		if(indexQueueHead->next!=indexQueueHead) {
			index_queue_t *iqueue=indexQueueHead->prev;
			index_queue_t *p=iqueue->prev,*n=iqueue->next;

			p->next=n;
			n->prev=p;

			ptr->index=iqueue->index;

			free(iqueue);
		} else {
			ptr->index=conn_num;
		}

		g_hash_table_insert(iconns, &ptr->index, ptr);
		g_hash_table_insert(fconns, &ptr->sockfd, ptr);
		g_hash_table_insert(pconns, &ptr->port, ptr);

	} END_WRITE_LOCK;
}

void remove_conn(conn_t *ptr){
	assert(indexQueueHead);

	BEGIN_WRITE_LOCK {
		conn_num--;

		g_hash_table_remove(iconns, &ptr->index);
		g_hash_table_remove(fconns, &ptr->sockfd);
		g_hash_table_remove(pconns, &ptr->port);

		index_queue_t *hnext=indexQueueHead->next;
		index_queue_t *iqueue=(index_queue_t *)malloc(sizeof(index_queue_t *));

		iqueue->index = ptr->index;

		iqueue->next=hnext;
		hnext->prev=iqueue;

		indexQueueHead->next=iqueue;
		iqueue->prev=indexQueueHead;

		free(ptr);
	} END_WRITE_LOCK;
}

void clean_conn(conn_t *ptr){
	shutdown(ptr->sockfd,2);
	close(ptr->sockfd);
	if(ptr->event.ev_base) {
		event_del(&ptr->event);
	}
}

void detach_conn(){
	assert(indexQueueHead);
	BEGIN_WRITE_LOCK {
		index_queue_t *p=indexQueueHead;
		indexQueueHead->prev->next=NULL;
		while(p->next!=NULL){
			p=p->next;
			free(p->prev);
		}
		free(p);
		indexQueueHead=NULL;

		g_hash_table_destroy(iconns);
		g_hash_table_destroy(fconns);
		g_hash_table_destroy(pconns);
	} END_WRITE_LOCK;

	pthread_mutex_destroy(&mx_reader);
	pthread_mutex_destroy(&mx_writer);
}
