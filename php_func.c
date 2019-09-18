#include "php_func.h"
#include "ssp.h"
#include "php_ext.h"

#include <php.h>
#include <php_globals.h>
#include <php_ini.h>
#include <php_main.h>
#include <php_variables.h>

#include <zend.h>
#include <zend_constants.h>
#include <zend_hash.h>
#include <zend_modules.h>

#include <SAPI.h>

#include <fopen_wrappers.h>
#include <ext/standard/php_standard.h>

#if HAVE_SIGNAL_H
	#include <signal.h>
#endif

#ifdef __riscos__
	#include <unixlib/local.h>
#endif

#include <zend_compile.h>
#include <zend_execute.h>
#include <zend_exceptions.h>

#define php_select(m, r, w, e, t)	select(m, r, w, e, t)

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

char *request_init_file=NULL;

const char HARDCODED_INI[] =
	"error_reporting = E_ALL ^ E_NOTICE\n"
	"html_errors=0\n"
	"display_errors=1\n"
	"register_argc_argv=1\n"
	"implicit_flush=1\n"
	"output_buffering=0\n"
	"max_execution_time=0\n"
	"max_input_time=-1\n\0";

static inline int sapi_ssp_select(int fd)
{
	fd_set wfd, dfd;
	struct timeval tv;
	int ret;

	FD_ZERO(&wfd);
	FD_ZERO(&dfd);

	PHP_SAFE_FD_SET(fd, &wfd);

	tv.tv_sec = (long)FG(default_socket_timeout);
	tv.tv_usec = 0;

	ret = php_select(fd+1, &dfd, &wfd, &dfd, &tv);

	return ret != -1;
}

static inline size_t sapi_ssp_single_write(const char *str, uint str_length) /* {{{ */
{
#ifdef PHP_WRITE_STDOUT
	register zend_long ret;
#else
	size_t ret;
#endif

#ifdef PHP_WRITE_STDOUT
	do {
		ret = write(STDOUT_FILENO, str, str_length);
	} while (ret <= 0 && errno == EAGAIN && sapi_ssp_select(STDOUT_FILENO));

	if (ret <= 0) {
		return 0;
	}

	return ret;
#else
	ret = fwrite(str, 1, MIN(str_length, 16384), stdout);
	return ret;
#endif
}
/* }}} */

static size_t sapi_ssp_ub_write(const char *str, size_t str_length) /* {{{ */
{
	register const char *ptr = str;
	register size_t remaining = str_length;
	register size_t ret;

	if (!str_length) {
		return 0;
	}

	while (remaining > 0)
	{
		ret = sapi_ssp_single_write(ptr, remaining);
		if (!ret) {
			php_handle_aborted_connection();
			break;
		}
		ptr += ret;
		remaining -= ret;
	}

	return (ptr - str);
}
/* }}} */

static void sapi_ssp_flush(void *server_context) /* {{{ */
{
	/* Ignore EBADF here, it's caused by the fact that STDIN/STDOUT/STDERR streams
	 * are/could be closed before fflush() is called.
	 */
	if (fflush(stdout)==EOF && errno!=EBADF) {
		php_handle_aborted_connection();
	}
}
/* }}} */

static void sapi_ssp_register_variables(zval *track_vars_array) /* {{{ */
{
	size_t len;
	char *docroot = "";

	/* In CGI mode, we consider the environment to be a part of the server
	 * variables
	 */
	php_import_environment_variables(track_vars_array);
	
	if(request_init_file == NULL) return;

	/* Build the special-case PHP_SELF variable for the CLI version */
	len = strlen(request_init_file);
	if (sapi_module.input_filter(PARSE_SERVER, "PHP_SELF", &request_init_file, len, &len)) {
		php_register_variable("PHP_SELF", request_init_file, track_vars_array);
	}
	if (sapi_module.input_filter(PARSE_SERVER, "SCRIPT_NAME", &request_init_file, len, &len)) {
		php_register_variable("SCRIPT_NAME", request_init_file, track_vars_array);
	}
	/* filenames are empty for stdin */
	len = strlen(request_init_file);
	if (sapi_module.input_filter(PARSE_SERVER, "SCRIPT_FILENAME", &request_init_file, len, &len)) {
		php_register_variable("SCRIPT_FILENAME", request_init_file, track_vars_array);
	}
	if (sapi_module.input_filter(PARSE_SERVER, "PATH_TRANSLATED", &request_init_file, len, &len)) {
		php_register_variable("PATH_TRANSLATED", request_init_file, track_vars_array);
	}
	/* just make it available */
	len = 0U;
	if (sapi_module.input_filter(PARSE_SERVER, "DOCUMENT_ROOT", &docroot, len, &len)) {
		php_register_variable("DOCUMENT_ROOT", docroot, track_vars_array);
	}
}
/* }}} */

