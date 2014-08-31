#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "ssp.h"
#include "data.h"
#include "php_ext.h"
#include "php_func.h"
#include "event.h"

static int init_count = 0;
static pthread_mutex_t init_lock, conn_lock;
static pthread_cond_t init_cond, conn_cond;

int ssp_nthreads = 10;

event_thread_t *threads;
dispatcher_thread_t dispatcher_thread;
int last_thread_id = 0;

static void listen_handler(const int fd, const short which, void *arg);
static void update_accept_event(const int new_flags) {
	if(dispatcher_thread.ev_flags==new_flags) {
		return;
	}

    if (event_del(&dispatcher_thread.listen_ev) == -1) {
		return;
	}

	dispatcher_thread.ev_flags=new_flags;

	event_set(&dispatcher_thread.listen_ev, dispatcher_thread.sockfd, new_flags, listen_handler, NULL);
	event_base_set(dispatcher_thread.base, &dispatcher_thread.listen_ev);
    event_add(&dispatcher_thread.listen_ev, NULL);
}

void is_accept_conn(bool do_accept) {
	if (do_accept) {
		update_accept_event(EV_READ | EV_PERSIST);
		if (listen(dispatcher_thread.sockfd, ssp_backlog) != 0) {
			perror("listen");
		}
	}
	else {
		update_accept_event(0);
		if (listen(dispatcher_thread.sockfd, 0) != 0) {
			perror("listen");
		}
	}
}

static void *worker_thread(void *arg)
{
	event_thread_t *me = arg;
	me->tid = pthread_self();

	dprintf("thread %d createed\n", me->id);

    pthread_mutex_lock(&init_lock);
    init_count++;
    pthread_cond_signal(&init_cond);
    pthread_mutex_unlock(&init_lock);

	event_base_loop(me->base, 0);

	dprintf("thread %d exited\n", me->id);
    pthread_mutex_lock(&init_lock);
    init_count--;
    pthread_cond_signal(&init_cond);
    pthread_mutex_unlock(&init_lock);

	pthread_detach(me->tid);
	pthread_exit(NULL);

	return NULL;
}

void worker_create(void *(*func)(void *), void *arg)
{
	pthread_t       thread;
	pthread_attr_t  attr;
	int             ret;

	pthread_attr_init(&attr);

	if ((ret = pthread_create(&thread, &attr, func, arg)) != 0)
	{
		fprintf(stderr, "Can't create thread: %s\n", strerror(ret));
		exit(1);
	}
}

static void read_handler(int sock, short event,	void* arg)
{
	conn_t *ptr = (conn_t *) arg;

	dprintf("%s: index(%d), sockfd(%d), host(%s), port(%d)!\n", __func__, ptr->index, ptr->sockfd, ptr->host, ptr->port);

	int data_len=0,ret;
	char *data=NULL;

	ret=socket_recv(ptr,&data,&data_len);
	if(ret<=0){//关闭连接
		trigger(PHP_SSP_CLOSE,ptr);
		clean_conn(ptr);
		remove_conn(ptr);

		pthread_mutex_lock(&conn_lock);
		is_accept_conn(true);
		pthread_mutex_unlock(&conn_lock);
	}else{//接收数据成功
		trigger(PHP_SSP_RECEIVE,ptr,&data,&data_len);
		if(data_len>0){
			trigger(PHP_SSP_SEND,ptr,&data,&data_len);
			socket_send(ptr,data,data_len);
		}
	}
	free(data);
}

