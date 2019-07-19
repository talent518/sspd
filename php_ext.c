#include "php_func.h"
#include "php_ext.h"
#include "ssp.h"
#include "data.h"
#include "api.h"
#include "crypt.h"
#include "socket.h"
#include <malloc.h>
#include <signal.h>
#include <math.h>

#include <standard/php_var.h>
#include <zend_constants.h>
#include <zend_smart_str.h>

#ifdef HAVE_LIBGTOP
#include <glibtop.h>
#include <glibtop/cpu.h>
#include <glibtop/mem.h>
#include <glibtop/proctime.h>
#include <glibtop/procmem.h>
#endif

static pthread_mutex_t unique_lock, counts_lock;

static char trigger_handlers[8][30] = {
	"ssp_start_handler",
	"ssp_connect_handler",
	"ssp_connect_denied_handler",
	"ssp_receive_handler",
	"ssp_send_handler",
	"ssp_close_handler",
	"ssp_stop_handler",
	"ssp_bench_handler"
};

long le_ssp_descriptor;
unsigned int ssp_vars_length = 10;
#ifdef SSP_CODE_TIMEOUT
long ssp_timeout=60;
#ifdef SSP_CODE_TIMEOUT_GLOBAL
long ssp_global_timeout=10;
#endif
#endif
int ssp_server_id = -1;
int ssp_server_max = -1;
server_t *servers = NULL;
queue_t *ssp_server_queue = NULL;
static struct event conv_timeout_int;

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_type, 0, 0, 2)
ZEND_ARG_INFO(0, res)
ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_info, 0, 0, 1)
ZEND_ARG_INFO(0, res)
ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_connect, 0, 0, 1)
ZEND_ARG_INFO(0, host)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_conv_setup, 0, 0, 2)
ZEND_ARG_INFO(0, sid)
ZEND_ARG_INFO(0, max_sid)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_conv_connect, 0, 0, 3)
ZEND_ARG_INFO(0, host)
ZEND_ARG_INFO(0, port)
ZEND_ARG_INFO(0, sid)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_conv_disconnect, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_send, 0, 0, 2)
ZEND_ARG_INFO(0, res)
ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_close, 0, 0, 1)
ZEND_ARG_INFO(0, res)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_lock, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_unlock, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_stats, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_counts, 0, 0, 1)
ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_requests, 0, 0, 1)
ZEND_ARG_INFO(0, res)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_setup, 0, 0, 2)
ZEND_ARG_INFO(0, res)
ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_crypt_encode, 0, 0, 2)
ZEND_ARG_INFO(0, str)
ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_crypt_decode, 0, 0, 2)
ZEND_ARG_INFO(0, str)
ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_msg_queue_init, 0, 0, 2)
ZEND_ARG_INFO(0, msgs)
ZEND_ARG_INFO(0, nthreads)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_msg_queue_push, 0, 0, 3)
ZEND_ARG_INFO(0, func)
ZEND_ARG_INFO(0, what)
ZEND_ARG_INFO(0, arg)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_msg_queue_destory, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_delayed_init, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_delayed_set, 0, 0, 3)
ZEND_ARG_INFO(0, func)
ZEND_ARG_INFO(0, delay)
ZEND_ARG_INFO(0, persist)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_delayed_del, 0, 0, 1)
ZEND_ARG_INFO(0, func)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_delayed_destory, 0, 0, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ ssp_functions[] */
static const zend_function_entry ssp_functions[] = {
	PHP_FE(ssp_type, arginfo_ssp_type)
	PHP_FE(ssp_info, arginfo_ssp_info)
	PHP_FE(ssp_connect, arginfo_ssp_connect)
	PHP_FE(ssp_conv_setup, arginfo_ssp_conv_setup)
	PHP_FE(ssp_conv_connect, arginfo_ssp_conv_connect)
	PHP_FE(ssp_conv_disconnect, arginfo_ssp_conv_disconnect)
	PHP_FE(ssp_send, arginfo_ssp_send)
	PHP_FE(ssp_close, arginfo_ssp_close)
	PHP_FE(ssp_lock, arginfo_ssp_lock)
	PHP_FE(ssp_unlock, arginfo_ssp_unlock)
	PHP_FE(ssp_stats, arginfo_ssp_stats)
	PHP_FE(ssp_counts, arginfo_ssp_counts)
	PHP_FE(ssp_requests, arginfo_ssp_requests)
	PHP_FE(ssp_setup, arginfo_ssp_setup)
	PHP_FE(crypt_encode, arginfo_crypt_encode)
	PHP_FE(crypt_decode, arginfo_crypt_decode)
	PHP_FE(ssp_msg_queue_init, arginfo_ssp_msg_queue_init)
	PHP_FE(ssp_msg_queue_push, arginfo_ssp_msg_queue_push)
	PHP_FE(ssp_msg_queue_destory, arginfo_ssp_msg_queue_destory)
	PHP_FE(ssp_delayed_init, arginfo_ssp_delayed_init)
	PHP_FE(ssp_delayed_set, arginfo_ssp_delayed_set)
	PHP_FE(ssp_delayed_del, arginfo_ssp_delayed_del)
	PHP_FE(ssp_delayed_destory, arginfo_ssp_delayed_destory)
	PHP_FE_END
};
/* }}} */

/* {{{ ssp_module_entry
 */
