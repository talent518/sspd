#include "php_func.h"
#include "php_ext.h"
#include "ssp.h"
#include "data.h"
#include "api.h"
#include <error.h>
#include <malloc.h>
#include <signal.h>
#include <proc/sysinfo.h>
#include <proc/readproc.h>
#include <math.h>

static pthread_mutex_t unique_lock;

static char trigger_handlers[7][30]={
	"ssp_start_handler",
	"ssp_receive_handler",
	"ssp_send_handler",
	"ssp_connect_handler",
	"ssp_connect_denied_handler",
	"ssp_close_handler",
	"ssp_stop_handler"
};

long le_ssp_descriptor,le_ssp_descriptor_ref,ssp_timeout=30;

/* {{{ arginfo */
ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_resource, 0, 0, 2)
	ZEND_ARG_INFO(0, var)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_info, 0, 0, 2)
	ZEND_ARG_INFO(0, socket)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_send, 0, 0, 2)
	ZEND_ARG_INFO(0, socket)
	ZEND_ARG_INFO(0, message)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_close, 0, 0, 1)
	ZEND_ARG_INFO(0, socket)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_destroy, 0, 0, 1)
	ZEND_ARG_INFO(0, socket)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_lock, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_unlock, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_ssp_stats, 0, 0, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ ssp_functions[] */
