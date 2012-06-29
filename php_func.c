#include "php_func.h"
#include "php_ext.h"

#include "php.h"
#include "php_globals.h"
#include "php_main.h"
#include "php_variables.h"

#include "zend.h"
#include "zend_hash.h"
#include "zend_modules.h"

#include "SAPI.h"

#include "fopen_wrappers.h"
#include "ext/standard/php_standard.h"
#ifdef PHP_WIN32
	#include <io.h>
	#include <fcntl.h>
	#include "win32/php_registry.h"
#endif

#if HAVE_SIGNAL_H
	#include <signal.h>
#endif

#ifdef __riscos__
	#include <unixlib/local.h>
#endif

#include "zend_compile.h"
#include "zend_execute.h"
#include "zend_exceptions.h"

#ifndef PHP_WIN32
	#define php_select(m, r, w, e, t)	select(m, r, w, e, t)
#else
	#include "win32/select.h"
#endif

const char HARDCODED_INI[] =
	"error_reporting = E_ALL & ~E_NOTICE\n"
	"display_errors=On\n"
	"html_errors=0\n"
	"register_argc_argv=0\n"
	"implicit_flush=1\n"
	"output_buffering=0\n"
	"max_execution_time=0\n"
	"max_input_time=-1\n\0";

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

static inline int sapi_cli_select(int fd TSRMLS_DC)
{
	fd_set wfd, dfd;
	struct timeval tv;
	int ret;

	FD_ZERO(&wfd);
	FD_ZERO(&dfd);

	PHP_SAFE_FD_SET(fd, &wfd);

	tv.tv_sec = FG(default_socket_timeout);
	tv.tv_usec = 0;

	ret = php_select(fd+1, &dfd, &wfd, &dfd, &tv);

	return ret != -1;
}

static inline size_t sapi_cli_single_write(const char *str, uint str_length TSRMLS_DC) /* {{{ */
{
#ifdef PHP_WRITE_STDOUT
	long ret;

	do {
		ret = write(STDOUT_FILENO, str, str_length);
	} while (ret <= 0 && errno == EAGAIN && sapi_cli_select(STDOUT_FILENO TSRMLS_CC));

	if (ret <= 0) {
		return 0;
	}

	return ret;
#else
	size_t ret;

	ret = fwrite(str, 1, MIN(str_length, 16384), stdout);
	return ret;
#endif
}
/* }}} */

static int sapi_cli_ub_write(const char *str, uint str_length TSRMLS_DC) /* {{{ */
{
	const char *ptr = str;
	uint remaining = str_length;
	size_t ret;

	while (remaining > 0)
	{
		ret = sapi_cli_single_write(ptr, remaining TSRMLS_CC);
		if (!ret) {
#ifndef PHP_CLI_WIN32_NO_CONSOLE
			php_handle_aborted_connection();
#endif
			break;
		}
		ptr += ret;
		remaining -= ret;
	}

	return (ptr - str);
}
/* }}} */

static void sapi_cli_flush(void *server_context) /* {{{ */
{
	/* Ignore EBADF here, it's caused by the fact that STDIN/STDOUT/STDERR streams
	 * are/could be closed before fflush() is called.
	 */
	if (fflush(stdout)==EOF && errno!=EBADF) {
#ifndef PHP_CLI_WIN32_NO_CONSOLE
		php_handle_aborted_connection();
#endif
	}
}
/* }}} */

static void sapi_cli_register_variables(zval *track_vars_array TSRMLS_DC) /* {{{ */
{
	unsigned int len;
	char   *docroot;

	/* In CGI mode, we consider the environment to be a part of the server
	 * variables
	 */
	php_import_environment_variables(track_vars_array TSRMLS_CC);

	/* Build the special-case PHP_SELF variable for the CLI version */
	len = strlen(php_self);
	if (sapi_module.input_filter(PARSE_SERVER, "PHP_SELF", &php_self, len, &len TSRMLS_CC)) {
		php_register_variable("PHP_SELF", php_self, track_vars_array TSRMLS_CC);
	}
	if (sapi_module.input_filter(PARSE_SERVER, "SCRIPT_NAME", &php_self, len, &len TSRMLS_CC)) {
		php_register_variable("SCRIPT_NAME", php_self, track_vars_array TSRMLS_CC);
	}
	/* filenames are empty for stdin */
	len = strlen(script_filename);
	if (sapi_module.input_filter(PARSE_SERVER, "SCRIPT_FILENAME", &script_filename, len, &len TSRMLS_CC)) {
		php_register_variable("SCRIPT_FILENAME", script_filename, track_vars_array TSRMLS_CC);
	}
	if (sapi_module.input_filter(PARSE_SERVER, "PATH_TRANSLATED", &script_filename, len, &len TSRMLS_CC)) {
		php_register_variable("PATH_TRANSLATED", script_filename, track_vars_array TSRMLS_CC);
	}
	/* just make it available */
	len = 0U;
	if (sapi_module.input_filter(PARSE_SERVER, "DOCUMENT_ROOT", &docroot, len, &len TSRMLS_CC)) {
		php_register_variable("DOCUMENT_ROOT", docroot, track_vars_array TSRMLS_CC);
	}
}
/* }}} */

