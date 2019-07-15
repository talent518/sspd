<?php
define('COUNT_REQ', 0);

function ssp_start_handler () {
	for($i=0; $i<SSP_MAX_CLIENTS; $i++) ssp_connect('127.0.0.1', 8082 + ($i%3)*2);
}

function ssp_bench_handler () {
	$counts = ssp_counts();
	@list($reqs) = $counts;
	$conns = $counts['conns'];
	
	echo 'threads: ', SSP_NTHREADS, ', conns: ', $conns, ', reqs: ', $reqs, PHP_EOL;

	$reqs or ssp_send(1, date('H:i:s'));
}

function ssp_connect_handler ( $ClientId ) {
}

function ssp_connect_denied_handler ( $ClientId ) {
}

function ssp_receive_handler ( $ClientId, $xml ) {
	ssp_counts(COUNT_REQ);
	
	if(ssp_info($ClientId, 'index') == 1) return date('H:i:s');
}

function ssp_send_handler ( $ClientId, $xml ) {
	return $xml;
}

function ssp_close_handler ( $ClientId ) {
}

function ssp_stop_handler () {
}