function_entry ssp_functions[] = {
	PHP_FE(ssp_resource, arginfo_ssp_resource)
	PHP_FE(ssp_info, arginfo_ssp_info)
	PHP_FE(ssp_send, arginfo_ssp_send)
	PHP_FE(ssp_close, arginfo_ssp_close)
	PHP_FE(ssp_destroy, arginfo_ssp_destroy)
	PHP_FE(ssp_lock, arginfo_ssp_lock)
	PHP_FE(ssp_unlock, arginfo_ssp_unlock)
	PHP_FE(ssp_stats, arginfo_ssp_stats)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ ssp_module_entry
 */
zend_module_entry ssp_module_entry = {
	STANDARD_MODULE_HEADER,
	"ssp",
	ssp_functions,
	PHP_MINIT(ssp),
	PHP_MSHUTDOWN(ssp),
	NULL,
	NULL,
	PHP_MINFO(ssp),
	PHP_SSP_VERSION,
	PHP_MODULE_GLOBALS(ssp),
	PHP_GINIT(ssp),
	PHP_GSHUTDOWN(ssp),
	NULL,
	STANDARD_MODULE_PROPERTIES_EX
};
/* }}} */

static void php_destroy_ssp(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */
{
	conn_t *ptr = (conn_t *) rsrc->ptr;

	conn_info(ptr);
}

static void php_destroy_ssp_ref(zend_rsrc_list_entry *rsrc TSRMLS_DC) /* {{{ */
{
	conn_t *ptr = (conn_t *) rsrc->ptr;

	unref_conn(ptr);
}

/* {{{ MINIT */
static PHP_MINIT_FUNCTION(ssp)
{
	REGISTER_STRING_CONSTANT("SSP_VERSION",  PHP_SSP_VERSION,  CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("SSP_START",  PHP_SSP_START,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_RECEIVE",  PHP_SSP_RECEIVE,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_SEND",  PHP_SSP_SEND,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_CONNECT",  PHP_SSP_CONNECT,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_CONNECT_DENIED",  PHP_SSP_CONNECT_DENIED,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_CLOSE",  PHP_SSP_CLOSE,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_STOP",  PHP_SSP_STOP,  CONST_CS | CONST_PERSISTENT);

	REGISTER_LONG_CONSTANT("SSP_RES_INDEX",  PHP_SSP_RES_INDEX,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_RES_SOCKFD",  PHP_SSP_RES_SOCKFD,  CONST_CS | CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("SSP_RES_PORT",  PHP_SSP_RES_PORT,  CONST_CS | CONST_PERSISTENT);

	le_ssp_descriptor = zend_register_list_destructors_ex(php_destroy_ssp, NULL, PHP_SSP_DESCRIPTOR_RES_NAME,module_number);
	le_ssp_descriptor_ref = zend_register_list_destructors_ex(php_destroy_ssp_ref, NULL, PHP_SSP_DESCRIPTOR_REF_RES_NAME,module_number);

	pthread_mutex_init(&unique_lock, NULL);

	zend_register_auto_global("_SSP", sizeof("_SSP")-1, NULL TSRMLS_CC);

#ifdef SSP_DEBUG_EXT
	printf("ssp module init\n");
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

	zend_delete_global_variable("_SSP", sizeof("_SSP")-1 TSRMLS_CC);

#ifdef SSP_DEBUG_EXT
	printf("ssp module shutdown\n");
#endif
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_GINIT_FUNCTION
*/
static PHP_GINIT_FUNCTION(ssp)
{
	SSP_G(timeout)=(long)time(NULL)+ssp_timeout;
	SSP_G(trigger_count)=0;

#ifdef SSP_DEBUG_EXT
	printf("ssp_globals init\n");
#endif
}
/* }}} */

/* {{{ PHP_GSHUTDOWN_FUNCTION */
static PHP_GSHUTDOWN_FUNCTION(ssp)
{
#ifdef SSP_DEBUG_EXT
	printf("ssp_globals shutdown\n");
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

void ssp_auto_globals_recreate(TSRMLS_D)
{
	zend_delete_global_variable("_SSP", sizeof("_SSP")-1 TSRMLS_CC);

	zval *vars;

	MAKE_STD_ZVAL(vars);
	array_init_size(vars, 8);

	ZEND_SET_GLOBAL_VAR("_SSP", vars);
}

static zval *_ssp_resource_zval(conn_t *value)
{
	zval *ret;
	MAKE_STD_ZVAL(ret);
	ZEND_REGISTER_RESOURCE(ret,value,le_ssp_descriptor);
	return ret;
}

static zval *_ssp_string_zval(const char *str,int len)
{
	zval *ret;
	MAKE_STD_ZVAL(ret);

	Z_TYPE_P(ret) = IS_STRING;
	Z_STRLEN_P(ret) = len;
	Z_STRVAL_P(ret) = estrndup(str, len);
	return ret;
}

bool trigger(unsigned short type,...){
	TRIGGER_STARTUP();
	if(trigger_handlers[type]==NULL){
		TRIGGER_SHUTDOWN();
		return FAILURE;
	}
	zval *zval_ptr,*zval_data,*pfunc;
	zval ***params,*retval;
	int i,param_count,ret;
	bool retbool=true;
	char *call_func_name;
	va_list args;
	conn_t *ptr;
	char **data=NULL;
	long *data_len;

	call_func_name=strdup(trigger_handlers[type]);
	pfunc=_ssp_string_zval(call_func_name,strlen(call_func_name));

	va_start(args,type);
	switch(type){
		case PHP_SSP_START:
		case PHP_SSP_STOP:
			param_count=0;
			params=NULL;
			break;
		case PHP_SSP_RECEIVE:
		case PHP_SSP_SEND:
			param_count=2;
			ptr=va_arg(args,conn_t*);
			data=va_arg(args,char**);
			data_len=va_arg(args,long*);
			params=(zval ***) emalloc(sizeof(zval **)*param_count);
			zval_ptr=_ssp_resource_zval(ptr);
			zval_data=_ssp_string_zval(*data,*data_len);
			params[0]=&zval_ptr;
			params[1]=&zval_data;
			break;
		case PHP_SSP_CONNECT:
		case PHP_SSP_CONNECT_DENIED:
		case PHP_SSP_CLOSE:
			param_count=1;
			ptr=va_arg(args,conn_t*);
			params=(zval ***) emalloc(sizeof(zval **)*param_count);
			zval_ptr=_ssp_resource_zval(ptr);
			params[0]=&zval_ptr;
			break;
		default:
			perror("Trigger type not exists!");
			return FAILURE;
	}
	va_end(args);

	ret=call_user_function_ex(CG(function_table), NULL, pfunc, &retval, param_count, params,1,NULL TSRMLS_CC);
	if(ret==SUCCESS){
		if(Z_TYPE_P(retval) == IS_BOOL) {
			retbool=Z_LVAL_P(retval);
		}
		if(param_count>1){
			convert_to_string_ex(&retval);
			free(*data);
			if(Z_STRLEN_P(retval)>0){
				char *_data=strndup(Z_STRVAL_P(retval),Z_STRLEN_P(retval));
				*data=_data;
				*data_len=Z_STRLEN_P(retval);
			}else{
				*data=NULL;
				*data_len=0;
			}
		}
	}else{
		php_printf("\nUnable to call handler(%s)", call_func_name);
	}
	zval_ptr_dtor(&retval);
	if(param_count>0){
		int i;
		for(i=0;i<param_count;i++){
			zval_ptr_dtor(params[i]);
		}
		efree(params);
	}
	zval_ptr_dtor(&pfunc);
	free(call_func_name);
	TRIGGER_SHUTDOWN();
	return retbool;
}

static PHP_FUNCTION(ssp_resource){
	long var,type;
	conn_t *ptr=NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll", &var, &type) == FAILURE) {
		RETURN_FALSE;
	}
	switch(type) {
		case PHP_SSP_RES_SOCKFD:
			ptr=sockfd_conn(var);
			break;
		case PHP_SSP_RES_PORT:
			ptr=port_conn(var);
			break;
		case PHP_SSP_RES_INDEX:
			ptr=index_conn(var);
			break;
		default:
			break;
	}
	if(ptr!=NULL){
		ZEND_REGISTER_RESOURCE(return_value,ptr,le_ssp_descriptor_ref);
	}else{
		RETURN_FALSE;
	}
}

static PHP_FUNCTION(ssp_info){
	zval *res;
	conn_t *ptr=NULL;
	char *key;
	int key_len=0;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r|s", &res,&key,&key_len) == FAILURE) {
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(ptr,conn_t*, &res, -1, PHP_SSP_DESCRIPTOR_RES_NAME,le_ssp_descriptor);
	if(!ptr) {
		ZEND_FETCH_RESOURCE(ptr,conn_t*, &res, -1, PHP_SSP_DESCRIPTOR_REF_RES_NAME,le_ssp_descriptor_ref);
	}
	if(key_len==0){
		array_init_size(return_value,5);
		add_assoc_long(return_value,"index",ptr->index);
		add_assoc_long(return_value,"sockfd",ptr->sockfd);
		add_assoc_string(return_value,"host",ptr->host,1); /* cast to avoid gcc-warning */
		add_assoc_long(return_value,"port",ptr->port);
		add_assoc_long(return_value,"tid",ptr->thread->id);
	}else{
		if(!strcasecmp(key,"index")){
			RETURN_LONG(ptr->index);
		}else if(!strcasecmp(key,"sockfd")){
			RETURN_LONG(ptr->sockfd);
		}else if(!strcasecmp(key,"host")){
			RETURN_STRING(strdup(ptr->host),strlen(ptr->host));
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
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "rs", &res, &data, &data_len) == FAILURE) {
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(ptr,conn_t*, &res, -1, PHP_SSP_DESCRIPTOR_REF_RES_NAME,le_ssp_descriptor_ref);
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
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res) == FAILURE) {
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(ptr,conn_t*, &res, -1, PHP_SSP_DESCRIPTOR_REF_RES_NAME,le_ssp_descriptor_ref);

	if(ptr) {
		zend_list_delete(Z_LVAL_P(res));
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

static PHP_FUNCTION(ssp_close)
{
	zval *res;
	conn_t *ptr=NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "r", &res) == FAILURE) {
		RETURN_FALSE;
	}
	ZEND_FETCH_RESOURCE(ptr,conn_t*, &res, -1, PHP_SSP_DESCRIPTOR_REF_RES_NAME,le_ssp_descriptor_ref);

	if(ptr) {
		socket_close(ptr);

		zend_list_delete(Z_LVAL_P(res));

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
	zend_bool include_dead_children=0;
	long sleep_time=1;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|lb", &sleep_time, &include_dead_children) == FAILURE) {
		RETURN_FALSE;
	}

	unsigned int running, blocked, btime, processes;
	jiff cpu_use[2], cpu_nic[2], cpu_sys[2], cpu_idl[2], cpu_iow[2], cpu_xxx[2], cpu_yyy[2], cpu_zzz[2];
	unsigned long pgpgin[2], pgpgout[2], pswpin[2], pswpout[2];
	unsigned int intr[2], ctxt[2];

	unsigned int sleep_half;

	unsigned long seconds_since_boot;
	proc_t proc;

	sleep_half=(sleep_time/2);

	bzero(&proc,sizeof(proc_t));

	static pid_t pid=0;
	if(pid == 0) {
		pid = getpid();
	}

	getstat(&cpu_use[0], &cpu_nic[0], &cpu_sys[0], &cpu_idl[0],
		&cpu_iow[0], &cpu_xxx[0], &cpu_yyy[0], &cpu_zzz[0],
		&pgpgin[0], &pgpgout[0], &pswpin[0], &pswpout[0],
		&intr[0], &ctxt[0],
		&running, &blocked,
		&btime, &processes);

	seconds_since_boot = uptime(0,0);

	sleep(sleep_time); // wait 1s

	getstat(&cpu_use[1], &cpu_nic[1], &cpu_sys[1], &cpu_idl[1],
		&cpu_iow[1], &cpu_xxx[1], &cpu_yyy[1], &cpu_zzz[1],
		&pgpgin[1], &pgpgout[1], &pswpin[1], &pswpout[1],
		&intr[1], &ctxt[1],
		&running, &blocked,
		&btime, &processes);

	get_proc_stats(pid, &proc);

	meminfo();

	array_init_size(return_value,2);

	zval *sysinfo;
	MAKE_STD_ZVAL(sysinfo);
	array_init_size(sysinfo,26);

	add_assoc_long(sysinfo,"memTotal", kb_main_total);
	add_assoc_long(sysinfo,"memUsed", kb_main_used);
	add_assoc_long(sysinfo,"memActive", kb_active);
	add_assoc_long(sysinfo,"memInactive", kb_inactive);
	add_assoc_long(sysinfo,"memFree", kb_main_free);
	add_assoc_long(sysinfo,"memBuffer", kb_main_buffers);
	add_assoc_long(sysinfo,"swapCache", kb_main_cached);
	add_assoc_long(sysinfo,"swapTotal", kb_swap_total);
	add_assoc_long(sysinfo,"swapUsed", kb_swap_used);
	add_assoc_long(sysinfo,"swapFree", kb_swap_free);

	add_assoc_long(sysinfo,"running", running);
	add_assoc_long(sysinfo,"blocked", blocked);
	add_assoc_long(sysinfo,"bootTime", btime);
	add_assoc_long(sysinfo,"processes", processes);

	jiff duse, dsys, didl, diow, dstl, Div, divo2;
	unsigned long kb_per_page = sysconf(_SC_PAGESIZE) / 1024ul;
	unsigned int hz = Hertz;
	int debt = 0;  // handle idle ticks running backwards

    duse= cpu_use[1]-cpu_use[0] + cpu_nic[1]-cpu_nic[0];
    dsys= cpu_sys[1]-cpu_sys[0] + cpu_xxx[1]-cpu_xxx[0] + cpu_yyy[1]-cpu_yyy[0];
    didl= cpu_idl[1]-cpu_idl[0];
    diow= cpu_iow[1]-cpu_iow[0];
    dstl= cpu_zzz[1]-cpu_zzz[0];

    /* idle can run backwards for a moment -- kernel "feature" */
    if(debt){
      didl = (int)didl + debt;
      debt = 0;
    }
    if( (int)didl < 0 ){
      debt = (int)didl;
      didl = 0;
    }

    Div= duse+dsys+didl+diow+dstl;
    divo2= Div/2UL;

	add_assoc_double(sysinfo, "si", 1.0*( (pswpin [1] - pswpin [0])*kb_per_page+sleep_half )/sleep_time ); /*si*/
	add_assoc_double(sysinfo, "so", 1.0*( (pswpout[1] - pswpout[0])*kb_per_page+sleep_half )/sleep_time ); /*so*/
	add_assoc_double(sysinfo, "bi", 1.0*(  pgpgin [1] - pgpgin [0]             +sleep_half )/sleep_time ); /*bi*/
	add_assoc_double(sysinfo, "bo", 1.0*(  pgpgout[1] - pgpgout[0]             +sleep_half )/sleep_time ); /*bo*/
	add_assoc_double(sysinfo, "in", 1.0*(  intr   [1] - intr   [0]             +sleep_half )/sleep_time ); /*in*/
	add_assoc_double(sysinfo, "cs", 1.0*(  ctxt   [1] - ctxt   [0]             +sleep_half )/sleep_time ); /*cs*/
	add_assoc_double(sysinfo, "us", 1.0*(100*duse+divo2)/Div ); /*us*/
	add_assoc_double(sysinfo, "sy", 1.0*(100*dsys+divo2)/Div ); /*sy*/
	add_assoc_double(sysinfo, "id", 1.0*(100*didl+divo2)/Div ); /*id*/
	add_assoc_double(sysinfo, "wa", 1.0*(100*diow+divo2)/Div ); //wa

	add_assoc_zval(return_value, "sysinfo", sysinfo);

	if(proc.tid) {
		unsigned long long total_time;   /* jiffies used by this process */
		unsigned long pcpu = 0;               /* scaled %cpu, 999 means 99.9% */
		unsigned long long seconds;      /* seconds of process life */
		total_time = proc.utime + proc.stime;
		if(include_dead_children) total_time += (proc.cutime + proc.cstime);
		seconds = seconds_since_boot - proc.start_time / Hertz ;
		if(seconds) pcpu = (total_time * 1000ULL / Hertz) / seconds;

		proc.pcpu = pcpu;  // fits in an int, summing children on 128 CPUs

		zval *procinfo;
		MAKE_STD_ZVAL(procinfo);
		array_init_size(procinfo,74);
		add_assoc_long(procinfo,"pid",proc.tid); // (special)       task id, the POSIX thread ID (see also: tgid)
		add_assoc_long(procinfo,"ppid",proc.ppid); // stat,status     pid of parent process
		add_assoc_double(procinfo,"pcpu",proc.pcpu/10.0L); // stat (special)  %CPU usage (is not filled in by readproc!!!)
		add_assoc_stringl(procinfo,"state",&proc.state,1,1); // stat,status     single-char code for process state (S=sleeping)
		add_assoc_long(procinfo,"utime",proc.utime); // user-mode CPU time accumulated by process
		add_assoc_long(procinfo,"stime",proc.stime); // kernel-mode CPU time accumulated by process
		add_assoc_long(procinfo,"cutime",proc.cutime); // stat            cumulative utime of process and reaped children
		add_assoc_long(procinfo,"cstime",proc.cstime); // stat            cumulative stime of process and reaped children
		add_assoc_long(procinfo,"start_time",proc.start_time);	// stat            start time of process -- seconds since 1-1-70
		add_assoc_string(procinfo,"signal",proc.signal,1); // status          mask of pending signals, per-task for readtask() but per-proc for readproc()
		add_assoc_string(procinfo,"blocked",proc.blocked,1); // status          mask of blocked signals
		add_assoc_string(procinfo,"sigignore",proc.sigignore,1); // status          mask of ignored signals
		add_assoc_string(procinfo,"sigcatch",proc.sigcatch,1); // status          mask of caught  signals
		add_assoc_string(procinfo,"_sigpnd",proc._sigpnd,1); // status          mask of PER TASK pending signals
		add_assoc_long(procinfo,"start_code",proc.start_code); // stat            address of beginning of code segment
		add_assoc_long(procinfo,"end_code",proc.end_code); // stat            address of end of code segment
		add_assoc_long(procinfo,"start_stack",proc.start_stack); // stat            address of the bottom of stack for the process
		add_assoc_long(procinfo,"kstk_esp",proc.kstk_esp); // stat            kernel stack pointer
		add_assoc_long(procinfo,"kstk_eip",proc.kstk_eip); // stat            kernel instruction pointer
		add_assoc_long(procinfo,"wchan",proc.wchan); // stat (special)  address of kernel wait channel proc is sleeping in
		add_assoc_long(procinfo,"priority",proc.priority); // stat            kernel scheduling priority
		add_assoc_long(procinfo,"nice",proc.nice); // stat            standard unix nice level of process
		add_assoc_long(procinfo,"rss",proc.rss); // stat            resident set size from /proc/#/stat (pages)
		add_assoc_long(procinfo,"alarm",proc.alarm); // stat            ?
		add_assoc_long(procinfo,"size",proc.size); // statm           total # of pages of memory
		add_assoc_long(procinfo,"resident",proc.resident); // statm           number of resident set (non-swapped) pages (4k)
		add_assoc_long(procinfo,"share",proc.share); // statm           number of pages of shared (mmap'd) memory
		add_assoc_long(procinfo,"trs",proc.trs); // statm           text resident set size
		add_assoc_long(procinfo,"lrs",proc.lrs); // statm           shared-lib resident set size
		add_assoc_long(procinfo,"drs",proc.drs); // statm           data resident set size
		add_assoc_long(procinfo,"dt",proc.dt); // statm           dirty pages
		add_assoc_long(procinfo,"vm_size",proc.vm_size); // status          same as vsize in kb
		add_assoc_long(procinfo,"vm_lock",proc.vm_lock); // status          locked pages in kb
		add_assoc_long(procinfo,"vm_rss",proc.vm_rss); // status          same as rss in kb
		add_assoc_long(procinfo,"vm_data",proc.vm_data); // status          data size
		add_assoc_long(procinfo,"vm_stack",proc.vm_stack); // status          stack size
		add_assoc_long(procinfo,"vm_exe",proc.vm_exe); // status          executable size
		add_assoc_long(procinfo,"vm_lib",proc.vm_lib); // status          library size (all pages, not just used ones)
		add_assoc_long(procinfo,"rtprio",proc.rtprio); // stat            real-time priority
		add_assoc_long(procinfo,"sched",proc.sched); // stat            scheduling class
		add_assoc_long(procinfo,"vsize",proc.vsize); // stat            number of pages of virtual memory ...
		add_assoc_long(procinfo,"rss_rlim",proc.rss_rlim); // stat            resident set size limit?
		add_assoc_long(procinfo,"flags",proc.flags); // stat            kernel flags for the process
		add_assoc_long(procinfo,"min_flt",proc.min_flt); // stat            number of minor page faults since process start
		add_assoc_long(procinfo,"maj_flt",proc.maj_flt); // stat            number of major page faults since process start
		add_assoc_long(procinfo,"cmin_flt",proc.cmin_flt); // stat            cumulative min_flt of process and child processes
		add_assoc_long(procinfo,"cmaj_flt",proc.cmaj_flt); // stat            cumulative maj_flt of process and child processes

		if(proc.environ)
			add_assoc_string(procinfo,"environ",*proc.environ,0); // (special)       environment string vector (/proc/#/environ)

		if(proc.cmdline)
			add_assoc_string(procinfo,"cmdline",*proc.cmdline,0); // (special)       command line string vector (/proc/#/cmdline)

		add_assoc_string(procinfo,"euser",proc.euser,1); // stat(),status   effective user name
		add_assoc_string(procinfo,"ruser",proc.ruser,1); // status          real user name
		add_assoc_string(procinfo,"suser",proc.suser,1); // status          saved user name
		add_assoc_string(procinfo,"fuser",proc.fuser,1); // status          filesystem user name
		add_assoc_string(procinfo,"rgroup",proc.rgroup,1); // status          real group name
		add_assoc_string(procinfo,"egroup",proc.egroup,1); // status          effective group name
		add_assoc_string(procinfo,"sgroup",proc.sgroup,1); // status          saved group name
		add_assoc_string(procinfo,"fgroup",proc.fgroup,1); // status          filesystem group name
		add_assoc_string(procinfo,"cmd",proc.cmd,1); // stat,status     basename of executable file in call to exec(2)
		add_assoc_long(procinfo,"pgrp",proc.pgrp); // stat            process group id
		add_assoc_long(procinfo,"session",proc.session); // stat            session id
		add_assoc_long(procinfo,"nlwp",proc.nlwp); // stat,status     number of threads, or 0 if no clue
		add_assoc_long(procinfo,"tgid",proc.tgid); // (special)       task group ID, the POSIX PID (see also: tid)
		add_assoc_long(procinfo,"tty",proc.tty); // stat            full device number of controlling terminal
		add_assoc_long(procinfo,"euid",proc.euid); // stat(),status   effective
		add_assoc_long(procinfo,"egid",proc.egid); // stat(),status   effective
		add_assoc_long(procinfo,"ruid",proc.ruid); // status          real
		add_assoc_long(procinfo,"rgid",proc.rgid); // status          real
		add_assoc_long(procinfo,"suid",proc.suid); // status          saved
		add_assoc_long(procinfo,"sgid",proc.sgid); // status          saved
		add_assoc_long(procinfo,"fuid",proc.fuid); // status          fs (used for file access only)
		add_assoc_long(procinfo,"fgid",proc.fgid); // status          fs (used for file access only)
		add_assoc_long(procinfo,"tpgid",proc.tpgid); // stat            terminal process group id
		add_assoc_long(procinfo,"exit_signal",proc.exit_signal); // stat            might not be SIGCHLD
		add_assoc_long(procinfo,"processor",proc.processor); // stat            current (or most recent?) CPU

		add_assoc_zval(return_value, "procinfo", procinfo);
	}
}