zend_module_entry ssp_module_entry = {
	STANDARD_MODULE_HEADER,
	"ssp",
	ssp_functions,
	PHP_MINIT(ssp), // module_startup_func
	PHP_MSHUTDOWN(ssp), // module_shutdown_func
	PHP_RINIT(ssp), // request_startup_func
	PHP_RSHUTDOWN(ssp), // request_shutdown_func
	PHP_MINFO(ssp), // info_func
	PHP_SSP_VERSION,
	PHP_MODULE_GLOBALS(ssp), // globals_id_ptr / globals_ptr
	PHP_GINIT(ssp), // globals_ctor
	PHP_GSHUTDOWN(ssp), // globals_dtor
	NULL, // post_deactivate_func
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

#define __NULL (void)0

#define SERIALIZE(z,ok) SERIALIZE_EX(z,__NULL,ok,__NULL,__NULL)
#define SERIALIZE_EX(z,r1,ok,r2,r3) \
	do { \
		php_serialize_data_t var_hash; \
		smart_str buf = {0}; \
		PHP_VAR_SERIALIZE_INIT(var_hash); \
		php_var_serialize(&buf, z, &var_hash); \
		PHP_VAR_SERIALIZE_DESTROY(var_hash); \
		if (EG(exception)) { \
			smart_str_free(&buf); \
			r1; \
		} else if (buf.s) { \
			ok; \
			smart_str_free(&buf); \
			r2; \
		} else { \
			r3; \
		} \
	} while(0)

#define UNSERIALIZE(s,l,ok) UNSERIALIZE_EX(s,l,__NULL,ok)
#define UNSERIALIZE_EX(s,l,r,ok) \
	do { \
		php_unserialize_data_t var_hash; \
		char *buf = s; \
		const unsigned char *p = buf; \
		size_t buf_len = l; \
		PHP_VAR_UNSERIALIZE_INIT(var_hash); \
		zval *retval = var_tmp_var(&var_hash); \
		if(!php_var_unserialize(retval, &p, p + buf_len, &var_hash)) { \
			if (!EG(exception)) { \
				php_error_docref(NULL, E_NOTICE, "Error at offset " ZEND_LONG_FMT " of %zd bytes", (zend_long)((char*)p - buf), buf_len); \
			} \
			r; \
		} else { \
			ok; \
		} \
		PHP_VAR_UNSERIALIZE_DESTROY(var_hash); \
	} while(0)

#ifdef SSP_DEBUG_PRINTF
static void php_destroy_ssp(zend_resource *rsrc) /* {{{ */
{
	conn_t *ptr = (conn_t *) rsrc->ptr;

	conn_info(ptr);
}
#else
#define php_destroy_ssp NULL
#endif

/* {{{ MINIT */
static PHP_MINIT_FUNCTION(ssp)
{
	REGISTER_STRING_CONSTANT("SSP_VERSION", PHP_SSP_VERSION, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("SSP_RES_INDEX", PHP_SSP_RES_INDEX, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_RES_SOCKFD", PHP_SSP_RES_SOCKFD, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_RES_PORT", PHP_SSP_RES_PORT, CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("SETUP_USERNAME", SETUP_USERNAME, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SETUP_SENDKEY", SETUP_SENDKEY, CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SETUP_RECEIVEKEY", SETUP_RECEIVEKEY, CONST_CS | CONST_PERSISTENT);

	le_ssp_descriptor = zend_register_list_destructors_ex(php_destroy_ssp, NULL, PHP_SSP_DESCRIPTOR_RES_NAME, module_number);

	pthread_mutex_init(&unique_lock, NULL);
	pthread_mutex_init(&counts_lock, NULL);

	zend_register_auto_global(zend_string_init_interned("_SSP", sizeof("_SSP") - 1, 1), 0, NULL);

#ifdef SSP_DEBUG_EXT
	printf("module startup function for %s\n", __func__);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
static PHP_MSHUTDOWN_FUNCTION(ssp)
{
	zend_hash_str_del(EG(zend_constants), "SSP_VERSION", sizeof("SSP_VERSION") - 1);

	zend_hash_str_del(EG(zend_constants), "SSP_RES_INDEX", sizeof("SSP_RES_INDEX") - 1);
	zend_hash_str_del(EG(zend_constants), "SSP_RES_SOCKFD", sizeof("SSP_RES_SOCKFD") - 1);
	zend_hash_str_del(EG(zend_constants), "SSP_RES_PORT", sizeof("SSP_RES_PORT") - 1);

	zend_hash_str_del(EG(zend_constants), "SETUP_USERNAME", sizeof("SETUP_USERNAME") - 1);
	zend_hash_str_del(EG(zend_constants), "SETUP_SENDKEY", sizeof("SETUP_SENDKEY") - 1);
	zend_hash_str_del(EG(zend_constants), "SETUP_RECEIVEKEY", sizeof("SETUP_RECEIVEKEY") - 1);

	pthread_mutex_destroy(&unique_lock);
	pthread_mutex_destroy(&counts_lock);

#ifdef SSP_DEBUG_EXT
	printf("module shutdown function for %s\n", __func__);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(ssp)
{
	ssp_auto_globals_recreate();

#ifdef SSP_DEBUG_EXT
	printf("request startup function for %s\n", __func__);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(ssp)
{
	zend_hash_str_del(&EG(symbol_table), "_SSP", sizeof("_SSP") - 1);
	zval_ptr_dtor(&SSP_G(ssp_vars));
	ZVAL_UNDEF(&SSP_G(ssp_vars));

#ifdef SSP_DEBUG_EXT
	printf("request shutdown function for %s\n", __func__);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_GINIT_FUNCTION
 */
static PHP_GINIT_FUNCTION(ssp)
{
	ssp_globals->trigger_type = -1;
	ssp_globals->trigger_count = 0;
	ZVAL_UNDEF(&ssp_globals->ssp_vars);

#ifdef SSP_DEBUG_EXT
	printf("globals constructor function for %s\n", __func__);
#endif
}
/* }}} */

/* {{{ PHP_GSHUTDOWN_FUNCTION */
static PHP_GSHUTDOWN_FUNCTION(ssp)
{
#ifdef SSP_DEBUG_EXT
	printf("globals destructor function for %s\n", __func__);
#endif
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
static PHP_MINFO_FUNCTION(ssp)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "ssp support", "enabled");
	php_info_print_table_row(2, "ssp version", PHP_SSP_VERSION);
	php_info_print_table_end();
}

void ssp_auto_globals_recreate()
{
	if (!Z_ISUNDEF(SSP_G(ssp_vars))) {
		zval_ptr_dtor(&SSP_G(ssp_vars));
		ZVAL_UNDEF(&SSP_G(ssp_vars));
	}

	array_init_size(&SSP_G(ssp_vars), ssp_vars_length);
	Z_ADDREF_P(&SSP_G(ssp_vars));
	zend_hash_str_update(&EG(symbol_table), "_SSP", sizeof("_SSP") - 1, &SSP_G(ssp_vars));
}

bool trigger(unsigned short type, ...) {
	TRIGGER_STARTUP();
	if (trigger_handlers[type] == NULL) {
		TRIGGER_SHUTDOWN();
		return FAILURE;
	}
	zval pfunc, retval;
	zval *params = NULL;
	int i, param_count = 0, ret;
	bool retbool = true;
	va_list args;
	conn_t *ptr = NULL;

	ZVAL_STRING(&pfunc, trigger_handlers[type]);

	va_start(args, type);
	switch (type) {
		case PHP_SSP_START:
		case PHP_SSP_STOP:
		case PHP_SSP_BENCH:
			break;
		case PHP_SSP_CONNECT:
		case PHP_SSP_CONNECT_DENIED:
		case PHP_SSP_CLOSE:
			param_count = 1;
			ptr = va_arg(args, conn_t*);
			params = (zval *) emalloc(sizeof(zval) * param_count);
			ZEND_REGISTER_RESOURCE(&params[0], ptr, le_ssp_descriptor);
			break;
		case PHP_SSP_RECEIVE:
			param_count = 2;
			ptr = va_arg(args, conn_t*);
			params = (zval *) emalloc(sizeof(zval) * param_count);
			ZEND_REGISTER_RESOURCE(&params[0], ptr, le_ssp_descriptor);
			{
				char *s;
				int l;
				s = va_arg(args, char*);
				l = va_arg(args, int);
				ZVAL_STRINGL(&params[1], s, l);
			}
			break;
		case PHP_SSP_SEND:
			param_count = 2;
			params = (zval *) emalloc(sizeof(zval) * param_count);
			ptr = va_arg(args, conn_t*);
			ZEND_REGISTER_RESOURCE(&params[0], ptr, le_ssp_descriptor);
			ZVAL_COPY(&params[1], va_arg(args, zval*));
			break;
		default:
			perror("Trigger type not exists!");
			va_end(args);
			TRIGGER_SHUTDOWN();
			return FAILURE;
	}
	va_end(args);

#ifdef SSP_DEBUG_TRIGGER
	printf("before trigger: %s\n", trigger_handlers[type]);
#endif

	ZVAL_UNDEF(&retval);

	long __type = SSP_G(trigger_type);
	SSP_G(trigger_type) = type;
	ret = call_user_function(EG(function_table), NULL, &pfunc, &retval, param_count, params);
	SSP_G(trigger_type) = __type;

#ifdef SSP_DEBUG_TRIGGER
	printf("after trigger: %s,%d\n", trigger_handlers[type], ret == SUCCESS);
#endif

	if (ret == SUCCESS) {
		// php_printf("%s: ", trigger_handlers[type]);php_var_dump(&retval, 1);fflush(stdout);
		if (Z_TYPE_P(&retval) == IS_FALSE) {
			retbool = false;
		}
		if (param_count > 1 && Z_TYPE(retval) != IS_NULL) {
			if(type == PHP_SSP_RECEIVE) {
				retbool = trigger(PHP_SSP_SEND, ptr, &retval);
			} else { // type == PHP_SSP_SEND
				if(Z_TYPE(retval) != IS_STRING) php_printf("function %s return value not is string type\n", trigger_handlers[type]);
				retbool = (Z_TYPE(retval) == IS_STRING && Z_STRLEN(retval)>0) ? socket_send(ptr, Z_STRVAL(retval), Z_STRLEN(retval)) > 0 : false;
			}
		}
	} else {
		php_printf("\nUnable to call handler(%s)\n", trigger_handlers[type]);
	}
	if (param_count > 0) {
		int i;
		for (i = 0; i < param_count; i++) {
			zval_ptr_dtor(&params[i]);
			ZVAL_UNDEF(&params[i]);
		}
		efree(params);
	}

	zval_ptr_dtor(&retval);
	ZVAL_UNDEF(&retval);

	zval_ptr_dtor(&pfunc);
	ZVAL_UNDEF(&pfunc);

	TRIGGER_SHUTDOWN();

	return retbool;
}

static PHP_FUNCTION(ssp_info) {
	zval *res;
	conn_t *ptr;
	char *key = NULL;
	size_t key_len = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r|s", &res, &key, &key_len) == FAILURE) {
		return;
	}
	ptr = (conn_t *) zend_fetch_resource_ex(res, PHP_SSP_DESCRIPTOR_RES_NAME, le_ssp_descriptor);
	if(!ptr) RETURN_NULL();
	if (key_len == 0) {
		array_init_size(return_value, 5);
		add_assoc_long(return_value, "index", ptr->index+1);
		add_assoc_long(return_value, "sockfd", ptr->sockfd);
		add_assoc_string(return_value, "host", ptr->host);
		add_assoc_long(return_value, "port", ptr->port);
		add_assoc_long(return_value, "tid", ptr->thread->id);
		if(listen_thread.sockfd < 0) add_assoc_long(return_value, "type", ptr->type);
	} else {
		if (!strcasecmp(key, "index")) {
			RETURN_LONG(ptr->index+1);
		} else if (!strcasecmp(key, "sockfd")) {
			RETURN_LONG(ptr->sockfd);
		} else if (!strcasecmp(key, "host")) {
			ZVAL_STRING(return_value, ptr->host);
		} else if (!strcasecmp(key, "port")) {
			RETURN_LONG(ptr->port);
		} else if (!strcasecmp(key, "tid")) {
			RETURN_LONG(ptr->thread->id);
		} else if (!strcasecmp(key, "type")) {
			RETURN_LONG(ptr->type);
		} else {
			RETURN_NULL();
		}
	}
}

static PHP_FUNCTION(ssp_connect)
{
	char *host;
	size_t host_len;
	zend_long port = ssp_port;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "s|l", &host, &host_len, &port) == FAILURE) {
		return;
	}

	if(listen_thread.sockfd < 0 && CONN_NUM < ssp_maxclients) {
		conn_t *ptr;
		int s;
		struct sockaddr_in addr;
		bzero(&addr, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port=htons(port);
		addr.sin_addr.s_addr = inet_addr(host);

		if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
			perror("socket");
			RETURN_FALSE;
		}

		if(connect(s, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
			perror("connect");
			close(s);
			RETURN_FALSE;
		}

		socket_set_connect(s,SOCKET_SNDTIMEO,SOCKET_RCVTIMEO,SOCKET_SNDBUF,SOCKET_RCVBUF);

		ptr = insert_conn();
		ptr->sockfd = s;
		strcpy(ptr->host, ssp_host);
		ptr->port = ssp_port;

		ptr->thread = worker_threads + ptr->index % ssp_nthreads;

		dprintf("notify thread %d\n", ptr->thread->id);

		queue_push(ptr->thread->accept_queue, ptr);

		conn_info(ptr);

		char chr='l';
		write(ptr->thread->write_fd, &chr, 1);

		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}

}

void ssp_conv_disconnect(server_t *s) {
	if(s->sockfd < 0) return;

	vprintf("%s: %d, %d, %d, %d close\n", s->host, s->port, s->sid, ssp_server_id, ssp_server_max);

	shutdown(s->sockfd, SHUT_RDWR);
	close(s->sockfd);
	if(s->event.ev_base) event_del(&s->event);

	if(s->rbuf) {
		free(s->rbuf);
	}

#if ASYNC_SEND
	if(s->wbuf) {
		free(s->wbuf);
	}
#endif

	memset(s, 0, sizeof(server_t));

	s->sockfd = -1;
}

#define SID_CONDITION(sid) (sid >= 0 && sid < ssp_server_max && sid != ssp_server_id && servers[sid].sockfd >= 0)

static void conv_socket_send(int sid, int id, char *buf, int size) {
	if(size <= 0) return;

	server_t *s = &servers[sid];
	conv_server_t *c = (conv_server_t*) malloc(sizeof(conv_server_t) + size);

	c->ptr = s;
	c->index = id;
	c->size = size;
	memcpy(c->buf, buf, size);
	c->buf[size] = '\0';

	vprintf("sid: %d, id: %d, size: %d, buf: %s\n", sid, id, size, buf);

#if ASYNC_SEND
	queue_push(ssp_server_queue, c);

	write(listen_thread.write_fd, "s", 1);
#else
	size_t plen = size + sizeof(int)*2;
	int n = send(s->sockfd, &c->index, plen, MSG_WAITALL);
	if(n != plen) {
		ssp_conv_disconnect(s);
	}

	free(c);
#endif
}

static void conv_timeout_handler(int sock, short event, void *arg) {
	int i;
	for(i=0; i<ssp_server_max; i++) if(SID_CONDITION(i)) conv_socket_send(i, 0, "w", 1);
}

static PHP_FUNCTION(ssp_conv_setup)
{
	if(SSP_G(trigger_type) != PHP_SSP_START) {
		php_printf("The ssp_conv_setup function can only be executed in the ssp_start_handler function\n");
		return;
	}

	zend_long sid, max_sid;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &sid, &max_sid) == FAILURE || sid < 0 || max_sid <= 0 || servers) {
		return;
	}

	vprintf("sid: %ld, max_sid: %ld\n", sid, max_sid);

	ssp_server_id = sid;
	ssp_server_max = max_sid;

	size_t n = sizeof(server_t)*max_sid;
	servers = (server_t*) malloc(n);
	memset(servers, 0, n);

	for(n=0; n<max_sid; n++) {
		servers[n].sockfd = -1;
	}

	ssp_server_queue = queue_init();

	struct timeval tv;
	evutil_timerclear(&tv);
	tv.tv_sec = 10;
	event_set(&conv_timeout_int, -1, EV_PERSIST, conv_timeout_handler, NULL);
	event_base_set(listen_thread.base, &conv_timeout_int);
	if (event_add(&conv_timeout_int, &tv) == -1) {
		perror("timeout event");
		exit(1);
	}
}

#if ASYNC_SEND
static void conv_read_write_handler(int sock, short event, void *arg);
void is_writable_conv(server_t *ptr, bool iswrite) {
	if(event_del(&ptr->event) == -1) perror("event_del");

	if(iswrite) event_set(&ptr->event, ptr->sockfd, EV_READ|EV_WRITE|EV_PERSIST, conv_read_write_handler, ptr);
	else event_set(&ptr->event, ptr->sockfd, EV_READ|EV_PERSIST, conv_read_write_handler, ptr);

	event_base_set(listen_thread.base, &ptr->event);
	event_add(&ptr->event, NULL);
}
#endif

static void conv_read_write_handler(int sock, short event, void *arg)
{
	server_t *ptr = arg;
	int n = 0, ret;

#if ASYNC_SEND
	if(event & EV_WRITE) {
		ret = send(ptr->sockfd, ptr->wbuf+ptr->wbytes, ptr->wsize - ptr->wbytes, MSG_DONTWAIT);
		if(ret >= 0) {
			ptr->wbytes += ret;
			if(ptr->wsize == ptr->wbytes) {
				free(ptr->wbuf);
				ptr->wbuf = NULL;
				ptr->wbytes = ptr->wsize = 0;

				is_writable_conv(ptr, false);
			}
		} else {
			// fprintf(stderr, "%s: send error\n", __func__);
			ssp_conv_disconnect(ptr);
			return;
		}
	}
	if(event & EV_READ) {
#endif
	if(ptr->rsize == 0) {
		n = sizeof(int)*2;
		ret = recv(ptr->sockfd, &ptr->index, n, MSG_WAITALL);
		if(ret <= 0) {
			// fprintf(stderr, "%s: recv package head error %d\n", __func__, ssp_server_id);

			ssp_conv_disconnect(ptr);
		} else if(n != ret) {
			fprintf(stderr, "%s: recv package head length error, %d\n", __func__, ret);

			ssp_conv_disconnect(ptr);
		} else {
			ptr->rbuf = (char*) malloc(ptr->rsize+1);
			ptr->rbytes = 0;
			ptr->rbuf[ptr->rsize] = '\0';
		}
	} else {
		ret = recv(ptr->sockfd, ptr->rbuf + ptr->rbytes, ptr->rsize - ptr->rbytes, MSG_DONTWAIT);
		if(ret <= 0) {
			// fprintf(stderr, "%s: recv package data error\n", __func__);

			ssp_conv_disconnect(ptr);
		} else {
			ptr->rbytes += ret;

			if(ptr->rsize != ptr->rbytes) return;

			vprintf("conv recv: %s\n", ptr->rbuf);

			if(ptr->rsize == 1 && ptr->rbuf[0] == 'w') {
				vprintf("conv keep alive: %d, %d\n", ptr->sid, ssp_server_id);
				goto end;
			}

			conn_t *c = index_conn(ptr->index);
			if(c) {
				if(ptr->rsize == 1 && ptr->rbuf[0] == 'x') {
					socket_close(c);
				} else {
					UNSERIALIZE(ptr->rbuf, ptr->rsize, trigger(PHP_SSP_SEND, c, retval));
				}

				unref_conn(c);
			}

			end:
			free(ptr->rbuf);
			ptr->rbuf = NULL;
			ptr->rbytes = 0;
			ptr->rsize = 0;
		}
	}
#if ASYNC_SEND
	}
#endif
}

static void conv_listen_handler(int sock, short event, void *arg)
{
	server_t *ptr = arg;
	struct sockaddr_in pin;
	socklen_t len = sizeof(pin);
	int s , n = -1;
	if((s = accept(sock, (struct sockaddr *)&pin, &len)) < 0) {
		perror("accept");
		return;
	}

	socket_set_accept(s,SOCKET_CONV_SNDTIMEO,SOCKET_CONV_RCVTIMEO,SOCKET_CONV_SNDBUF,SOCKET_CONV_RCVBUF);

	if(recv(s, &n, sizeof(int), MSG_WAITALL) != sizeof(int)) {
		perror("recv lenght no equal 4");
		return;
	}

	if(n == ptr->sid || n < 0 || n >= ssp_server_max || servers[n].sockfd >= 0) {
		fprintf(stderr, "conv argument error: %d, %d, %d\n", n, ssp_server_id, ssp_server_max);
		close(s);
		return;
	}

	ptr = &servers[n];

	inet_ntop(AF_INET, &pin.sin_addr, ptr->host, len);
	ptr->port = ntohs(pin.sin_port);

	ptr->sockfd = s;

	event_set(&ptr->event, ptr->sockfd, EV_READ|EV_PERSIST, conv_read_write_handler, ptr);
	event_base_set(listen_thread.base, &ptr->event);
	event_add(&ptr->event, NULL);
}

static PHP_FUNCTION(ssp_conv_connect)
{
	if(SSP_G(trigger_type) != PHP_SSP_START) {
		php_printf("The ssp_conv_connect function can only be executed in the ssp_start_handler function\n");
		return;
	}

	char *host;
	size_t host_len;
	zend_long port, sid;
	server_t *ptr;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "sll", &host, &host_len, &port, &sid) == FAILURE || sid < 0 || sid >= ssp_server_max) {
		return;
	}

	ptr = &servers[sid];
	ptr->sid = sid;

	strncpy(ptr->host, host, host_len);
	ptr->port = port;
	ptr->sockfd = -1;

	int s;
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(host);

	if((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		RETURN_FALSE;
	}

	if(sid == ssp_server_id) {
		socket_set_listen(s);

		if(bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
			perror("bind");
			close(s);
			RETURN_FALSE;
		}
		if(listen(s, ssp_backlog) < 0) {
			perror("listen");
			close(s);
			RETURN_FALSE;
		}

		vprintf("%s: %ld, %ld listen\n", host, port, sid);

		ptr->sockfd = s;

		event_set(&ptr->event, ptr->sockfd, EV_READ|EV_PERSIST, conv_listen_handler, ptr);
		event_base_set(listen_thread.base, &ptr->event);
		event_add(&ptr->event, NULL);
	} else {
		if(connect(s, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
			//perror("connect");
			close(s);
			RETURN_FALSE;
		}
		socket_set_connect(s,SOCKET_CONV_SNDTIMEO,SOCKET_CONV_RCVTIMEO,SOCKET_CONV_SNDBUF,SOCKET_CONV_RCVBUF);
		if(send(s, &ssp_server_id, sizeof(int), MSG_WAITALL) != sizeof(int)) {
			perror("send");
			close(s);
			RETURN_FALSE;
		}

		vprintf("%s: %ld, %ld connect\n", host, port, sid);

		ptr->sockfd = s;

		event_set(&ptr->event, ptr->sockfd, EV_READ|EV_PERSIST, conv_read_write_handler, ptr);
		event_base_set(listen_thread.base, &ptr->event);
		event_add(&ptr->event, NULL);
	}
}

static PHP_FUNCTION(ssp_conv_disconnect)
{
	int i;
	if(!servers) return;

	if(SSP_G(trigger_type) != PHP_SSP_STOP) {
		php_printf("The ssp_conv_disconnect function can only be executed in the ssp_stop_handler function\n");
		return;
	}

	for(i=0; i<ssp_server_max; i++) {
		ssp_conv_disconnect(&servers[i]);
	}

	free(servers);

	ssp_server_id = ssp_server_max = -1;
	servers = NULL;

	conv_server_t *c;

	while((c = queue_pop(ssp_server_queue)) != NULL) {
		free(c);
	}

	queue_free(ssp_server_queue);
}

static PHP_FUNCTION(ssp_send)
{
	zval *res, *data, *sid = NULL;
	conn_t *ptr = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "zz|z", &res, &data, &sid) == FAILURE) {
		return;
	}

	if(Z_TYPE_P(res) == IS_RESOURCE) {
		ptr = (conn_t *) zend_fetch_resource_ex(res, PHP_SSP_DESCRIPTOR_RES_NAME, le_ssp_descriptor);
	} else {
		convert_to_long_ex(res);
		if(sid) {
			convert_to_long_ex(sid);
			zend_long i = Z_LVAL_P(sid);
			if(SID_CONDITION(i)) {
				SERIALIZE_EX(data,RETVAL_FALSE,conv_socket_send(i, Z_LVAL_P(res)-1, ZSTR_VAL(buf.s), ZSTR_LEN(buf.s)),RETVAL_TRUE,RETVAL_NULL());
				return;
			} else if(i != ssp_server_id) {
				RETURN_FALSE;
			}
		}
		ptr = index_conn(Z_LVAL_P(res)-1);
	}

	if (ptr) {
		bool b = trigger(PHP_SSP_SEND, ptr, data);

		if(Z_TYPE_P(res) == IS_LONG) unref_conn(ptr);

		RETURN_BOOL(b);
	} else {
		RETURN_NULL();
	}
}

