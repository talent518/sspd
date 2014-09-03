#ifndef PHP_EXT_H
#define PHP_EXT_H

#include <php.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <TSRM.h>

#include "config.h"

#define PHP_SSP_DESCRIPTOR_RES_NAME "ssp user conn_t"
#define PHP_SSP_VERSION "v2.1.0"

#define PHP_SSP_BIND_LEN 7

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

extern int le_ssp_descriptor;

extern function_entry ssp_functions[];
extern zend_module_entry ssp_module_entry;

#define phpext_ssp_ptr &ssp_module_entry

ZEND_BEGIN_MODULE_GLOBALS(ssp)
	int requestes;
	char **bind;
ZEND_END_MODULE_GLOBALS(ssp)

#define SSP_G(v) TSRMG(ssp_globals_id, zend_ssp_globals *, v)

#define TRIGGER_STARTUP() \
	if((SSP_G(requestes)++) == 0) {\
		ssp_request_startup();\
	}

#define TRIGGER_SHUTDOWN() \
	if((--SSP_G(requestes)) == 0) {\
		ssp_request_shutdown();\
	}

static PHP_MINIT_FUNCTION(ssp);
static PHP_MSHUTDOWN_FUNCTION(ssp);
static PHP_GINIT_FUNCTION(ssp);
static PHP_GSHUTDOWN_FUNCTION(ssp);
static PHP_MINFO_FUNCTION(ssp);

bool trigger(unsigned short type,...);

static PHP_FUNCTION(ssp_mallinfo);
static PHP_FUNCTION(ssp_resource);
static PHP_FUNCTION(ssp_info);
static PHP_FUNCTION(ssp_send);
static PHP_FUNCTION(ssp_close);

ZEND_DECLARE_MODULE_GLOBALS(ssp)

#endif  /* PHP_EXT_H */
