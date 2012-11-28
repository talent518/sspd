#include "php_func.h"
#include "server.h"
#include "php_ext.h"

#include "php.h"
#include "php_globals.h"
#include "php_ini.h"
#include "php_main.h"
#include "php_variables.h"

#include "zend.h"
#include "zend_constants.h"
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

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1
#endif

char *request_init_file=NULL;

const char HARDCODED_INI[] =
	"error_reporting = E_ALL ^ E_NOTICE\n"
	"html_errors=0\n"
	"register_argc_argv=0\n"
	"implicit_flush=1\n"
	"output_buffering=0\n"
	"max_execution_time=0\n"
	"max_input_time=-1\n\0";

/* {{{ ssp_seek_file_begin
 */
static int ssp_seek_file_begin(zend_file_handle *file_handle, char *script_file, int *lineno TSRMLS_DC)
{
	char c;

	*lineno = 1;

	file_handle->type = ZEND_HANDLE_FP;
	file_handle->opened_path = NULL;
	file_handle->free_filename = 0;
	if (!(file_handle->handle.fp = VCWD_FOPEN(script_file, "rb"))) {
		php_printf("Could not open input file: %s\n", script_file);
		return FAILURE;
	}
	file_handle->filename = script_file;

	/* #!php support */
	c = fgetc(file_handle->handle.fp);
	if (c == '#' && (c = fgetc(file_handle->handle.fp)) == '!') {
		while (c != '\n' && c != '\r' && c != EOF) {
			c = fgetc(file_handle->handle.fp);	/* skip to end of line */
		}
		/* handle situations where line is terminated by \r\n */
		if (c == '\r') {
			if (fgetc(file_handle->handle.fp) != '\n') {
				long pos = ftell(file_handle->handle.fp);
				fseek(file_handle->handle.fp, pos - 1, SEEK_SET);
			}
		}
		*lineno = 2;
	} else {
		rewind(file_handle->handle.fp);
	}

	return SUCCESS;
}
/* }}} */

static php_stream *s_in_process = NULL;

