#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>

#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "api.h"
#include "ssp.h"
#include "data.h"
#include "php_ext.h"
#include "php_func.h"
#include "ssp_event.h"
#include "socket.h"

static pthread_mutex_t init_lock, conn_lock;
static pthread_cond_t init_cond;

int ssp_nthreads = 10;

listen_thread_t listen_thread;
worker_thread_t *worker_threads;

static void *worker_thread_handler(void *arg)
{
	worker_thread_t *me = arg;
	me->tid = pthread_self();

	ts_resource(0);

	sprintf(SSP_G(threadname), "worker %d thread", me->id);
	dprintf("thread %d created\n", me->id);

	THREAD_STARTUP();

	pthread_mutex_lock(&init_lock);

	listen_thread.nthreads++;
	pthread_cond_signal(&init_cond);
	pthread_mutex_unlock(&init_lock);

	event_base_loop(me->base, 0);

	THREAD_SHUTDOWN();

	ts_free_thread();

	queue_free(me->accept_queue);
	queue_free(me->write_queue);
	queue_free(me->close_queue);

	dprintf("thread %d exited\n", me->id);
	pthread_mutex_lock(&init_lock);
	listen_thread.nthreads--;
	pthread_cond_signal(&init_cond);
	pthread_mutex_unlock(&init_lock);

	pthread_detach(me->tid);
	pthread_exit(NULL);

	return NULL;
}

void worker_create(void *(*func)(void *), void *arg) {
	pthread_t       thread;
	pthread_attr_t  attr;
	int             ret;

	pthread_attr_init(&attr);

	if ((ret = pthread_create(&thread, &attr, func, arg)) != 0) {
		fprintf(stderr, "Can't create thread: %s\n", strerror(ret));
		exit(1);
	}
}

#if ASYNC_SEND
static void read_write_handler(int sock, short event, void* arg);

void is_writable_conn(conn_t *ptr, bool iswrite) {
	if(ptr->sockfd < 0) return;

	dprintf("%s: %4d, %d, begin\n", __func__, ptr->index, iswrite);
	event_del(&ptr->event);

	if(iswrite) event_set(&ptr->event, ptr->sockfd, EV_READ|EV_WRITE|EV_PERSIST, read_write_handler, ptr);
	else event_set(&ptr->event, ptr->sockfd, EV_READ|EV_PERSIST, read_write_handler, ptr);

	event_base_set(ptr->thread->base, &ptr->event);
	event_add(&ptr->event, NULL);
	dprintf("%s: %4d, %d, end\n", __func__, ptr->index, iswrite);
}
#endif

static void read_write_handler(int sock, short event, void* arg)
{
	conn_t *ptr = (conn_t *) arg;
	int data_len=0,ret;
	char *data=NULL;

	assert(ptr->refable);
	assert(ptr->sockfd == sock);

	conn_info(ptr);

#if ASYNC_SEND
	if(event & EV_WRITE) {
		ret = send(ptr->sockfd, ptr->wbuf+ptr->wbytes, ptr->wsize - ptr->wbytes, MSG_DONTWAIT);
		if(ret > 0) {
			ptr->wbytes += ret;
			if(ptr->wsize == ptr->wbytes) {
				free(ptr->wbuf);
				ptr->wbuf = NULL;
				ptr->wbytes = ptr->wsize = 0;

				is_writable_conn(ptr, false);
			}
		} else {
			ptr->thread->conn_num--;

			//fprintf(stderr, "%s: send error\n", __func__);

			clean_conn(ptr);
			trigger(PHP_SSP_CLOSE,ptr);
			remove_conn(ptr);
			return;
		}
	}
	if(event & EV_READ) {
#endif
	ret=socket_recv(ptr,&data,&data_len);
	if(ret<0) { //已放入缓冲区
	} else if(ret==0) { // 关闭连接
		//fprintf(stderr, "%s: recv error\n", __func__);

		ptr->thread->conn_num--;

		clean_conn(ptr);
		trigger(PHP_SSP_CLOSE,ptr);
		remove_conn(ptr);
	} else { // 接收数据成功
		INIT_RUNTIME();
		trigger(PHP_SSP_RECEIVE,ptr,data,data_len);
		INFO_RUNTIME("RECV");
	}
	if(data) {
		free(data);
	}
#if ASYNC_SEND
	}
#endif
}

