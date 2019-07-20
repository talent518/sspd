<?php
define('COUNT_CONN', 0);
define('COUNT_REQ', 1);
define('COUNT_REQ2', 2);
define('COUNT_AVG', 3);

function connect_handler($what, $arg, $arg1, $arg2, $arg3, $arg4, $arg5) {
	for($i=0; $i<SSP_MAX_CLIENTS; $i++) ssp_connect('127.0.0.1', 8082 + ($i%3)*2);

	echo 'connect usage time: ', round(microtime(true) - $arg, 3), ' seconds', PHP_EOL;
}

function timeout1_handler($delay, $persist, $arg, $arg1, $arg2, $arg3, $arg4, $arg5) {
	echo 'timeout1_handler: ', $delay, ', ', $persist ? 'true' : 'false', PHP_EOL;
	// ssp_delayed_del("timeout2_handler");
	// ssp_delayed_del("timeout3_handler");
}

function timeout2_handler($delay, $persist, $arg, $arg1, $arg2, $arg3, $arg4, $arg5) {
	echo 'timeout2_handler: ', $delay, ', ', $persist ? 'true' : 'false', PHP_EOL;
}

function timeout3_handler($delay, $persist, $arg, $arg1, $arg2, $arg3, $arg4, $arg5) {
	echo 'timeout3_handler: ', $delay, ', ', $persist ? 'true' : 'false', ', ', microtime(true)-$arg, PHP_EOL;
	
	if($delay === 5000) return false;
	ssp_delayed_set("timeout3_handler", 5000, true, microtime(true));
}

function timeout4_handler() {
	ssp_delayed_del('timeout1_handler');
	ssp_delayed_del('timeout2_handler');
	ssp_delayed_del('timeout3_handler');
	ssp_delayed_set('timeout3_handler', 10, false, microtime(true));
}

function timeout5_handler($delay, $persist, $arg) {
	echo 'timeout5_handler: ', $delay, ', ', $persist ? 'true' : 'false', ', ', microtime(true)-$arg, PHP_EOL;
	ssp_delayed_set("timeout6_handler", 10, false, microtime(true));
}

function timeout6_handler($delay, $persist, $arg) {
	echo 'timeout6_handler: ', $delay, ', ', $persist ? 'true' : 'false', ', ', microtime(true)-$arg, PHP_EOL;
}

function count_handler($delay, $persist, $arg) {
	$s = ssp_counts(COUNT_REQ2, 0);
	$a = 0;
	if($s) {
		$a = ssp_counts(COUNT_AVG, -3);
		if($a) {
			$a = floor(($a+$s) / 2);
			ssp_counts(COUNT_AVG, 0, $a);
		} else {
			$a = $s;
			ssp_counts(COUNT_AVG, 0, $s);
		}
	}
	echo gmdate('H:i:s', time()-$arg), ' - threads: ', SSP_NTHREADS, ', conns: ', ssp_counts(COUNT_CONN, -3), ', reqs: ', $s, '(', $a, ')', PHP_EOL;
}

function ssp_start_handler () {
	ssp_msg_queue_init(10000, 1);
	ssp_msg_queue_push('connect_handler', 0, microtime(true));
	ssp_delayed_init();
	ssp_delayed_set("count_handler", 1000, true, time());
	// ssp_delayed_set("timeout1_handler", 1000, true);
	// ssp_delayed_set("timeout2_handler", 2000, true);
	// ssp_delayed_set("timeout3_handler", 3000, false, microtime(true));
	// ssp_delayed_set("timeout4_handler", 20000, false);
	// ssp_delayed_set("timeout5_handler", 5010, false, microtime(true));
}

function ssp_bench_handler () {
	// echo 'threads: ', SSP_NTHREADS, ', conns: ', ssp_counts(COUNT_CONN, -3), ', reqs: ', ssp_counts(COUNT_REQ2, 0), PHP_EOL;
}

function ssp_connect_handler ( $ClientId ) {
	ssp_counts(COUNT_CONN);
}

function ssp_connect_denied_handler ( $ClientId ) {
}

function ssp_receive_handler ( $ClientId, $xml ) {
	ssp_counts(COUNT_REQ2);

	if(ssp_counts(COUNT_REQ, SSP_MAX_CLIENTS)) return date('H:i:s');
}

function ssp_send_handler ( $ClientId, $xml ) {
	return $xml;
}

function ssp_close_handler ( $ClientId ) {
	ssp_counts(COUNT_CONN, -2);
}

function ssp_stop_handler () {
	ssp_msg_queue_destory();
	ssp_delayed_destory();
}
