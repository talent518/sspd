#include "php_ext.h"
#include "server.h"
#include <error.h>

int le_ssp_descriptor;

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_bind, 0, 0, 2)
	ZEND_ARG_INFO(0, eventtype)
	ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_info, 0, 0, 1)
	ZEND_ARG_INFO(0, socket)
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
	PHP_FE(ssp_bind, arginfo_ssp_bind)
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
	REGISTER_LONG_CONSTANT("SSP_START",  PHP_SSP_START,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_RECEIVE",  PHP_SSP_RECEIVE,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_SEND",  PHP_SSP_SEND,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_CONNECT",  PHP_SSP_CONNECT,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_CONNECT_DENIED",  PHP_SSP_CONNECT_DENIED,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_CLOSE",  PHP_SSP_CLOSE,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_STOP",  PHP_SSP_STOP,  CONST_CS | CONST_PERSISTENT);
	le_ssp_descriptor = zend_register_list_destructors_ex(NULL, NULL, PHP_SSP_DESCRIPTOR_RES_NAME,module_number);
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
	TSRMLS_FETCH();
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

char *trigger(unsigned short eventtype,...){
	if(SSP_G(bind)[eventtype]==NULL){
		return NULL;
	}
	va_list args;
	zval ***params;
	zval *retval=NULL,*call_func_handler,*res=NULL,*data=NULL;
	int i,param_count;

	call_func_handler = _ssp_string_zval(SSP_G(bind)[eventtype]);

	va_start(args, eventtype);
	switch(eventtype){
		case PHP_SSP_START:
		case PHP_SSP_STOP:
			param_count=0;
			params = safe_emalloc(sizeof(zval **),0,0);
			break;
		case PHP_SSP_RECEIVE:
		case PHP_SSP_SEND:
			param_count=2;
			params = safe_emalloc(sizeof(zval **),2,0);
			res=_ssp_resource_zval(va_arg(args,node*));
			data=_ssp_string_zval(va_arg(args,char*));
			params[0]=&res;
			params[1]=&data;
			break;
		case PHP_SSP_CONNECT:
		case PHP_SSP_CONNECT_DENIED:
		case PHP_SSP_CLOSE:
			param_count=1;
			params = safe_emalloc(sizeof(zval **),1,0);
			res=_ssp_resource_zval(va_arg(args,node*));
			params[0]=&res;
			break;
		default:
			perror("Unable to call handler");
			break;
	}
	va_end(args);

	MAKE_STD_ZVAL(retval);
	convert_to_string(retval);

	php_printf("\ncall function:%s\n",Z_STRVAL_P(call_func_handler));
	int result=call_user_function_ex(CG(function_table), NULL, call_func_handler, &retval, param_count, params, 0, NULL TSRMLS_CC);
	php_printf("return result:%d\n\n",result==FAILURE);

	for (i = 0; i < param_count; i++) {
		zval_ptr_dtor(params[i]);
	}

	if (result==FAILURE) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Unable to call handler %s()", Z_STRVAL_P(call_func_handler));
		return NULL;
	} else {
		//php_printf("function:%s,retval len:%d,retval:%s\n",Z_STRVAL_P(call_func_handler),Z_STRLEN_P(retval),Z_STRVAL_P(retval));
		if(EG(exception)){
			EG(exception)=NULL;
			zend_exception_error(EG(exception), E_ALL TSRMLS_CC);
			return NULL;
		}
		return Z_STRVAL_P(retval);
	}
}

static PHP_FUNCTION(ssp_bind)
{
	unsigned short eventtype;
	char *callback;
	long callback_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &eventtype, &callback, &callback_len) == FAILURE) {
		php_printf("function ssp_bind parameters error.\n");
		RETURN_FALSE;
	}
	if(eventtype>PHP_SSP_LEN){
		php_printf("function ssp_bind parameters error.\n");
		RETURN_FALSE;
	}
	SSP_G(bind)[eventtype]=strdup(callback);
	RETURN_TRUE;
}

static PHP_FUNCTION(ssp_info){
	zval *res;
	node *ptr;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res) == FAILURE) {
		php_printf("function ssp_info parameters error.\n");
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(ptr,node*, &res, -1, PHP_SSP_DESCRIPTOR_RES_NAME,le_ssp_descriptor);
	array_init(return_value);
	add_assoc_long(return_value,"tid",ptr->tid);
	add_assoc_long(return_value,"sockfd",ptr->sockfd);
	add_assoc_string(return_value,"host",ptr->host,0); /* cast to avoid gcc-warning */
	add_assoc_long(return_value,"port",ptr->port);
	add_assoc_long(return_value,"flag",ptr->flag);
}

static PHP_FUNCTION(ssp_send)
{
	char *data,*_data=NULL;
	long data_len;
	zval *res;
	node *ptr;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &res, &data, &data_len) == FAILURE) {
		php_printf("function ssp_send parameters error.\n");
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(ptr,node*, &res, -1, PHP_SSP_DESCRIPTOR_RES_NAME,le_ssp_descriptor);
	_data=trigger(PHP_SSP_SEND,ptr,data);
	if(_data!=NULL){
		data=_data;
		data_len=strlen(_data);
	}
	php_printf("ssp_send start");
	long ret=send(ptr->sockfd,data,data_len,0);
	php_printf("ssp_send end");
	RETURN_LONG(ret);
}

static PHP_FUNCTION(ssp_close)
{
	zval *res;
	node *ptr;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res) == FAILURE) {
		php_printf("function ssp_info parameters error.\n");
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(ptr,node*, &res, -1, PHP_SSP_DESCRIPTOR_RES_NAME,le_ssp_descriptor);
	ptr->flag=false;
	close(ptr->sockfd);
	del(head,ptr->sockfd);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
