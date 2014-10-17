<?php
define('SSP_VERSION');

define('SSP_RES_INDEX', 0);
define('SSP_RES_SOCKFD', 1);
define('SSP_RES_PORT', 2);

define('SSP_PIDFILE', '/var/run/ssp.pid');
define('SSP_USER', 'daemon');
define('SSP_HOST', '0.0.0.0');
define('SSP_PORT', 8083);
define('SSP_MAX_CLIENTS', 1024);
define('SSP_MAX_RECVS', 2 * 1024 * 1024);
define('SSP_NTHREADS', 10);
define('SSP_BACKLOG', 1024);
define('SSP_TIMEOUT', time());

define('STD_CHARSET', PHP_OS == 'Linux' ? 'gbk' : 'utf-8');

/* @var array $_SSP */
/* @global array $_SSP */
// @var array $_SSP
$_SSP = $GLOBALS['_SSP'] = array();

/**
 * 获取连接资源
 * 
 * @param resource $var index/sockfd/port
 * @param integer $type SSP_RES_INDEX、SSP_RES_SOCKFD 或 SSP_RES_PORT
 */
function ssp_resource ( $var, $type ) {
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
 * 发送消息
 *
 * @param resource $res
 * @param string $message
 */
function ssp_send ( $res, $message ) {
}

/**
 * 关闭连接
 *
 * @param resource $res
 */
function ssp_close ( $res ) {
}

/**
 * 释放连接
 *
 * @param resource $res
 */
function ssp_destroy ( $res ) {
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