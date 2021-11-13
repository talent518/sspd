#include "php_func.h"
#include "ssp.h"
#include "php_ext.h"
#include "server.h"

#include <php.h>
#include <php_ini.h>
#include <php_getopt.h>
#include <zend_extensions.h>
#include <zend_hash.h>

#include <stdio.h>
#include <stdlib.h>
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

#define OPT_HOST 1
#define OPT_PORT 2
#define OPT_PIDFILE 3
#define OPT_INFILE 4
#define OPT_OUTFILE 5
#define OPT_ERRFILE 6
#define OPT_USER 7
#define OPT_MAX_CLIENTS 8
#define OPT_MAX_RECVS 9
#define OPT_NTHREADS 10
#define OPT_TIMEOUT 11
#define OPT_TIMEOUT_GLOBAL 12
#define OPT_SSP_VARS 13

static char *php_optarg = NULL;
static int php_optind = 1;

static const opt_struct OPTIONS[] = {
	{ 'c', 1, "php-ini" },
	{ 'd', 1, "define" },
	{ 'f', 1, "file" },
	{ 'h', 0, "help" },
	{ 'i', 0, "info" },
	{ 'm', 0, "modules" },
	{ 'n', 0, "no-php-ini" },
	{ 'H', 0, "hide-args" },
	{ '?', 0, "usage" },/* help alias (both '?' and 'usage') */
	{ 'v', 0, "version" },
	{ 'z', 1, "zend-extension" },
	{ 'b', 1, "backlog" },
	{ 's', 1, "service" },

	{ OPT_HOST, 1, "host" },
	{ OPT_PORT, 1, "port" },
	{ OPT_PIDFILE, 1, "pidfile" },
	{ OPT_INFILE, 1, "infile" },
	{ OPT_OUTFILE, 1, "outfile" },
	{ OPT_ERRFILE, 1, "errfile" },

	{ OPT_USER, 1, "user" },
	{ OPT_NTHREADS, 1, "nthreads" },
	{ OPT_MAX_CLIENTS, 1, "max-clients" },
	{ OPT_MAX_RECVS, 1, "max-recvs" },

	{ OPT_TIMEOUT, 1, "timeout" },
	{ OPT_TIMEOUT_GLOBAL, 1, "gtimeout" },

	{ OPT_SSP_VARS, 1, "sspvars" },

	{ '-', 0, NULL } /* end of args */
};

/* {{{ php_ssp_usage
 */
static void php_ssp_usage(char *argv0) {
	char *prog;

	prog = strrchr(argv0, '/');
	if (prog) {
		prog++;
	} else {
		prog = "ssp";
	}

	char *maxrecvs;

	maxrecvs = (char*) fsize(ssp_maxrecvs);

	php_printf(
		"Usage: %s [options] [args]\n"
			"\n"
			"  options:\n"
			"  -c <path>|<file>        Look for php.ini file in this directory\n"
			"  -n                      No php.ini file will be used\n"
			"  -d foo[=bar]            Define INI entry foo with value 'bar'\n"
			"  -h,-?                   This help\n"
			"  -i                      PHP information\n"
			"  -m                      Show compiled in modules\n"
			"  -H                      Hide any passed arguments from external tools.\n"
			"  -v                      Version number\n"
			"  -z <file>               Load Zend extension <file>.\n"
			"\n"
			"  -f <file>               Parse and execute <file>.\n"
			"  --host <IP>             Listen host (default: %s)\n"
			"  --port <port>           Listen port (default: %d)\n"
			"  --pidfile <file>        Service pidfile (default: %s)\n"
			"  --infile <file>         Service infile (default: %s)\n"
			"  --outfile <file>        Service outfile (default: %s)\n"
			"  --errfile <file>        Service errfile (default: %s)\n"
			"  --user <username>       Run for user (default: %s)\n"
			"  --nthreads <number>     LibEvent thread number (default: %d)\n"
			"  --max-clients <number>  Max client connect number (default: %d)\n"
			"  --max-recvs <size>      Max recv data size (default: %s)\n"
			"  -b <backlog>            Set the backlog queue limit (default: %d)\n"
			"  --sspvars <sspvars>     Set the global php variable _SSP init array length (default: %ds)\n"
#ifdef SSP_CODE_TIMEOUT
		"  --timeout <timeout>     Set the timeout php cache timeout (default: %lds)\n"
#ifdef SSP_CODE_TIMEOUT_GLOBAL
		"  --gtimeout <gtimeout>     Set the gtimeout global php variable _SSP timeout (default: %lds)\n"
#endif
#endif
		"\n"
		"  -s <option>             socket service option\n"
		"  option:\n"
		"       script             run script\n"
		"       bench              run bench\n"
		"       start              start ssp service\n"
		"       stop               stop ssp service\n"
		"       restart            restart ssp service\n"
		"       status             ssp service status\n"
		"\n", prog, ssp_host, ssp_port, ssp_pidfile, ssp_infile, ssp_outfile, ssp_errfile, ssp_user,
		ssp_nthreads, ssp_maxclients, maxrecvs, ssp_backlog, ssp_vars_length
#ifdef SSP_CODE_TIMEOUT
		, ssp_timeout
#ifdef SSP_CODE_TIMEOUT_GLOBAL
		, ssp_global_timeout
#endif
#endif
		);

	free(maxrecvs);
}
/* }}} */

