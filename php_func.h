#ifndef PHP_FUNC_H
#define PHP_FUNC_H

#include <stdbool.h>
#include "SAPI.h"

extern sapi_module_struct ssp_sapi_module;

extern char *request_init_file;

#define CSM(v) (ssp_sapi_module.v)

void php_init();
int php_begin();

void ssp_request_startup();
void ssp_request_shutdown();

void php_end();

#endif  /* PHP_FUNC_H */