static PHP_FUNCTION(ssp_close)
{
	zval *res, *sid = NULL;
	conn_t *ptr = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|z", &res, &sid) == FAILURE) {
		return;
	}

	if(Z_TYPE_P(res) == IS_RESOURCE) {
		ptr = (conn_t *) zend_fetch_resource_ex(res, PHP_SSP_DESCRIPTOR_RES_NAME, le_ssp_descriptor);
	} else {
		convert_to_long_ex(res);
		if(sid) {
			convert_to_long_ex(sid);
			zend_long i = Z_LVAL_P(sid);
			if(SID_CONDITION(i)) {
				conv_socket_send(i, Z_LVAL_P(res)-1, "x", 1);
				RETURN_TRUE;
			} else if(i != ssp_server_id) {
				RETURN_FALSE;
			}
		}
		ptr = index_conn(Z_LVAL_P(res)-1);
	}

	if (ptr) {
		socket_close(ptr);

		if(Z_TYPE_P(res) == IS_LONG) unref_conn(ptr);

		RETURN_TRUE;
	} else {
		RETURN_NULL();
	}
}

static PHP_FUNCTION(ssp_lock)
{
	pthread_mutex_lock(&unique_lock);
}
static PHP_FUNCTION(ssp_unlock)
{
	pthread_mutex_unlock(&unique_lock);
}

