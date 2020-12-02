<?php
require dirname(__FILE__) . DIRECTORY_SEPARATOR . 'core.php';

define('SETUP_KEY', 0);
define('SETUP_USERNAME', 1);
define('SETUP_SENDKEY', 2);
define('SETUP_RECEIVEKEY', 3);
define('SETUP_REQUESTS', 4);

define('TYPE_KEY', 0);
define('TYPE_LOGIN', 1);
define('TYPE_REQUEST', 2);

define('COUNT_CONN', 0);
define('COUNT_KEY', 1);
define('COUNT_LOGIN', 2);
define('COUNT_REQUEST', 3);
define('COUNT_REQUEST_AVG', 4);
define('COUNT_REQUEST_FAILURE', 106);
define('COUNT_REQUEST_FAILURE_AVG', 107);

import('lib.xml');
import('mod.uc.base');

function ssp_monitor_handler(array $scpu, array $pcpu, array $smem, array $pmem, int $threads, int $etime, array $args) {
}

function connect_handler($what, $arg, $arg1, $arg2, $arg3, $arg4, $arg5) {
	for($i=0; $i<SSP_MAX_CLIENTS; $i++) ssp_connect(SSP_HOST);

	echo 'connect usage time: ', round(microtime(true) - $arg, 3), ' seconds', PHP_EOL;
}

function count_handler($delay, $persist, $arg) {
	$n = time() - $arg;
	$reqs = ssp_counts(COUNT_REQUEST, COUNT_TYPE_SET);
	$fails = ssp_counts(COUNT_REQUEST_FAILURE, COUNT_TYPE_SET);
	echo gmdate('H:i:s', $n), ' - threads: ', SSP_NTHREADS, ', conns: ', ssp_counts(COUNT_CONN, COUNT_TYPE_GET), ', keys: ', ssp_counts(COUNT_KEY, COUNT_TYPE_SET), ', logins: ', ssp_counts(COUNT_LOGIN, COUNT_TYPE_SET), ', requests: ', $reqs, '(', ssp_counts(COUNT_REQUEST_AVG, COUNT_TYPE_AVG, $reqs, 100), ')', ', requestFailures: ', $fails, '(', ssp_counts(COUNT_REQUEST_FAILURE_AVG, COUNT_TYPE_AVG, $fails, 100), ')', PHP_EOL;
}

function ssp_start_handler () {
	ssp_var_init();
	ssp_msg_queue_init(10000, 1);
	ssp_msg_queue_push('connect_handler', 0, microtime(true));
	ssp_delayed_init();
	ssp_delayed_set("count_handler", 1000, true, time());
}

function ssp_bench_handler () {
	// echo 'threads: ', SSP_NTHREADS, ', conns: ', ssp_counts(COUNT_CONN, COUNT_TYPE_GET), ', keys: ', ssp_counts(COUNT_KEY, COUNT_TYPE_SET), ', logins: ', ssp_counts(COUNT_LOGIN, COUNT_TYPE_SET), ', requests: ', ssp_counts(COUNT_REQUEST, COUNT_TYPE_SET), ', requestFailures: ', ssp_counts(COUNT_REQUEST_FAILURE, COUNT_TYPE_SET), PHP_EOL;
}

function ssp_connect_handler ( $ClientId ) {
	ssp_counts(COUNT_CONN);

	$info = ssp_info($ClientId);
	$index = $sockfd = $host = $port = $tid = null;
	extract($info,EXTR_OVERWRITE|EXTR_REFS);
	$info = null;
	
	$username = UDB()->result(UDB()->select(array(
		'table' => 'members',
		'field' => 'username',
		'order' => 'uid',
		'limit' => ($index-1) . ',1'
	), SQL_SELECT_STRING), 0);
	
	if(!$username) return false;
	
	$sendKey = LIB('string')->rand(128, STRING_RAND_BOTH);
	
	ssp_var_put($index, SETUP_SENDKEY, $sendKey);
	ssp_var_put($index, SETUP_USERNAME, $username);
	ssp_var_put($index, SETUP_KEY, TYPE_KEY);
	
	$request = new XML_Element('request');
	$request->type = 'Connect.Key';
	$request->setText($sendKey);
	ssp_send($ClientId, (string) $request);
	$request = null;
}