#if ASYNC_SEND
void socket_send_buf(conn_t *ptr, char *package, int plen) {
	assert(ptr->refable);

	if(ptr->wbuf) {
		int bn = ptr->wsize - ptr->wbytes;
		int n = plen + bn;
		char *data = (char*) malloc(n);
		memcpy(data, ptr->wbuf + ptr->wbytes, bn);
		memcpy(data + bn, package, plen);
		free(package);
		free(ptr->wbuf);
		ptr->wbuf = data;
		ptr->wbytes = 0;
		ptr->wsize = n;
	} else {
	#if 0
		int n = send(ptr->sockfd, package, plen, MSG_DONTWAIT);
		if(n >= 0) {
			if(n < plen) {
				ptr->wbuf = package;
				ptr->wbytes = n;
				ptr->wsize = plen;

				is_writable_conn(ptr, true);
			} else {
				free(package);
			}
		} else {
			free(package);

			ptr->thread->conn_num--;

			clean_conn(ptr);
			trigger(PHP_SSP_CLOSE, ptr);
			remove_conn(ptr);
		}
	#else
		ptr->wbuf = package;
		ptr->wbytes = 0;
		ptr->wsize = plen;

		is_writable_conn(ptr, true);
	#endif
	}
}
#endif // ASYNC_SEND

static void notify_handler(const int fd, const short which, void *arg)
{
#ifdef SSP_CODE_TIMEOUT
	bool isclean=false;
#ifdef SSP_CODE_TIMEOUT_GLOBAL
	bool isglobal=false;
#endif
#endif
	int buffer_len,i;
	char chr;
	char buffer[1024];
	worker_thread_t *me = arg;
	conn_t *ptr;

	buffer_len = read(fd, buffer, sizeof(buffer));

	for(i=0;i<buffer_len;i++) {
		chr=buffer[i];

		dprintf("notify_handler: notify(%c) threadId(%d)\n", chr, me->id);

		switch(chr) {
		#if ASYNC_SEND
			case 'w': {
				send_t *s = queue_pop(me->write_queue);
				if(!s) break;

				ptr = s->ptr;

				assert(ptr->thread == me);

				socket_send_buf(ptr, s->str, s->len);
				free(s);
				break;
			}
		#endif // ASYNC_SEND
			case 'x': // 处理连接关闭对列
				ptr = queue_pop(me->close_queue);
				if(!ptr) break;

				assert(ptr->thread == me);

			#if ASYNC_SEND
				if(ptr->wbuf) {
					dprintf("close: %4d,1\n", ptr->index);
					queue_push(me->close_queue, ptr);
					write(me->write_fd, "x", 1);
					break;
				}
			#endif // ASYNC_SEND

				dprintf("close: %4d,2\n", ptr->index);

				conn_info(ptr);
				clean_conn(ptr);
				trigger(PHP_SSP_CLOSE,ptr);
				remove_conn(ptr);

				me->conn_num--;
				break;
			case '-': // 结束worker/notify线程
				event_base_loopbreak(me->base);
				break;
			case 'l':
				ptr=queue_pop(me->accept_queue);
				if(!ptr) break;

				assert(ptr->thread == me);

				conn_info(ptr);

				me->conn_num++;
				me->clean_times = 0;

				event_set(&ptr->event, ptr->sockfd, EV_READ|EV_PERSIST, read_write_handler, ptr);
				event_base_set(me->base, &ptr->event);
				event_add(&ptr->event, NULL);

				if(!trigger(PHP_SSP_CONNECT, ptr)) {
					me->conn_num--;
					conn_info(ptr);
					clean_conn(ptr);
					trigger(PHP_SSP_CLOSE,ptr);
					remove_conn(ptr);
				}
				break;
	#ifdef SSP_CODE_TIMEOUT
			case 't':
				if(isclean) {
					break;
				}
				if(me->conn_num<=0) {
					if(me->clean_times<SSP_CODE_TIMEOUT) {
						me->clean_times++;
					} else {
						break;
					}
				}
				dprintf("==================================================================================================================================\n");
				THREAD_SHUTDOWN();
				dprintf("========================================================PHP_REQUEST_CLEAN=========================================================\n");
				THREAD_STARTUP();
				dprintf("==================================================================================================================================\n");

				isclean=true;
		#ifdef SSP_CODE_TIMEOUT_GLOBAL
				isglobal=true;
				break;
			case 'g':
				if(isglobal) {
					break;
				}
				if(me->conn_num<=0) {
					break;
				}

				ssp_auto_globals_recreate();
				trigger(PHP_SSP_CLEAN);
			#ifndef DISABLE_GC_COLLECT_CYCLES
				gc_collect_cycles();
			#endif

				isglobal=true;
		#endif
				break;
	#endif
			default:
				break;
		}
	}
}

