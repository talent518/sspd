<?php
require dirname(__FILE__) . DIRECTORY_SEPARATOR . 'core.php';

import('lib.xml');

function ssp_delay_close($delay, $persist, $id) {
	ssp_close($id);
}

function ssp_monitor_handler(array $scpu, array $pcpu, array $smem, array $pmem, int $threads, int $etime, array $args) {
}

function ssp_start_handler () {
	ssp_delayed_init();
	if(IS_DEBUG) {
		echo PHP_EOL,'Server started at ', date('m-d H:i:s', time()), PHP_EOL, 'Listening on port ', SSP_PORT, PHP_EOL;
	}
	MOD('user.online')->clean();
}

function ssp_connect_handler ( $ClientId ) {
	$info = ssp_info($ClientId);
	$index = $sockfd = $host = $port = $tid = null;
	extract($info,EXTR_OVERWRITE|EXTR_REFS);
	$info = null;

	if(IS_DEBUG) {
		echo PHP_EOL, 'New connection ( ', $index, ' ) from ', $host, ' on port ', $port, '. Time at ', date('m-d H:i:s', time()), PHP_EOL;
	}

	$data = array(
		'id' => $index, 
		'sockfd' => $sockfd, 
		'host' => $host, 
		'port' => $port, 
		'tid' => $tid, 
		'time' => time()
	);
	MOD('user.online')->add($data);

	// return $index % 2 === 1; // 禁止index为偶数的连接
}

function ssp_connect_denied_handler ( $ClientId ) {
	if(IS_DEBUG) {
		echo PHP_EOL, 'Too many connections.', PHP_EOL;
	}

	$response = new XML_Element('response');
	$response->type = 'Connect.Denied';
	$response->setText('服务器连接正在排对中……');
	ssp_send($ClientId, ( string ) $response);
}

function ssp_receive_handler ( $ClientId, $xml ) {
	$info = ssp_info($ClientId);
	$index = $sockfd = $host = $port = $tid = null;
	extract($info,EXTR_OVERWRITE|EXTR_REFS);
	$info = null;

	/*MOD('user.online')->update(array(
		'microtime' => microtime(true),
		'recvs' => 'recvs+1',
		'recv_bytes' => 'recv_bytes+' . strlen($xml)
	), $index, false);*/
	
	$request = xml_to_object($xml, false, $error);
	if ( $request ) {
		switch ( $request->type ) {
			case 'Connect.Key':
				$sendKey = LIB('string')->rand(128, STRING_RAND_BOTH);
				MOD('user.online')->edit($index, [
					'sendKey' => $sendKey, 
					'receiveKey' => $request->getText()
				]);
				
				if(IS_DEBUG) {
					echo PHP_EOL, 'Received: "', trim($request), '" from ', $index, PHP_EOL;
				}
				
				$request = null;
				return '<response type="Connect.Key">' . $sendKey . '</response>';
			case 'Connect.Data':
				$key = MOD('user.online')->get_by_client($index, 'receiveKey');
				if ( $key ) {
					$request = xml_to_object(crypt_decode($request->getText(), $key));
				}
				if ( empty($request) ) {
					return;
				}
				break;
			case 'Connect.Ping':
				if(IS_DEBUG) {
					echo PHP_EOL, 'Received: "', trim($request), '" from ', $index, PHP_EOL;
				}
				return '<response type="Connect.Ping"/>';
				break;
			default:
				break;
		}
		
		if(IS_DEBUG) {
			echo PHP_EOL, 'Received: "', trim($request), '" from ', $index, PHP_EOL;
		}
		
		$request->ClientId = $ClientId;
		$type = trim($request->type, '.');
		$authTypes = array(
			'user.login', 
			'user.register', 
			'user.lostpasswd'
		);
		if (  ! in_array(strtolower($type), $authTypes) &&  ! MOD('user.online')->get_by_client($index, 'uid') ) {
			$response = CTL('user')->login($ClientId);
			if ( $response instanceof XML_Element || substr($response, 0, 1) == '<' ) {
				$request = null;
				return $response;
			}
		}
		$ctl = explode('.', $type);
		if ( count($ctl) > 1 ) {
			$mod = 'on' . ucfirst(array_pop($ctl));
			$ctl = implode('.', $ctl);
			$ctl_obj = CTL($ctl);
			if ( $ctl_obj === false ) {
				if(IS_DEBUG) {
					echo PHP_EOL, 'Client ', $index, ' controller "', $ctl, '" not exists!', PHP_EOL;
				}
			} elseif ( method_exists($ctl_obj, $mod) ) {
				$response = $ctl_obj->$mod($request);
				if ( $response instanceof XML_Element || substr($response, 0, 1) == '<' ) {
					$request = null;
					return $response;
				}
			} else {
				if(IS_DEBUG) {
					echo PHP_EOL, 'Client ', $index, ' controller "', $ctl, '" for method "', $mod, '" not exists!', PHP_EOL;
				}
			}
		} elseif(IS_DEBUG) {
			echo PHP_EOL, 'Client ', $index, ' type "', $type, '" error!', PHP_EOL;
		}
		$request = null;
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
	if(IS_DEBUG) {
		echo PHP_EOL, 'Sending: "', (string) $xml, '" to: ', $index, PHP_EOL;
	}

	if(!is_string($xml)) {
		$key = MOD('user.online')->get_by_client($index, 'sendKey');
		$response = new XML_Element('response');
		$response->type = 'Connect.Data';
		$response->setText(crypt_encode((string) $xml, $key));
		
		$return = (string) $response;
	} else {
		$return = (string) $xml;
	}
	/*MOD('user.online')->update(array(
		'microtime' => microtime(true),
		'sends' => 'sends+1',
		'send_bytes' => 'send_bytes+' . strlen($return)
	), $index, false);*/
	return $return;
}

function ssp_close_handler ( $ClientId ) {
	$info = ssp_info($ClientId);
	$index = $sockfd = $host = $port = $tid = null;
	extract($info,EXTR_OVERWRITE|EXTR_REFS);
	$info = null;
	if(IS_DEBUG) {
		echo PHP_EOL, 'Close connection ( ', $index, ' ) from ', $host, ' on port ', $port, '. Time at ', date('m-d H:i:s', MOD('user.online')->get_by_client($index, 'time')), PHP_EOL;
	}
	CTL('user')->logout($ClientId);
	MOD('user.online')->drop($index);
}

function ssp_clean_handler () {
}

function ssp_stop_handler () {
	if(IS_DEBUG) {
		echo PHP_EOL, 'Server Stoped at ', date('H:i:s', time()), PHP_EOL;
	}
	MOD('user.online')->clean();
	DB()->close();
	ssp_delayed_destory();
}
