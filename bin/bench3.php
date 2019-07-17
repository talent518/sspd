<?php
define('COUNT_CONN', 0);
define('COUNT_REQ', 1);
define('COUNT_REQ2', 2);

function ssp_start_handler () {
	for($i=0; $i<SSP_MAX_CLIENTS; $i++) ssp_connect('127.0.0.1', 8082 + ($i%3)*2);

	ssp_send(1, date('H:i:s'));
}

function ssp_bench_handler () {
	echo 'threads: ', SSP_NTHREADS, ', conns: ', ssp_counts(COUNT_CONN, -3), ', reqs: ', ssp_counts(COUNT_REQ2, 0), PHP_EOL;
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
}