void thread_init() {
	pthread_mutex_init(&init_lock, NULL);
    pthread_cond_init(&init_cond, NULL);

	pthread_mutex_init(&conn_lock, NULL);

	worker_threads = calloc(ssp_nthreads, sizeof(worker_thread_t));
	assert(worker_threads);

	register int i;
	int fds[2];
	for (i = 0; i < ssp_nthreads; i++) {
        if (pipe(fds)) {
            perror("Can't create notify pipe");
            exit(1);
        }

		worker_threads[i].id = i;
		worker_threads[i].read_fd = fds[0];
		worker_threads[i].write_fd = fds[1];

		worker_threads[i].base = event_init();
		if (worker_threads[i].base == NULL) {
			perror("event_init()");
			exit(1);
		}

		event_set(&worker_threads[i].event, worker_threads[i].read_fd, EV_READ | EV_PERSIST, notify_handler, &worker_threads[i]);
		event_base_set(worker_threads[i].base, &worker_threads[i].event);
		if (event_add(&worker_threads[i].event, 0) == -1) {
			perror("event_add()");
			exit(1);
		}

		worker_threads[i].accept_queue=queue_init();
		worker_threads[i].write_queue=queue_init();
		worker_threads[i].close_queue=queue_init();
		worker_threads[i].conn_num = 0;
		worker_threads[i].clean_times = 0;
	}

	for (i = 0; i < ssp_nthreads; i++) {
		worker_create(worker_thread_handler, &worker_threads[i]);
	}

	/* Wait for all the worker_threads to set themselves up before returning. */
    pthread_mutex_lock(&init_lock);
    while (listen_thread.nthreads < ssp_nthreads) {
        pthread_cond_wait(&init_cond, &init_lock);
    }
    pthread_mutex_unlock(&init_lock);
}

static void listen_notify_handler(const int fd, const short which, void *arg)
{
#ifdef SSP_CODE_TIMEOUT
	bool isclean=false;
#ifdef SSP_CODE_TIMEOUT_GLOBAL
	bool isglobal=false;
#endif
#endif
	register int buf_len,i;
	char buf[1024];

	assert(fd == listen_thread.read_fd);

	buf_len = read(fd, buf, 1024);
	if (buf_len <= 0) {
		return;
	}

	for(i=0; i<buf_len; i++) {
		dprintf("%s(%c)\n", __func__, buf[i]);

		switch(buf[i]) {
		#if ASYNC_SEND
			case 's': {
				conv_server_t *c = queue_pop(ssp_server_queue);
				if(!c) break;

				server_t *ptr = c->ptr;
				size_t plen = c->size + sizeof(int)*2;

				vprintf("conv send: %s\n", c->buf);

				if(ptr->wbuf) {
					int bn = ptr->wsize - ptr->wbytes;
					char *data = (char*) malloc(plen + bn);
					memcpy(data, ptr->wbuf + ptr->wbytes, bn);
					memcpy(data + bn, &c->index, plen);
					free(ptr->wbuf);
					ptr->wbuf = data;
					ptr->wbytes = 0;
					ptr->wsize = plen + bn;
				} else {
				#if 0
					int n = send(ptr->sockfd, &c->index, plen, MSG_DONTWAIT);
					if(n >= 0) {
						if(n < plen) {
							ptr->wbuf = (char *) malloc(plen-n);
							ptr->wbytes = 0;
							ptr->wsize = plen;

							memcpy(ptr->wbuf, &c->index+n, plen-n);

							is_writable_conv(ptr, true);
						} else {
							free(ptr->wbuf);
							ptr->wbytes = ptr->wsize = 0;
							is_writable_conv(ptr, false);
						}
					} else {
						printf("conv send notify error %d\n", ptr->sid);
						ssp_conv_disconnect(ptr);
					}
				#else
					ptr->wbuf = (char *) malloc(plen);
					ptr->wbytes = 0;
					ptr->wsize = plen;

					memcpy(ptr->wbuf, &c->index, plen);

					is_writable_conv(ptr, true);
				#endif
				}

				free(c);
				break;
			}
		#endif // ASYNC_SEND
		#ifdef SSP_CODE_TIMEOUT
			case 't':
				if(isclean) {
					break;
				}
				dprintf("==================================================================================================================================\n");
				THREAD_SHUTDOWN();
				dprintf("========================================================PHP_REQUEST_CLEAN=========================================================\n");
				THREAD_STARTUP();
				dprintf("==================================================================================================================================\n");
				isclean = true;
		#ifdef SSP_CODE_TIMEOUT_GLOBAL
				isglobal = true;
				break;
			case 'g':
				if(isglobal) {
					break;
				}

				ssp_auto_globals_recreate();
				trigger(PHP_SSP_CLEAN);
			#ifndef DISABLE_GC_COLLECT_CYCLES
				gc_collect_cycles();
			#endif
				isglobal = true;
		#endif // SSP_CODE_TIMEOUT_GLOBAL
				break;
		#endif // SSP_CODE_TIMEOUT
			default:
				break;
		}
	}
}

