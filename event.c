#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>

#include "ssp.h"
#include "node.h"
#include "php_ext.h"
#include "php_func.h"

static int init_count = 0;
static pthread_mutex_t init_lock, conn_lock;
static pthread_cond_t init_cond, conn_cond;

int ssp_nthreads = 10;

typedef struct
{
	pthread_t tid;
	struct event_base *base;
	struct event event;
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
} dispatcher_thread_t;

event_thread_t *threads;
dispatcher_thread_t dispatcher_thread;
int last_thread_id = 0;

static void *worker_thread(void *arg)
{
	event_thread_t *me = arg;
	me->tid = pthread_self();

	printf("thread %d createed\n", me->id);

    pthread_mutex_lock(&init_lock);
    init_count++;
    pthread_cond_signal(&init_cond);
    pthread_mutex_unlock(&init_lock);

	event_base_loop(me->base, 0);

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
	node *ptr = (node *) arg;

	int data_len=0,ret;
	char *data=NULL;

	ret=socket_recv(ptr,&data,&data_len);
	if(ret<=0){//关闭连接
		trigger(PHP_SSP_CLOSE,ptr);
		clean_node(ptr);
		remove_node(ptr);
	}else{//接收数据成功
		trigger(PHP_SSP_RECEIVE,ptr,&data,&data_len);
		if(data_len>0){
			trigger(PHP_SSP_SEND,ptr,&data,&data_len);
			socket_send(ptr,data,data_len);
		}
	}
	free(data);
	if(ret!=0){
		ptr->tid=0;
		ptr->reading=false;
	}

}

static void notify_handler(int fd, short which, void *arg)
{
	int ret;
	char buf[1];
	event_thread_t *me = arg;

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

	dprintf("notify_handler: threadId(%d)\n", me->id);

	int conn_fd;
	struct sockaddr_in pin;
	socklen_t len=sizeof(pin);
	conn_fd=accept(dispatcher_thread.sockfd,(struct sockaddr *)&pin,&len);

    pthread_mutex_lock(&conn_lock);
	pthread_cond_signal(&conn_cond);
    pthread_mutex_unlock(&conn_lock);
	
	if(conn_fd<=0){
		return;
	}

	node *ptr;
	ptr=(node *)malloc(sizeof(node));

	bzero(ptr,sizeof(node));

	ptr->sockfd=conn_fd;
	inet_ntop(AF_INET,&pin.sin_addr,ptr->host,sizeof(ptr->host));
	ptr->port=ntohs(pin.sin_port);

	ptr->reading=true;

	insert_node(ptr);

	if(ptr->index==0){
		trigger(PHP_SSP_CONNECT_DENIED,ptr);

		clean_node(ptr);
		free(ptr);
	}else{
		trigger(PHP_SSP_CONNECT,ptr);

		ptr->tid=0;
		ptr->reading=false;
	}

	struct event *read_ev = (struct event*)malloc(sizeof(struct event));

	event_set(read_ev, conn_fd, EV_READ|EV_PERSIST, read_handler, ptr);
	event_base_set(me->base, read_ev);
	event_add(read_ev, NULL);
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

	buf[0] = '0';
	write(thread->write_fd, buf, 1);

	dprintf("listen_handler: threadId(%d)\n", thread->id);

    pthread_mutex_lock(&conn_lock);
	pthread_cond_wait(&conn_cond, &conn_lock);
    pthread_mutex_unlock(&conn_lock);
}

void event_daemon (int sockfd)
{
	dispatcher_thread.sockfd = sockfd;
	dispatcher_thread.base = event_init();
	if (dispatcher_thread.base == NULL)
	{
		perror("event_init( base )");
		exit(1);
	}
	dispatcher_thread.tid = pthread_self();

	thread_init(ssp_nthreads);

	event_set(&dispatcher_thread.listen_ev, sockfd, EV_READ|EV_PERSIST,	listen_handler, NULL);
	event_base_set(dispatcher_thread.base, &dispatcher_thread.listen_ev);
	event_add(&dispatcher_thread.listen_ev, NULL);
	event_base_loop(dispatcher_thread.base, 0);
}