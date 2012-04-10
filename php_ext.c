#include "php_ext.h"

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_bind, 0, 0, 2)
	ZEND_ARG_INFO(0, eventtype)
	ZEND_ARG_INFO(0, callback)
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

static PHP_FUNCTION(ssp_bind)
{
	unsigned short eventtype;
	char *callback;
	long callback_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ls", &eventtype, &callback, &callback_len) == FAILURE) {
		php_printf("Parameters error.\n");
		RETURN_FALSE;
	}
	if(eventtype>PHP_SSP_LEN){
		php_printf("Parameters error.\n");
		RETURN_FALSE;
	}
	SSP_G(bind)[eventtype]=strdup(callback);
	RETURN_TRUE;
}

static PHP_FUNCTION(ssp_send)
{
	RETURN_STRING("decode",1);
}

static PHP_FUNCTION(ssp_close)
{
	RETURN_STRING("decode",1);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
