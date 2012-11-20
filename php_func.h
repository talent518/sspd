#ifndef PHP_FUNC_H
#define PHP_FUNC_H

#include <stdbool.h>
#include "SAPI.h"

extern sapi_module_struct ssp_sapi_module;

extern char *request_init_file;

#define CSM(v) (ssp_sapi_module.v)

#ifdef ZTS
	#define SERVICE_STARTUP() ssp_request_startup()
	#define SERVICE_SHUTDOWN() ssp_request_shutdown();ssp_module_shutdown();ssp_destroy()
#else
	#define SERVICE_STARTUP()
	#define SERVICE_SHUTDOWN() ssp_module_shutdown();ssp_destroy()
#endif

#define TRIGGER_STARTUP()
#define TRIGGER_SHUTDOWN()

#ifdef ZTS
	#define THREAD_STARTUP() ssp_request_startup()
	#define THREAD_SHUTDOWN() ssp_request_shutdown();ts_free_thread()
#else
	#define THREAD_STARTUP()
	#define THREAD_SHUTDOWN()
#endif

void ssp_init();
void ssp_module_startup();
void ssp_request_startup();
void ssp_request_shutdown();
void ssp_module_shutdown();
void ssp_destroy();

#endif  /* PHP_FUNC_H */
