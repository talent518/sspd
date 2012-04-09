/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2011 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Omar Kilani <omar@php.net>                                   |
  +----------------------------------------------------------------------+
*/

/* $Id: ssp.c 308529 2011-02-21 08:09:02Z scottmac $ */

#include "php_func.h"
#include "server.h"

#include "php.h"
#include "php_ini.h"
#include "php_getopt.h"
#include "zend_extensions.h"
#include "zend_hash.h"

#include <stdio.h>
#ifdef PHP_WIN32
	#include "win32/time.h"
	#include "win32/signal.h"
	#include <process.h>
#endif
#if HAVE_SYS_TIME_H
	#include <sys/time.h>
#endif
#if HAVE_UNISTD_H
	#include <unistd.h>
#endif
#if HAVE_SIGNAL_H
	#include <signal.h>
#endif
#if HAVE_SETLOCALE
	#include <locale.h>
#endif

#define SSP_RESTART		0
#define SSP_START		1
#define SSP_STOP		2
#define SSP_INSTALL		3
#define SSP_UNINSTALL	4

#define PHP_MODE_STANDARD			1
#define PHP_MODE_CLI_DIRECT			2
#define PHP_MODE_SHOW_INI_CONFIG	3

static char *php_optarg = NULL;
static int php_optind = 1;

static const char *param_mode_conflict = "Either execute direct code, process stdin or use a file.\n";

PHPAPI extern char *php_ini_opened_path;
PHPAPI extern char *php_ini_scanned_path;
PHPAPI extern char *php_ini_scanned_files;

static const opt_struct OPTIONS[] = {
	{'c', 1, "php-ini"},
	{'d', 1, "define"},
	{'f', 1, "file"},
	{'h', 0, "help"},
	{'i', 0, "info"},
	{'m', 0, "modules"},
	{'n', 0, "no-php-ini"},
	{'R', 1, "process-code"},
	{'H', 0, "hide-args"},
	{'r', 1, "run"},
	{'?', 0, "usage"},/* help alias (both '?' and 'usage') */
	{'v', 0, "version"},
	{'z', 1, "zend-extension"},
	{14,  0, "ini"},
	{'-', 0, NULL} /* end of args */
};

/* {{{ php_cli_usage
 */
static void php_cli_usage(char *argv0)
{
	char *prog;

	prog = strrchr(argv0, '/');
	if (prog) {
		prog++;
	} else {
		prog = "php";
	}
	
	php_printf( "Usage: %s [options] [args]\n"
	            "       %s [options] [args]\n"
	            "\n"
				"  options:\n"
				"  -c <path>|<file> Look for php.ini file in this directory\n"
				"  -n               No php.ini file will be used\n"
				"  -d foo[=bar]     Define INI entry foo with value 'bar'\n"
				"  -f <file>        Parse and execute <file>.\n"
				"  -h,-?            This help\n"
				"  -i               PHP information\n"
				"  -m               Show compiled in modules\n"
				"  -r <code>        Run PHP <code> without using script tags <?..?>\n"
				"  -H               Hide any passed arguments from external tools.\n"
				"  -v               Version number\n"
				"  -z <file>        Load Zend extension <file>.\n"
				"\n"
				"  --ini            Show configuration file names\n"
				"\n"
				"  args:\n"
				"       install     install ssp service\n"
				"       start       start ssp service\n"
				"       stop        stop ssp service\n"
				"       restart     restart ssp service\n"
				"       uninstall   uninstall ssp service\n"
				"\n"
				, prog, prog, prog, prog, prog, prog);
}
/* }}} */

