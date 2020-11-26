<?php
define('SSP_VERSION');

define('SSP_PIDFILE', '/var/run/ssp.pid');
define('SSP_USER', 'daemon');
define('SSP_HOST', '0.0.0.0');
define('SSP_PORT', 8083);
define('SSP_MAX_CLIENTS', 1024);
define('SSP_MAX_RECVS', 2 * 1024 * 1024);
define('SSP_NTHREADS', 10);
define('SSP_BACKLOG', 1024);
define('SSP_TIMEOUT', time());

define('STDIN', fopen('php://stdin')); // 默认来自/dev/urandom
define('STDOUT', fopen('php://stdout')); // 默认输出到/var/run/ssp.out
define('STDERR', fopen('php://stderr')); // 默认输出到/var/run/ssp.err

/* @var array $_SSP */
/* @global array $_SSP */
// @var array $_SSP
$_SSP = $GLOBALS['_SSP'] = array();

/**
 * 设置ssp转发服务器(ssp服务端在ssp_start_handler中使用)
 *
 * @param integer $sid
 * @param integer $max_sid
 */
function ssp_conv_setup ( $sid, $max_sid ) {
}

/**
 * 连接ssp转发服务器(ssp服务端在ssp_start_handler中使用)
 *
 * @param string $host
 * @param integer port
 * @param integer sid
 */
function ssp_conv_connect ( $host, $port, $sid ) {
}

/**
 * 关闭所有ssp转发服务器的连接(ssp服务端在ssp_stop_handler中使用)
 */
function ssp_conv_disconnect ( ) {
}

/**
 * 获取连接资源信息
 *
 * @param resource $res
 * @param string $key
 */
function ssp_info ( $res, $key = null ) {
}

/**
 * 连接ssp服务器(压力测试专用且只能在ssp_start_handler)
 *
 * @param string $host
 * @param integer $port
 */
function ssp_connect ( $host, $port = SSP_PORT ) {
}

/**
 * 发送消息
 *
 * @param integer|resource $index|$res
 * @param string $message
 */
function ssp_send ( $index, $message, $sid = null ) {
}

/**
 * 关闭连接
 *
 * @param integer|resource $index|$res
 */
function ssp_close ( $index, $sid = null ) {
}

/**
 * 互斥加锁
 */
function ssp_lock () {
}

/**
 * 互斥解锁
 */
function ssp_unlock () {
}

/**
 * 获取系统和服务占用系统资源情况
 *
 * @param boolean $isMonitor
 */
function ssp_stats ( $isMonitor = false ) {
}

define('COUNT_KEY_SIZE', 1024);
define('COUNT_TYPE_AVG', -4);
define('COUNT_TYPE_GET', -3);
define('COUNT_TYPE_DEC', -2);
define('COUNT_TYPE_INC', -1);
define('COUNT_TYPE_SET', 0);
/**
 * 按指定的key进行计数
 *
 * @param integer $key 取值范围：大于等于0且小于COUNT_KEY_SIZE
 * @param integer $type 等于COUNT_TYPE_AVG求最近的$size个$val值的平均值(需要占用$size+1个，所以后面的需要使用时$key值需要加上$size+2方可), 等于COUNT_TYPE_GET直接返回，等于COUNT_TYPE_DEC则减1后返回，等于COUNT_TYPE_INC则加1后返回，等于COUNT_TYPE_SET则返回后设为$val，大于0则加1后与$type相等则设为$val并返回boolean
 * @param integer $val
 * @return integer|boolean|null $key小于0或$key大于等于16，则返回null
 */
function ssp_counts($key = null, $type = COUNT_TYPE_INC, $val = 0, $size = 10) {
}

/**
 * 加密
 *
 * @param string $string
 * @param string $key
 * @param integer $expiry
 *
 * @return 密文
 */
function crypt_encode ( $string, $key, $expiry = 0 ) {
}

/**
 * 解密
 *
 * @param string $string
 * @param string $key
 * @param integer $expiry
 *
 * @return 明文
 */
function crypt_decode ( $string, $key, $expiry = 0 ) {
}

/**
 * 初始化消息队列(只能在ssp_start_handler中使用)
 * @param integer $msgs
 * @param integer $nthreads
 */
function ssp_msg_queue_init ( $msgs, $nthreads ) {
}

/**
 * 添加消息到队列
 *
 * @param string $funcname
 * @param integer $what
 * @param mixed $arg
 * @param integer $arg1
 * @param integer $arg2
 * @param integer $arg3
 * @param integer $arg4
 * @param integer $arg5
 */
function ssp_msg_queue_push ( $funcname, $what, $arg, $arg1 = 0, $arg2 = 0, $arg3 = 0, $arg4 = 0, $arg5 = 0 ) {
}

/**
 * 回收消息队列(只能在ssp_stop_handler中使用)
 */
function ssp_msg_queue_destory () {
}

/**
 * 初始化计时器(只能在ssp_start_handler中使用)
 */
function ssp_delayed_init () {
}

/**
 * 添加计时器
 *
 * @param string $funcname
 * @param integer $delay
 * @param boolean $persist
 * @param mixed $arg
 * @param integer $arg1
 * @param integer $arg2
 * @param integer $arg3
 * @param integer $arg4
 * @param integer $arg5
 */
function ssp_delayed_set ( $funcname, $delay, $persist, $arg = null, $arg1 = 0, $arg2 = 0, $arg3 = 0, $arg4 = 0, $arg5 = 0 ) {
}

/**
 * 删除计时器
 *
 * @param string $funcname
 */
function ssp_delayed_del ( $funcname ) {
}

/**
 * 回收计时器(只能在ssp_stop_handler中使用)
 */
function ssp_delayed_destory () {
}

/**
 * 初始化共享变量(只能在ssp_start_handler中使用)，要求ssp_var_init必需在ssp_msg_queue_init和ssp_delayed_init前调用
 */
function ssp_var_init($size = SSP_MAX_CLIENTS) {
}

/**
 * 是否存在指定的共享变量
 *
 * 至少一个参数，每个参数代码要查询的多维数组的key
 */
function ssp_var_exists($key1[,...]) {
}

/**
 * 读取共享变量
 *
 * 每个参数代码要查询的多维数组的key
 */
function ssp_var_get([$key1,...]) {
}

/**
 * 写入共享变量
 *
 * 至少一个参数，每个参数代码要查询的多维数组的key，最后一个是数组可与存在数组合并，否则则替换
 */
function ssp_var_put() {
}

/**
 * 写入共享变量：$value 正数为递增，负数为递减
 */
function ssp_var_inc($key..., $value) {
}

/**
 * 写入共享变量
 */
function ssp_var_set($key..., $value) {
}

/**
 * 删除共享变量
 *
 * 至少一个参数，每个参数代码要查询的多维数组的key
 */
function ssp_var_del() {
}

/**
 * 清空共享变量
 */
function ssp_var_clean() {
}

/**
 * 回收共享变量(只能在ssp_stop_handler中使用)，要求ssp_var_destory必需在ssp_msg_queue_destory和ssp_delayed_destory后调用
 */
function ssp_var_destory() {
}