static void listen_handler(const int fd, const short which, void *arg)
{
	int conn_fd, ret;
	struct sockaddr_in pin;
	socklen_t len = sizeof(pin);
	conn_fd = accept(fd, (struct sockaddr *)&pin, &len);

	// printf("accept: %d\n", conn_fd);

	if(conn_fd <= 0) {
		return;
	}

	socket_set_accept(conn_fd,SOCKET_SNDTIMEO,SOCKET_RCVTIMEO,SOCKET_SNDBUF,SOCKET_RCVBUF);

	conn_t *ptr;

	if(CONN_NUM >= ssp_maxclients) {
		conn_t conn;
		ptr = &conn;

		bzero(ptr, sizeof(conn_t));

		ptr->sockfd = conn_fd;
		inet_ntop(AF_INET, &pin.sin_addr, ptr->host, len);
		ptr->port = ntohs(pin.sin_port);

		conn_info(ptr);

		trigger(PHP_SSP_CONNECT_DENIED, ptr);

		shutdown(ptr->sockfd, SHUT_RDWR);
		close(ptr->sockfd);
	} else {
		ptr = insert_conn();
		ptr->sockfd = conn_fd;
		inet_ntop(AF_INET, &pin.sin_addr, ptr->host, len);
		ptr->port = ntohs(pin.sin_port);

		ptr->thread = worker_threads + ptr->index % ssp_nthreads;

		dprintf("notify thread %d\n", ptr->thread->id);

		queue_push(ptr->thread->accept_queue, ptr);

		conn_info(ptr);

		write(ptr->thread->write_fd, "l", 1);
	}
}

conn_t *index_conn_signal(int i);
static void signal_handler(const int fd, short event, void *arg) {
	dprintf("%s: got signal %d\n", __func__, EVENT_SIGNAL(&listen_thread.signal_int));

	event_del(&listen_thread.signal_int);
	if(listen_thread.sockfd >= 0) event_del(&listen_thread.listen_ev);

	register int i;
	for(i=0; i<ssp_nthreads; i++) {
		dprintf("%s: notify thread exit %d\n", __func__, i);
		write(worker_threads[i].write_fd, "-", 1);
	}

	dprintf("%s: wait worker thread %d\n", __func__, listen_thread.nthreads);
    pthread_mutex_lock(&init_lock);
    while (listen_thread.nthreads > 0) {
        pthread_cond_wait(&init_cond, &init_lock);
    }
    pthread_mutex_unlock(&init_lock);

	dprintf("%s: close conn\n", __func__);

	conn_t *ptr;

	for(i=0; i<ssp_maxclients; i++) {
		ptr = index_conn_signal(i);

		if(!ptr->port) {
			continue;
		}

		conn_info(ptr);

		trigger(PHP_SSP_CLOSE, ptr);

		ptr->port = 0;
		clean_conn(ptr);
	}

	dprintf("%s: exit main thread\n", __func__);
	event_base_loopbreak(listen_thread.base);
}

