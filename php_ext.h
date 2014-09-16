#ifndef PHP_EXT_H
#define PHP_EXT_H

#include <php.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <time.h>

#include <TSRM.h>

#include "config.h"

#define PHP_SSP_DESCRIPTOR_RES_NAME "ssp conn_t"
#define PHP_SSP_DESCRIPTOR_REF_RES_NAME "ssp ref conn_t"
#define PHP_SSP_VERSION "v2.1.0"

#define PHP_SSP_START 0
#define PHP_SSP_RECEIVE 1
#define PHP_SSP_SEND 2
#define PHP_SSP_CONNECT 3
#define PHP_SSP_CONNECT_DENIED 4
#define PHP_SSP_CLOSE 5
#define PHP_SSP_STOP 6

#define PHP_SSP_RES_INDEX 0
#define PHP_SSP_RES_SOCKFD 1
#define PHP_SSP_RES_PORT 2

extern long le_ssp_descriptor,le_ssp_descriptor_ref,ssp_timeout;

extern function_entry ssp_functions[];
extern zend_module_entry ssp_module_entry;

ZEND_BEGIN_MODULE_GLOBALS(ssp)
	long timeout;
	long trigger_count;
	zval *vars;
ZEND_END_MODULE_GLOBALS(ssp)

#define SSP_G(v) TSRMG(ssp_globals_id, zend_ssp_globals *, v)

#define TRIGGER_STARTUP() \
	TSRMLS_FETCH();\
	if((SSP_G(trigger_count)++) == 0) {\
		long timeout=(long)time(NULL);\
		if(SSP_G(timeout)<timeout) {\
			SSP_G(timeout)=timeout+ssp_timeout;\
			THREAD_SHUTDOWN()\
			dprintf("=======================================================================================================================\n\n");\
			dprintf("===================================================PHP_REQUEST_CLEAN===================================================\n\n");\
			dprintf("=======================================================================================================================\n\n");\
			THREAD_STARTUP();\
		}\
	}\
	dprintf("-----------------------------------------------------------------------------------------------------------------------\n");

#define TRIGGER_SHUTDOWN() \
	dprintf("-----------------------------------------------------------------------------------------------------------------------\n");\
	if((--SSP_G(trigger_count)) == 0) {\
		zend_hash_clean(Z_ARRVAL_P(SSP_G(vars)));\
	}

#define TRIGGER_STARTUP_EX() TRIGGER_STARTUP()

#define TRIGGER_SHUTDOWN_EX() TRIGGER_SHUTDOWN()

#define THREAD_STARTUP() ssp_request_startup();
#define THREAD_SHUTDOWN() ssp_request_shutdown();

static PHP_MINIT_FUNCTION(ssp);
static PHP_MSHUTDOWN_FUNCTION(ssp);
static PHP_GINIT_FUNCTION(ssp);
static PHP_GSHUTDOWN_FUNCTION(ssp);
static PHP_MINFO_FUNCTION(ssp);

zend_bool php_auto_globals_create_ssp(char *name, uint name_len TSRMLS_DC);
bool trigger(unsigned short type,...);

static PHP_FUNCTION(ssp_mallinfo);
static PHP_FUNCTION(ssp_resource);
static PHP_FUNCTION(ssp_info);
static PHP_FUNCTION(ssp_send);
static PHP_FUNCTION(ssp_close);
static PHP_FUNCTION(ssp_destroy);
static PHP_FUNCTION(ssp_lock);
static PHP_FUNCTION(ssp_unlock);

ZEND_DECLARE_MODULE_GLOBALS(ssp)

#endif  /* PHP_EXT_H */
