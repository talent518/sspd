#ifndef PHP_FUNC_H
#define PHP_FUNC_H

#include <stdbool.h>
#include "SAPI.h"

extern sapi_module_struct ssp_sapi_module;

extern char *request_init_file;

#define CSM(v) (ssp_sapi_module.v)

void ssp_init();
void ssp_module_startup();
void ssp_request_startup();
void ssp_request_shutdown();
void ssp_module_shutdown();
void ssp_destroy();

#endif  /* PHP_FUNC_H */
