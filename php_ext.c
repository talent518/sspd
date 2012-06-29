#include "php_ext.h"
#include "server.h"
#include <error.h>

int le_ssp_descriptor;
int le_ssp_mutex_descriptor;
pthread_mutex_t ssp_mutex;

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_setopt, 0, 0, 2)
	ZEND_ARG_INFO(0, option)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

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
	PHP_FE(ssp_setopt, arginfo_ssp_setopt)
	PHP_FE(ssp_bind, arginfo_ssp_bind)
	PHP_FE(ssp_resource, arginfo_ssp_resource)
	PHP_FE(ssp_info, arginfo_ssp_info)
	PHP_FE(ssp_mutex_create, arginfo_ssp_mutex_create)
	PHP_FE(ssp_mutex_destroy, arginfo_ssp_mutex_destroy)
	PHP_FE(ssp_mutex_lock, arginfo_ssp_mutex_lock)
	PHP_FE(ssp_mutex_unlock, arginfo_ssp_mutex_unlock)
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
	NULL,
	NULL,
	NULL,
	PHP_MINFO(ssp),
	PHP_SSP_VERSION,
	PHP_MODULE_GLOBALS(ssp),
	PHP_GINIT(ssp),
	NULL,
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

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

	REGISTER_LONG_CONSTANT("SSP_OPT_USER",  PHP_SSP_OPT_USER,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_OPT_PIDFILE",  PHP_SSP_OPT_PIDFILE,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_OPT_HOST",  PHP_SSP_OPT_HOST,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_OPT_PORT",  PHP_SSP_OPT_PORT,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_OPT_MAX_CLIENTS",  PHP_SSP_OPT_MAX_CLIENTS,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_OPT_MAX_RECVS",  PHP_SSP_OPT_MAX_RECVS,  CONST_CS | CONST_PERSISTENT);
	
	le_ssp_descriptor = zend_register_list_destructors_ex(NULL, NULL, PHP_SSP_DESCRIPTOR_RES_NAME,module_number);
	le_ssp_mutex_descriptor = zend_register_list_destructors_ex(NULL, NULL, PHP_SSP_MUTEX_DESCRIPTOR_RES_NAME,module_number);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_GINIT_FUNCTION
*/
static PHP_GINIT_FUNCTION(ssp)
{
	ssp_globals->user = "daemon";
	ssp_globals->pidfile = "/var/run/ssp.pid";
	ssp_globals->host	 = "0.0.0.0";
	ssp_globals->port	 = 8083;
	ssp_globals->maxclients	 = 1000;
	ssp_globals->maxrecvs	 = 2*1024*1024;
    pthread_mutex_init(&ssp_mutex, NULL);
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

static zval *_ssp_string_zval(const char *str)
{
	zval *ret;
	int len = strlen(str);
	MAKE_STD_ZVAL(ret);

	Z_TYPE_P(ret) = IS_STRING;
	Z_STRLEN_P(ret) = len;
	Z_STRVAL_P(ret) = estrndup(str, len);
	return ret;
}

int trigger(unsigned short type,...){
	if(SSP_G(bind)[type]==NULL){
		return FAILURE;
	}
	zval **params,*retval;
	int i,param_count,ret;
	char *call_func_name;
	va_list args;
	node *ptr;
	char **data=NULL;
	long *data_len;

	call_func_name=SSP_G(bind)[type];

	va_start(args,type);
	switch(type){
		case PHP_SSP_START:
		case PHP_SSP_STOP:
			param_count=0;
			break;
		case PHP_SSP_RECEIVE:
		case PHP_SSP_SEND:
			param_count=2;
			ptr=va_arg(args,node*);
			data=va_arg(args,char**);
			data_len=va_arg(args,long*);
			params=(zval **)safe_emalloc(2, sizeof(zval *), 0);
			params[0]=_ssp_resource_zval(ptr);
			params[1]=_ssp_string_zval(*data);
			break;
		case PHP_SSP_CONNECT:
		case PHP_SSP_CONNECT_DENIED:
		case PHP_SSP_CLOSE:
			param_count=1;
			ptr=va_arg(args,node*);
			params=(zval **)safe_emalloc(1, sizeof(zval *), 0);
			params[0]=_ssp_resource_zval(ptr);
			break;
		default:
			perror("Trigger type not exists!");
			break;
	}
	va_end(args);

	MAKE_STD_ZVAL(retval);
	ZVAL_NULL(retval);
#ifdef PHP_SSP_DEBUG
	php_printf("\ncall function:%s\n",call_func_name);
#endif
	TSRMLS_FETCH();
	ret=call_user_function(CG(function_table), NULL, _ssp_string_zval(call_func_name), retval, param_count, params TSRMLS_CC);
#ifdef PHP_SSP_DEBUG
	php_printf("return result:%s\n",ret==FAILURE?"false":"true");
#endif
	if(param_count>1){
		if(Z_TYPE_P(retval)!=IS_STRING){
			convert_to_string_ex(&retval);
		}
		if(Z_STRLEN_P(retval)>0){
#ifdef PHP_SSP_DEBUG
			php_printf("retval:%s\n",Z_STRVAL_P(retval));
#endif
			char *_data=strndup(Z_STRVAL_P(retval),Z_STRLEN_P(retval));
			*data=_data;
			*data_len=Z_STRLEN_P(retval);
#ifdef PHP_SSP_DEBUG
			php_printf("data:%s\n",*data);
#endif
		}else{
			*data=NULL;
			*data_len=0;
		}
	}

	for (i = 0; i < param_count; i++) {
		zval_ptr_dtor(&params[i]);
	}
	zval_ptr_dtor(&retval);

	if (ret!=SUCCESS) {
		php_error(E_WARNING, "Unable to call handler %s()\n", call_func_name);
	}
	if(EG(exception)){
		zend_exception_error(EG(exception), E_ALL TSRMLS_CC);
		EG(exception)=NULL;
	}
#ifdef PHP_SSP_DEBUG
	php_printf("return function:%s\n",call_func_name);
#endif
	return ret;
}

static PHP_FUNCTION(ssp_setopt)
{
	int option;
	zval *setval;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lz", &option, &setval) == FAILURE) {
		RETURN_FALSE;
	}
	switch(option){
		case PHP_SSP_OPT_USER:
			if(Z_TYPE_P(setval)==IS_STRING){
				SSP_G(user)=strdup(Z_STRVAL_P(setval));
			}
			break;
		case PHP_SSP_OPT_PIDFILE:
			if(Z_TYPE_P(setval)==IS_STRING){
				SSP_G(pidfile)=strdup(Z_STRVAL_P(setval));
			}
			break;
		case PHP_SSP_OPT_HOST:
			if(Z_TYPE_P(setval)==IS_STRING){
				SSP_G(host)=strdup(Z_STRVAL_P(setval));
			}
			break;
		case PHP_SSP_OPT_PORT:
			if(Z_TYPE_P(setval)==IS_LONG){
				SSP_G(port)=Z_LVAL_P(setval);
			}
			break;
		case PHP_SSP_OPT_MAX_CLIENTS:
			if(Z_TYPE_P(setval)==IS_LONG){
				SSP_G(maxclients)=Z_LVAL_P(setval);
			}
			break;
		case PHP_SSP_OPT_MAX_RECVS:
			if(Z_TYPE_P(setval)==IS_LONG){
				SSP_G(maxrecvs)=Z_LVAL_P(setval);
			}
			break;
	}
	RETURN_TRUE;
}

static PHP_FUNCTION(ssp_bind)
{
	unsigned short eventtype;
	char *callback;
	long callback_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &eventtype, &callback, &callback_len) == FAILURE) {
		RETURN_FALSE;
	}
	if(eventtype>PHP_SSP_LEN){
		php_printf("function ssp_bind parameters error.\n");
		RETURN_FALSE;
	}
	SSP_G(bind)[eventtype]=strdup(callback);
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

static PHP_FUNCTION(ssp_mutex_create){
	pthread_mutex_t *mutex;
	mutex=(pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
	if(pthread_mutex_init(mutex,NULL)==0){
		//php_printf("create mutex,%d\n",mutex);
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
		pthread_mutex_lock(&ssp_mutex);
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
		pthread_mutex_unlock(&ssp_mutex);
		//php_printf("unlock ssp_mutex\n");
	}
	RETURN_NULL();
}

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
	close(ptr->sockfd);
	del(head,ptr->sockfd);
}