static void ssp_register_file_handles(TSRMLS_D) /* {{{ */
{
	zval *zin, *zout, *zerr;
	php_stream *s_in, *s_out, *s_err;
	php_stream_context *sc_in=NULL, *sc_out=NULL, *sc_err=NULL;
	zend_constant ic, oc, ec;
	
	MAKE_STD_ZVAL(zin);
	MAKE_STD_ZVAL(zout);
	MAKE_STD_ZVAL(zerr);

	s_in  = php_stream_open_wrapper_ex("php://stdin",  "rb", 0, NULL, sc_in);
	s_out = php_stream_open_wrapper_ex("php://stdout", "wb", 0, NULL, sc_out);
	s_err = php_stream_open_wrapper_ex("php://stderr", "wb", 0, NULL, sc_err);

	if (s_in==NULL || s_out==NULL || s_err==NULL) {
		FREE_ZVAL(zin);
		FREE_ZVAL(zout);
		FREE_ZVAL(zerr);
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

	s_in_process = s_in;

	php_stream_to_zval(s_in,  zin);
	php_stream_to_zval(s_out, zout);
	php_stream_to_zval(s_err, zerr);
	
	ic.value = *zin;
	ic.flags = CONST_CS;
	ic.name = zend_strndup(ZEND_STRL("STDIN"));
	ic.name_len = sizeof("STDIN");
	ic.module_number = 0;
	zend_register_constant(&ic TSRMLS_CC);

	oc.value = *zout;
	oc.flags = CONST_CS;
	oc.name = zend_strndup(ZEND_STRL("STDOUT"));
	oc.name_len = sizeof("STDOUT");
	oc.module_number = 0;
	zend_register_constant(&oc TSRMLS_CC);

	ec.value = *zerr;
	ec.flags = CONST_CS;
	ec.name = zend_strndup(ZEND_STRL("STDERR"));
	ec.name_len = sizeof("STDERR");
	ec.module_number = 0;
	zend_register_constant(&ec TSRMLS_CC);

	FREE_ZVAL(zin);
	FREE_ZVAL(zout);
	FREE_ZVAL(zerr);
}
/* }}} */

static inline int sapi_ssp_select(int fd TSRMLS_DC)
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

static inline size_t sapi_ssp_single_write(const char *str, uint str_length TSRMLS_DC) /* {{{ */
{
#ifdef PHP_WRITE_STDOUT
	long ret;

	do {
		ret = write(STDOUT_FILENO, str, str_length);
	} while (ret <= 0 && errno == EAGAIN && sapi_ssp_select(STDOUT_FILENO TSRMLS_CC));

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

static int sapi_ssp_ub_write(const char *str, uint str_length TSRMLS_DC) /* {{{ */
{
	const char *ptr = str;
	uint remaining = str_length;
	size_t ret;

	while (remaining > 0)
	{
		ret = sapi_ssp_single_write(ptr, remaining TSRMLS_CC);
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

static void sapi_ssp_flush(void *server_context) /* {{{ */
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

static void sapi_ssp_register_variables(zval *track_vars_array TSRMLS_DC) /* {{{ */
{
	unsigned int len;
	char   *docroot;

	/* In CGI mode, we consider the environment to be a part of the server
	 * variables
	 */
	php_import_environment_variables(track_vars_array TSRMLS_CC);

	/* Build the special-case PHP_SELF variable for the CLI version */
	len = strlen(request_init_file);
	if (sapi_module.input_filter(PARSE_SERVER, "PHP_SELF", &request_init_file, len, &len TSRMLS_CC)) {
		php_register_variable("PHP_SELF", request_init_file, track_vars_array TSRMLS_CC);
	}
	if (sapi_module.input_filter(PARSE_SERVER, "SCRIPT_NAME", &request_init_file, len, &len TSRMLS_CC)) {
		php_register_variable("SCRIPT_NAME", request_init_file, track_vars_array TSRMLS_CC);
	}
	/* filenames are empty for stdin */
	len = strlen(request_init_file);
	if (sapi_module.input_filter(PARSE_SERVER, "SCRIPT_FILENAME", &request_init_file, len, &len TSRMLS_CC)) {
		php_register_variable("SCRIPT_FILENAME", request_init_file, track_vars_array TSRMLS_CC);
	}
	if (sapi_module.input_filter(PARSE_SERVER, "PATH_TRANSLATED", &request_init_file, len, &len TSRMLS_CC)) {
		php_register_variable("PATH_TRANSLATED", request_init_file, track_vars_array TSRMLS_CC);
	}
	/* just make it available */
	len = 0U;
	if (sapi_module.input_filter(PARSE_SERVER, "DOCUMENT_ROOT", &docroot, len, &len TSRMLS_CC)) {
		php_register_variable("DOCUMENT_ROOT", docroot, track_vars_array TSRMLS_CC);
	}
}
/* }}} */

static void sapi_ssp_log_message(char *message) /* {{{ */
{
	fprintf(stderr, "%s\n", message);
}
/* }}} */

static int sapi_ssp_deactivate(TSRMLS_D) /* {{{ */
{
	fflush(stdout);
	if(SG(request_info).argv0) {
		free(SG(request_info).argv0);
		SG(request_info).argv0 = NULL;
	}
	return SUCCESS;
}
/* }}} */

static char* sapi_ssp_read_cookies(TSRMLS_D) /* {{{ */
{
	return NULL;
}
/* }}} */

static int sapi_ssp_header_handler(sapi_header_struct *h, sapi_header_op_enum op, sapi_headers_struct *s TSRMLS_DC) /* {{{ */
{
	return 0;
}
/* }}} */

static int sapi_ssp_send_headers(sapi_headers_struct *sapi_headers TSRMLS_DC) /* {{{ */
{
	/* We do nothing here, this function is needed to prevent that the fallback
	 * header handling is called. */
	return SAPI_HEADER_SENT_SUCCESSFULLY;
}
/* }}} */

static void sapi_ssp_send_header(sapi_header_struct *sapi_header, void *server_context TSRMLS_DC) /* {{{ */
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
	Z_SET_REFCOUNT(tmp, 0);\
	Z_UNSET_ISREF(tmp);	\
	ZVAL_STRINGL(&tmp, zend_strndup(value, sizeof(value)-1), sizeof(value)-1, 0);\
	zend_hash_update(configuration_hash, name, sizeof(name), &tmp, sizeof(zval), NULL);\

static void sapi_ssp_ini_defaults(HashTable *configuration_hash)
{
	zval tmp;
	INI_DEFAULT("report_zend_debug", "1");
	INI_DEFAULT("display_errors", "1");
	INI_DEFAULT("memory_limit", "256M");
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

/* {{{ main
 */
void ssp_init(){
	CSM(ini_defaults) = sapi_ssp_ini_defaults;
	CSM(php_ini_path_override) = NULL;
	CSM(phpinfo_as_text) = 1;

#ifdef ZTS
	tsrm_startup(128, 1, 0, NULL);
#endif
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
	TSRMLS_FETCH();
	int lineno = 0;
	zend_file_handle file_handle;

	if (request_init_file) {
		if (ssp_seek_file_begin(&file_handle, request_init_file, &lineno TSRMLS_CC) != SUCCESS) {
			return;
		}
	} else {
		file_handle.filename = "-";
		//file_handle.handle.fp = stdin;
	}
	file_handle.type = ZEND_HANDLE_FILENAME;
	file_handle.opened_path = NULL;
	file_handle.free_filename = 0;

	SG(request_info).path_translated = file_handle.filename;

	if (php_request_startup(TSRMLS_C)==FAILURE) {
		fclose(file_handle.handle.fp);
		printf("Request could not startup.\n");
		return;
	}

	CG(unclean_shutdown)=0;

	CG(start_lineno) = lineno;

	zend_is_auto_global("_SERVER", sizeof("_SERVER")-1 TSRMLS_CC);

	REGISTER_MAIN_STRING_CONSTANT("SSP_PIDFILE",ssp_pidfile,CONST_CS | CONST_PERSISTENT);
	REGISTER_MAIN_STRING_CONSTANT("SSP_USER",ssp_user,CONST_CS | CONST_PERSISTENT);
	REGISTER_MAIN_STRING_CONSTANT("SSP_HOST",ssp_host,CONST_CS | CONST_PERSISTENT);
	REGISTER_MAIN_LONG_CONSTANT("SSP_PORT",ssp_port,CONST_CS | CONST_PERSISTENT);
	REGISTER_MAIN_LONG_CONSTANT("SSP_MAX_CLIENTS",ssp_maxclients,CONST_CS | CONST_PERSISTENT);
	REGISTER_MAIN_LONG_CONSTANT("SSP_MAX_RECVS",ssp_maxrecvs,CONST_CS | CONST_PERSISTENT);
#ifdef PHP_WIN32
	REGISTER_MAIN_STRINGL_CONSTANT("STD_CHARSET","gbk",sizeof("gbk"),CONST_CS | CONST_PERSISTENT);
#else
	REGISTER_MAIN_STRINGL_CONSTANT("STD_CHARSET","utf-8",sizeof("utf-8"),CONST_CS | CONST_PERSISTENT);
#endif
#ifdef PHP_SSP_DEBUG
	REGISTER_MAIN_LONG_CONSTANT("IS_DEBUG",1,CONST_CS | CONST_PERSISTENT);
#endif

	if (strcmp(file_handle.filename, "-")) {
		ssp_register_file_handles(TSRMLS_C);
		zend_execute_scripts(ZEND_REQUIRE TSRMLS_CC, NULL, 1, &file_handle);
		zend_destroy_file_handle(&file_handle TSRMLS_CC);
		//php_execute_script(&file_handle TSRMLS_CC);
	}
}

void ssp_request_shutdown(){
	TSRMLS_FETCH();

	php_request_shutdown(NULL);

	zend_hash_del(EG(zend_constants),"SSP_PIDFILE",sizeof("SSP_PIDFILE"));
	zend_hash_del(EG(zend_constants),"SSP_USER",sizeof("SSP_USER"));
	zend_hash_del(EG(zend_constants),"SSP_HOST",sizeof("SSP_HOST"));
	zend_hash_del(EG(zend_constants),"SSP_PORT",sizeof("SSP_PORT"));
	zend_hash_del(EG(zend_constants),"SSP_MAX_CLIENTS",sizeof("SSP_MAX_CLIENTS"));
	zend_hash_del(EG(zend_constants),"SSP_MAX_RECVS",sizeof("SSP_MAX_RECVS"));

	zend_hash_del(EG(zend_constants),"STD_CHARSET",sizeof("STD_CHARSET"));
	zend_hash_del(EG(zend_constants),"IS_DEBUG",sizeof("IS_DEBUG"));
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

	//printf("%s:0\n",__func__);
	sapi_shutdown();
	//printf("%s:1\n",__func__);
#ifdef ZTS
	tsrm_shutdown();
	//printf("%s:2\n",__func__);
#endif
}
/* }}} */
