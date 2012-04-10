<?PHP
require 'core.php';
require SRC_DIR.'function_server.php';

import('lib.xml');

$users=$clients=array();

ssp_bind(SSP_START,'socket_server_start');
ssp_bind(SSP_RECEIVE,'socket_server_receive');
ssp_bind(SSP_SEND,'socket_server_send');
ssp_bind(SSP_CONNECT,'socket_server_connect');
ssp_bind(SSP_CONNECT_DENIED,'socket_server_connect_denied');
ssp_bind(SSP_CLOSE,'socket_server_close');
ssp_bind(SSP_STOP,'socket_server_stop');

function socket_server_start(){
	echo 'Started server.',PHP_EOL;
	server_log('Server started at '.date('m-d H:i:s',time()).PHP_EOL.'Listening on port '.SSP_PORT);
	MOD('user.online')->clean();
}

function socket_server_connect($ClientId,$ClientHost,$ClientPort){
	$data=array(
		'host'=>$ClientHost,
		'port'=>$ClientPort,
		'time'=>time(),
		'sendKey'=>LIB('string')->rand(128,STRING_RAND_BOTH),
	);
	MOD('user.online')->add($data);
	server_log( 'New connection ( '.$ClientId.' ) from '.$ClientHost.' on port '.$ClientPort.'. Time at '.date('m-d H:i:s',$data['time']));
	$response=new XML_Element('response');
	$response->type='Connect.Key';
	$response->setText($data['sendKey']);
	ssp_send($ClientId,$response);
}

function socket_server_connect_denied($ClientId){
	server_log('Too many connections.');
	$response=new XML_Element('response');
	$response->type='Connect.Denied';
	$response->setText('服务器连接太多，请等待再试！');
	ssp_send($ClientId,$response);
}

function socket_server_receive($ClientId,$data){
	$requests=xml_to_object($data,true,$error);
	if(is_array($requests)){
		foreach($requests as $request){
			switch($request->type){
				case 'Connect.Key':
					MOD('user.online')->edit($ClientId,array('receiveKey'=>$request->getText()));
					data_log( 'Received: "'.trim( $request ).'" from '.$ClientId );
					continue 2;
					break;
				case 'Connect.Data':
					$key=MOD('user.online')->get_by_client($ClientId,'receiveKey');
					$request=xml_to_object(str_decode($request->getText(),$key));
					break;
				default:
					break;
			}

			data_log( 'Received: "'.trim( $request ).'" from '.$ClientId );

			$request->ClientId=$ClientId;
			$type=preg_replace("/^\.?([a-z\.]+)\.?$/i","\\1",$request->type);
			$authTypes=array('user.login','user.register','user.lostpasswd');
			if(!in_array(strtolower($type),$authTypes) && !MOD('user.online')->get_by_client($ClientId,'uid')){
				$response=CTL('user')->login($ClientId);
				ssp_send($ClientId,$response);
				$response=null;
				return;
			}
			$ctl=explode('.',$type);
			if(count($ctl)>1){
				$mod='on'.ucfirst(array_pop($ctl));
				$ctl=implode('.',$ctl);
				$ctl_obj=CTL($ctl);
				if($ctl_obj===false){
					server_log("Client $ClientId controller \"$ctl\" not exists!");
				}elseif(method_exists($ctl_obj,$mod)){
					$response=$ctl_obj->$mod($request);
					if(substr($response,0,1)=='<')
						ssp_send($ClientId,$response);
					$response=null;
				}else{
					server_log("Client $ClientId controller \"$ctl\" for method \"$mod\" not exists!");
				}
			}else{
				server_log("Client $ClientId type \"$type\" error!");
			}
			$request=null;
		}
	}
	if(is_array($error)){
		data_server('XML_Parser_Error:'.var_export($error,TRUE));
	}
	$data=$requests=$error=null;
}

function socket_server_send($ClientId,$data){
	data_log('Sending: "'.$data.'" to: '.$ClientId);
	if($data->type!='Connect.Key'){
		$key=MOD('user.online')->get_by_client($ClientId,'sendKey');
		$return=new XML_Element('response');
		$return->type='Connect.Data';
		$return->setText(str_encode((string)$data,$key));
	}else{
		$return=$data;
	}
	return $return;
}

function socket_server_close($ClientId){
	server_log( 'Close connection ( '.$ClientId.' ) from '.MOD('user.online')->get_by_client($ClientId,'host').' on port '.MOD('user.online')->get_by_client($ClientId,'port').'. Time at '.date('m-d H:i:s',MOD('user.online')->get_by_client($ClientId,'time')));
	$request=new XML_Element('request');
	$request->ClientId=$ClientId;
	CTL('user')->onLogout($request);
}

function socket_server_stop(){
	echo 'Stoped server.',PHP_EOL;
	server_log('Server Stoped at '.date('H:i:s',time()));
}