#ifdef SSP_CODE_TIMEOUT
	static void timeout_handler(evutil_socket_t fd, short event, void *arg) {
		register int i;

		write(listen_thread.write_fd, "t", 1);

		for(i=0;i<ssp_nthreads;i++) {
			dprintf("%s: notify thread timeout %d\n", __func__, i);
			write(worker_threads[i].write_fd, "t", 1);
		}

		dprintf("==================================================================================================================================\n");
		THREAD_SHUTDOWN();
		dprintf("========================================================PHP_REQUEST_CLEAN=========================================================\n");
		THREAD_STARTUP();
		dprintf("==================================================================================================================================\n");
	}

	#ifdef SSP_CODE_TIMEOUT_GLOBAL
		static void timeout_global_handler(evutil_socket_t fd, short event, void *arg) {
			register int i;

			write(listen_thread.write_fd, "g", 1);

			for(i=0;i<ssp_nthreads;i++) {
				dprintf("%s: notify thread timeout %d\n", __func__, i);
				write(worker_threads[i].write_fd, "g", 1);
			}
		}
	#endif
#endif

static void bench_handler(evutil_socket_t fd, short event, void *arg) {
	trigger(PHP_SSP_BENCH);
	if(CONN_NUM <= 0) signal_handler(0, 0, NULL);
}

top_info_t top_info;
#define MONITOR_PARAM_COUNT 7
#define CPU_ALL(all,cpu) all = cpu.user + cpu.nice + cpu.system + cpu.idle + cpu.iowait + cpu.irq + cpu.softirq + cpu.stolen + cpu.guest
#define MONITOR_FUNC_NAME "ssp_monitor_handler"

void set_monitor_zval(zval *scpu, zval *pcpu, zval *smem, zval *pmem, zval *args) {
	char buf[2048];
	FILE *fp;
	int i;
	char *p, *p2;

	array_init_size(scpu, 9);
	add_assoc_double(scpu, "user", top_info.scpu.user);
	add_assoc_double(scpu, "nice", top_info.scpu.nice);
	add_assoc_double(scpu, "system", top_info.scpu.system);
	add_assoc_double(scpu, "idle", top_info.scpu.idle);
	add_assoc_double(scpu, "iowait", top_info.scpu.iowait);
	add_assoc_double(scpu, "irq", top_info.scpu.irq);
	add_assoc_double(scpu, "softirq", top_info.scpu.softirq);
	add_assoc_double(scpu, "stolen", top_info.scpu.stolen);
	add_assoc_double(scpu, "guest", top_info.scpu.guest);

	array_init_size(pcpu, 2);
	add_assoc_long(pcpu, "utime", top_info.pcpu.utime);
	add_assoc_long(pcpu, "stime", top_info.pcpu.stime);

	array_init_size(smem, 8);
	add_assoc_long(smem, "total", top_info.mem.total);
	add_assoc_long(smem, "free", top_info.mem.free);
	add_assoc_long(smem, "cached", top_info.mem.cached);
	add_assoc_long(smem, "buffers", top_info.mem.buffers);
	add_assoc_long(smem, "locked", top_info.mem.locked);
	add_assoc_long(smem, "swapTotal", top_info.mem.swapTotal);
	add_assoc_long(smem, "swapFree", top_info.mem.swapFree);
	add_assoc_long(smem, "shared", top_info.mem.shared);

	array_init_size(pmem, 2);
	add_assoc_long(pmem, "size", top_info.proc.size * 4);
	add_assoc_long(pmem, "resident", top_info.proc.resident * 4);
	add_assoc_long(pmem, "share", top_info.proc.share * 4);
	add_assoc_long(pmem, "text", top_info.proc.text * 4);
	add_assoc_long(pmem, "lib", top_info.proc.lib * 4);
	add_assoc_long(pmem, "data", top_info.proc.data * 4);
	add_assoc_long(pmem, "dirty", top_info.proc.dirty);
	add_assoc_long(pmem, "rssFile", top_info.proc.rssFile);

	array_init_size(args, 32);
	sprintf(buf, "/proc/%d/cmdline", getpid());
	fp = fopen(buf, "r");
	if(fp) {
		i = fread(buf, 1, sizeof(buf), fp);
		fclose(fp);

		p = buf;
		p2 = buf + i;
		do {
			i = strlen(p);
			add_next_index_stringl(args, p, i);
			p += i+1;
		} while(p < p2);
	}
}

