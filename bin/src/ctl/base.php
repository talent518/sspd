<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

Class CtlBase{
	function checklogin($auth){
		$auth=explode("\t",str_decode($auth));
		list($uid,$password)=(count($auth)<2?array(0,''):$auth);
	}
	function send($i,$request){
		$res=is_resource($i)?$i:ssp_resource($i,SSP_RES_INDEX);

		if(is_resource($res)){
			return ssp_send($res,(string)$request);
		}else{
			server_log('Send "'.$request.'" To ('.$i.') error!');
			return false;
		}
	}
	function close($i){
		$res=is_resource($i)?$i:ssp_resource($i,SSP_RES_INDEX);

		return is_resource($res)?ssp_close($res):0;
	}
}
