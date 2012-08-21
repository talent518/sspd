#include "php_ext.h"
#include "server.h"
#include "node.h"
#include <error.h>
#include <malloc.h>

int le_ssp_descriptor;
int le_ssp_mutex_descriptor;

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_mallinfo, 0, 0, 0)
ZEND_END_ARG_INFO()

/*
ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_setopt, 0, 0, 2)
	ZEND_ARG_INFO(0, option)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()
*/

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_bind, 0, 0, 2)
	ZEND_ARG_INFO(0, eventtype)
	ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_resource, 0, 0, 2)
	ZEND_ARG_INFO(0, sockfd)
	ZEND_ARG_INFO(0, is_bool)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_info, 0, 0, 2)
	ZEND_ARG_INFO(0, socket)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

#ifndef ZTS
	ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_mutex_create, 0, 0, 0)
	ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_mutex_destroy, 0, 0, 1)
		ZEND_ARG_INFO(0, mutex)
	ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_mutex_lock, 0, 0, 0)
		ZEND_ARG_INFO(0, mutex)
	ZEND_END_ARG_INFO()

	ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_mutex_unlock, 0, 0, 0)
		ZEND_ARG_INFO(0, mutex)
	ZEND_END_ARG_INFO()
#endif

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_send, 0, 0, 2)
	ZEND_ARG_INFO(0, socket)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_close, 0, 0, 1)
	ZEND_ARG_INFO(0, socket)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ ssp_functions[] */