static PHP_FUNCTION(ssp_stats)
{
	long sleep_time = 100000;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|lb", &sleep_time) == FAILURE) {
		return;
	}
	if (sleep_time <= 0) {
		sleep_time = 100000;
	}

#ifdef HAVE_LIBGTOP
	glibtop_cpu cpu_begin,cpu_end; /////////////////////////////
	glibtop_proc_time proctime_begin,proctime_end;///Declare the CPU info and
	glibtop_mem memory;///memory info struct
	glibtop_proc_mem procmem;///////////////////////////////

	double us,ni,sy,id;
	int dpu,dps;
	double cpurate,memrate,scale;

	static int pid=0;
	if(pid == 0) {
		pid = getpid();
	}

	glibtop_get_cpu (&cpu_begin);
	glibtop_get_proc_time(&proctime_begin,pid);

	usleep(sleep_time); // 1s=1000000

	glibtop_get_cpu (&cpu_end);
	glibtop_get_proc_time(&proctime_end,pid);

	us = cpu_end.user - cpu_begin.user;
	ni = cpu_end.nice - cpu_begin.nice;
	sy = cpu_end.sys - cpu_begin.sys;
	id = cpu_end.idle - cpu_begin.idle;

	dpu = proctime_end.utime - proctime_begin.utime;
	dps = proctime_end.stime - proctime_begin.stime;

	scale = 100.0/(cpu_end.total - cpu_begin.total);

	glibtop_get_mem(&memory);
	glibtop_get_proc_mem(&procmem,pid);

	array_init_size(return_value,2);

	zval *sysinfo;
	MAKE_STD_ZVAL(sysinfo);
	array_init_size(sysinfo,27);

	add_assoc_double(sysinfo,"us", us*scale);
	add_assoc_double(sysinfo,"ni", ni*scale);
	add_assoc_double(sysinfo,"sy", sy*scale);
	add_assoc_double(sysinfo,"id", id*scale);

	add_assoc_long(sysinfo,"memTotal", memory.total);
	add_assoc_long(sysinfo,"memUsed", memory.used);
	add_assoc_long(sysinfo,"memFree", memory.free);
	add_assoc_long(sysinfo,"memShared", memory.shared);
	add_assoc_long(sysinfo,"memBuffer", memory.buffer);
	add_assoc_long(sysinfo,"memCached", memory.cached);
	add_assoc_long(sysinfo,"memUser", memory.user);
	add_assoc_long(sysinfo,"memLocked", memory.locked);

	add_assoc_zval(return_value, "sysinfo", sysinfo);

	zval *procinfo;
	MAKE_STD_ZVAL(procinfo);
	array_init_size(procinfo,74);

	add_assoc_double(procinfo,"pcpu", (dpu+dps)*scale);

	add_assoc_long(procinfo,"size",procmem.size); /* total # of pages of memory */
	add_assoc_long(procinfo,"vsize",procmem.vsize); /* number of pages of virtual memory ... */
	add_assoc_long(procinfo,"resident",procmem.resident); /* number of resident set (non-swapped) pages (4k) */
	add_assoc_long(procinfo,"share",procmem.share); /* number of pages of shared (mmap'd) memory */
	add_assoc_long(procinfo,"rss",procmem.rss); /* resident set size */
	add_assoc_long(procinfo,"rss_rlim",procmem.rss_rlim); /* current limit (in bytes) of the rss of the process; usually 2,147,483,647 */

	add_assoc_zval(return_value, "procinfo", procinfo);
