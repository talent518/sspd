#include "php_func.h"
#include "php_ext.h"
#include "server.h"
#include "node.h"
#include "api.h"
#include <error.h>
#include <malloc.h>

int le_ssp_descriptor;

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_mallinfo, 0, 0, 0)
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
#ifdef ZTS
	ts_free_id(ssp_globals_id);
#endif
#ifdef PHP_SSP_DEBUG
	printf("ssp module shutdown\n");
#endif
	return SUCCESS;
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
	printf("ssp_globals init\n");
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
	printf("ssp_globals shutdown\n");
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
	TRIGGER_STARTUP();
	if(SSP_G(bind)[type]==NULL){
		TRIGGER_SHUTDOWN();
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
	printf("\n");
	strnprint("=",tput_cols());
	printf("\nMemory(%s):\n\treal_size:%d\n\tsize:%d\n\treal_peak:%d\n\tpeak:%d",call_func_name,real_size,size,real_peak,peak);
	if(type==PHP_SSP_RECEIVE){
		printf("\nrecv_bytes:%d+%d",SSP_G(recv_bytes),*data_len);
		SSP_G(recv_bytes)+=*data_len;
	}
	if(type==PHP_SSP_SEND){
		printf("\nsend_bytes:%d+%d",SSP_G(send_bytes),*data_len);
		SSP_G(send_bytes)+=*data_len;
	}
	printf("\n");
	strnprint("-",tput_cols());fflush(stdout);
#endif
	ret=call_user_function_ex(CG(function_table), NULL, pfunc, &retval, param_count, params,1,NULL TSRMLS_CC);
#ifdef PHP_SSP_DEBUG
	strnprint("*",tput_cols());
	printf("gc_enabled:%d,gc_collect_cycles:%d\n",GC_G(gc_enabled),gc_collect_cycles(TSRMLS_C));
	strnprint("-",tput_cols());
#endif
	if(ret==SUCCESS){
		if(param_count>1){
			if(Z_TYPE_P(retval)!=IS_STRING){
				convert_to_string_ex(&retval);
			}
			if(Z_STRLEN_P(retval)>0){
				char *_data=strndup(Z_STRVAL_P(retval),Z_STRLEN_P(retval));
				free(*data);
				*data=_data;
				*data_len=Z_STRLEN_P(retval);
			}else{
				free(*data);
				*data=NULL;
				*data_len=0;
			}
		#ifdef PHP_SSP_DEBUG
			printf("\nreturn bytes(%s):%d",call_func_name,Z_STRLEN_P(retval));
		#endif
		}
	}else{
		php_printf("\nUnable to call handler(%s)", call_func_name);
	}
	zval_ptr_dtor(&retval);
#ifdef PHP_SSP_DEBUG
	printf("\nRemain Free Memory(%s):\n\treal_size:%d\n\tsize:%d\n\treal_peak:%d\n\tpeak:%d\n",
		call_func_name,
		zend_memory_usage(1 TSRMLS_CC)-real_size,
		zend_memory_usage(0 TSRMLS_CC)-size,
		zend_memory_peak_usage(1 TSRMLS_CC)-real_peak,
		zend_memory_peak_usage(0 TSRMLS_CC)-peak
	);
	strnprint("=",tput_cols());
	printf("\n");
	fflush(stdout);
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
	TRIGGER_SHUTDOWN();
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
	ptr=search_node(sockfd,is_port);
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
	}else{
		if(!strcasecmp(key,"sockfd")){
			RETURN_LONG(ptr->sockfd);
		}else if(!strcasecmp(key,"host")){
			RETURN_STRING(strdup(ptr->host),strlen(ptr->host));
		}else if(!strcasecmp(key,"port")){
			RETURN_LONG(ptr->port);
		}else{
			RETURN_NULL();
		}
	}
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
	int ret=socket_send(ptr,data,data_len);
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
	shutdown(ptr->sockfd,2);
	close(ptr->sockfd);
}