static int print_module_info(zend_module_entry *module TSRMLS_DC) /* {{{ */
{
	php_printf("%s\n", module->name);
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

static int module_name_cmp(const void *a, const void *b TSRMLS_DC) /* {{{ */
{
	Bucket *f = *((Bucket **) a);
	Bucket *s = *((Bucket **) b);

	return strcasecmp(((zend_module_entry *)f->pData)->name,
				  ((zend_module_entry *)s->pData)->name);
}
/* }}} */

static void print_modules(TSRMLS_D) /* {{{ */
{
	HashTable sorted_registry;
	zend_module_entry tmp;

	zend_hash_init(&sorted_registry, 50, NULL, NULL, 1);
	zend_hash_copy(&sorted_registry, &module_registry, NULL, &tmp, sizeof(zend_module_entry));
	zend_hash_sort(&sorted_registry, zend_qsort, module_name_cmp, 0 TSRMLS_CC);
	zend_hash_apply(&sorted_registry, (apply_func_t) print_module_info TSRMLS_CC);
	zend_hash_destroy(&sorted_registry);
}
/* }}} */

static int extension_name_cmp(const zend_llist_element **f, const zend_llist_element **s TSRMLS_DC) /* {{{ */
{
	return strcmp(((zend_extension *)(*f)->data)->name,
				  ((zend_extension *)(*s)->data)->name);
}
/* }}} */

static int print_extension_info(zend_extension *ext, void *arg TSRMLS_DC) /* {{{ */
{
	php_printf("%s\n", ext->name);
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

static void print_extensions(TSRMLS_D) /* {{{ */
{
	zend_llist sorted_exts;

	zend_llist_copy(&sorted_exts, &zend_extensions);
	sorted_exts.dtor = NULL;
	zend_llist_sort(&sorted_exts, extension_name_cmp TSRMLS_CC);
	zend_llist_apply(&sorted_exts, (llist_apply_func_t) print_extension_info TSRMLS_CC);
	zend_llist_destroy(&sorted_exts);
}
/* }}} */

/* {{{ cli_seek_file_begin
 */
static int cli_seek_file_begin(zend_file_handle *file_handle, char *script_file, int *lineno TSRMLS_DC)
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

static void cli_register_file_handles(TSRMLS_D) /* {{{ */
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

#ifdef PHP_CLI_WIN32_NO_CONSOLE
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
#else
int main(int argc, char *argv[])
#endif
{
#ifdef PHP_CLI_WIN32_NO_CONSOLE
	int argc = __argc;
	char **argv = __argv;
#endif

#if defined(PHP_WIN32) && defined(_DEBUG) && defined(PHP_WIN32_DEBUG_HEAP)
	{
		int tmp_flag;
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);
		_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);
		_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
		tmp_flag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
		tmp_flag |= _CRTDBG_DELAY_FREE_MEM_DF;
		tmp_flag |= _CRTDBG_LEAK_CHECK_DF;

		_CrtSetDbgFlag(tmp_flag);
	}
#endif

#ifdef HAVE_SIGNAL_H
#if defined(SIGPIPE) && defined(SIG_IGN)
	signal(SIGPIPE, SIG_IGN); /* ignore SIGPIPE in standalone mode so
								that sockets created via fsockopen()
								don't kill PHP if the remote site
								closes it.  in apache|apxs mode apache
								does that for us!  thies@thieso.net
								20000419 */
#endif
#endif

	volatile int exit_status = SUCCESS;
	int c;
	zend_file_handle file_handle;
/* temporary locals */
	int behavior=PHP_MODE_STANDARD;
	char *serv_opt="",*pidfile="/var/run/ssp.pid";
	short int port=8083;
	int orig_optind=php_optind;
	char *orig_optarg=php_optarg;
	char *arg_free=NULL, **arg_excp=&arg_free;
	char *script_file=NULL;
	volatile int request_started = 0;
	int lineno = 0;
	char *exec_direct=NULL, *exec_run=NULL, *exec_begin=NULL, *exec_end=NULL;
	const char *param_error=NULL;
	int hide_argv = 0;
/* end of temporary locals */
	int ini_entries_len = 0;

	if(argc==1){
		argc=2;
		argv[1]="-?";
	}

#ifdef PHP_WIN32
	_fmode = _O_BINARY;			/*sets default for file streams to binary */
	setmode(_fileno(stdin), O_BINARY);		/* make the stdio mode be binary */
	setmode(_fileno(stdout), O_BINARY);		/* make the stdio mode be binary */
	setmode(_fileno(stderr), O_BINARY);		/* make the stdio mode be binary */
#endif

	php_init();

	CSM(executable_location) = argv[0];

	while ((c = php_getopt(argc, argv, OPTIONS, &php_optarg, &php_optind, 0, 2))!=-1) {
		switch (c) {
			case 'c':
				if (CSM(php_ini_path_override)) {
					free(CSM(php_ini_path_override));
				}
 				CSM(php_ini_path_override) = strdup(php_optarg);
				break;
			case 'n':
				CSM(php_ini_ignore) = 1;
				break;
			case 'd': {
				/* define ini entries on command line */
				int len = strlen(php_optarg);
				char *val;

				if ((val = strchr(php_optarg, '='))) {
					val++;
					if (!isalnum(*val) && *val != '"' && *val != '\'' && *val != '\0') {
						CSM(ini_entries) = realloc(CSM(ini_entries), ini_entries_len + len + sizeof("\"\"\n\0"));
						memcpy(CSM(ini_entries) + ini_entries_len, php_optarg, (val - php_optarg));
						ini_entries_len += (val - php_optarg);
						memcpy(CSM(ini_entries) + ini_entries_len, "\"", 1);
						ini_entries_len++;
						memcpy(CSM(ini_entries) + ini_entries_len, val, len - (val - php_optarg));
						ini_entries_len += len - (val - php_optarg);
						memcpy(CSM(ini_entries) + ini_entries_len, "\"\n\0", sizeof("\"\n\0"));
						ini_entries_len += sizeof("\n\0\"") - 2;
					} else {
						CSM(ini_entries) = realloc(CSM(ini_entries), ini_entries_len + len + sizeof("\n\0"));
						memcpy(CSM(ini_entries) + ini_entries_len, php_optarg, len);
						memcpy(CSM(ini_entries) + ini_entries_len + len, "\n\0", sizeof("\n\0"));
						ini_entries_len += len + sizeof("\n\0") - 2;
					}
				} else {
					CSM(ini_entries) = realloc(CSM(ini_entries), ini_entries_len + len + sizeof("=1\n\0"));
					memcpy(CSM(ini_entries) + ini_entries_len, php_optarg, len);
					memcpy(CSM(ini_entries) + ini_entries_len + len, "=1\n\0", sizeof("=1\n\0"));
					ini_entries_len += len + sizeof("=1\n\0") - 2;
				}
				break;
			}
		}
	}
	php_optind = orig_optind;
	php_optarg = orig_optarg;

	if(php_begin()==FAILURE){
		exit_status=1;
		goto err;
	}

	zend_first_try {
		CG(in_compilation) = 0; /* not initialized but needed for several options */
		EG(uninitialized_zval_ptr) = NULL;

		while ((c = php_getopt(argc, argv, OPTIONS, &php_optarg, &php_optind, 0, 2)) != -1) {
			switch (c) {

			case 'h': /* help & quit */
			case '?':
				if (php_request_startup(TSRMLS_C)==FAILURE) {
					goto err;
				}
				request_started = 1;
				php_cli_usage(argv[0]);
				php_end_ob_buffers(1 TSRMLS_CC);
				exit_status=0;
				goto out;

			case 'i': /* php info & quit */
				if (php_request_startup(TSRMLS_C)==FAILURE) {
					goto err;
				}
				request_started = 1;
				php_print_info(0xFFFFFFFF TSRMLS_CC);
				php_end_ob_buffers(1 TSRMLS_CC);
				exit_status=0;
				goto out;

			case 'm': /* list compiled in modules */
				if (php_request_startup(TSRMLS_C)==FAILURE) {
					goto err;
				}
				request_started = 1;
				php_printf("[PHP Modules]\n");
				print_modules(TSRMLS_C);
				php_printf("\n[Zend Modules]\n");
				print_extensions(TSRMLS_C);
				php_printf("\n");
				php_end_ob_buffers(1 TSRMLS_CC);
				exit_status=0;
				goto out;

			case 'v': /* show php version & quit */
				if (php_request_startup(TSRMLS_C) == FAILURE) {
					goto err;
				}

				request_started = 1;
				php_printf("PHP %s (%s) (built: %s %s) %s\nCopyright (c) 1997-2011 The PHP Group\n%s",
					PHP_VERSION, sapi_module.name, __DATE__, __TIME__,
#if ZEND_DEBUG && defined(HAVE_GCOV)
					"(DEBUG GCOV)",
#elif ZEND_DEBUG
					"(DEBUG)",
#elif defined(HAVE_GCOV)
					"(GCOV)",
#else
					"",
#endif
					get_zend_version()
				);
				php_end_ob_buffers(1 TSRMLS_CC);
				exit_status=0;
				goto out;

			default:
				break;
			}
		}

		/* Set some CLI defaults */
		SG(options) |= SAPI_OPTION_NO_CHDIR;

		php_optind = orig_optind;
		php_optarg = orig_optarg;
		while ((c = php_getopt(argc, argv, OPTIONS, &php_optarg, &php_optind, 0, 2)) != -1) {
			switch (c) {

			case 'f': /* parse file */
				if (behavior == PHP_MODE_CLI_DIRECT) {
					param_error = param_mode_conflict;
					break;
				} else if (script_file) {
					param_error = "You can use -f only once.\n";
					break;
				}
				script_file = php_optarg;
				break;

			case 'r': /* run code from command line */
				if (behavior == PHP_MODE_CLI_DIRECT) {
					if (exec_direct || script_file) {
						param_error = "You can use -r only once.\n";
						break;
					}
				} else if (behavior != PHP_MODE_STANDARD) {
					param_error = param_mode_conflict;
					break;
				}
				behavior=PHP_MODE_CLI_DIRECT;
				exec_direct=php_optarg;
				break;

			case 'z': /* load extension file */
				zend_load_extension(php_optarg);
				break;
			case 'H':
				hide_argv = 1;
				break;

			case 'p': /* parse file */
				port = atoi(php_optarg);
				if(port<=0)
					param_error="The port number must not be less than 1.";
				break;

			case 14:
				behavior = PHP_MODE_SHOW_INI_CONFIG;
				break;
			default:
				break;
			}
		}

		if (param_error) {
			PUTS(param_error);
			exit_status=1;
			goto err;
		}

		/* only set script_file if not set already and not in direct mode and not at end of parameter list */
		if (argc > php_optind) 
		{
			serv_opt=argv[php_optind];
			php_optind++;
		}
		if (script_file) {
			if (cli_seek_file_begin(&file_handle, script_file, &lineno TSRMLS_CC) != SUCCESS) {
				goto err;
			}
			script_filename = script_file;
		} else {
			file_handle.filename = "-";
			//file_handle.handle.fp = stdin;
		}
		file_handle.type = ZEND_HANDLE_FP;
		file_handle.opened_path = NULL;
		file_handle.free_filename = 0;
		php_self = file_handle.filename;

		/* before registering argv to module exchange the *new* argv[0] */
		/* we can achieve this without allocating more memory */
		SG(request_info).argc=argc-php_optind+1;
		arg_excp = argv+php_optind-1;
		arg_free = argv[php_optind-1];
		SG(request_info).path_translated = file_handle.filename;
		argv[php_optind-1] = file_handle.filename;
		SG(request_info).argv=argv+php_optind-1;

		if (php_request_startup(TSRMLS_C)==FAILURE) {
			*arg_excp = arg_free;
			fclose(file_handle.handle.fp);
			PUTS("Could not startup.\n");
			goto err;
		}

		request_started = 1;
		CG(start_lineno) = lineno;
		*arg_excp = arg_free; /* reconstuct argv */

		if (hide_argv) {
			int i;
			for (i = 1; i < argc; i++) {
				memset(argv[i], 0, strlen(argv[i]));
			}
		}

		zend_is_auto_global("_SERVER", sizeof("_SERVER")-1 TSRMLS_CC);

		PG(during_request_startup) = 0;
		switch (behavior) {
			case PHP_MODE_STANDARD:
				if (strcmp(file_handle.filename, "-")) {
					cli_register_file_handles(TSRMLS_C);
					php_execute_script(&file_handle TSRMLS_CC);
				}
				exit_status = EG(exit_status);
				break;

			case PHP_MODE_CLI_DIRECT:
				cli_register_file_handles(TSRMLS_C);
				if (zend_eval_string_ex(exec_direct, NULL, "Command line code", 1 TSRMLS_CC) == FAILURE) {
					exit_status=254;
				}
				break;
			
			case PHP_MODE_SHOW_INI_CONFIG:
				zend_printf("Configuration File (php.ini) Path: %s\n", PHP_CONFIG_FILE_PATH);
				zend_printf("Loaded Configuration File:         %s\n", php_ini_opened_path ? php_ini_opened_path : "(none)");
				zend_printf("Scan for additional .ini files in: %s\n", *PHP_CONFIG_FILE_SCAN_DIR ? PHP_CONFIG_FILE_SCAN_DIR : "(none)");
				zend_printf("Additional .ini files parsed:      %s\n", php_ini_scanned_files ? php_ini_scanned_files : "(none)");
				goto out;
				break;
		}

		if(strcmp(serv_opt,"restart")==0 || strcmp(serv_opt,"stop")==0){
			goto out;
		}
		if(strcmp(serv_opt,"restart")==0 || strcmp(serv_opt,"start")==0){
			socket_listen(port,pidfile);
			goto out;
		}
		if(strcmp(serv_opt,"install")==0){
		}else if(strcmp(serv_opt,"uninstall")==0){
		}else{
			php_cli_usage(argv[0]);
			php_end_ob_buffers(1 TSRMLS_CC);
			exit_status=1;
			goto out;
		}

	} zend_end_try();

out:
	if (request_started) {
		php_request_shutdown((void *) 0);
	}
	if (exit_status == 0) {
		exit_status = EG(exit_status);
	}
err:
	php_end();
	exit(exit_status);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
