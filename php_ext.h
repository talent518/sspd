#ifndef PHP_EXT_H
#define PHP_EXT_H

#include <php.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <time.h>

#include <TSRM.h>
#include <zend.h>

#include <event.h>

#include "config.h"
#include "api.h"
#include "queue.h"

#define PHP_SSP_DESCRIPTOR_RES_NAME "ssp conn_t"
#define PHP_SSP_VERSION "v2.2.1"

#define PHP_SSP_START 0
#define PHP_SSP_CONNECT 1
#define PHP_SSP_CONNECT_DENIED 2
#define PHP_SSP_RECEIVE 3
#define PHP_SSP_SEND 4
#define PHP_SSP_CLOSE 5
#define PHP_SSP_STOP 6
#define PHP_SSP_BENCH 7

#define PHP_SSP_RES_INDEX 0
#define PHP_SSP_RES_SOCKFD 1
#define PHP_SSP_RES_PORT 2

extern long le_ssp_descriptor;
extern unsigned int ssp_vars_length;
#ifdef SSP_CODE_TIMEOUT
	extern long ssp_timeout;
	#ifdef SSP_CODE_TIMEOUT_GLOBAL
		extern long ssp_global_timeout;
	#endif
#endif

extern zend_module_entry ssp_module_entry;

ZEND_BEGIN_MODULE_GLOBALS(ssp)
	unsigned short trigger_type;
	long trigger_count;
	zval ssp_vars;
	char threadname[64];
	char strftime[24]; // 2019-11-06 00:18:30
ZEND_END_MODULE_GLOBALS(ssp)

#define SSP_G(v) TSRMG(ssp_globals_id, zend_ssp_globals *, v)

#define ZEND_REGISTER_RESOURCE(var, ctx, le_inflate) ZVAL_RES(var, zend_register_resource(ctx, le_inflate))

const char *gettimeofstr();

#ifdef SSP_CODE_TIMEOUT
	#ifdef SSP_CODE_TIMEOUT_GLOBAL
		#define TRIGGER_STARTUP()

		#define TRIGGER_SHUTDOWN()

		#define MSG_QUEUE_STARTUP() \
			__time2 = microtime() - __time; \
			if(__time2 > ssp_timeout) { \
				ssp_request_shutdown();ssp_request_startup(); \
			} else if(__time2 > ssp_global_timeout) { \
				ssp_auto_globals_recreate(); \
			}
		#define MSG_QUEUE_SHUTDOWN() __time = microtime()

		#define DELAYED_STARTUP() \
			double __time = microtime() - ssp_delayed_time; \
			if(__time > ssp_timeout) { \
				ssp_request_shutdown();ssp_request_startup(); \
			} else if(__time > ssp_global_timeout) { \
				ssp_auto_globals_recreate(); \
			}
		#define DELAYED_SHUTDOWN() ssp_delayed_time = microtime();

		#define THREAD_STARTUP() ssp_request_startup();double __time = microtime(), __time2
		#define THREAD_SHUTDOWN() ssp_request_shutdown()
	#else
		#define TRIGGER_STARTUP() \
			if((SSP_G(trigger_count)++) == 0) {\
				dprintf("--------------------------------------------TRIGGER_STARTUP---------------------------------------------------------------------------\n");\
				ssp_auto_globals_recreate();\
			}

		#define TRIGGER_SHUTDOWN() \
			if((--SSP_G(trigger_count)) == 0) {\
				dprintf("--------------------------------------------TRIGGER_SHUTDOWN---------------------------------------------------------------------------\n");\
			}

		#define MSG_QUEUE_STARTUP() TRIGGER_STARTUP()
		#define MSG_QUEUE_SHUTDOWN() TRIGGER_SHUTDOWN()

		#define DELAYED_STARTUP() TRIGGER_STARTUP()
		#define DELAYED_SHUTDOWN() TRIGGER_SHUTDOWN()

		#define THREAD_STARTUP() ssp_request_startup()
		#define THREAD_SHUTDOWN() ssp_request_shutdown()
	#endif

	#define MONITOR_STARTUP()
	#define MONITOR_SHUTDOWN()
