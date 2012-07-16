#ifndef PHP_FUNC_H
#define PHP_FUNC_H

#include "SAPI.h"

extern sapi_module_struct ssp_sapi_module;

int ini_entries_len;
static char *php_self = "";
static char *script_filename = "";

static int ssp_request_started=0;

#define CSM(v) (ssp_sapi_module.v)

void php_init();
int php_begin();

int ssp_request_startup(char *script_file);

void php_end();

#endif  /* PHP_FUNC_H */