static void sapi_cli_log_message(char *message) /* {{{ */
{
	fprintf(stderr, "%s\n", message);
}
/* }}} */

static int sapi_cli_deactivate(TSRMLS_D) /* {{{ */
{
	fflush(stdout);
	if(SG(request_info).argv0) {
		free(SG(request_info).argv0);
		SG(request_info).argv0 = NULL;
	}
	return SUCCESS;
}
/* }}} */

static char* sapi_cli_read_cookies(TSRMLS_D) /* {{{ */
{
	return NULL;
}
/* }}} */

static int sapi_cli_header_handler(sapi_header_struct *h, sapi_header_op_enum op, sapi_headers_struct *s TSRMLS_DC) /* {{{ */
{
	return 0;
}
/* }}} */

static int sapi_cli_send_headers(sapi_headers_struct *sapi_headers TSRMLS_DC) /* {{{ */
{
	/* We do nothing here, this function is needed to prevent that the fallback
	 * header handling is called. */
	return SAPI_HEADER_SENT_SUCCESSFULLY;
}
/* }}} */

static void sapi_cli_send_header(sapi_header_struct *sapi_header, void *server_context TSRMLS_DC) /* {{{ */
{
}
/* }}} */

static int php_cli_startup(sapi_module_struct *sapi_module) /* {{{ */
{
	if (php_module_startup(sapi_module, &ssp_module_entry, 1)==FAILURE) {
		return FAILURE;
	}
	return SUCCESS;
}
/* }}} */

/* {{{ sapi_cli_ini_defaults */

/* overwriteable ini defaults must be set in sapi_cli_ini_defaults() */
#define INI_DEFAULT(name,value)\
	Z_SET_REFCOUNT(tmp, 0);\
	Z_UNSET_ISREF(tmp);	\
	ZVAL_STRINGL(&tmp, zend_strndup(value, sizeof(value)-1), sizeof(value)-1, 0);\
	zend_hash_update(configuration_hash, name, sizeof(name), &tmp, sizeof(zval), NULL);\

static void sapi_cli_ini_defaults(HashTable *configuration_hash)
{
	zval tmp;
	INI_DEFAULT("report_zend_debug", "0");
	INI_DEFAULT("display_errors", "1");
}
/* }}} */

/* {{{ sapi_module_struct cli_sapi_module
 */
sapi_module_struct cli_sapi_module = {
	"SSP",							/* name */
	"Command Line Interface",    	/* pretty name */

	php_cli_startup,				/* startup */
	php_module_shutdown_wrapper,	/* shutdown */

	NULL,							/* activate */
	sapi_cli_deactivate,			/* deactivate */

	sapi_cli_ub_write,		    	/* unbuffered write */
	sapi_cli_flush,				    /* flush */
	NULL,							/* get uid */
	NULL,							/* getenv */

	php_error,						/* error handler */

	sapi_cli_header_handler,		/* header handler */
	sapi_cli_send_headers,			/* send headers handler */
	sapi_cli_send_header,			/* send header handler */

	NULL,				            /* read POST data */
	sapi_cli_read_cookies,          /* read Cookies */

	sapi_cli_register_variables,	/* register server variables */
	sapi_cli_log_message,			/* Log message */
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

volatile int module_started = 0;

/* {{{ main
 */
void php_init(){
#ifdef ZTS
	void ***tsrm_ls;
#endif

#ifdef ZTS
	tsrm_startup(1, 1, 0, NULL);
	tsrm_ls = ts_resource(0);
#endif

	CSM(ini_defaults) = sapi_cli_ini_defaults;
	CSM(php_ini_path_override) = NULL;
	CSM(phpinfo_as_text) = 1;
	sapi_startup(&cli_sapi_module);

	ini_entries_len = sizeof(HARDCODED_INI)-2;
	CSM(ini_entries) = malloc(sizeof(HARDCODED_INI));
	memcpy(CSM(ini_entries), HARDCODED_INI, sizeof(HARDCODED_INI));

	CSM(additional_functions) = additional_functions;
}

int php_begin(){
	/* startup after we get the above ini override se we get things right */
	if (CSM(startup)(&cli_sapi_module)==FAILURE) {
		/* there is no way to see if we must call zend_ini_deactivate()
		 * since we cannot check if EG(ini_directives) has been initialised
		 * because the executor's constructor does not set initialize it.
		 * Apart from that there seems no need for zend_ini_deactivate() yet.
		 * So we goto out_err.*/
		#ifdef ZTS
				tsrm_shutdown();
		#endif
		return FAILURE;
	}
	module_started=1;
	return SUCCESS;
}

void php_end(){
	sapi_deactivate(TSRMLS_C);
	zend_ini_deactivate(TSRMLS_C);

	if (CSM(php_ini_path_override)) {
		free(CSM(php_ini_path_override));
	}
	if (CSM(ini_entries)) {
		free(CSM(ini_entries));
	}

	if (module_started) {
		php_module_shutdown(TSRMLS_C);
	}
	sapi_shutdown();
#ifdef ZTS
	tsrm_shutdown();
#endif
}
/* }}} */
