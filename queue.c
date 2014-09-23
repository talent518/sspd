#include <stdlib.h>
#include "queue.h"

queue_t *queue_init() {
	queue_t *queue;

	queue=(queue_t *) malloc(sizeof(queue_t));

	queue->head=queue->tail=NULL;

	pthread_mutex_init(&queue->lock, NULL);

	return queue;
}

void queue_push(queue_t *queue, void *data) {
	pthread_mutex_lock(&queue->lock);

	queue_item_t *ptr = (queue_item_t *)malloc(sizeof(queue_item_t));
	ptr->next=NULL;
	ptr->data=data;

	if(queue->tail) {
		queue->tail->next=ptr;
		queue->tail=ptr;
	} else {
		queue->head=queue->tail=ptr;
	}

	pthread_mutex_unlock(&queue->lock);
}

void *queue_pop(queue_t *queue) {
	void *data=NULL;

	pthread_mutex_lock(&queue->lock);

	queue_item_t *ptr=queue->head;

	if(ptr) {
		if(ptr->next) {
			queue->head=ptr->next;
		} else {
			queue->head=queue->tail=NULL;
		}
		data=ptr->data;
		free(ptr);
	}

	pthread_mutex_unlock(&queue->lock);

	return data;
}

bool queue_free(queue_t *queue) {
	queue_item_t *ptr=queue->head,*tmp;
	while(ptr) {
		tmp=ptr;
		ptr=ptr->next;
		free(tmp);
	}
	pthread_mutex_destroy(&queue->lock);
	free(queue);
}
