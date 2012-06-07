<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('ctl.base');
import('lib.xml');

Class CtlBroadcast extends CtlBase{
	function onState($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');
		$response=new XML_Element('response');
		if(UGK($uid,'broadcast_add')){
			$response->type='Broadcast.State.Succeed';
			$response->state='add';
		}elseif(UGK($uid,'broadcast')){
			$response->type='Broadcast.State.Succeed';
			$response->state='normal';
		}else{
			$response->type='Broadcast.State.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		return $response;
	}
	function onOpen($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');

		$response=new XML_Element('response');
		if(UGK($uid,'broadcast')){
			$response->type='Broadcast.Open.Succeed';
			MOD('user.online')->edit($sockfd,array('broadcast'=>1));
		}else{
			$response->type='Broadcast.Open.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		return $response;
	}
	function onDate($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');

		if(!UGK($uid,'broadcast')){
			$response=new XML_Element('response');
			$response->type='Broadcast.Date.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}

		$response=new XML_Element('response');
		$response->type='Broadcast.Date.Succeed';
		$messages=MOD('broadcast')->get_date_by_list();
		foreach($messages as $date=>$r){
			$r['dateday']=udate('Y-m-d',$r['dateday'],$uid);
			$response->$date=array_to_xml($r,'date');
		}
		return $response;
	}
	function onHistory($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');

		if(!UGK($uid,'broadcast')){
			$response=new XML_Element('response');
			$response->type='Broadcast.History.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}

		$date=(string)($request->params->date);
		if(empty($date) || !LIB('validate')->date($date)){
			$date='today';
		}
		$response=new XML_Element('response');
		$response->type='Broadcast.History.Succeed';
		$messages=MOD('broadcast')->get_list_by_date(strtotime($date));
		foreach($messages as $bid=>$r){
			$r['dateline']=udate('H:i:s',$r['dateline'],$uid);
			$message=$r['message'];
			unset($r['message']);
			$response->$bid=array_to_xml($r,'message');
			$response->$bid->message=xml_to_object($message);
			$response->$bid->message->setTag('message');
		}
		return $response;
	}
	function onSend($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');

		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		$profile=MOD('user')->get_by_profile($uid);

		if(!UGK($uid,'broadcast_add')){
			$response=new XML_Element('response');
			$response->type='Broadcast.Send.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}

		$data=array(
			'uid'=>$uid,
			'message'=>(string)($request->params->rtf),
			'dateline'=>time(),
			'dateday'=>@strtotime('today'),
		);
		$response=new XML_Element('response');
		if(MOD('broadcast')->add($data)){
			$count=0;
			$sendXML=new XML_Element('response');
			$sendXML->type='Broadcast.Receive';
			$data['nickname']=($profile['nickname']?$profile['nickname']:$profile['username']);
			$message=$data['message'];
			unset($data['message']);
			$sendXML->message=array_to_xml($data,'message');
			$sendXML->message->message=xml_to_object($message);
			$sendXML->message->message->setTag('message');
			$list=MOD('user.online')->get_list_by_where('uid>0 && broadcast>0');
			foreach($list as $onid=>$r){
				$sendXML->message->dateline=udate('H:i:s',$data['dateline'],$r['uid']);
				if($onid!=$sockfd && $this->send($onid,$sendXML)){
					$count++;
				}
			}
			$response->type='Broadcast.Send.Succeed';
			$response->setText('发送成功！'.($count?$count.'人收到！':''));
		}elseif($error=MOD('broadcast')->error){
			$response->type='Broadcast.Send.Failed';
			$response->setText($error);
		}else{
			$response->type='Broadcast.Send.Failed';
			$response->setText('发送失败！');
		}
		$response->dateline=$message['dateline'];
		return $response;
	}
	function onClose($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');

		$response=new XML_Element('response');
		if(UGK($uid,'broadcast')){
			$response->type='Broadcast.Close.Succeed';
			MOD('user.online')->edit($sockfd,array('broadcast'=>0));
		}else{
			$response->type='Broadcast.Close.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		return $response;
	}
}
