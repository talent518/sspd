<?PHP
require dirname(__FILE__) . DIRECTORY_SEPARATOR . 'core.php';
require SRC_DIR . 'function_server.php';

import('lib.xml');

function ssp_start_handler () {
	server_log('Server started at ' . date('m-d H:i:s', time()) . PHP_EOL . 'Listening on port ' . SSP_PORT);
	MOD('user.online')->clean();
}

function ssp_connect_handler ( $ClientId ) {
	$info = ssp_info($ClientId);
	$index = $sockfd = $host = $port = $tid = null;
	extract($info,EXTR_OVERWRITE|EXTR_REFS);
	$info = null;

	server_log('New connection ( ' . $index . ' ) from ' . $host . ' on port ' . $port . '. Time at ' . date('m-d H:i:s', time()));

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
	server_log('Too many connections.');
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

	MOD('user.online')->update(array(
		'microtime' => microtime(true),
		'recvs' => 'recvs+1', 
		'recv_bytes' => 'recv_bytes+' . strlen($xml)
	), $index, false);
	
	$request = xml_to_object($xml, false, $error);
	if ( $request ) {
		switch ( $request->type ) {
			case 'Connect.Key':
				$sendKey = LIB('string')->rand(128, STRING_RAND_BOTH);
				$data = array(
					'sendKey' => $sendKey, 
					'receiveKey' => $request->getText()
				);
				MOD('user.online')->edit($index,$data);
				
				data_log('Received: "' . trim($request) . '" from ' . $index);
				
				$response = new XML_Element('response');
				$response->type = 'Connect.Key';
				$response->setText($data['sendKey']);
				
				$return = ( string ) $response;
				$request = $response = $data = null;
				return $return;
			case 'Connect.Data':
				$key = MOD('user.online')->get_by_client($index, 'receiveKey');
				if ( $key ) {
					$request = xml_to_object(str_decode($request->getText(), $key));
				}
				if ( empty($request) ) {
					return;
				}
				break;
			case 'Connect.Ping':
				data_log('Received: "' . trim($request) . '" from ' . $index);
				return '<response type="Connect.Ping"/>';
				break;
			default:
				break;
		}
		
		data_log('Received: "' . trim($request) . '" from ' . $index);
		
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
				$return = ( string ) $response;
				$request = $response = null;
				return $return;
			}
		}
		$ctl = explode('.', $type);
		if ( count($ctl) > 1 ) {
			$mod = 'on' . ucfirst(array_pop($ctl));
			$ctl = implode('.', $ctl);
			$ctl_obj = CTL($ctl);
			if ( $ctl_obj === false ) {
				server_log("Client $index controller \"$ctl\" not exists!");
			} elseif ( method_exists($ctl_obj, $mod) ) {
				$response = $ctl_obj->$mod($request);
				if ( $response instanceof XML_Element || substr($response, 0, 1) == '<' ) {
					$return = ( string ) $response;
					$request = $response = null;
					return $return;
				}
			} else {
				server_log("Client $index controller \"$ctl\" for method \"$mod\" not exists!");
			}
		} else {
			server_log("Client $index type \"$type\" error!");
		}
		$request = null;
	}
	if ( is_array($error) ) {
		server_log('XML_Parser_Error:' . var_export($error, TRUE));
		$error = null;
	}
	$data = null;
}

function ssp_send_handler ( $ClientId, $xml ) {
	$index = ssp_info($ClientId, 'index');
	data_log('Sending: "' . $xml . '" to: ' . $index);

	$tagOpenString = substr($xml, 0, strpos($xml, '>'));

	if (  $tagOpenString !== false && strpos($tagOpenString, 'type="Connect.Key"') === false && strpos($tagOpenString, 'type="Connect.Ping"') === false ) {
		$key = MOD('user.online')->get_by_client($index, 'sendKey');
		$response = new XML_Element('response');
		$response->type = 'Connect.Data';
		$response->setText(str_encode($xml, $key));
		
		$return = ( string ) $response;
		$xml = $response = null;
	} else {
		$return = ( string ) $xml;
		$xml = null;
	}
	MOD('user.online')->update(array(
		'microtime' => microtime(true),
		'sends' => 'sends+1', 
		'send_bytes' => 'send_bytes+' . strlen($return)
	), $index, false);
	return $return;
}

function ssp_close_handler ( $ClientId ) {
	$info = ssp_info($ClientId);
	$index = $sockfd = $host = $port = $tid = null;
	extract($info,EXTR_OVERWRITE|EXTR_REFS);
	$info = null;
	server_log('Close connection ( ' . $index . ' ) from ' . $host . ' on port ' . $port . '. Time at ' . date('m-d H:i:s', MOD('user.online')->get_by_client($index, 'time')));
	CTL('user')->logout($ClientId);
	MOD('user.online')->drop($index);
}

function ssp_stop_handler () {
	server_log('Server Stoped at ' . date('H:i:s', time()));
	MOD('user.online')->clean();
	DB()->close();
}