#else
	#define TRIGGER_STARTUP() \
		if((SSP_G(trigger_count)++) == 0) {\
			ssp_request_startup();\
			dprintf("--------------------------------------------TRIGGER_STARTUP---------------------------------------------------------------------------\n");\
		}

	#define TRIGGER_SHUTDOWN() \
		if((--SSP_G(trigger_count)) == 0) {\
			dprintf("--------------------------------------------TRIGGER_SHUTDOWN---------------------------------------------------------------------------\n");\
			ssp_request_shutdown();\
		}

	#define MSG_QUEUE_STARTUP() TRIGGER_STARTUP()
	#define MSG_QUEUE_SHUTDOWN() TRIGGER_SHUTDOWN()

	#define DELAYED_STARTUP() TRIGGER_STARTUP()
	#define DELAYED_SHUTDOWN() TRIGGER_SHUTDOWN()

	#define THREAD_STARTUP() ts_resource(0)
	#define THREAD_SHUTDOWN() ts_resource(0)

	#define MONITOR_STARTUP() ssp_request_startup()
	#define MONITOR_SHUTDOWN() ssp_request_shutdown()
#endif

static PHP_MINIT_FUNCTION(ssp);
static PHP_MSHUTDOWN_FUNCTION(ssp);
static PHP_RINIT_FUNCTION(ssp);
static PHP_RSHUTDOWN_FUNCTION(ssp);
static PHP_GINIT_FUNCTION(ssp);
static PHP_GSHUTDOWN_FUNCTION(ssp);
static PHP_MINFO_FUNCTION(ssp);

void ssp_auto_globals_recreate();
bool trigger(unsigned short type,...);

static PHP_FUNCTION(ssp_info);
static PHP_FUNCTION(ssp_connect);
static PHP_FUNCTION(ssp_send);
static PHP_FUNCTION(ssp_close);
static PHP_FUNCTION(ssp_lock);
static PHP_FUNCTION(ssp_unlock);

// begin stats
extern pthread_mutex_t ssp_stats_rlock;
extern pthread_mutex_t ssp_stats_wlock;
extern int ssp_stats_locks;

#define SSP_STATS_RLOCK() \
	pthread_mutex_lock(&ssp_stats_rlock); \
	if ((++(ssp_stats_locks)) == 1) { \
		pthread_mutex_lock(&ssp_stats_wlock); \
	} \
	pthread_mutex_unlock(&ssp_stats_rlock)

#define SSP_STATS_RUNLOCK() \
	pthread_mutex_lock(&ssp_stats_rlock); \
	if ((--(ssp_stats_locks)) == 0) { \
		pthread_mutex_unlock(&ssp_stats_wlock); \
	} \
	pthread_mutex_unlock(&ssp_stats_rlock)

#define SSP_STATS_WLOCK() pthread_mutex_lock(&ssp_stats_wlock)
#define SSP_STATS_WUNLOCK() pthread_mutex_unlock(&ssp_stats_wlock)

static PHP_FUNCTION(ssp_stats);
// end stats

// bench usage
static PHP_FUNCTION(ssp_counts);
// end bench usage

static PHP_FUNCTION(crypt_encode);
static PHP_FUNCTION(crypt_decode);

ZEND_DECLARE_MODULE_GLOBALS(ssp);

typedef struct _server_t {
	int sid;

	int sockfd;
	char host[16];
	int port;

	char *rbuf;
	int rbytes;
	int index;
	int rsize;

#if ASYNC_SEND
	char *wbuf;
	int wbytes;
	int wsize;
#endif // ASYNC_SEND

	struct event event;
} server_t;

typedef struct _conv_server_t {
	server_t *ptr;
	int index;
	int size;
	char buf[1];
} conv_server_t;

extern int ssp_server_id;
extern int ssp_server_max;
extern server_t *servers;
extern queue_t *ssp_server_queue;
static PHP_FUNCTION(ssp_conv_setup);
static PHP_FUNCTION(ssp_conv_connect);
static PHP_FUNCTION(ssp_conv_disconnect);

#ifdef SSP_DEBUG_CONV
#define vprintf(args...) fprintf(stderr, args);
#else
#define vprintf(args...)
#endif

static PHP_FUNCTION(ssp_msg_queue_init);
static PHP_FUNCTION(ssp_msg_queue_push);
static PHP_FUNCTION(ssp_msg_queue_destory);

static PHP_FUNCTION(ssp_delayed_init);
static PHP_FUNCTION(ssp_delayed_set);
static PHP_FUNCTION(ssp_delayed_del);
static PHP_FUNCTION(ssp_delayed_destory);

static PHP_FUNCTION(ssp_var_init);
static PHP_FUNCTION(ssp_var_exists);
static PHP_FUNCTION(ssp_var_get);
static PHP_FUNCTION(ssp_var_put);
static PHP_FUNCTION(ssp_var_inc);
static PHP_FUNCTION(ssp_var_del);
static PHP_FUNCTION(ssp_var_clean);
static PHP_FUNCTION(ssp_var_destory);

#endif  /* PHP_EXT_H */
