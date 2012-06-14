<?PHP
require dirname(__FILE__).DIRECTORY_SEPARATOR.'core.php';
require SRC_DIR.'function_server.php';

import('lib.xml');

$users=$clients=array();

if(defined('SSP_USER')){
	ssp_setopt(SSP_OPT_HOST,SSP_USER);
}
ssp_setopt(SSP_OPT_PIDFILE,ROOT.'log'.DIR_SEP.'ssp.pid');
if(defined('SSP_HOST')){
	ssp_setopt(SSP_OPT_HOST,SSP_HOST);
}
if(defined('SSP_PORT')){
	ssp_setopt(SSP_OPT_PORT,SSP_PORT);
}
if(defined('SSP_MAX_CLIENTS')){
	ssp_setopt(SSP_OPT_MAX_CLIENTS,SSP_MAX_CLIENTS);
}
if(defined('SSP_MAX_RECVS')){
	ssp_setopt(SSP_OPT_MAX_RECVS,SSP_MAX_RECVS);
}

ssp_bind(SSP_START,'ssp_start_handler');
ssp_bind(SSP_RECEIVE,'ssp_receive_handler');
ssp_bind(SSP_SEND,'ssp_send_handler');
ssp_bind(SSP_CONNECT,'ssp_connect_handler');
ssp_bind(SSP_CONNECT_DENIED,'ssp_connect_denied_handler');
ssp_bind(SSP_CLOSE,'ssp_close_handler');
ssp_bind(SSP_STOP,'ssp_stop_handler');

function ssp_start_handler(){
	server_log('Server started at '.date('m-d H:i:s',time()).PHP_EOL.'Listening on port '.SSP_PORT);
	MOD('user.online')->clean();
}

function ssp_connect_handler($ClientId){
	$info=ssp_info($ClientId);
	extract($info);
	$data=array(
		'onid'=>$sockfd,
		'host'=>$host,
		'port'=>$port,
		'time'=>time(),
		'sendKey'=>LIB('string')->rand(128,STRING_RAND_BOTH),
	);
	MOD('user.online')->add($data);
	server_log( 'New connection ( '.$sockfd.' ) from '.$host.' on port '.$port.'. Time at '.date('m-d H:i:s',$data['time']));
	$response=new XML_Element('response');
	$response->type='Connect.Key';
	$response->setText($data['sendKey']);
	ssp_send($ClientId,(string)$response);
}

function ssp_connect_denied_handler($ClientId){
	server_log('Too many connections.');
	$response=new XML_Element('response');
	$response->type='Connect.Denied';
	$response->setText('服务器连接正在排对中……');
	ssp_send($ClientId,(string)$response);
}

function ssp_receive_handler($ClientId,$data){
	$info=ssp_info($ClientId);
	extract($info);
	if($request=xml_to_object($data,false,$error)){
		switch($request->type){
			case 'Connect.Key':
				MOD('user.online')->edit($sockfd,array('receiveKey'=>$request->getText()));
				data_log( 'Received: "'.trim( $request ).'" from '.$sockfd );
				return;
			case 'Connect.Data':
				$key=MOD('user.online')->get_by_client($sockfd,'receiveKey');
				$request=xml_to_object(str_decode($request->getText(),$key));
				if(empty($request)){
					return;
				}
				break;
			default:
				break;
		}

		data_log( 'Received: "'.trim( $request ).'" from '.$sockfd );

		$request->ClientId=$ClientId;
		$type=preg_replace("/^\.?([a-z\.]+)\.?$/i","\\1",$request->type);
		$authTypes=array('user.login','user.register','user.lostpasswd');
		if(!in_array(strtolower($type),$authTypes) && !MOD('user.online')->get_by_client($sockfd,'uid')){
			$response=CTL('user')->login($ClientId);
			if($response instanceof XML_Element || substr($response,0,1)=='<'){
				$request=null;
				return (string)$response;
			}
		}
		$ctl=explode('.',$type);
		if(count($ctl)>1){
			$mod='on'.ucfirst(array_pop($ctl));
			$ctl=implode('.',$ctl);
			$ctl_obj=CTL($ctl);
			if($ctl_obj===false){
				server_log("Client $sockfd controller \"$ctl\" not exists!");
			}elseif(method_exists($ctl_obj,$mod)){
				$response=$ctl_obj->$mod($request);
				if($response instanceof XML_Element || substr($response,0,1)=='<'){
					$request=null;
					return (string)$response;
				}
			}else{
				server_log("Client $sockfd controller \"$ctl\" for method \"$mod\" not exists!");
			}
		}else{
			server_log("Client $sockfd type \"$type\" error!");
		}
		$request=null;
	}
	if(is_array($error)){
		server_log('XML_Parser_Error:'.var_export($error,TRUE));
		$error=null;
	}
	$data=null;
}

function ssp_send_handler($ClientId,$data){
	$info=ssp_info($ClientId);
	extract($info);
	data_log('Sending: "'.$data.'" to: '.$sockfd);
	$xml=xml_to_object($data);
	if($xml->type!='Connect.Key'){
		$key=MOD('user.online')->get_by_client($sockfd,'sendKey');
		$return=new XML_Element('response');
		$return->type='Connect.Data';
		$return->setText(str_encode($data,$key));
	}else{
		$return=$data;
	}
	$xml=null;
	return (string)$return;
}

function ssp_close_handler($ClientId){
	$info=ssp_info($ClientId);
	extract($info);
	server_log( 'Close connection ( '.$sockfd.' ) from '.$host.' on port '.$port.'. Time at '.date('m-d H:i:s',MOD('user.online')->get_by_client($sockfd,'time')));
	MOD('user.online')->drop($sockfd);
}

function ssp_stop_handler(){
	server_log('Server Stoped at '.date('H:i:s',time()));
}
