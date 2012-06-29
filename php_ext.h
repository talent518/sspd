#ifndef PHP_EXT_H
#define PHP_EXT_H

#include "php.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//#define PHP_SSP_DEBUG

#define PHP_SSP_DESCRIPTOR_RES_NAME "ssp user node"
#define PHP_SSP_MUTEX_DESCRIPTOR_RES_NAME "ssp thread mutex"
#define PHP_SSP_VERSION "v1.0.6"

#define PHP_SSP_START 0
#define PHP_SSP_RECEIVE 1
#define PHP_SSP_SEND 2
#define PHP_SSP_CONNECT 3
#define PHP_SSP_CONNECT_DENIED 4
#define PHP_SSP_CLOSE 5
#define PHP_SSP_STOP 6

#define PHP_SSP_LEN 7

#define PHP_SSP_OPT_USER 0
#define PHP_SSP_OPT_PIDFILE 1
#define PHP_SSP_OPT_HOST 2
#define PHP_SSP_OPT_PORT 3
#define PHP_SSP_OPT_MAX_CLIENTS 4
#define PHP_SSP_OPT_MAX_RECVS 5

extern int le_ssp_descriptor;

extern function_entry ssp_functions[];
extern zend_module_entry ssp_module_entry;

#define phpext_ssp_ptr &ssp_module_entry

#ifdef ZTS
#include "TSRM.h"
#endif

ZEND_BEGIN_MODULE_GLOBALS(ssp)
	char *user;
	char *pidfile;
	char *host;
	short int port;
	int maxclients;
	int maxrecvs;
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

int trigger(unsigned short type,...);

static PHP_FUNCTION(ssp_setopt);
static PHP_FUNCTION(ssp_bind);
static PHP_FUNCTION(ssp_resource);
static PHP_FUNCTION(ssp_info);
static PHP_FUNCTION(ssp_mutex_create);
static PHP_FUNCTION(ssp_mutex_destroy);
static PHP_FUNCTION(ssp_mutex_lock);
static PHP_FUNCTION(ssp_mutex_unlock);
static PHP_FUNCTION(ssp_send);
static PHP_FUNCTION(ssp_close);

ZEND_DECLARE_MODULE_GLOBALS(ssp)

#endif  /* PHP_EXT_H */
