#ifndef PHP_EXT_H
#define PHP_EXT_H

#include <php.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <time.h>

#include <TSRM.h>
#include <zend.h>

#include "config.h"
#include "api.h"

#define PHP_SSP_DESCRIPTOR_RES_NAME "ssp conn_t"
#define PHP_SSP_DESCRIPTOR_REF_RES_NAME "ssp ref conn_t"
#define PHP_SSP_VERSION "v2.1.0"

#define PHP_SSP_START 0
#define PHP_SSP_CONNECT 1
#define PHP_SSP_CONNECT_DENIED 2
#define PHP_SSP_RECEIVE 3
#define PHP_SSP_SEND 4
#define PHP_SSP_CLOSE 5
#define PHP_SSP_STOP 6

#define PHP_SSP_RES_INDEX 0
#define PHP_SSP_RES_SOCKFD 1
#define PHP_SSP_RES_PORT 2

extern long le_ssp_descriptor,le_ssp_descriptor_ref;
extern unsigned int ssp_vars_length;
#ifdef SSP_CODE_TIMEOUT
	extern long ssp_timeout;
	#ifdef SSP_CODE_TIMEOUT_GLOBAL
		extern long ssp_global_timeout;
	#endif
#endif

extern zend_function_entry ssp_functions[];
extern zend_module_entry ssp_module_entry;

ZEND_BEGIN_MODULE_GLOBALS(ssp)
	long trigger_count;
	zval ssp_vars;
ZEND_END_MODULE_GLOBALS(ssp)

#define SSP_G(v) TSRMG(ssp_globals_id, zend_ssp_globals *, v)

#define MAKE_STD_ZVAL(vars) vars = (zval *) emalloc(sizeof(zval))
#define ZEND_REGISTER_RESOURCE(var, ctx, le_inflate) ZVAL_RES(var, zend_register_resource(ctx, le_inflate));

#ifdef SSP_CODE_TIMEOUT
	#ifdef SSP_CODE_TIMEOUT_GLOBAL
		#define TRIGGER_STARTUP()

		#define TRIGGER_SHUTDOWN()
	#else
		#define TRIGGER_STARTUP() \
			if((SSP_G(trigger_count)++) == 0) {\
				dprintf("--------------------------------------------TRIGGER_STARTUP---------------------------------------------------------------------------\n");\
				ssp_auto_globals_recreate();\
			}

		#define TRIGGER_SHUTDOWN() \
			if((--SSP_G(trigger_count)) == 0) {\
				dprintf("--------------------------------------------TRIGGER_SHUTDOWN---------------------------------------------------------------------------\n");\
			}
	#endif

	#define TRIGGER_STARTUP_EX() dprintf("############################################TRIGGER_STARTUP_EX#########################################################################\n");\
		TRIGGER_STARTUP()

	#define TRIGGER_SHUTDOWN_EX() TRIGGER_SHUTDOWN();\
		dprintf("############################################TRIGGER_SHUTDOWN_EX########################################################################\n")

	#define THREAD_STARTUP() ssp_request_startup()
	#define THREAD_SHUTDOWN() ssp_request_shutdown()
#else
	#define TRIGGER_STARTUP() \
		if((SSP_G(trigger_count)++) == 0) {\
			ssp_request_startup();\
			dprintf("-----------------------------------------ssp_request_startup------------------------------------------------------------------------------\n");\
			ssp_auto_globals_recreate();\
		}

	#define TRIGGER_SHUTDOWN() \
		if((--SSP_G(trigger_count)) == 0) {\
			dprintf("-----------------------------------------ssp_request_shutdown-----------------------------------------------------------------------------\n");\
			ssp_request_shutdown();\
		}

	#define TRIGGER_STARTUP_EX() TRIGGER_STARTUP()

	#define TRIGGER_SHUTDOWN_EX() TRIGGER_SHUTDOWN()

	#define THREAD_STARTUP() (void)ts_resource(0)
	#define THREAD_SHUTDOWN() (void)ts_resource(0)
#endif

static PHP_MINIT_FUNCTION(ssp);
static PHP_MSHUTDOWN_FUNCTION(ssp);
static PHP_RINIT_FUNCTION(ssp);
static PHP_RSHUTDOWN_FUNCTION(ssp);
static PHP_GINIT_FUNCTION(ssp);
static PHP_GSHUTDOWN_FUNCTION(ssp);
static PHP_MINFO_FUNCTION(ssp);

void ssp_auto_globals_recreate();
bool trigger(unsigned short type,...);

static PHP_FUNCTION(ssp_resource);
static PHP_FUNCTION(ssp_info);
static PHP_FUNCTION(ssp_send);
static PHP_FUNCTION(ssp_close);
static PHP_FUNCTION(ssp_destroy);
static PHP_FUNCTION(ssp_lock);
static PHP_FUNCTION(ssp_unlock);

static PHP_FUNCTION(ssp_stats);

ZEND_DECLARE_MODULE_GLOBALS(ssp)

#endif  /* PHP_EXT_H */
