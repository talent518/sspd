#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

queue_t *queue_init() {
	queue_t *queue;

	queue = (queue_t *) malloc(sizeof(queue_t));

	queue->head = queue->tail = NULL;

	pthread_mutex_init(&queue->lock, NULL);

	return queue;
}

void queue_push(queue_t *queue, void *data) {
	pthread_mutex_lock(&queue->lock);

	queue_item_t *ptr = (queue_item_t *) malloc(sizeof(queue_item_t));
	ptr->next = NULL;
	ptr->data = data;

	if (queue->tail) {
		queue->tail->next = ptr;
		queue->tail = ptr;
	} else {
		queue->head = queue->tail = ptr;
	}

	pthread_mutex_unlock(&queue->lock);
}

void *queue_pop(queue_t *queue) {
	void *data = NULL;

	pthread_mutex_lock(&queue->lock);

	queue_item_t *ptr = queue->head;

	if (ptr) {
		if (ptr->next) {
			queue->head = ptr->next;
		} else {
			queue->head = queue->tail = NULL;
		}
		data = ptr->data;
		free(ptr);
	}

	pthread_mutex_unlock(&queue->lock);

	return data;
}

void queue_clean(queue_t *queue, void *data) {
	return queue_clean_ex(queue, data, NULL);
}

void queue_clean_ex(queue_t *queue, void *data, queue_cmp_t cmp) {
	pthread_mutex_lock(&queue->lock);

	queue_item_t *ptr = queue->head;
	queue_item_t *tmp, *prev = NULL;

	while(ptr) {
		if((cmp == NULL && ptr->data == data) || (cmp && cmp(ptr->data, data))) {
			tmp = ptr->next;
			free(ptr);

			if(prev) {
				prev->next = tmp;
			} else {
				queue->head = tmp;
			}

			if(tmp == NULL) {
				queue->tail = prev;
			}

			ptr = tmp;
		} else {
			prev = ptr;
			ptr = ptr->next;
		}
	}

	pthread_mutex_unlock(&queue->lock);
}

void queue_free(queue_t *queue) {
	queue_item_t *ptr = queue->head, *tmp;
	while (ptr) {
		tmp = ptr;
		ptr = ptr->next;
		free(tmp);
	}
	pthread_mutex_destroy(&queue->lock);
	free(queue);
}
