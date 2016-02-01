#ifndef HAVE_QUEUE_H
#define HAVE_QUEUE_H

#include <stdbool.h>
#include <pthread.h>

typedef struct _queue_item_t {
	void *data;
	struct _queue_item_t *next;
} queue_item_t;

typedef struct _queue_t {
	queue_item_t *head;
	queue_item_t *tail;

	pthread_mutex_t lock;
} queue_t;

queue_t *queue_init();
void queue_push(queue_t *queue, void *data);
void *queue_pop(queue_t *queue);
bool queue_free(queue_t *queue);

#endif
