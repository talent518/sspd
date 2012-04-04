#include "php_ext.h"

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_encode, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, options)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_decode, 0, 0, 1)
	ZEND_ARG_INFO(0, ssp)
	ZEND_ARG_INFO(0, assoc)
	ZEND_ARG_INFO(0, depth)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO(arginfo_ssp_last_error, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ ssp_functions[] */
function_entry ssp_functions[] = {
	PHP_FE(ssp_encode, arginfo_ssp_encode)
	PHP_FE(ssp_decode, arginfo_ssp_decode)
	PHP_FE(ssp_last_error, arginfo_ssp_last_error)
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
	REGISTER_LONG_CONSTANT("SSP_CONST",  SSP_CONST,  CONST_CS | CONST_PERSISTENT);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_GINIT_FUNCTION
*/
static PHP_GINIT_FUNCTION(ssp)
{
	ssp_globals->error_code = 0;
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
/* }}} */

/* {{{ proto string ssp_encode(mixed data [, int options])
   Returns the ssp representation of a value */
static PHP_FUNCTION(ssp_encode)
{
	RETURN_STRING("encode",1);
}
/* }}} */

/* {{{ proto mixed ssp_decode(string ssp [, bool assoc [, long depth]])
   Decodes the ssp representation into a PHP value */
static PHP_FUNCTION(ssp_decode)
{
	RETURN_STRING("decode",1);
}
/* }}} */

/* {{{ proto int ssp_last_error()
   Returns the error code of the last ssp_decode(). */
static PHP_FUNCTION(ssp_last_error)
{
}
/* }}} */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