static void timeout_monitor_handler(evutil_socket_t fd, short event, void *arg) {
	unsigned long int all;
	double total;
	cpu_t cpu;
	process_t proc;

	int i;
	zval pfunc, rv;
	zval params[MONITOR_PARAM_COUNT];

	getcpu(&cpu);
	getprocessinfo(getpid(), &proc);

	SSP_STATS_WLOCK();
	getmem(&top_info.mem);

	// system cpu persent calculate ==========================

	CPU_ALL(all, cpu);
	total                  = (double) (all - top_info.all) / 100.0;
	top_info.scpu.user     = (double)(cpu.user - top_info.cpu.user) / total;
	top_info.scpu.nice     = (double)(cpu.nice - top_info.cpu.nice) / total;
	top_info.scpu.system   = (double)(cpu.system - top_info.cpu.system) / total;
	top_info.scpu.idle     = (double)(cpu.idle - top_info.cpu.idle) / total;
	top_info.scpu.iowait   = (double)(cpu.iowait - top_info.cpu.iowait) / total;
	top_info.scpu.irq      = (double)(cpu.irq - top_info.cpu.irq) / total;
	top_info.scpu.softirq  = (double)(cpu.softirq - top_info.cpu.softirq) / total;
	top_info.scpu.stolen   = (double)(cpu.stolen - top_info.cpu.stolen) / total;
	top_info.scpu.guest    = (double)(cpu.guest - top_info.cpu.guest) / total;
	top_info.all           = all;
	top_info.cpu           = cpu;

	// process cpu persent calculate =========================
	top_info.pcpu.utime    =  proc.utime/* + proc.cutime*/ - top_info.proc.utime/* - top_info.proc.cutime*/;
	top_info.pcpu.stime    =  proc.stime/* + proc.cstime*/ - top_info.proc.stime/* - top_info.proc.cstime*/;
	top_info.proc          = proc;
	SSP_STATS_WUNLOCK();

	// php function call =====================================
	MONITOR_STARTUP();

	ZVAL_STRING(&pfunc, MONITOR_FUNC_NAME);

	SSP_STATS_RLOCK();
	set_monitor_zval(&params[0], &params[1], &params[2], &params[3], &params[6]);

	ZVAL_LONG(&params[4], top_info.proc.threads); // threads
	ZVAL_LONG(&params[5], top_info.proc.etime); // etime
	SSP_STATS_RUNLOCK();

	ZVAL_NULL(&rv);

	i = call_user_function(EG(function_table), NULL, &pfunc, &rv, MONITOR_PARAM_COUNT, params);
	if(i == FAILURE) php_printf("Unable to call function(%s)\n", MONITOR_FUNC_NAME);

	zval_ptr_dtor(&pfunc);
	zval_ptr_dtor(&rv);

	for (i = 0; i < MONITOR_PARAM_COUNT; i++) {
		zval_ptr_dtor(&params[i]);
	}

	MONITOR_SHUTDOWN();
}

