<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

Class CtlBase{
	function checklogin($auth){
		$auth=explode("\t",str_decode($auth));
		list($uid,$password)=(count($auth)<2?array(0,''):$auth);
	}
	function send($i,$request){
		return ssp_send(is_resource($i)?$i:ssp_resource($i,false),(string)$request);
	}
	function close($i){
		return ssp_close($i);
	}
}
