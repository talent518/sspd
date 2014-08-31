#ifndef _EVENT_H
#define _EVENT_H

#include <stdbool.h>
#include <event.h>

typedef struct _thread_queue_t
{
	void *value;

	struct _thread_queue_t *prev;
	struct _thread_queue_t *next;
} thread_queue_t;

typedef struct
{
	pthread_t tid;
	struct event_base *base;
	struct event event;
	pthread_mutex_t queue_lock;
	thread_queue_t *queue;
	short id;
	int read_fd;
	int write_fd;
} event_thread_t;

typedef struct
{
	int sockfd;
	pthread_t tid;
	struct event_base *base;
	struct event listen_ev;
	struct event signal_int;
	short  ev_flags;
} dispatcher_thread_t;

extern event_thread_t *threads;
extern dispatcher_thread_t dispatcher_thread;
extern int last_thread_id;

void is_accept_conn(bool do_accept);
void loop_event (int sockfd);

#endif