void loop_event (int sockfd) {
	signal(SIGPIPE, SIG_IGN);
	signal(SIGURG, SIG_IGN);
	signal(SIGALRM, SIG_IGN);

	strcpy(SSP_G(threadname), "main thread");

	struct rlimit rlim;
	if(getrlimit(RLIMIT_NOFILE, &rlim) < 0) {
		perror("getrlimit nofile");
	} else {
		// printf("rlim.rlim_cur: %lu, rlim.rlim_max: %lu\n", rlim.rlim_cur, rlim.rlim_max);
		rlim.rlim_cur = ssp_maxclients + 128 * ssp_nthreads;
		if(rlim.rlim_cur > rlim.rlim_max) rlim.rlim_max = rlim.rlim_cur;
		if(setrlimit(RLIMIT_NOFILE, &rlim) < 0) perror("setrlimit nofile");
	}

	// init main thread
	listen_thread.sockfd = sockfd;
	listen_thread.base = event_init();
	if (listen_thread.base == NULL) {
		perror("event_init( base )");
		exit(1);
	}
	listen_thread.tid = pthread_self();
	listen_thread.nthreads = 0;

	int fds[2];
	if (pipe(fds)) {
		perror("Can't create notify pipe");
		exit(1);
	}

	listen_thread.read_fd = fds[0];
	listen_thread.write_fd = fds[1];

	// init notify thread
	thread_init();

	// listen notify event
	event_set(&listen_thread.notify_ev, listen_thread.read_fd, EV_READ | EV_PERSIST, listen_notify_handler, NULL);
	event_base_set(listen_thread.base, &listen_thread.notify_ev);
	if (event_add(&listen_thread.notify_ev, NULL) == -1) {
		perror("event_add()");
		exit(1);
	}

	if(listen_thread.sockfd < 0) {
		// bench event
		struct timeval btv;
		evutil_timerclear(&btv);
		btv.tv_sec = 1;
		event_set(&listen_thread.bench_int, -1, EV_PERSIST, bench_handler, NULL);
		event_base_set(listen_thread.base, &listen_thread.bench_int);
		if (event_add(&listen_thread.bench_int, &btv) == -1) {
			perror("timeout event");
			exit(1);
		}
	} else {
		// listen event
		listen_thread.ev_flags=EV_READ | EV_PERSIST;
		event_set(&listen_thread.listen_ev, sockfd, listen_thread.ev_flags, listen_handler, NULL);
		event_base_set(listen_thread.base, &listen_thread.listen_ev);
		if (event_add(&listen_thread.listen_ev, NULL) == -1) {
			perror("listen event");
			exit(1);
		}
	}

	// int signal event
	event_set(&listen_thread.signal_int, SIGINT, EV_SIGNAL|EV_PERSIST, signal_handler, NULL);
	event_base_set(listen_thread.base, &listen_thread.signal_int);
	if (event_add(&listen_thread.signal_int, NULL) == -1) {
		perror("int signal event");
		exit(1);
	}

#ifdef SSP_CODE_TIMEOUT
	// timeout event
	struct timeval tv;
	evutil_timerclear(&tv);
	tv.tv_sec = ssp_timeout;
	event_set(&listen_thread.timeout_int, -1, EV_PERSIST, timeout_handler, NULL);
	event_base_set(listen_thread.base, &listen_thread.timeout_int);
	if (event_add(&listen_thread.timeout_int, &tv) == -1) {
		perror("timeout event");
		exit(1);
	}

	#ifdef SSP_CODE_TIMEOUT_GLOBAL
		// timeout global event
		struct timeval tv2;
		evutil_timerclear(&tv2);
		tv2.tv_sec = ssp_global_timeout;
		event_set(&listen_thread.timeout_global_int, -1, EV_PERSIST, timeout_global_handler, NULL);
		event_base_set(listen_thread.base, &listen_thread.timeout_global_int);
		if (event_add(&listen_thread.timeout_global_int, &tv2) == -1) {
			perror("timeout global event");
			exit(1);
		}
	#endif
#endif

	// timeout global event
	struct event event;
	struct timeval tv3;
	evutil_timerclear(&tv3);
	tv3.tv_sec = 1;
	event_set(&event, -1, EV_PERSIST, timeout_monitor_handler, NULL);
	event_base_set(listen_thread.base, &event);
	if (event_add(&event, &tv3) == -1) {
		perror("timeout monitor event");
		exit(1);
	}

	pthread_mutex_init(&ssp_stats_rlock, NULL);
	pthread_mutex_init(&ssp_stats_wlock, NULL);

	memset(&top_info, 0, sizeof(top_info));
	getcpu(&top_info.cpu);
	getmem(&top_info.mem);
	getprocessinfo(getpid(), &top_info.proc);
	CPU_ALL(top_info.all, top_info.cpu);

	attach_conn();

	THREAD_STARTUP();

	trigger(PHP_SSP_START);

	if(sockfd >= 0) kill(getppid(), SIGINT);

	event_base_loop(listen_thread.base, 0);

	trigger(PHP_SSP_STOP);

	THREAD_SHUTDOWN();

	shutdown(sockfd, SHUT_RDWR);
	if(sockfd >= 0) close(sockfd);

	detach_conn();

	pthread_mutex_destroy(&ssp_stats_rlock);
	pthread_mutex_destroy(&ssp_stats_wlock);

	event_base_free(listen_thread.base);

	close(listen_thread.read_fd);
	close(listen_thread.write_fd);

	register int i;
	for (i = 0; i < ssp_nthreads; i++) {
		event_base_free(worker_threads[i].base);
		close(worker_threads[i].read_fd);
		close(worker_threads[i].write_fd);
	}
	free(worker_threads);
}
