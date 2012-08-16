#include "php_func.h"
#include "server.h"
#include "php_ext.h"

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

static char *php_optarg = NULL;
static int php_optind = 1;

static const opt_struct OPTIONS[] = {
	{'c', 1, "php-ini"},
	{'d', 1, "define"},
	{'f', 1, "file"},
	{'h', 0, "help"},
	{'i', 0, "info"},
	{'m', 0, "modules"},
	{'n', 0, "no-php-ini"},
	{'H', 0, "hide-args"},
	{'?', 0, "usage"},/* help alias (both '?' and 'usage') */
	{'v', 0, "version"},
	{'z', 1, "zend-extension"},
	{'s', 1, "service"},
	{14,  0, "debug"},
	{'-', 0, NULL} /* end of args */
};

/* {{{ php_ssp_usage
 */
static void php_ssp_usage(char *argv0)
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
				"  -H               Hide any passed arguments from external tools.\n"
				"  -v               Version number\n"
				"  -z <file>        Load Zend extension <file>.\n"
				"\n"
				"  --debug          Show debug info\n"
				"\n"
				"  -s <option>      socket service option\n"
				"  option:\n"
				"       start       start ssp service\n"
				"       stop        stop ssp service\n"
				"       restart     restart ssp service\n"
				"       status      ssp service status\n"
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
	signal(SIGPIPE, SIG_IGN);
#endif
#if defined(SIGCHLD) && defined(SIG_IGN)
	signal(SIGCHLD,SIG_IGN);
#endif
#endif

	volatile int exit_status = SUCCESS;
	int c;
/* temporary locals */
	char *serv_opt=NULL;
	int orig_optind=php_optind;
	char *orig_optarg=php_optarg;
	char *arg_free=NULL, **arg_excp=&arg_free;
	char *script_file=NULL;
	volatile int request_started = 0;
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

	CSM(executable_location) = strdup(argv[0]);

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

	TSRMLS_FETCH();

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
					php_ssp_usage(argv[0]);
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
					php_printf("PHP %s (%s %s) (built: %s %s) %s\nCopyright (c) 1997-2012 The Abao\n%s",
						PHP_VERSION, sapi_module.name,PHP_SSP_VERSION, __DATE__, __TIME__,
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
				script_file = strdup(php_optarg);
				break;

			case 'z': /* load extension file */
				zend_load_extension(php_optarg);
				break;

			case 's': /* service control */
				serv_opt = strdup(php_optarg);
				break;

			case 'H':
				hide_argv = 1;
				break;

			case 14:
				debug = true;
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

		if (hide_argv) {
			int i;
			for (i = 1; i < argc; i++) {
				memset(argv[i], 0, strlen(argv[i]));
			}
		}
	} zend_end_try();

	if(script_file!=NULL){
		php_self=script_file;
		ssp_request_startup(script_file);
	}
	if(serv_opt==NULL){
	}else if(strcmp(serv_opt,"restart")==0){
		socket_stop();
		socket_start();
	}else if(strcmp(serv_opt,"stop")==0){
		socket_stop();
	}else if(strcmp(serv_opt,"start")==0){
		socket_start();
	}else if(strcmp(serv_opt,"status")==0){
		socket_status();
	}else if(serv_opt){
		php_ssp_usage(argv[0]);
		php_end_ob_buffers(1 TSRMLS_CC);
		exit_status=1;
		goto out;
	}

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
