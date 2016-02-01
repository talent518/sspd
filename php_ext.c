#include "php_func.h"
#include "php_ext.h"
#include "ssp.h"
#include "data.h"
#include "api.h"
#include <malloc.h>
#include <signal.h>
#include <math.h>

#ifdef HAVE_LIBGTOP
	#include <glibtop.h>
	#include <glibtop/cpu.h>
	#include <glibtop/mem.h>
	#include <glibtop/proctime.h>
	#include <glibtop/procmem.h>
#endif

static pthread_mutex_t unique_lock;

static char trigger_handlers[7][30]={
	"ssp_start_handler",
	"ssp_connect_handler",
	"ssp_connect_denied_handler",
	"ssp_receive_handler",
	"ssp_send_handler",
	"ssp_close_handler",
	"ssp_stop_handler"
};

long le_ssp_descriptor,le_ssp_descriptor_ref;
unsigned int ssp_vars_length=10;
#ifdef SSP_CODE_TIMEOUT
	long ssp_timeout=60;
	#ifdef SSP_CODE_TIMEOUT_GLOBAL
		long ssp_global_timeout=10;
	#endif
#endif

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_resource, 0, 0, 1)
	ZEND_ARG_INFO(0, var)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_info, 0, 0, 1)
	ZEND_ARG_INFO(0, res)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_send, 0, 0, 2)
	ZEND_ARG_INFO(0, res)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_close, 0, 0, 1)
	ZEND_ARG_INFO(0, res)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_destroy, 0, 0, 1)
	ZEND_ARG_INFO(0, res)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_lock, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_unlock, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_stats, 0, 0, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ ssp_functions[] */
zend_function_entry ssp_functions[] = {
	PHP_FE(ssp_resource, arginfo_ssp_resource)
	PHP_FE(ssp_info, arginfo_ssp_info)
	PHP_FE(ssp_send, arginfo_ssp_send)
	PHP_FE(ssp_close, arginfo_ssp_close)
	PHP_FE(ssp_destroy, arginfo_ssp_destroy)
	PHP_FE(ssp_lock, arginfo_ssp_lock)
	PHP_FE(ssp_unlock, arginfo_ssp_unlock)
	PHP_FE(ssp_stats, arginfo_ssp_stats)
	PHP_FE_END
};
/* }}} */

/* {{{ ssp_module_entry
 */