#else
	RETURN_FALSE;
#endif
}

static PHP_FUNCTION(ssp_type) {
	zval *res;
	conn_t *ptr;
	zend_long type = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rl", &res, &type) == FAILURE) {
		return;
	}
	ptr = (conn_t *) zend_fetch_resource_ex(res, PHP_SSP_DESCRIPTOR_RES_NAME, le_ssp_descriptor);
	if(!ptr) RETURN_NULL();
	ptr->type = type;
}

static PHP_FUNCTION(ssp_requests) {
	zval *res;
	conn_t *ptr;
	zend_long type = 1;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r|l", &res, &type) == FAILURE) {
		return;
	}
	ptr = (conn_t *) zend_fetch_resource_ex(res, PHP_SSP_DESCRIPTOR_RES_NAME, le_ssp_descriptor);
	if(!ptr) RETURN_NULL();

	ptr->requests += type;

	RETURN_LONG(ptr->requests);
}

unsigned int counts[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

static PHP_FUNCTION(ssp_counts) {
	zend_long key = 0, type = -1;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|l", &key, &type) == FAILURE || key < 0 || key >= 16) {
		return;
	}

	pthread_mutex_lock(&counts_lock);
	if(type <= -3) {
		RETVAL_LONG(counts[key]);
	} else if(type == -2) {
		RETVAL_LONG(--counts[key]);
	} else if(type == -1) {
		RETVAL_LONG(++counts[key]);
	} else if(type == 0) {
		RETVAL_LONG(counts[key]);
		counts[key] = 0;
	} else {
		if(++counts[key] == type) {
			counts[key] = 0;
			RETVAL_TRUE;
		} else {
			RETVAL_FALSE;
		}
	}
	pthread_mutex_unlock(&counts_lock);
}

