#ifndef PHP_SSP_H
#define PHP_SSP_H

#include "php.h"

#define PHP_SSP_VERSION "1.2.1"

extern function_entry ssp_functions[];
extern zend_module_entry ssp_module_entry;
#define phpext_ssp_ptr &ssp_module_entry

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(ssp)
	int error_code;
ZEND_END_MODULE_GLOBALS(ssp)

#ifdef ZTS
	#define SSP_G(v) TSRMG(ssp_globals_id, zend_ssp_globals *, v)
#else
	#define SSP_G(v) (ssp_globals.v)
#endif

#define SSP_CONST 1

static PHP_MINIT_FUNCTION(ssp);
static PHP_GINIT_FUNCTION(ssp);
static PHP_MINFO_FUNCTION(ssp);
static PHP_FUNCTION(ssp_encode);
static PHP_FUNCTION(ssp_decode);
static PHP_FUNCTION(ssp_last_error);

ZEND_DECLARE_MODULE_GLOBALS(ssp)

#endif  /* PHP_SSP_H */

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