static void notify_handler(int fd, short which, void *arg)
{
	int ret;
	char buf[1];
	event_thread_t *me = arg;
	conn_t *ptr;

	if (fd != me->read_fd)
	{
		printf("notify_handler error : fd != me->read_fd\n");
		exit(1);
	}

	ret = read(fd, buf, 1);
	if (ret <= 0)
	{
		return;
	}

	dprintf("notify_handler: notify(%c) threadId(%d)\n", buf[0], me->id);

	// 处理连接关闭对列
	if(buf[0] == 'x') {
		pthread_mutex_lock(&me->queue_lock);
		{
			thread_queue_t *queue=me->queue,*p=queue->prev;

			while(p->value) {
				ptr=p->value;
				dprintf("notify_handler(socket_close): sockfd(%d), host(%s), port(%d)!\n", ptr->sockfd, ptr->host, ptr->port);
				trigger(PHP_SSP_CLOSE,ptr);
				clean_conn(ptr);
				remove_conn(ptr);

				p=p->prev;
				free(p->next);
			}

			queue->next=queue->prev=queue;
		}
		pthread_mutex_unlock(&me->queue_lock);

		pthread_mutex_lock(&conn_lock);
		is_accept_conn(true);
		pthread_mutex_lock(&conn_lock);
		return;
	} else if (buf[0] == '-') {
		event_base_loopbreak(me->base);
		return;
	}

	int conn_fd;
	bool is_deny = CONN_NUM >= ssp_maxclients;
	struct sockaddr_in pin;
	socklen_t len=sizeof(pin);
	conn_fd=accept(dispatcher_thread.sockfd,(struct sockaddr *)&pin,&len);

	if(is_deny) {
		is_accept_conn(false);
	}

    pthread_mutex_lock(&conn_lock);
	pthread_cond_signal(&conn_cond);
    pthread_mutex_unlock(&conn_lock);
	
	if(conn_fd<=0){
		return;
	}
	ptr=(conn_t *)malloc(sizeof(conn_t));

	bzero(ptr,sizeof(conn_t));

	ptr->sockfd=conn_fd;
	inet_ntop(AF_INET,&pin.sin_addr,ptr->host,sizeof(ptr->host));
	ptr->port=ntohs(pin.sin_port);

	ptr->thread=me;

	if(is_deny){
		dprintf("%s: index(%d), sockfd(%d), host(%s), port(%d)!\n", __func__, ptr->index, ptr->sockfd, ptr->host, ptr->port);

		trigger(PHP_SSP_CONNECT_DENIED,ptr);

		clean_conn(ptr);
		free(ptr);
	}else{
		ret=trigger(PHP_SSP_CONNECT,ptr);
		if(ret) {
			insert_conn(ptr);

			dprintf("%s: index(%d), sockfd(%d), host(%s), port(%d)!\n", __func__, ptr->index, ptr->sockfd, ptr->host, ptr->port);

			event_set(&ptr->event, conn_fd, EV_READ|EV_PERSIST, read_handler, ptr);
			event_base_set(me->base, &ptr->event);
			event_add(&ptr->event, NULL);
		} else {
			dprintf("not allow connect: sockfd(%d), host(%s), port(%d)!\n", ptr->sockfd, ptr->host, ptr->port);
			clean_conn(ptr);
			free(ptr);
		}
	}
}

void thread_init(int nthreads) {
	pthread_mutex_init(&init_lock, NULL);
    pthread_cond_init(&init_cond, NULL);

	pthread_mutex_init(&conn_lock, NULL);
    pthread_cond_init(&conn_cond, NULL);

	threads = calloc(nthreads, sizeof(event_thread_t));
	if (threads == NULL)
	{
		perror("calloc");
		exit(1);
	}

	int i;
	int fds[2];
	for (i = 0; i < nthreads; i++)
	{
        if (pipe(fds)) {
            perror("Can't create notify pipe");
            exit(1);
        }

		threads[i].id = i;
		threads[i].read_fd = fds[0];
		threads[i].write_fd = fds[1];

		threads[i].base = event_init();
		if (threads[i].base == NULL)
		{
			perror("event_init()");
			exit(1);
		}

		event_set(&threads[i].event, threads[i].read_fd, EV_READ | EV_PERSIST, notify_handler, &threads[i]);
		event_base_set(threads[i].base, &threads[i].event);
		if (event_add(&threads[i].event, 0) == -1)
		{
			perror("event_add()");
			exit(1);
		}

		pthread_mutex_init(&threads[i].queue_lock, NULL);
		threads[i].queue=(thread_queue_t *) malloc(sizeof(thread_queue_t));
		threads[i].queue->value=NULL;
		threads[i].queue->prev=threads[i].queue->next=threads[i].queue;
	}

	for (i = 0; i < nthreads; i++)
	{
		worker_create(worker_thread, &threads[i]);
	}

	/* Wait for all the threads to set themselves up before returning. */
    pthread_mutex_lock(&init_lock);
    while (init_count < nthreads) {
        pthread_cond_wait(&init_cond, &init_lock);
    }
    pthread_mutex_unlock(&init_lock);
}