function ssp_connect_denied_handler ( $ClientId ) {
}

function ssp_receive_handler ( $ClientId, $xml ) {
	$info = ssp_info($ClientId);
	$index = $sockfd = $host = $port = $tid = $type = null;
	extract($info,EXTR_OVERWRITE|EXTR_REFS);
	$info = null;
	
	$request = xml_to_object($xml, false, $error);
	if ( $request ) {
		if($request->type === 'Connect.Data') {
			$key = ssp_var_get($index, SETUP_RECEIVEKEY);
			if ( $key ) {
				$request = xml_to_object(crypt_decode($request->getText(), $key));
			} else {
				$request = false;
			}
			if ( empty($request) ) {
				ssp_close($ClientId);
				return;
			}
		}
		switch ( ssp_var_get($index, SETUP_KEY) ) {
			case TYPE_KEY:
				if($request->type === 'Connect.Key') {
					ssp_counts(COUNT_KEY);
					ssp_var_put($index, SETUP_RECEIVEKEY, $request->getText());

					$request=new XML_Element('request');
					$request->type='User.Login';
					$request->is_simple=true;
					$request->params=array_to_xml(array('username'=>ssp_var_get($index, SETUP_USERNAME),'password'=>'123456'),'params');
					
					ssp_var_put($index, SETUP_KEY, TYPE_LOGIN);
					
					return $request;
				} else {
					ssp_close($ClientId);
					return;
				}
			case TYPE_LOGIN:
				if($request->type === 'User.Login.Succeed') {
					ssp_counts(COUNT_LOGIN);
					ssp_var_put($index, SETUP_KEY, TYPE_REQUEST);
					$request=new XML_Element('request');
					$request->type='Gold.State';
					return $request;
				} else {
					echo $request->getText(), PHP_EOL;
					ssp_close($ClientId);
					return;
				}
				break;
			case TYPE_REQUEST:
				if($request->type === 'Gold.State.Succeed') {
					ssp_counts(COUNT_REQUEST);
					$s = ssp_var_get($index, SETUP_REQUESTS)+1;
					ssp_var_put($index, SETUP_REQUESTS, $s);
					if($s >= SSP_REQUESTS) {
						ssp_close($ClientId);
						return;
					} else {
						$request=new XML_Element('request');
						$request->type='Gold.State';
						return $request;
					}
				} elseif($request->type === 'Gold.State.Failed') {
					ssp_counts(COUNT_REQUEST_FAILURE);
					$s = ssp_var_get($index, SETUP_REQUESTS)+1;
					ssp_var_put($index, SETUP_REQUESTS, $s);
					if($s >= SSP_REQUESTS) {
						ssp_close($ClientId);
						return;
					} else {
						$request=new XML_Element('request');
						$request->type='Gold.State';
						return $request;
					}
				} else {
					echo $request->getText(), PHP_EOL;
					ssp_close($ClientId);
					return;
				}
				break;
			default:
				break;
		}
	}
	if ( is_array($error) && IS_DEBUG ) {
		echo PHP_EOL, PHP_EOL, 'XML_Parser_Error:';
		var_export($error);
		echo PHP_EOL;
		echo PHP_EOL;
	}
	$error = null;
	$data = null;
}

function ssp_send_handler ( $ClientId, $xml ) {
	if(!$xml) return;

	$index = ssp_info($ClientId, 'index');

	if ( !is_string($xml) ) {
		$key = ssp_var_get($index, SETUP_SENDKEY);
		$response = new XML_Element('response');
		$response->type = 'Connect.Data';
		$response->setText(crypt_encode((string) $xml, $key));
		
		return (string) $response;
	} else {
		return (string) $xml;
	}
}

function ssp_close_handler ( $ClientId ) {
	ssp_counts(COUNT_CONN, COUNT_TYPE_DEC);
	$info = ssp_info($ClientId);
	$index = $sockfd = $host = $port = $tid = null;
	extract($info,EXTR_OVERWRITE|EXTR_REFS);
	$info = null;
}

function ssp_clean_handler () {
}

function ssp_stop_handler () {
	DB()->close();
	ssp_delayed_destory();
	ssp_msg_queue_destory();
	ssp_var_destory();
}

