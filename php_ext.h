#ifndef PHP_SSP_H
#define PHP_SSP_H

#include "php.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define PHP_SSP_DESCRIPTOR_RES_NAME "ssp user node"
#define PHP_SSP_VERSION "1.2.1"

#define PHP_SSP_START 0
#define PHP_SSP_RECEIVE 1
#define PHP_SSP_SEND 2
#define PHP_SSP_CONNECT 3
#define PHP_SSP_CONNECT_DENIED 4
#define PHP_SSP_CLOSE 5
#define PHP_SSP_STOP 6

#define PHP_SSP_LEN 7

extern int le_ssp_descriptor;

extern function_entry ssp_functions[];
extern zend_module_entry ssp_module_entry;

#define phpext_ssp_ptr &ssp_module_entry

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(ssp)
	char *pidfile;
	char *user;
	char *host;
	short int port;
	char *bind[PHP_SSP_LEN];
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

char *trigger(unsigned short eventtype,...);

static PHP_FUNCTION(ssp_bind);
static PHP_FUNCTION(ssp_info);
static PHP_FUNCTION(ssp_send);
static PHP_FUNCTION(ssp_close);

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