static int print_module_info(zval *element) /* {{{ */
{
	zend_module_entry *module = (zend_module_entry*) Z_PTR_P(element);
	php_printf("%s\n", module->name);
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

#if PHP_VERSION_ID >= 80000
static int module_name_cmp(Bucket *f, Bucket *s) /* {{{ */
{
#else
static int module_name_cmp(const void *a, const void *b) /* {{{ */
{
	Bucket *f = (Bucket *) a;
	Bucket *s = (Bucket *) b;
#endif

	return strcasecmp(((zend_module_entry *) Z_PTR(f->val))->name,
		((zend_module_entry *) Z_PTR(s->val))->name);
}
/* }}} */

static void print_modules() /* {{{ */
{
	HashTable sorted_registry;

	zend_hash_init(&sorted_registry, 50, NULL, NULL, 0);
	zend_hash_copy(&sorted_registry, &module_registry, NULL);
	zend_hash_sort(&sorted_registry, module_name_cmp, 0);
	zend_hash_apply(&sorted_registry, print_module_info);
	zend_hash_destroy(&sorted_registry);
}
/* }}} */

static int extension_name_cmp(const zend_llist_element **f,
	const zend_llist_element **s) /* {{{ */
{
	return strcmp(((zend_extension *) (*f)->data)->name,
		((zend_extension *) (*s)->data)->name);
}
/* }}} */

static int print_extension_info(zend_extension *ext, void *arg) /* {{{ */
{
	php_printf("%s\n", ext->name);
	return ZEND_HASH_APPLY_KEEP;
}
/* }}} */

static void print_extensions() /* {{{ */
{
	zend_llist sorted_exts;

	zend_llist_copy(&sorted_exts, &zend_extensions);
	sorted_exts.dtor = NULL;
	zend_llist_sort(&sorted_exts, extension_name_cmp);
	zend_llist_apply(&sorted_exts, (llist_apply_func_t) print_extension_info);
	zend_llist_destroy(&sorted_exts);
}
/* }}} */

int main(int argc, char *argv[])
{

#ifdef HAVE_SIGNAL_H
#if defined(SIGPIPE) && defined(SIG_IGN)
	signal(SIGPIPE, SIG_IGN);
#endif
#if defined(SIGCHLD) && defined(SIG_IGN)
	signal(SIGCHLD, SIG_IGN);
#endif
#endif

	volatile int exit_status = SUCCESS;
	int c;
	/* temporary locals */
	char *serv_opt = NULL;
	int orig_optind = php_optind;
	char *orig_optarg = php_optarg;
	char *arg_free = NULL, **arg_excp = &arg_free;
	volatile int request_started = 0;
	const char *param_error = NULL;
	int hide_argv = 0;
	/* end of temporary locals */
	int ini_entries_len = 0;

	if (argc == 1) {
		argc = 2;
		argv[1] = "-?";
	}

	while ((c = php_getopt(argc, argv, OPTIONS, &php_optarg, &php_optind, 0, 2))
		!= -1) {
		switch (c) {
			case OPT_HOST:
				ssp_host = strdup(php_optarg);
				break;
			case OPT_PORT:
				ssp_port = atoi(php_optarg);
				break;
			case OPT_PIDFILE:
				ssp_pidfile = strdup(php_optarg);
				break;
			case OPT_OUTFILE:
				ssp_outfile = strdup(php_optarg);
				break;
			case OPT_INFILE:
				ssp_infile = strdup(php_optarg);
				break;
			case OPT_ERRFILE:
				ssp_errfile = strdup(php_optarg);
				break;

			case OPT_USER:
				ssp_user = strdup(php_optarg);
				break;
			case OPT_MAX_CLIENTS:
				ssp_maxclients = atoi(php_optarg);
				break;
			case OPT_MAX_RECVS:
				ssp_maxrecvs = zend_atoi(php_optarg, strlen(php_optarg));
				break;
			case OPT_NTHREADS:
				ssp_nthreads = atoi(php_optarg);
				break;
			case 'b':
				ssp_backlog = atoi(php_optarg);
				break;
			case OPT_SSP_VARS:
				ssp_vars_length = atoi(php_optarg);
				break;
		#ifdef SSP_CODE_TIMEOUT
				case OPT_TIMEOUT:
				ssp_timeout = atoi(php_optarg);
				break;
			#ifdef SSP_CODE_TIMEOUT_GLOBAL
				case OPT_TIMEOUT_GLOBAL:
				ssp_global_timeout = atoi(php_optarg);
				break;
			#endif
		#endif
		}
	}

	php_optind = orig_optind;
	php_optarg = orig_optarg;

	ssp_init();

	CSM(executable_location) = argv[0];

	while ((c = php_getopt(argc, argv, OPTIONS, &php_optarg, &php_optind, 0, 2))
		!= -1) {
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
					if (!isalnum(*val) && *val != '"' && *val != '\'' && *val
						!= '\0') {
						CSM(ini_entries) = realloc(CSM(ini_entries),
							ini_entries_len + len + sizeof("\"\"\n\0"));
						memcpy(CSM(ini_entries) + ini_entries_len, php_optarg, (val
							- php_optarg));
						ini_entries_len += (val - php_optarg);
						memcpy(CSM(ini_entries) + ini_entries_len, "\"", 1);
						ini_entries_len++;
						memcpy(CSM(ini_entries) + ini_entries_len, val, len - (val
							- php_optarg));
						ini_entries_len += len - (val - php_optarg);
						memcpy(CSM(ini_entries) + ini_entries_len, "\"\n\0",
							sizeof("\"\n\0"));
						ini_entries_len += sizeof("\n\0\"") - 2;
					} else {
						CSM(ini_entries) = realloc(CSM(ini_entries),
							ini_entries_len + len + sizeof("\n\0"));
						memcpy(CSM(ini_entries) + ini_entries_len, php_optarg, len);
						memcpy(CSM(ini_entries) + ini_entries_len + len, "\n\0",
							sizeof("\n\0"));
						ini_entries_len += len + sizeof("\n\0") - 2;
					}
				} else {
					CSM(ini_entries) = realloc(CSM(ini_entries),
						ini_entries_len + len + sizeof("=1\n\0"));
					memcpy(CSM(ini_entries) + ini_entries_len, php_optarg, len);
					memcpy(CSM(ini_entries) + ini_entries_len + len, "=1\n\0",
						sizeof("=1\n\0"));
					ini_entries_len += len + sizeof("=1\n\0") - 2;
				}
				break;
			}
		}
	}
	php_optind = orig_optind;
	php_optarg = orig_optarg;

	ssp_module_startup();

	zend_first_try
			{
				CG(in_compilation) = 0; /* not initialized but needed for several options */

				while ((c = php_getopt(argc, argv, OPTIONS, &php_optarg,
					&php_optind, 0, 2)) != -1) {
					switch (c) {
						case 'h': /* help & quit */
						case '?':
							if (php_request_startup() == FAILURE) {
								goto err;
							}
							request_started = 1;
							php_ssp_usage(argv[0]);
							php_output_end_all();
							exit_status = 0;
							goto out;

						case 'i': /* php info & quit */
							if (php_request_startup() == FAILURE) {
								goto err;
							}
							request_started = 1;
							php_print_info(0xFFFFFFFF);
							php_output_end_all();
							exit_status = 0;
							goto out;

						case 'm': /* list compiled in modules */
							if (php_request_startup() == FAILURE) {
								goto err;
							}
							request_started = 1;
							php_printf("[PHP Modules]\n");
							print_modules();
							php_printf("\n[Zend Modules]\n");
							print_extensions();
							php_printf("\n");
							php_output_end_all();
							exit_status = 0;
							goto out;

						case 'v': /* show php version & quit */
							if (php_request_startup() == FAILURE) {
								goto err;
							}

							request_started = 1;
							php_printf(
								"PHP %s (%s %s) (built: %s %s) %s\nCopyright (c) 1997-2012 The Abao\n%s",
								PHP_VERSION, sapi_module.name, PHP_SSP_VERSION,
								__DATE__, __TIME__,
								#if ZEND_DEBUG && defined(HAVE_GCOV)
								"(DEBUG GCOV)",
#elif ZEND_DEBUG
								"(DEBUG)",
#elif defined(HAVE_GCOV)
								"(GCOV)",
#else
								"",
								#endif
								get_zend_version());
							php_output_end_all();
							exit_status = 0;
							goto out;

						default:
							break;
					}
				}

				/* Set some CLI defaults */
				SG(options) |= SAPI_OPTION_NO_CHDIR;

				php_optind = orig_optind;
				php_optarg = orig_optarg;
				while ((c = php_getopt(argc, argv, OPTIONS, &php_optarg,
					&php_optind, 0, 2)) != -1) {
					switch (c) {
						case 'f': /* parse file */
							request_init_file = strdup(php_optarg);
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

						default:
							break;
					}
				}

				if (param_error) {
					PUTS(param_error);
					exit_status = 1;
					goto err;
				}

				if (hide_argv) {
					int i;
					for (i = 1; i < argc; i++) {
						memset(argv[i], 0, strlen(argv[i]));
					}
				}
			}zend_end_try();

	if(!serv_opt || (!request_init_file && strcmp(serv_opt, "stop") && strcmp(serv_opt, "status"))) {
		php_ssp_usage(argv[0]);
		php_output_end_all();
		exit_status = 1;
	} else if (strcmp(serv_opt, "script") == 0) {
		SG(request_info).argc = argc - php_optind + 1;
		argv[php_optind - 1] = request_init_file;
		SG(request_info).argv = argv + php_optind - 1;

		strcpy(SSP_G(threadname), "main thread");

		ssp_request_startup_ex();
		ssp_request_shutdown();
	} else if (strcmp(serv_opt, "bench") == 0) {
		server_bench();
	} else if (strcmp(serv_opt, "restart") == 0) {
		server_stop();
		server_start();
	} else if (strcmp(serv_opt, "stop") == 0) {
		server_stop();
	} else if (strcmp(serv_opt, "start") == 0) {
		server_start();
	} else if (strcmp(serv_opt, "status") == 0) {
		server_status();
	} else if (serv_opt) {
		php_ssp_usage(argv[0]);
		php_output_end_all();
		exit_status = 1;
	} else {
		exit_status = 0;
		goto err;
	}

	free(serv_opt);

	out:
	if (request_started) {
		php_request_shutdown(NULL);
	}
	if (exit_status == 0) {
		exit_status = EG(exit_status);
	}

	err:
	ssp_module_shutdown();
	ssp_destroy();

	exit(exit_status);
}