static PHP_FUNCTION(ssp_setup) {
	zval *res;
	conn_t *ptr;
	zend_long type = 0;

	char *str = NULL;
	size_t str_len = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rl|s", &res, &type, &str, &str_len) == FAILURE) {
		return;
	}
	ptr = (conn_t *) zend_fetch_resource_ex(res, PHP_SSP_DESCRIPTOR_RES_NAME, le_ssp_descriptor);
	if(!ptr) RETURN_NULL();

	if(str) {
		switch(type) {
			case SETUP_USERNAME:
				strncpy(ptr->username, str, str_len);
				break;
			case SETUP_SENDKEY:
				strncpy(ptr->sendKey, str, str_len);
				break;
			case SETUP_RECEIVEKEY:
				strncpy(ptr->receiveKey, str, str_len);
				break;
			default:
				RETURN_FALSE;
				break;
		}

		RETURN_TRUE;
	} else {
		switch(type) {
			case SETUP_USERNAME:
				RETURN_STRING(ptr->username);
				break;
			case SETUP_SENDKEY:
				RETURN_STRING(ptr->sendKey);
				break;
			case SETUP_RECEIVEKEY:
				RETURN_STRING(ptr->receiveKey);
				break;
			default:
				RETURN_FALSE;
				break;
		}
	}
}

static PHP_FUNCTION(crypt_encode) {
	zend_long expiry = 0;
	char *str = NULL, *key = NULL;
	size_t str_len = 0, key_len = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|l", &str, &str_len, &key, &key_len, &expiry) == FAILURE) {
		return;
	}

	char *enc = NULL;

	key_len = crypt_code(str, str_len, &enc, key, false, expiry);

	if(enc && key_len) {
		RETVAL_STRINGL(enc, key_len);
		free(enc);
	}
}

static PHP_FUNCTION(crypt_decode) {
	zend_long expiry = 0;
	char *enc = NULL, *key = NULL;
	size_t enc_len = 0, key_len = 0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "ss|l", &enc, &enc_len, &key, &key_len, &expiry) == FAILURE) {
		return;
	}

	char *dec = NULL;

	key_len = crypt_code(enc, enc_len, &dec, key, true, expiry);

	if(dec && key_len) {
		RETVAL_STRINGL(dec, key_len);
		free(dec);
	}
}

static long ssp_msg_queue_max_msgs = -1;
static long ssp_msg_queue_nthreads = -1;
static long ssp_msg_queue_running = 0;
static bool ssp_msg_queue_stop = false;
static long ssp_msg_queue_msgs = 0;
static queue_t *ssp_msg_queue = NULL;
static pthread_mutex_t ssp_msg_queue_lock;
static pthread_cond_t ssp_msg_queue_cond;

#define MSG_PARAM_COUNT 7
typedef struct _ssp_msg_t {
	char func[128];
	long what;
	long arg1;
	long arg2;
	long arg3;
	long arg4;
	long arg5;
	int arglen;
	char arg[1];
} ssp_msg_t;

static void *ssp_msg_queue_handler(void *arg) {
	(void)arg;
	ssp_msg_t *msg;
	pthread_t tid = pthread_self();
	zval pfunc, rv;
	zval params[MSG_PARAM_COUNT];
	int i;

	pthread_mutex_lock(&ssp_msg_queue_lock);
	ssp_msg_queue_running++;
	pthread_cond_signal(&ssp_msg_queue_cond);
	pthread_mutex_unlock(&ssp_msg_queue_lock);

	THREAD_STARTUP();

	for(;;) {
		msg = NULL;
		pthread_mutex_lock(&ssp_msg_queue_lock);
		while(!ssp_msg_queue_msgs && !ssp_msg_queue_stop) pthread_cond_wait(&ssp_msg_queue_cond, &ssp_msg_queue_lock);
		if(ssp_msg_queue_stop) {
			pthread_cond_signal(&ssp_msg_queue_cond);
		} else {
			msg = queue_pop(ssp_msg_queue);
			if(msg) {
				ssp_msg_queue_msgs--;
			}
		}
		pthread_mutex_unlock(&ssp_msg_queue_lock);

		if(ssp_msg_queue_stop) break;
		if(!msg) continue;

		dprintf("msgs: %s, %ld, %s(%d), %ld, %ld, %ld, %ld, %ld\n", msg->func, msg->what, msg->arg, msg->arglen, msg->arg1, msg->arg2, msg->arg3, msg->arg4, msg->arg5);

		MSG_QUEUE_STARTUP();

		ZVAL_STRING(&pfunc, msg->func);
		ZVAL_LONG(&params[0], msg->what);
		ZVAL_NULL(&params[1]);
		if(msg->arglen) {
			UNSERIALIZE(msg->arg, msg->arglen, ZVAL_COPY(&params[1], retval));
		}
		ZVAL_LONG(&params[2], msg->arg1);
		ZVAL_LONG(&params[3], msg->arg2);
		ZVAL_LONG(&params[4], msg->arg3);
		ZVAL_LONG(&params[5], msg->arg4);
		ZVAL_LONG(&params[6], msg->arg5);

		call_user_function(EG(function_table), NULL, &pfunc, &rv, MSG_PARAM_COUNT, params);

		zval_ptr_dtor(&pfunc);
		zval_ptr_dtor(&rv);

		for (i = 0; i < MSG_PARAM_COUNT; i++) {
			zval_ptr_dtor(&params[i]);
		}

		free(msg);

		MSG_QUEUE_SHUTDOWN();
	}

	THREAD_SHUTDOWN();

	pthread_mutex_lock(&ssp_msg_queue_lock);
	ssp_msg_queue_running--;
	pthread_cond_signal(&ssp_msg_queue_cond);
	pthread_mutex_unlock(&ssp_msg_queue_lock);

	pthread_detach(tid);
	pthread_exit(NULL);

	return NULL;
}