static void sapi_ssp_log_message(char *message, int syslog_type_int) /* {{{ */
{
	fprintf(stderr, "%s\n", message);
}
/* }}} */

static int sapi_ssp_deactivate() /* {{{ */
{
	fflush(stdout);
	return SUCCESS;
}
/* }}} */

static char* sapi_ssp_read_cookies() /* {{{ */
{
	return NULL;
}
/* }}} */

static int sapi_ssp_header_handler(sapi_header_struct *h, sapi_header_op_enum op, sapi_headers_struct *s) /* {{{ */
{
	return 0;
}
/* }}} */

static int sapi_ssp_send_headers(sapi_headers_struct *sapi_headers) /* {{{ */
{
	/* We do nothing here, this function is needed to prevent that the fallback
	 * header handling is called. */
	return SAPI_HEADER_SENT_SUCCESSFULLY;
}
/* }}} */

static void sapi_ssp_send_header(sapi_header_struct *sapi_header, void *server_context) /* {{{ */
{
}
/* }}} */

static int php_ssp_startup(sapi_module_struct *sapi_module) /* {{{ */
{
	if (php_module_startup(sapi_module, &ssp_module_entry, 1)==FAILURE) {
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

static int php_ssp_shutdown(sapi_module_struct *sapi_module) /* {{{ */
{
	return php_module_shutdown_wrapper(sapi_module);
}
/* }}} */

/* {{{ sapi_ssp_ini_defaults */

/* overwriteable ini defaults must be set in sapi_ssp_ini_defaults() */
#define INI_DEFAULT(name,value)\
	ZVAL_NEW_STR(&tmp, zend_string_init(value, sizeof(value)-1, 1));\
	zend_hash_str_update(configuration_hash, name, sizeof(name)-1, &tmp);\

static void sapi_ssp_ini_defaults(HashTable *configuration_hash)
{
	zval tmp;
	INI_DEFAULT("report_zend_debug", "0");
	INI_DEFAULT("display_errors", "1");
	INI_DEFAULT("memory_limit", "-1");
	INI_DEFAULT("zend.enable_gc","1");
}
/* }}} */

/* {{{ sapi_module_struct ssp_sapi_module
 */
sapi_module_struct ssp_sapi_module = {
	"SSP",							/* name */
	"Command Line Interface",    	/* pretty name */

	php_ssp_startup,				/* startup */
	php_ssp_shutdown,				/* shutdown */

	NULL,							/* activate */
	sapi_ssp_deactivate,			/* deactivate */

	sapi_ssp_ub_write,		    	/* unbuffered write */
	sapi_ssp_flush,				    /* flush */
	NULL,							/* get uid */
	NULL,							/* getenv */

	php_error,						/* error handler */

	sapi_ssp_header_handler,		/* header handler */
	sapi_ssp_send_headers,			/* send headers handler */
	sapi_ssp_send_header,			/* send header handler */

	NULL,				            /* read POST data */
	sapi_ssp_read_cookies,          /* read Cookies */

	sapi_ssp_register_variables,	/* register server variables */
	sapi_ssp_log_message,			/* Log message */
	NULL,							/* Get request time */
	NULL,							/* Child terminate */

	STANDARD_SAPI_MODULE_PROPERTIES
};
/* }}} */

/* {{{ arginfo ext/standard/dl.c */
ZEND_BEGIN_ARG_INFO(arginfo_dl, 0)
	ZEND_ARG_INFO(0, extension_filename)
ZEND_END_ARG_INFO()
/* }}} */

static const zend_function_entry additional_functions[] = {
	ZEND_FE(dl, arginfo_dl)
	{NULL, NULL, NULL}
};

#if 0
ZEND_API zval *zend_get_configuration_directive(zend_string *name) {
	printf("%s: %s\n", __func__, ZSTR_VAL(name));

	return cfg_get_entry_ex(name);
}
#endif

/* {{{ main
 */
void ssp_init(){
	signal(SIGPIPE, SIG_IGN);

	CSM(ini_defaults) = sapi_ssp_ini_defaults;
	CSM(php_ini_path_override) = NULL;
	CSM(phpinfo_as_text) = 1;
	CSM(php_ini_ignore_cwd) = 1;

	tsrm_startup(1, 1, 0, NULL);
	(void)ts_resource(0);

	zend_signal_startup();

	sapi_startup(&ssp_sapi_module);

	CSM(ini_entries) = malloc(sizeof(HARDCODED_INI));
	memcpy(CSM(ini_entries), HARDCODED_INI, sizeof(HARDCODED_INI));

	CSM(additional_functions) = additional_functions;
}

void ssp_module_startup(){
	if (CSM(startup)(&ssp_sapi_module)==FAILURE) {
		printf("Module could not startup.\n");
	}
}

void ssp_request_startup(){
	zend_file_handle zfd;

	(void)ts_resource(0);

	zfd.type = ZEND_HANDLE_FILENAME;
	zfd.opened_path = NULL;

	char real_path[MAXPATHLEN];
	if (VCWD_REALPATH(request_init_file, real_path)) {
		zfd.filename = strdup(real_path);
		zfd.free_filename = 1;
	} else {
		zfd.filename = request_init_file;
		zfd.free_filename = 0;
	}

	if (php_request_startup()==FAILURE) {
		printf("Request could not startup.\n");
		return;
	}

	REGISTER_MAIN_STRING_CONSTANT("SSP_PIDFILE",ssp_pidfile,CONST_CS | CONST_PERSISTENT);
	REGISTER_MAIN_STRING_CONSTANT("SSP_USER",ssp_user,CONST_CS | CONST_PERSISTENT);
	REGISTER_MAIN_STRING_CONSTANT("SSP_HOST",ssp_host,CONST_CS | CONST_PERSISTENT);
	REGISTER_MAIN_LONG_CONSTANT("SSP_PORT",ssp_port,CONST_CS | CONST_PERSISTENT);
	REGISTER_MAIN_LONG_CONSTANT("SSP_MAX_CLIENTS",ssp_maxclients,CONST_CS | CONST_PERSISTENT);
	REGISTER_MAIN_LONG_CONSTANT("SSP_MAX_RECVS",ssp_maxrecvs,CONST_CS | CONST_PERSISTENT);
	REGISTER_MAIN_LONG_CONSTANT("SSP_NTHREADS",ssp_nthreads,CONST_CS | CONST_PERSISTENT);
	REGISTER_MAIN_LONG_CONSTANT("SSP_BACKLOG",ssp_backlog,CONST_CS | CONST_PERSISTENT);
#ifdef SSP_CODE_TIMEOUT
	REGISTER_MAIN_LONG_CONSTANT("SSP_TIMEOUT",ssp_timeout,CONST_CS | CONST_PERSISTENT);
	#ifdef SSP_CODE_TIMEOUT_GLOBAL
		REGISTER_MAIN_LONG_CONSTANT("SSP_CODE_TIMEOUT_GLOBAL",ssp_global_timeout,CONST_CS | CONST_PERSISTENT);
	#endif
#endif

	zend_is_auto_global_str(ZEND_STRL("_SERVER"));

	php_execute_script(&zfd);
}

void ssp_request_shutdown(){
	zend_hash_str_del(EG(zend_constants), "SSP_PIDFILE", sizeof("SSP_PIDFILE") - 1);
	zend_hash_str_del(EG(zend_constants), "SSP_USER", sizeof("SSP_USER") - 1);
	zend_hash_str_del(EG(zend_constants), "SSP_HOST", sizeof("SSP_HOST") - 1);
	zend_hash_str_del(EG(zend_constants), "SSP_PORT", sizeof("SSP_PORT") - 1);
	zend_hash_str_del(EG(zend_constants), "SSP_MAX_CLIENTS", sizeof("SSP_MAX_CLIENTS") - 1);
	zend_hash_str_del(EG(zend_constants), "SSP_MAX_RECVS", sizeof("SSP_MAX_RECVS") - 1);
	zend_hash_str_del(EG(zend_constants), "SSP_NTHREADS", sizeof("SSP_NTHREADS") - 1);
	zend_hash_str_del(EG(zend_constants), "SSP_BACKLOG", sizeof("SSP_BACKLOG") - 1);

#ifdef SSP_CODE_TIMEOUT
	zend_hash_str_del(EG(zend_constants), "SSP_TIMEOUT", sizeof("SSP_TIMEOUT")-1);
	#ifdef SSP_CODE_TIMEOUT_GLOBAL
		zend_hash_str_del(EG(zend_constants), "SSP_CODE_TIMEOUT_GLOBAL", sizeof("SSP_CODE_TIMEOUT_GLOBAL")-1);
	#endif
#endif

	php_request_shutdown(NULL);
}

void ssp_module_shutdown(){
	CSM(shutdown)(&ssp_sapi_module);
}

void ssp_destroy(){
	if (CSM(php_ini_path_override)) {
		free(CSM(php_ini_path_override));
	}
	if (CSM(ini_entries)) {
		free(CSM(ini_entries));
	}

	sapi_shutdown();
	tsrm_shutdown();
}
/* }}} */
