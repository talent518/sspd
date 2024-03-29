#ifndef _EVENT_H
#define _EVENT_H

#include <pthread.h>
#include <TSRM.h>
#include <zend_types.h>
#include <stdbool.h>
#include <event.h>
#include "queue.h"
#include "config.h"

typedef struct {
	pthread_t tid;
	struct event_base *base;
	struct event notify_ev;
	struct event listen_ev;
	struct event signal_int;
#ifdef SSP_CODE_TIMEOUT
	struct event timeout_int;
#ifdef SSP_CODE_TIMEOUT_GLOBAL
	struct event timeout_global_int;
#endif
#endif
	struct event bench_int;

	int nthreads;

	int sockfd;

	short ev_flags;
	int read_fd;
	int write_fd;
} listen_thread_t;

typedef struct {
	pthread_t tid;
	struct event_base *base;
	struct event event;

	queue_t *accept_queue;
	queue_t *write_queue;
	queue_t *close_queue;

	short id;
	int read_fd;
	int write_fd;

	int conn_num;
	int clean_times;
} worker_thread_t;

extern listen_thread_t listen_thread;
extern worker_thread_t *worker_threads;

void is_accept_conn(bool do_accept);
void loop_event(int sockfd);
void worker_create(void *(*func)(void *), void *arg);

#include "top.h"
typedef struct {
	cpu_t cpu;
	mem_t mem;
	process_t proc;

	unsigned long int all;

	struct {
		double user;
		double nice;
		double system;
		double idle;
		double iowait;
		double irq;
		double softirq;
		double stolen;
		double guest;
	} scpu; // system cpu persent

	struct {
		long int utime;
		long int stime;
	} pcpu; // process cpu persent
} top_info_t;
extern top_info_t top_info;
void set_monitor_zval(zval *scpu, zval *pcpu, zval *smem, zval *pmem, zval *args);

#endif