static PHP_FUNCTION(ssp_msg_queue_init)
{
	if(SSP_G(trigger_type) != PHP_SSP_START) {
		php_printf("The ssp_msg_queue_init function can only be executed in the ssp_start_handler function\n");
		return;
	}

	zend_long msgs, nthreads;
	if (ssp_msg_queue_running || ssp_msg_queue || zend_parse_parameters(ZEND_NUM_ARGS(), "ll", &msgs, &nthreads) == FAILURE || msgs <= 0 || msgs > MSG_QUEUE_MAX_MSGS || nthreads <= 0 || nthreads > MSG_QUEUE_NTHREADS) {
		return;
	}

	ssp_msg_queue_max_msgs = msgs;
	ssp_msg_queue_nthreads = nthreads;
	ssp_msg_queue = queue_init();

	pthread_mutex_init(&ssp_msg_queue_lock, NULL);
	pthread_cond_init(&ssp_msg_queue_cond, NULL);

	int i;
	for(i=0; i<nthreads; i++) {
		worker_create(ssp_msg_queue_handler, NULL);
	}

	pthread_mutex_lock(&ssp_msg_queue_lock);
	while (ssp_msg_queue_running < nthreads) {
		pthread_cond_wait(&ssp_msg_queue_cond, &ssp_msg_queue_lock);
	}
	pthread_mutex_unlock(&ssp_msg_queue_lock);
}

static PHP_FUNCTION(ssp_msg_queue_push)
{
	char *func;
	size_t funclen;
	zend_long what = 0;
	zval *arg = NULL;
	zend_long arg1=0,arg2=0,arg3=0,arg4=0,arg5=0;
	ssp_msg_t *msg = NULL;

	if(zend_parse_parameters(ZEND_NUM_ARGS(), "slz|lllll", &func, &funclen, &what, &arg, &arg1, &arg2, &arg3, &arg4, &arg5) == FAILURE) return;
	if(funclen >= 128) {
		php_printf("ssp_msg_queue_push first argument too length(less 128)\n");
		return;
	}
	if(!ssp_msg_queue_running || ssp_msg_queue_stop) return;

	#define __SERI_OK \
		msg = (ssp_msg_t*) malloc(sizeof(ssp_msg_t)*ZSTR_LEN(buf.s)); \
		memcpy(msg->arg, ZSTR_VAL(buf.s), ZSTR_LEN(buf.s)); \
		msg->arglen = ZSTR_LEN(buf.s); \
		msg->arg[msg->arglen] = '\0'
	SERIALIZE(arg, __SERI_OK);
	#undef __SERI_OK
	
	if(!msg) {
		msg = (ssp_msg_t*) malloc(sizeof(ssp_msg_t));
		msg->arg[0] = '\0';
		msg->arglen = 0;
	}

	strncpy(msg->func, func, funclen);
	msg->func[funclen] = '\0';
	msg->what = what;
	msg->arg1 = arg1;
	msg->arg2 = arg2;
	msg->arg3 = arg3;
	msg->arg4 = arg4;
	msg->arg5 = arg5;

	pthread_mutex_lock(&ssp_msg_queue_lock);
	if(ssp_msg_queue_msgs >= ssp_msg_queue_max_msgs) {
		php_printf("ssp_msg_queue_push: too many messages\n");
	} else {
		ssp_msg_queue_msgs++;
		queue_push(ssp_msg_queue, msg);
		pthread_cond_signal(&ssp_msg_queue_cond);
	}
	pthread_mutex_unlock(&ssp_msg_queue_lock);
}

static bool ssp_msg_queue_cmp(ssp_msg_t *s, void *ptr) {
	free(s);
	return true;
}

static PHP_FUNCTION(ssp_msg_queue_destory)
{
	if(SSP_G(trigger_type) != PHP_SSP_STOP) {
		php_printf("The ssp_msg_queue_destory function can only be executed in the ssp_stop_handler function\n");
		return;
	}

	if(!ssp_msg_queue_running || ssp_msg_queue_stop) return;

	pthread_mutex_lock(&ssp_msg_queue_lock);
	ssp_msg_queue_stop = true;
	pthread_cond_signal(&ssp_msg_queue_cond);
	pthread_mutex_unlock(&ssp_msg_queue_lock);

	pthread_mutex_lock(&ssp_msg_queue_lock);
	while(ssp_msg_queue_running > 0) {
		pthread_cond_wait(&ssp_msg_queue_cond, &ssp_msg_queue_lock);
	}
	pthread_mutex_unlock(&ssp_msg_queue_lock);

	pthread_mutex_destroy(&ssp_msg_queue_lock);
	pthread_cond_destroy(&ssp_msg_queue_cond);

	queue_clean_ex(ssp_msg_queue, NULL, (queue_cmp_t) ssp_msg_queue_cmp);
	queue_free(ssp_msg_queue);
}

typedef struct _ssp_delayed_t {
	char func[128];
	long delay;
	bool persist;
	long arg1;
	long arg2;
	long arg3;
	long arg4;
	long arg5;

	struct event event;
	struct timeval tv;

	struct _ssp_delayed_t *prev;
	struct _ssp_delayed_t *next;

	int arglen;
	char arg[1];
} ssp_delayed_t;
static ssp_delayed_t *ssp_delayed = NULL;
static queue_t *ssp_delayed_queue = NULL;
static int ssp_delayed_running = 0;
static pthread_mutex_t ssp_delayed_lock;
static pthread_cond_t ssp_delayed_cond;
static int ssp_delayed_write_fd;
static double ssp_delayed_time;
static struct event_base *ssp_delayed_base;
static queue_t *ssp_delayedel_queue = NULL;

#define DELAYED_PARAM_COUNT 8
#define DELAYED_DEL(dly) \
	do { \
		if(dly->prev == dly) { \
			ssp_delayed = NULL; \
		} else { \
			dly->prev->next = dly->next; \
			dly->next->prev = dly->prev; \
			if(dly == ssp_delayed) ssp_delayed = dly->next; \
		} \
		event_del(&dly->event); \
		free(dly); \
	} while(0) \

static void ssp_delayed_timeout_handler(evutil_socket_t fd, short event, void *arg) {
	int i;
	zval pfunc, rv;
	zval params[DELAYED_PARAM_COUNT];
	ssp_delayed_t *dly = arg;

	DELAYED_STARTUP();

	dprintf("delayed: %s, %ld, %d, %s(%d), %ld, %ld, %ld, %ld, %ld\n", dly->func, dly->delay, dly->persist, dly->arg, dly->arglen, dly->arg1, dly->arg2, dly->arg3, dly->arg4, dly->arg5);

	ZVAL_STRING(&pfunc, dly->func);
	ZVAL_LONG(&params[0], dly->delay);
	ZVAL_BOOL(&params[1], dly->persist);
	ZVAL_NULL(&params[2]);
	if(dly->arglen) {
		UNSERIALIZE(dly->arg, dly->arglen, ZVAL_COPY(&params[2], retval));
	}
	ZVAL_LONG(&params[3], dly->arg1);
	ZVAL_LONG(&params[4], dly->arg2);
	ZVAL_LONG(&params[5], dly->arg3);
	ZVAL_LONG(&params[6], dly->arg4);
	ZVAL_LONG(&params[7], dly->arg5);

	ZVAL_NULL(&rv);

	i = call_user_function(EG(function_table), NULL, &pfunc, &rv, DELAYED_PARAM_COUNT, params);

	if(i == SUCCESS && Z_TYPE(rv) == IS_FALSE) dly->persist = false;

	zval_ptr_dtor(&pfunc);
	zval_ptr_dtor(&rv);

	for (i = 0; i < DELAYED_PARAM_COUNT; i++) {
		zval_ptr_dtor(&params[i]);
	}

	if(!dly->persist) {
		DELAYED_DEL(dly);
	}

	DELAYED_SHUTDOWN();
}

