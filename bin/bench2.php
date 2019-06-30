<?php
require dirname(__FILE__) . DIRECTORY_SEPARATOR . 'core.php';

define('TYPE_KEY', 0);
define('TYPE_LOGIN', 1);
define('TYPE_REQUEST', 2);

define('COUNT_KEY', 0);
define('COUNT_LOGIN', 1);
define('COUNT_REQUEST', 2);
define('COUNT_REQUEST_FAILURE', 3);

import('lib.xml');
import('mod.uc.base');

function ssp_start_handler () {
}

function ssp_bench_handler () {
	$counts = ssp_counts();
	@list($keys, $logins, $requests, $requestFailures) = $counts;
	$conns = $counts['conns'];
	
	echo 'threads: ', SSP_NTHREADS, ', conns: ', $conns, ', keys: ', $keys, ', logins: ', $logins, ', requests: ', $requests, ', requestFailures: ', $requestFailures, PHP_EOL;
}

function ssp_connect_handler ( $ClientId ) {
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
	
	ssp_setup($ClientId, SETUP_SENDKEY, $sendKey);
	ssp_setup($ClientId, SETUP_USERNAME, $username);
	
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
			$key = ssp_setup($ClientId, SETUP_RECEIVEKEY);
			if ( $key ) {
				$request = xml_to_object(str_decode($request->getText(), $key));
			} else {
				$request = false;
			}
			if ( empty($request) ) {
				ssp_close($ClientId);
				return;
			}
		}
		switch ( $type ) {
			case TYPE_KEY:
				if($request->type === 'Connect.Key') {
					ssp_counts(COUNT_KEY);
					ssp_setup($ClientId, SETUP_RECEIVEKEY, $request->getText());

					$request=new XML_Element('request');
					$request->type='User.Login';
					$request->is_simple=true;
					$request->params=array_to_xml(array('username'=>ssp_setup($ClientId, SETUP_USERNAME),'password'=>'123456'),'params');
				
					$return = ( string ) $request;
					$request = $data = null;
					
					ssp_type($ClientId, TYPE_LOGIN);
					
					return $return;
				} else {
					ssp_close($ClientId);
					return;
				}
			case TYPE_LOGIN:
				if($request->type === 'User.Login.Succeed') {
					ssp_counts(COUNT_LOGIN);
					ssp_type($ClientId, TYPE_REQUEST);
					return '<request type="Gold.State" />';
				} else {
					echo $request->getText(), PHP_EOL;
					ssp_close($ClientId);
					return;
				}
				break;
			case TYPE_REQUEST:
				if($request->type === 'Gold.State.Succeed') {
					ssp_counts(COUNT_REQUEST);
					if(ssp_requests($ClientId) >= SSP_REQUESTS) {
						ssp_close($ClientId);
						return;
					} else {
						return '<request type="Gold.State" />';
					}
				} elseif($request->type === 'Gold.State.Failed') {
					ssp_counts(COUNT_REQUEST_FAILURE);
					if(ssp_requests($ClientId) >= SSP_REQUESTS) {
						ssp_close($ClientId);
						return;
					} else {
						return '<request type="Gold.State" />';
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
	$index = ssp_info($ClientId, 'index');
	$tagOpenString = substr($xml, 0, strpos($xml, '>'));

	if (  $tagOpenString !== false && strpos($tagOpenString, 'type="Connect.Key"') === false ) {
		$key = ssp_setup($ClientId, SETUP_SENDKEY);
		$response = new XML_Element('response');
		$response->type = 'Connect.Data';
		$response->setText(str_encode($xml, $key));
		
		$return = ( string ) $response;
		$xml = $response = null;
	} else {
		$return = ( string ) $xml;
		$xml = null;
	}
	return $return;
}

function ssp_close_handler ( $ClientId ) {
	$info = ssp_info($ClientId);
	$index = $sockfd = $host = $port = $tid = null;
	extract($info,EXTR_OVERWRITE|EXTR_REFS);
	$info = null;
}

function ssp_stop_handler () {
	DB()->close();
}