static void listen_handler(const int fd, const short which, void *arg)
{
	char buf[1];
	event_thread_t *thread = threads + last_thread_id;

	last_thread_id = (last_thread_id + 1) % ssp_nthreads;        //memcached中线程负载均衡算法

	buf[0] = 'l';
	write(thread->write_fd, buf, 1);

	dprintf("listen_handler: threadId(%d)\n", thread->id);

    pthread_mutex_lock(&conn_lock);
	pthread_cond_wait(&conn_cond, &conn_lock);
    pthread_mutex_unlock(&conn_lock);
}

static void foreach_conn(gpointer key, gpointer value, gpointer user_data)
{
	conn_t *ptr = (conn_t *) value;

	dprintf("%s: index(%d), sockfd(%d), host(%s), port(%d)!\n", __func__, ptr->index, ptr->sockfd, ptr->host, ptr->port);
	
	trigger(PHP_SSP_CLOSE, ptr);
	clean_conn(ptr);
}

static void signal_handler(evutil_socket_t fd, short event, void *arg)
{
	struct event *signal = arg;

	dprintf("%s: got signal %d\n", __func__, EVENT_SIGNAL(signal));

	event_del(signal);

	pthread_mutex_lock(&conn_lock);
	is_accept_conn(false);
	pthread_mutex_unlock(&conn_lock);

	int i;
	char buf[1];
	buf[0] = '-';
	for(i=0;i<ssp_nthreads;i++) {
		dprintf("%s: notify thread exit %d\n", __func__, i);
		write(threads[i].write_fd, buf, 1);
	}

	dprintf("%s: wait worker thread %d\n", __func__);
    pthread_mutex_lock(&init_lock);
    while (init_count > 0) {
        pthread_cond_wait(&init_cond, &init_lock);
    }
    pthread_mutex_unlock(&init_lock);

	dprintf("%s: close conn\n", __func__);
	g_hash_table_foreach(iconns, foreach_conn, NULL);

	dprintf("%s: exit main thread\n", __func__);
	event_base_loopbreak(dispatcher_thread.base);
}

void loop_event (int sockfd)
{
	// init main thread
	dispatcher_thread.sockfd = sockfd;
	dispatcher_thread.base = event_init();
	if (dispatcher_thread.base == NULL)
	{
		perror("event_init( base )");
		exit(1);
	}
	dispatcher_thread.tid = pthread_self();

	// init notify thread
	thread_init(ssp_nthreads);

	// listen event
	event_assign(&dispatcher_thread.listen_ev, dispatcher_thread.base, sockfd, EV_READ|EV_PERSIST, listen_handler, NULL);
	event_add(&dispatcher_thread.listen_ev, NULL);

	// int signal event
	event_assign(&dispatcher_thread.signal_int, dispatcher_thread.base, SIGINT, EV_SIGNAL|EV_PERSIST, signal_handler, &dispatcher_thread.signal_int);
	event_add(&dispatcher_thread.signal_int, NULL);
	
	event_base_loop(dispatcher_thread.base, 0);
}