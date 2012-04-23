<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

Class CtlBase{
	function checklogin($auth){
		$auth=explode("\t",str_decode($auth));
		list($uid,$password)=(count($auth)<2?array(0,''):$auth);
	}
	function send($i,$request){
		$res=is_resource($i)?$i:ssp_resource($i,false);
		return is_resource($res)?ssp_send($res,(string)$request):0;
	}
	function close($i){
		$res=is_resource($i)?$i:ssp_resource($i,false);
		return is_resource($res)?ssp_close($res):0;
	}
}
