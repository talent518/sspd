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

define('SETUP_USERNAME', 0);
define('SETUP_SENDKEY', 1);
define('SETUP_RECEIVEKEY', 2);

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
 * @param integer $sleep_time
 */
function ssp_stats ( $sleep_time = 100000 ) {
}

/**
 * 设置连接类型
 *
 * @param resource $res
 * @param integer $type
 */
function ssp_type($res, $type) {
}

/**
 * 设置连接请求数
 *
 * @param resource $res
 * @param integer $type 按连接requests(请求数)进行加$type存储并返回requests结果
 */
function ssp_requests($res, $type = 1) {
}

/**
 * 按指定的key进行计数
 *
 * @param integer $key
 * @param integer $type 小于等于-3直接返回，等于-2则减1后返回，等于-1则加1后返回，等于0则返回后清零，大于0则加1后与$type比较是否相等返回bool
 * @return integer|boolean|null $key小于0或$key大于等于16，则返回null
 */
function ssp_counts($key = null, $type = -1) {
}

/**
 * 设置连接连接参数
 *
 * @param resource $res
 * @param integer $type
 * @param string $str 不指定该参数则返回$type类型的值
 */
function ssp_setup($res, $type, $str = null) {
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


