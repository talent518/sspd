#ifndef PHP_FUNC_H
#define PHP_FUNC_H

#include "SAPI.h"

extern sapi_module_struct cli_sapi_module;

int ini_entries_len;
static char *php_self = "";
static char *script_filename = "";

#define CSM(v) (cli_sapi_module.v)

void php_init();
int php_begin();
void php_end();

#endif  /* PHP_FUNC_H */