static void delayed_notify_handler(const int fd, const short which, void *arg) {
	ssp_delayed_t *dly, *tmp;
	char buf[128], *str;
	int i, ret;

	ret = read(fd, buf, sizeof(buf));
	for(i=0; i<ret; i++) {
		switch(buf[i]) {
		case 'x':
			event_base_loopbreak(ssp_delayed_base);
			return;
		case 'd':
			str = queue_pop(ssp_delayedel_queue);
			if(!str) break;
			if(!ssp_delayed) {
				free(str);
				break;
			}
			dly = ssp_delayed;
			do {
				if(!strcmp(dly->func, str)) {
					tmp = dly->next;
					DELAYED_DEL(dly);
					if(!ssp_delayed) break;
					dly = tmp;
				} else {
					dly = dly->next;
				}
			} while(dly != ssp_delayed);
			free(str);
			break;
		case 's':
			dly = queue_pop(ssp_delayed_queue);
			if(!dly) break;

			if(ssp_delayed) {
				dly->next = ssp_delayed->next;
				dly->prev = ssp_delayed;

				ssp_delayed->next->prev = dly;
				ssp_delayed->next = dly;
			} else {
				ssp_delayed = dly;
				dly->next = dly;
				dly->prev = dly;
			}

			evutil_timerclear(&dly->tv);
			dly->tv.tv_sec = dly->delay / 1000;
			dly->tv.tv_usec = (dly->delay % 1000) * 1000;
			event_set(&dly->event, -1, EV_PERSIST, ssp_delayed_timeout_handler, dly);
			event_base_set(ssp_delayed_base, &dly->event);
			if(event_add(&dly->event, &dly->tv) == -1) perror("delayed event");
		}
	}
}

static void *ssp_delayed_handler(void *arg) {
	(void)arg;
	ssp_msg_t *msg;
	pthread_t tid = pthread_self();
	struct event event;

	int fds[2];
	if (pipe(fds)) {
		perror("Can't create delayed pipe");
		return NULL;
	}

	ssp_delayed_base = event_init();

	ssp_delayed_write_fd = fds[1];
	event_set(&event, fds[0], EV_READ | EV_PERSIST, delayed_notify_handler, NULL);
	event_base_set(ssp_delayed_base, &event);
	if (event_add(&event, NULL) == -1) {
		perror("event_add()");
		exit(1);
	}

	pthread_mutex_lock(&ssp_delayed_lock);
	ssp_delayed_running++;
	pthread_cond_signal(&ssp_delayed_cond);
	pthread_mutex_unlock(&ssp_delayed_lock);

	THREAD_STARTUP();

	ssp_delayed_time = microtime();

	event_base_loop(ssp_delayed_base, 0);

	THREAD_SHUTDOWN();

	pthread_mutex_lock(&ssp_delayed_lock);
	ssp_delayed_running--;
	pthread_cond_signal(&ssp_delayed_cond);
	pthread_mutex_unlock(&ssp_delayed_lock);

	pthread_detach(tid);
	pthread_exit(NULL);

	return NULL;
}

static PHP_FUNCTION(ssp_delayed_init)
{
	if(SSP_G(trigger_type) != PHP_SSP_START) {
		php_printf("The ssp_delayed_init function can only be executed in the ssp_start_handler function\n");
		return;
	}

	if (ssp_delayed_running || ssp_delayed_queue) {
		return;
	}

	ssp_delayed_queue = queue_init();
	ssp_delayedel_queue = queue_init();

	pthread_mutex_init(&ssp_delayed_lock, NULL);
	pthread_cond_init(&ssp_delayed_cond, NULL);

	worker_create(ssp_delayed_handler, NULL);

	pthread_mutex_lock(&ssp_delayed_lock);
	while (ssp_delayed_running == 0) {
		pthread_cond_wait(&ssp_delayed_cond, &ssp_delayed_lock);
	}
	pthread_mutex_unlock(&ssp_delayed_lock);
}

static PHP_FUNCTION(ssp_delayed_set)
{
	ssp_delayed_t *dly = NULL;
	char *func;
	size_t funclen;
	zend_long delay;
	zend_bool persist;
	zval *arg = NULL;
	zend_long arg1=0,arg2=0,arg3=0,arg4=0,arg5=0;
	if(!ssp_delayed_queue || zend_parse_parameters(ZEND_NUM_ARGS(), "slb|zlllll", &func, &funclen, &delay, &persist, &arg, &arg1, &arg2, &arg3, &arg4, &arg5) == FAILURE) return;
	if(funclen >= 128) {
		php_printf("ssp_delayed_set first argument too length(less 128)\n");
		return;
	}

	if(arg) {
		#define __SERI_OK \
			dly = (ssp_delayed_t*) malloc(sizeof(ssp_delayed_t)*ZSTR_LEN(buf.s)); \
			memcpy(dly->arg, ZSTR_VAL(buf.s), ZSTR_LEN(buf.s)); \
			dly->arglen = ZSTR_LEN(buf.s); \
			dly->arg[dly->arglen] = '\0'
		SERIALIZE(arg, __SERI_OK);
		#undef __SERI_OK
	}

	if(!dly) {
		dly = (ssp_delayed_t*) malloc(sizeof(ssp_delayed_t));
		dly->arg[0] = '\0';
		dly->arglen = 0;
	}

	strncpy(dly->func, func, funclen);
	dly->func[funclen] = '\0';
	dly->delay = delay;
	dly->persist = persist;
	dly->arg1 = arg1;
	dly->arg2 = arg2;
	dly->arg3 = arg3;
	dly->arg4 = arg4;
	dly->arg5 = arg5;

	queue_push(ssp_delayed_queue, dly);
	write(ssp_delayed_write_fd, "s", 1);
}

static PHP_FUNCTION(ssp_delayed_del)
{
	char *func;
	size_t funclen;
	if(!ssp_delayedel_queue || zend_parse_parameters(ZEND_NUM_ARGS(), "s", &func, &funclen) == FAILURE) return;
	if(funclen >= 128) {
		php_printf("ssp_delayed_set first argument too length(less 128)\n");
		return;
	}

	queue_push(ssp_delayedel_queue, strndup(func, funclen));
	write(ssp_delayed_write_fd, "d", 1);
}

static bool ssp_delayed_cmp(ssp_delayed_t *s, void *ptr) {
	free(s);
	return true;
}

static bool ssp_delayedel_cmp(void *s, void *ptr) {
	free(s);
	return true;
}

static PHP_FUNCTION(ssp_delayed_destory)
{
	if(SSP_G(trigger_type) != PHP_SSP_STOP) {
		php_printf("The ssp_delayed_destory function can only be executed in the ssp_stop_handler function\n");
		return;
	}

	write(ssp_delayed_write_fd, "x", 1);

	pthread_mutex_lock(&ssp_delayed_lock);
	while(ssp_delayed_running > 0) {
		pthread_cond_wait(&ssp_delayed_cond, &ssp_delayed_lock);
	}
	pthread_mutex_unlock(&ssp_delayed_lock);

	pthread_mutex_destroy(&ssp_delayed_lock);
	pthread_cond_destroy(&ssp_delayed_cond);

	queue_clean_ex(ssp_delayed_queue, NULL, (queue_cmp_t) ssp_delayed_cmp);
	queue_free(ssp_delayed_queue);
	queue_clean_ex(ssp_delayedel_queue, NULL, ssp_delayedel_cmp);
	queue_free(ssp_delayedel_queue);
}