zend_module_entry ssp_module_entry = {
	STANDARD_MODULE_HEADER,
	"ssp",
	ssp_functions,
	PHP_MINIT(ssp), // module_startup_func
	PHP_MSHUTDOWN(ssp), // module_shutdown_func
	PHP_RINIT(ssp), // request_startup_func
	PHP_RSHUTDOWN(ssp), // request_shutdown_func
	PHP_MINFO(ssp), // info_func
	PHP_SSP_VERSION,
	PHP_MODULE_GLOBALS(ssp), // globals_id_ptr / globals_ptr
	PHP_GINIT(ssp), // globals_ctor
	PHP_GSHUTDOWN(ssp), // globals_dtor
	NULL, // post_deactivate_func
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

static void php_destroy_ssp(zend_resource *rsrc) /* {{{ */
{
	conn_t *ptr = (conn_t *) rsrc->ptr;

	conn_info(ptr);
}

static void php_destroy_ssp_ref(zend_resource *rsrc) /* {{{ */
{
	conn_t *ptr = (conn_t *) rsrc->ptr;

	unref_conn(ptr);
}

/* {{{ MINIT */
static PHP_MINIT_FUNCTION(ssp)
{
	REGISTER_STRING_CONSTANT("SSP_VERSION",  PHP_SSP_VERSION,  CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("SSP_RES_INDEX",  PHP_SSP_RES_INDEX,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_RES_SOCKFD",  PHP_SSP_RES_SOCKFD,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_RES_PORT",  PHP_SSP_RES_PORT,  CONST_CS | CONST_PERSISTENT);

	le_ssp_descriptor = zend_register_list_destructors_ex(php_destroy_ssp, NULL, PHP_SSP_DESCRIPTOR_RES_NAME,module_number);
	le_ssp_descriptor_ref = zend_register_list_destructors_ex(php_destroy_ssp_ref, NULL, PHP_SSP_DESCRIPTOR_REF_RES_NAME,module_number);

	pthread_mutex_init(&unique_lock, NULL);

	zend_register_auto_global(zend_string_init("_SSP", sizeof("_SSP") - 1, 1), 0, NULL);

#ifdef SSP_DEBUG_EXT
	printf("module startup function for %s\n", __func__);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
static PHP_MSHUTDOWN_FUNCTION(ssp)
{
	ts_free_id(ssp_globals_id);

	pthread_mutex_destroy(&unique_lock);

	zend_delete_global_variable(zend_string_init("_SSP", sizeof("_SSP") - 1, 1));

#ifdef SSP_DEBUG_EXT
	printf("module shutdown function for %s\n", __func__);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
*/
PHP_RINIT_FUNCTION(ssp)
{
	ZVAL_UNDEF(&SSP_G(ssp_vars));
	ssp_auto_globals_recreate();

#ifdef SSP_DEBUG_EXT
	printf("request startup function for %s\n", __func__);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
*/
PHP_RSHUTDOWN_FUNCTION(ssp)
{
	if (!Z_ISUNDEF(SSP_G(ssp_vars))) {
		zval_ptr_dtor(&SSP_G(ssp_vars));
	}
	
#ifdef SSP_DEBUG_EXT
	printf("request shutdown function for %s\n", __func__);
#endif

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_GINIT_FUNCTION
*/
static PHP_GINIT_FUNCTION(ssp)
{
	SSP_G(trigger_count)=0;

#ifdef SSP_DEBUG_EXT
	printf("globals constructor function for %s\n", __func__);
#endif
}
/* }}} */

/* {{{ PHP_GSHUTDOWN_FUNCTION */
static PHP_GSHUTDOWN_FUNCTION(ssp)
{
#ifdef SSP_DEBUG_EXT
	printf("globals destructor function for %s\n", __func__);
#endif
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
static PHP_MINFO_FUNCTION(ssp)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "ssp support", "enabled");
	php_info_print_table_row(2, "ssp version", PHP_SSP_VERSION);
	php_info_print_table_end();
}

void ssp_auto_globals_recreate()
{
	zend_string *var_name = zend_string_init("_SSP", sizeof("_SSP") - 1, 0);

	zend_delete_global_variable(var_name);

	ZVAL_NULL(&SSP_G(ssp_vars));

	array_init_size(&SSP_G(ssp_vars), ssp_vars_length);

	Z_ADDREF_P(&SSP_G(ssp_vars));

	zend_hash_update_ind(&EG(symbol_table), var_name, &SSP_G(ssp_vars));
	zend_string_release(var_name);
}

bool trigger(unsigned short type,...) {
	TRIGGER_STARTUP();
	if(trigger_handlers[type]==NULL){
		TRIGGER_SHUTDOWN();
		return FAILURE;
	}
	zval pfunc,retval;
	zval *params = NULL;
	int i,param_count,ret;
	bool retbool=true;
	va_list args;
	conn_t *ptr;
	char **data=NULL;
	int *data_len;

	ZVAL_STRINGL(&pfunc, trigger_handlers[type], strlen(trigger_handlers[type]));

	va_start(args,type);
	switch(type){
		case PHP_SSP_START:
		case PHP_SSP_STOP:
			param_count=0;
			params=NULL;
			break;
		case PHP_SSP_CONNECT:
		case PHP_SSP_CONNECT_DENIED:
		case PHP_SSP_CLOSE:
			param_count=1;
			ptr=va_arg(args,conn_t*);
			params=(zval *) emalloc(sizeof(zval)*param_count);
			ZEND_REGISTER_RESOURCE(&params[0],ptr,le_ssp_descriptor);
			break;
		case PHP_SSP_RECEIVE:
		case PHP_SSP_SEND:
			param_count=2;
			ptr=va_arg(args,conn_t*);
			data=va_arg(args,char**);
			data_len=va_arg(args,int*);
			params=(zval *) emalloc(sizeof(zval)*param_count);
			ZEND_REGISTER_RESOURCE(&params[0],ptr,le_ssp_descriptor);
			ZVAL_STRINGL(&params[1], *data, *data_len);
			break;
		default:
			perror("Trigger type not exists!");
			return FAILURE;
	}
	va_end(args);

	ret=call_user_function(EG(function_table), NULL, &pfunc, &retval, param_count, params);
	if(ret==SUCCESS){
		if(Z_TYPE_P(&retval) == IS_FALSE) {
			retbool=false;
		} else if(Z_TYPE_P(&retval) == IS_TRUE) {
			retbool=true;
		}
		if(param_count>1){
			convert_to_string_ex(&retval);
			free(*data);
			if(Z_STRLEN_P(&retval)>0){
				char *_data=strndup(Z_STRVAL_P(&retval),Z_STRLEN_P(&retval));
				*data=_data;
				*data_len=Z_STRLEN_P(&retval);
			}else{
				*data=NULL;
				*data_len=0;
			}
		}
	}else{
		php_printf("\nUnable to call handler(%s)", trigger_handlers[type]);
	}
	if(param_count>0){
		int i;
		for(i=0;i<param_count;i++){
			zval_ptr_dtor(&params[i]);
		}
		efree(params);
	}

	zval_ptr_dtor(&retval);
	ZVAL_UNDEF(&retval);

	zval_ptr_dtor(&pfunc);
	ZVAL_UNDEF(&pfunc);

	TRIGGER_SHUTDOWN();

	return retbool;
}

static PHP_FUNCTION(ssp_resource){
	long var,type;
	conn_t *ptr=NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l", &var) == FAILURE) {
		RETURN_FALSE;
	}

	ptr=index_conn(var);

	if(ptr!=NULL){
		ZEND_REGISTER_RESOURCE(return_value,ptr,le_ssp_descriptor_ref);
	}else{
		RETURN_FALSE;
	}
}

static PHP_FUNCTION(ssp_info){
	zval *res;
	conn_t *ptr=NULL;
	char *key=NULL;
	int key_len=0;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r|s", &res,&key,&key_len) == FAILURE) {
		RETURN_FALSE;
	}
	ptr = (conn_t *) zend_fetch_resource(Z_RES_P(res), PHP_SSP_DESCRIPTOR_RES_NAME, le_ssp_descriptor);
	if(!ptr) {
		ptr = (conn_t *) zend_fetch_resource(Z_RES_P(res), PHP_SSP_DESCRIPTOR_REF_RES_NAME,le_ssp_descriptor_ref);
	}
	if(key_len==0){
		array_init_size(return_value,5);
		add_assoc_long(return_value,"index",ptr->index);
		add_assoc_long(return_value,"sockfd",ptr->sockfd);
		add_assoc_string(return_value,"host",ptr->host);
		add_assoc_long(return_value,"port",ptr->port);
		add_assoc_long(return_value,"tid",ptr->thread->id);
	}else{
		if(!strcasecmp(key,"index")){
			RETURN_LONG(ptr->index);
		}else if(!strcasecmp(key,"sockfd")){
			RETURN_LONG(ptr->sockfd);
		}else if(!strcasecmp(key,"host")){
			RETVAL_STRINGL(ptr->host,strlen(ptr->host));
		}else if(!strcasecmp(key,"port")){
			RETURN_LONG(ptr->port);
		}else if(!strcasecmp(key,"tid")){
			RETURN_LONG(ptr->thread->id);
		}else{
			RETURN_NULL();
		}
	}
}

static PHP_FUNCTION(ssp_send)
{
	char *data;
	long data_len;
	zval *res;
	conn_t *ptr=NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "rs", &res, &data, &data_len) == FAILURE) {
		RETURN_FALSE;
	}
	ptr = (conn_t *) zend_fetch_resource(Z_RES_P(res), PHP_SSP_DESCRIPTOR_RES_NAME, le_ssp_descriptor);
	if(!ptr) {
		ptr = (conn_t *) zend_fetch_resource(Z_RES_P(res), PHP_SSP_DESCRIPTOR_REF_RES_NAME,le_ssp_descriptor_ref);
	}
	if(ptr) {
		int ret=0;
		char *_data=strndup(data,data_len+1);
		trigger(PHP_SSP_SEND,ptr,&_data,&data_len);
		ret=socket_send(ptr,_data,data_len);
		free(_data);
		RETURN_LONG(ret);
	} else {
		RETURN_FALSE;
	}
}

static PHP_FUNCTION(ssp_destroy)
{
	zval *res;
	conn_t *ptr=NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &res) == FAILURE) {
		RETURN_FALSE;
	}

	ptr = (conn_t *) zend_fetch_resource(Z_RES_P(res), PHP_SSP_DESCRIPTOR_REF_RES_NAME,le_ssp_descriptor_ref);
	if(ptr) {
		zend_list_delete(Z_RES_P(res));
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

static PHP_FUNCTION(ssp_close)
{
	zval *res;
	conn_t *ptr=NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "r", &res) == FAILURE) {
		RETURN_FALSE;
	}

	ptr = (conn_t *) zend_fetch_resource(Z_RES_P(res), PHP_SSP_DESCRIPTOR_REF_RES_NAME,le_ssp_descriptor_ref);
	if(ptr) {
		socket_close(ptr);

		zend_list_delete(Z_RES_P(res));

		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

static PHP_FUNCTION(ssp_lock)
{
	pthread_mutex_lock(&unique_lock);
}
static PHP_FUNCTION(ssp_unlock)
{
	pthread_mutex_unlock(&unique_lock);
}

static PHP_FUNCTION(ssp_stats)
{
	long sleep_time=100000;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|lb", &sleep_time) == FAILURE) {
		RETURN_FALSE;
	}
	if(sleep_time<=0) {
		sleep_time=100000;
	}

#ifdef HAVE_LIBGTOP
	glibtop_cpu cpu_begin,cpu_end;                                 /////////////////////////////
	glibtop_proc_time proctime_begin,proctime_end;                 ///Declare the CPU info and
	glibtop_mem memory;                                            ///memory info struct
	glibtop_proc_mem procmem;                                      ///////////////////////////////

	double us,ni,sy,id;
	int dpu,dps;
	double cpurate,memrate,scale;

	static int pid=0;
	if(pid == 0) {
		pid = getpid();
	}

	glibtop_get_cpu (&cpu_begin);
	glibtop_get_proc_time(&proctime_begin,pid);

	usleep(sleep_time); // 1s=1000000

	glibtop_get_cpu (&cpu_end);
	glibtop_get_proc_time(&proctime_end,pid);

	us = cpu_end.user - cpu_begin.user;
	ni = cpu_end.nice - cpu_begin.nice;
	sy = cpu_end.sys - cpu_begin.sys;
	id = cpu_end.idle - cpu_begin.idle;

	dpu = proctime_end.utime -  proctime_begin.utime;
	dps = proctime_end.stime - proctime_begin.stime;

	scale = 100.0/(cpu_end.total - cpu_begin.total);

	glibtop_get_mem(&memory);
	glibtop_get_proc_mem(&procmem,pid);

	array_init_size(return_value,2);

	zval *sysinfo;
	MAKE_STD_ZVAL(sysinfo);
	array_init_size(sysinfo,27);

	add_assoc_double(sysinfo,"us", us*scale);
	add_assoc_double(sysinfo,"ni", ni*scale);
	add_assoc_double(sysinfo,"sy", sy*scale);
	add_assoc_double(sysinfo,"id", id*scale);

	add_assoc_long(sysinfo,"memTotal", memory.total);
	add_assoc_long(sysinfo,"memUsed", memory.used);
	add_assoc_long(sysinfo,"memFree", memory.free);
	add_assoc_long(sysinfo,"memShared", memory.shared);
	add_assoc_long(sysinfo,"memBuffer", memory.buffer);
	add_assoc_long(sysinfo,"memCached", memory.cached);
	add_assoc_long(sysinfo,"memUser", memory.user);
	add_assoc_long(sysinfo,"memLocked", memory.locked);

	add_assoc_zval(return_value, "sysinfo", sysinfo);

	zval *procinfo;
	MAKE_STD_ZVAL(procinfo);
	array_init_size(procinfo,74);

	add_assoc_double(procinfo,"pcpu", (dpu+dps)*scale);

	add_assoc_long(procinfo,"size",procmem.size);			/* total # of pages of memory */
	add_assoc_long(procinfo,"vsize",procmem.vsize);			/* number of pages of virtual memory ... */
	add_assoc_long(procinfo,"resident",procmem.resident);	/* number of resident set (non-swapped) pages (4k) */
	add_assoc_long(procinfo,"share",procmem.share);			/* number of pages of shared (mmap'd) memory */
	add_assoc_long(procinfo,"rss",procmem.rss);				/* resident set size */
	add_assoc_long(procinfo,"rss_rlim",procmem.rss_rlim);	/* current limit (in bytes) of the rss of the process; usually 2,147,483,647 */

	add_assoc_zval(return_value, "procinfo", procinfo);
#else
	RETURN_FALSE;
#endif
}