function_entry ssp_functions[] = {
	PHP_FE(ssp_mallinfo, arginfo_ssp_mallinfo)
	//PHP_FE(ssp_setopt, arginfo_ssp_setopt)
	PHP_FE(ssp_bind, arginfo_ssp_bind)
	PHP_FE(ssp_resource, arginfo_ssp_resource)
	PHP_FE(ssp_info, arginfo_ssp_info)
#ifndef ZTS
	PHP_FE(ssp_mutex_create, arginfo_ssp_mutex_create)
	PHP_FE(ssp_mutex_destroy, arginfo_ssp_mutex_destroy)
	PHP_FE(ssp_mutex_lock, arginfo_ssp_mutex_lock)
	PHP_FE(ssp_mutex_unlock, arginfo_ssp_mutex_unlock)
#endif
	PHP_FE(ssp_send, arginfo_ssp_send)
	PHP_FE(ssp_close, arginfo_ssp_close)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ ssp_module_entry
 */
zend_module_entry ssp_module_entry = {
	STANDARD_MODULE_HEADER,
	"ssp",
	ssp_functions,
	PHP_MINIT(ssp),
	PHP_MSHUTDOWN(ssp),
	NULL,
	NULL,
	PHP_MINFO(ssp),
	PHP_SSP_VERSION,
	PHP_MODULE_GLOBALS(ssp),
	PHP_GINIT(ssp),
	PHP_GSHUTDOWN(ssp),
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

static void php_destroy_ssp(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */
{
	node *ptr = (node *) rsrc->ptr;

#ifdef PHP_SSP_DEBUG
	php_printf("\nDestroy SSP Resource (%d) for the host %s, port %d.\n",ptr->sockfd,ptr->host,ptr->port);
#endif

	//close(ptr->sockfd);
	//efree(ptr);
}

static void php_destroy_ssp_mutex(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */
{
	pthread_mutex_t *mutex = (pthread_mutex_t *) rsrc->ptr;

#ifdef PHP_SSP_DEBUG
	php_printf("\nDestroy SSP Mutex Resource type(%d),refcount(%d).\n",rsrc->type,rsrc->refcount);
#endif

	pthread_mutex_destroy(mutex);
}

/* {{{ MINIT */
static PHP_MINIT_FUNCTION(ssp)
{
	REGISTER_STRING_CONSTANT("SSP_VERSION",  PHP_SSP_VERSION,  CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("SSP_START",  PHP_SSP_START,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_RECEIVE",  PHP_SSP_RECEIVE,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_SEND",  PHP_SSP_SEND,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_CONNECT",  PHP_SSP_CONNECT,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_CONNECT_DENIED",  PHP_SSP_CONNECT_DENIED,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_CLOSE",  PHP_SSP_CLOSE,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_STOP",  PHP_SSP_STOP,  CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("SSP_OPT_USER",  SSP_OPT_USER,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_OPT_PIDFILE",  SSP_OPT_PIDFILE,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_OPT_HOST",  SSP_OPT_HOST,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_OPT_PORT",  SSP_OPT_PORT,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_OPT_MAX_CLIENTS",  SSP_OPT_MAX_CLIENTS,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_OPT_MAX_RECVS",  SSP_OPT_MAX_RECVS,  CONST_CS | CONST_PERSISTENT);
	
	le_ssp_descriptor = zend_register_list_destructors_ex(php_destroy_ssp, NULL, PHP_SSP_DESCRIPTOR_RES_NAME,module_number);
	le_ssp_mutex_descriptor = zend_register_list_destructors_ex(php_destroy_ssp_mutex, NULL, PHP_SSP_MUTEX_DESCRIPTOR_RES_NAME,module_number);

#ifdef PHP_SSP_DEBUG
	printf("ssp module init\n");
#endif	

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
static PHP_MSHUTDOWN_FUNCTION(ssp)
{
#ifdef PHP_SSP_DEBUG
	printf("ssp module shutdown\n");
#endif
}
/* }}} */

/* {{{ PHP_GINIT_FUNCTION
*/
static PHP_GINIT_FUNCTION(ssp)
{
	ssp_globals->bind=(char **)malloc(sizeof(char*)*PHP_SSP_BIND_LEN);
	int i;
	for(i=0;i<PHP_SSP_BIND_LEN;i++){
		ssp_globals->bind[i]=NULL;
	}
#ifdef PHP_SSP_DEBUG
	ssp_globals->recv_bytes=0;
	ssp_globals->send_bytes=0;
#endif
#ifdef PHP_SSP_DEBUG
	printf("ssp module globals init\n");
#endif
}
/* }}} */

/* {{{ PHP_GSHUTDOWN_FUNCTION */
static PHP_GSHUTDOWN_FUNCTION(ssp)
{
	int i;
	for(i=0;i<PHP_SSP_BIND_LEN;i++){
		if(ssp_globals->bind[i]){
			free(ssp_globals->bind[i]);
		}
	}
	free(ssp_globals->bind);
#ifdef PHP_SSP_DEBUG
	printf("ssp module globals shutdown\n");
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

static zval *_ssp_resource_zval(node *value)
{
	zval *ret;
	MAKE_STD_ZVAL(ret);
	ZEND_REGISTER_RESOURCE(ret,value,le_ssp_descriptor);
	return ret;
}

static zval *_ssp_string_zval(const char *str,int len)
{
	zval *ret;
	MAKE_STD_ZVAL(ret);

	Z_TYPE_P(ret) = IS_STRING;
	Z_STRLEN_P(ret) = len;
	Z_STRVAL_P(ret) = estrndup(str, len);
	return ret;
}

int trigger(unsigned short type,...){
	TSRMLS_FETCH();
	if(SSP_G(bind)[type]==NULL){
		return FAILURE;
	}
	zval *zval_ptr,*zval_data,*pfunc;
	zval ***params,*retval;
	int i,param_count,ret;
	char *call_func_name;
	va_list args;
	node *ptr;
	char **data=NULL;
	long *data_len;

	call_func_name=strdup(SSP_G(bind)[type]);
	pfunc=_ssp_string_zval(call_func_name,strlen(call_func_name));

	va_start(args,type);
	switch(type){
		case PHP_SSP_START:
		case PHP_SSP_STOP:
			param_count=0;
			params=NULL;
			break;
		case PHP_SSP_RECEIVE:
		case PHP_SSP_SEND:
			param_count=2;
			ptr=va_arg(args,node*);
			data=va_arg(args,char**);
			data_len=va_arg(args,long*);
			params=(zval ***) emalloc(sizeof(zval **)*param_count);
			zval_ptr=_ssp_resource_zval(ptr);
			zval_data=_ssp_string_zval(*data,*data_len);
			params[0]=&zval_ptr;
			params[1]=&zval_data;
			break;
		case PHP_SSP_CONNECT:
		case PHP_SSP_CONNECT_DENIED:
		case PHP_SSP_CLOSE:
			param_count=1;
			ptr=va_arg(args,node*);
			params=(zval ***) emalloc(sizeof(zval **)*param_count);
			zval_ptr=_ssp_resource_zval(ptr);
			params[0]=&zval_ptr;
			break;
		default:
			perror("Trigger type not exists!");
			return FAILURE;
	}
	va_end(args);

#ifdef PHP_SSP_DEBUG
	long real_size=zend_memory_usage(1 TSRMLS_CC),size=zend_memory_usage(0 TSRMLS_CC);
	long real_peak=zend_memory_peak_usage(1 TSRMLS_CC),peak=zend_memory_peak_usage(0 TSRMLS_CC);
	printf("\nMemory:\n\treal_size:%d,size:%d,real_peak:%d,peak:%d\n",real_size,size,real_peak,peak);
	if(type==PHP_SSP_RECEIVE){
		printf("recv_bytes:%d+%d\n",SSP_G(recv_bytes),*data_len);
		SSP_G(recv_bytes)+=*data_len;
	}
	if(type==PHP_SSP_SEND){
		printf("send_bytes:%d+%d\n",SSP_G(send_bytes),*data_len);
		SSP_G(send_bytes)+=*data_len;
	}
#endif
	ret=call_user_function_ex(CG(function_table), NULL, pfunc, &retval, param_count, params,0,NULL TSRMLS_CC);
	if(ret==SUCCESS){
		if(param_count>1){
			if(Z_TYPE_P(retval)!=IS_STRING){
				convert_to_string_ex(&retval);
			}
			if(Z_STRLEN_P(retval)>0){
				free(*data);
				char *_data=strndup(Z_STRVAL_P(retval),Z_STRLEN_P(retval));
				*data=_data;
				*data_len=Z_STRLEN_P(retval);
			}else{
				*data=NULL;
				*data_len=0;
			}
#ifdef PHP_SSP_DEBUG
			printf("return bytes:%d\n",Z_STRLEN_P(retval));
#endif
		}
	}else{
		php_printf("Unable to call handler %s()\n", call_func_name);
	}

	zval_ptr_dtor(&retval);

#ifdef PHP_SSP_DEBUG
	printf("Remain Free Memory:\n\treal_size:%d,size:%d,real_peak:%d,peak:%d\n",
		zend_memory_usage(1 TSRMLS_CC)-real_size,
		zend_memory_usage(0 TSRMLS_CC)-size,
		zend_memory_peak_usage(1 TSRMLS_CC)-real_peak,
		zend_memory_peak_usage(0 TSRMLS_CC)-peak
	);
#endif

	if(param_count>0){
		int i;
		for(i=0;i<param_count;i++){
			zval_ptr_dtor(params[i]);
		}
		efree(params);
	}
	zval_ptr_dtor(&pfunc);
	free(call_func_name);
	return ret;
}

static PHP_FUNCTION(ssp_mallinfo){
    struct mallinfo info = mallinfo();
	array_init(return_value);
	add_assoc_long(return_value,"arena",info.arena);//size of data segment used by malloc
	add_assoc_long(return_value,"ordblks",info.ordblks);//number of free chunks
	add_assoc_long(return_value,"smblks",info.smblks);//number of fast bins
	add_assoc_long(return_value,"hblks",info.hblks);//number of anonymous mappings
	add_assoc_long(return_value,"hblkhd",info.hblkhd);//size of anonymous mappings
	add_assoc_long(return_value,"usmblks",info.usmblks);//maximum total allocated size
	add_assoc_long(return_value,"fsmblks",info.fsmblks);//size of available fast bins
	add_assoc_long(return_value,"uordblks",info.uordblks);//size of total allocated space
	add_assoc_long(return_value,"fordblks",info.fordblks);//size of available chunks
	add_assoc_long(return_value,"keepcost",info.keepcost);//size of trimmable space
}

/*
static PHP_FUNCTION(ssp_setopt)
{
	int option;
	zval *setval;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lz", &option, &setval) == FAILURE) {
		RETURN_FALSE;
	}
	switch(option){
		case SSP_OPT_USER:
			if(Z_TYPE_P(setval)==IS_STRING){
				ssp_user=strndup(Z_STRVAL_P(setval),Z_STRLEN_P(setval));
			}
			break;
		case SSP_OPT_PIDFILE:
			if(Z_TYPE_P(setval)==IS_STRING){
				ssp_pidfile=strndup(Z_STRVAL_P(setval),Z_STRLEN_P(setval));
			}
			break;
		case SSP_OPT_HOST:
			if(Z_TYPE_P(setval)==IS_STRING){
				ssp_host=strndup(Z_STRVAL_P(setval),Z_STRLEN_P(setval));
			}
			break;
		case SSP_OPT_PORT:
			if(Z_TYPE_P(setval)==IS_LONG){
				ssp_port=Z_LVAL_P(setval);
			}
			break;
		case SSP_OPT_MAX_CLIENTS:
			if(Z_TYPE_P(setval)==IS_LONG){
				int mc=Z_LVAL_P(setval);
				if(mc>0xffffff){
					ssp_maxclients=mc;
				}
			}
			break;
		case SSP_OPT_MAX_RECVS:
			if(Z_TYPE_P(setval)==IS_LONG){
				ssp_maxrecvs=Z_LVAL_P(setval);
			}
			break;
	}
	RETURN_TRUE;
}*/

static PHP_FUNCTION(ssp_bind)
{
	unsigned short eventtype;
	char *callback;
	long callback_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &eventtype, &callback, &callback_len) == FAILURE) {
		RETURN_FALSE;
	}
	if(eventtype>=PHP_SSP_BIND_LEN){
		php_printf("function ssp_bind parameters error.\n");
		RETURN_FALSE;
	}
	SSP_G(bind)[eventtype]=strndup(callback,callback_len);
	RETURN_TRUE;
}
static PHP_FUNCTION(ssp_resource){
	zend_bool is_port=0;
	int sockfd;
	node *ptr;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l|b", &sockfd,&is_port) == FAILURE) {
		RETURN_FALSE;
	}
	ptr=find(sockfd,is_port);
	if(ptr!=NULL){
		ZEND_REGISTER_RESOURCE(return_value,ptr,le_ssp_descriptor);
	}else{
		RETURN_FALSE;
	}
}

static PHP_FUNCTION(ssp_info){
	zval *res;
	node *ptr;
	char *key;
	int key_len=0;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|s", &res,&key,&key_len) == FAILURE) {
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(ptr,node*, &res, -1, PHP_SSP_DESCRIPTOR_RES_NAME,le_ssp_descriptor);
	if(key_len==0){
		array_init(return_value);
		add_assoc_long(return_value,"sockfd",ptr->sockfd);
		add_assoc_string(return_value,"host",ptr->host,1); /* cast to avoid gcc-warning */
		add_assoc_long(return_value,"port",ptr->port);
		add_assoc_long(return_value,"flag",ptr->flag);
	}else{
		if(!strcasecmp(key,"sockfd")){
			RETURN_LONG(ptr->sockfd);
		}else if(!strcasecmp(key,"host")){
			RETURN_STRING(strdup(ptr->host),strlen(ptr->host));
		}else if(!strcasecmp(key,"port")){
			RETURN_LONG(ptr->port);
		}else if(!strcasecmp(key,"flag")){
			RETURN_LONG(ptr->flag);
		}else{
			RETURN_NULL();
		}
	}
}

#ifndef ZTS
	static PHP_FUNCTION(ssp_mutex_create){
		pthread_mutex_t *mutex;
		mutex=(pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
		//php_printf("create mutex,%d\n",mutex);
		if(pthread_mutex_init(mutex,NULL)==0){
			//php_printf("inited create mutex,%d\n",mutex);
			ZEND_REGISTER_RESOURCE(return_value,mutex,le_ssp_mutex_descriptor);
		}else{
			RETURN_FALSE;
		}
	}
	static PHP_FUNCTION(ssp_mutex_destroy){
		zval *res;
		pthread_mutex_t *mutex;
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res) == FAILURE) {
			RETURN_FALSE;
		}
		ZEND_FETCH_RESOURCE(mutex,pthread_mutex_t*, &res, -1, PHP_SSP_DESCRIPTOR_RES_NAME,le_ssp_mutex_descriptor);
		pthread_mutex_destroy(mutex);
		//php_printf("destroy mutex,%d\n",mutex);
		RETURN_NULL();
	}
	static PHP_FUNCTION(ssp_mutex_lock){
		zval *res=NULL;
		pthread_mutex_t *mutex;
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &res) == FAILURE) {
			RETURN_FALSE;
		}
		if(res){
			ZEND_FETCH_RESOURCE(mutex,pthread_mutex_t*, &res, -1, PHP_SSP_MUTEX_DESCRIPTOR_RES_NAME,le_ssp_mutex_descriptor);
			pthread_mutex_lock(mutex);
			//php_printf("lock mutex,%d\n",mutex);
		}else{
			pthread_mutex_lock(ssp_mutex);
			//php_printf("lock ssp_mutex\n");
		}
		RETURN_NULL();
	}
	static PHP_FUNCTION(ssp_mutex_unlock){
		zval *res=NULL;
		pthread_mutex_t *mutex;
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|r", &res) == FAILURE) {
			RETURN_FALSE;
		}
		if(res){
			ZEND_FETCH_RESOURCE(mutex,pthread_mutex_t*, &res, -1, PHP_SSP_MUTEX_DESCRIPTOR_RES_NAME,le_ssp_mutex_descriptor);
			pthread_mutex_unlock(mutex);
			//php_printf("unlock mutex,%d\n",mutex);
		}else{
			pthread_mutex_unlock(ssp_mutex);
			//php_printf("unlock ssp_mutex\n");
		}
		RETURN_NULL();
	}
#endif

static PHP_FUNCTION(ssp_send)
{
	char *data;
	long data_len;
	zval *res;
	node *ptr;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &res, &data, &data_len) == FAILURE) {
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(ptr,node*, &res, -1, PHP_SSP_DESCRIPTOR_RES_NAME,le_ssp_descriptor);
	trigger(PHP_SSP_SEND,ptr,&data,&data_len);
	int ret=socket_send(ptr->sockfd,data,data_len);
	RETURN_LONG(ret);
}

static PHP_FUNCTION(ssp_close)
{
	zval *res;
	node *ptr;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res) == FAILURE) {
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(ptr,node*, &res, -1, PHP_SSP_DESCRIPTOR_RES_NAME,le_ssp_descriptor);
	trigger(PHP_SSP_CLOSE,ptr);
	ptr->flag=false;
	shutdown(ptr->sockfd,2);
	//close(ptr->sockfd);
	//delete(ptr);
}
