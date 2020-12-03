#include "ssp_event.h"
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
	"html_errors=0\n"
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

#if PHP_VERSION_ID >= 80000
static void sapi_ssp_log_message(const char *message, int syslog_type_int) /* {{{ */
#else
static void sapi_ssp_log_message(char *message, int syslog_type_int) /* {{{ */
#endif
{
	if(php_get_module_initialized())
		fprintf(stderr, "%s - %s - %s\n", gettimeofstr(), SSP_G(threadname), message);
	else
		fprintf(stderr, "%s\n", message);
}
/* }}} */

static int sapi_ssp_deactivate() /* {{{ */
{
	fflush(stdout);
	fflush(stderr);
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
	INI_DEFAULT("display_errors", "0");
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

static void cli_register_file_handles(void) /* {{{ */
{
	php_stream *s_in, *s_out, *s_err;
	php_stream_context *sc_in=NULL, *sc_out=NULL, *sc_err=NULL;
	zend_constant ic, oc, ec;

	s_in  = php_stream_open_wrapper_ex("php://stdin",  "rb", 0, NULL, sc_in);
	s_out = php_stream_open_wrapper_ex("php://stdout", "wb", 0, NULL, sc_out);
	s_err = php_stream_open_wrapper_ex("php://stderr", "wb", 0, NULL, sc_err);

	if (s_in==NULL || s_out==NULL || s_err==NULL) {
		if (s_in) php_stream_close(s_in);
		if (s_out) php_stream_close(s_out);
		if (s_err) php_stream_close(s_err);
		return;
	}

#if PHP_DEBUG
	/* do not close stdout and stderr */
	s_out->flags |= PHP_STREAM_FLAG_NO_CLOSE;
	s_err->flags |= PHP_STREAM_FLAG_NO_CLOSE;
#endif

	php_stream_to_zval(s_in,  &ic.value);
	php_stream_to_zval(s_out, &oc.value);
	php_stream_to_zval(s_err, &ec.value);

	ZEND_CONSTANT_SET_FLAGS(&ic, CONST_CS, 0);
	ic.name = zend_string_init_interned("STDIN", sizeof("STDIN")-1, 0);
	zend_register_constant(&ic);

	ZEND_CONSTANT_SET_FLAGS(&oc, CONST_CS, 0);
	oc.name = zend_string_init_interned("STDOUT", sizeof("STDOUT")-1, 0);
	zend_register_constant(&oc);

	ZEND_CONSTANT_SET_FLAGS(&ec, CONST_CS, 0);
	ec.name = zend_string_init_interned("STDERR", sizeof("STDERR")-1, 0);
	zend_register_constant(&ec);
}

/* {{{ main
 */
void ssp_init(){
	signal(SIGPIPE, SIG_IGN);

	CSM(ini_defaults) = sapi_ssp_ini_defaults;
	CSM(php_ini_path_override) = NULL;
	CSM(phpinfo_as_text) = 1;
	CSM(php_ini_ignore_cwd) = 1;

	php_tsrm_startup();

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

static char *sg_argv[] = {"SERVER"};

void ssp_request_startup() {
	SG(request_info).argc = 1;
	SG(request_info).argv = sg_argv;
	SG(options) |= SAPI_OPTION_NO_CHDIR;
	
	ssp_request_startup_ex();
}

void ssp_request_startup_ex(){
	zend_file_handle zfd;
	char real_path[MAXPATHLEN];

	if (php_request_startup()==FAILURE) {
		printf("Request could not startup.\n");
		return;
	}

	if(listen_thread.tid != pthread_self()) {
		sigset_t mask;
		sigemptyset(&mask);
		sigaddset(&mask, SIGINT);
		sigprocmask(SIG_SETMASK, &mask, NULL);
	}

	cli_register_file_handles();

	zend_is_auto_global_str(ZEND_STRL("_SERVER"));

	PG(during_request_startup) = 0;

	FILE *fp = VCWD_FOPEN(request_init_file, "rb");
	if (fp) {
		zend_stream_init_fp(&zfd, fp, request_init_file);
		if (VCWD_REALPATH(request_init_file, real_path)) {
			SG(request_info).path_translated = strdup(real_path);
		} else {
			SG(request_info).path_translated = strdup(request_init_file);
		}
	} else {
		php_printf("Could not open input file: %s\n", request_init_file);
		return;
	}

	php_execute_script(&zfd);
}

void ssp_request_shutdown(){
	if(SG(request_info).path_translated) {
		free(SG(request_info).path_translated);
		SG(request_info).path_translated = NULL;
	}

	php_request_shutdown(NULL);
}

void ssp_module_shutdown(){
	CSM(shutdown)(&ssp_sapi_module);
}

void ssp_destroy(){
	sapi_shutdown();
	tsrm_shutdown();

	if (CSM(php_ini_path_override)) {
		free(CSM(php_ini_path_override));
	}
	if (CSM(ini_entries)) {
		free(CSM(ini_entries));
	}
}
/* }}} */
