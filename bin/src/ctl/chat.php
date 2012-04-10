<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('ctl.base');
import('lib.xml');

Class CtlChat extends CtlBase{
	function onSend($request){
		$params=&$request->params;

		$fromUid=MOD('user.online')->get_by_client($request->ClientId,'uid');
		$fromUser=MOD('user')->get_by_profile($fromUid);

		$toUid=(string)$params->uid+0;
		$toUser=MOD('user.online')->get_by_user($toUid);
		$message=array(
			'from_uid'=>$fromUid,
			'to_uid'=>$toUid,
			'message'=>(string)$request->message,
			'dateline'=>time(),
		);
		MOD('chat')->add($message);
		$response=new XML_Element('response');
		if($toUser){
			$sendXML=new XML_Element('response');
			$sendXML->type='Remind.Chat';
			$sendXML->params=array_to_xml(array('uid'=>$fromUid,'nickname'=>empty($fromUser['nickname'])?$fromUser['username']:$fromUser['nickname'],'avatar'=>avatar($fromUid,'big')),'params');
			if($this->send($toUser['onid'],$sendXML)){
				$response->type='Chat.Send.Succeed';
				$response->setText('发送成功！');
			}else{
				$response->type='Chat.Send.Failed';
				$response->setText('发送失败！');
			}
		}else{
			$response->type='Chat.Send.Offlined';
			$response->setText('发送离线消息成功！');
		}
		$response->dateline=$message['dateline'];
		return $response;
	}
	function onReceive($request){
		$response=new XML_Element('response');
		return $response;
	}
}
