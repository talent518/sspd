<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('ctl.base');
import('lib.xml');

Class CtlConsult extends CtlBase{
	function userTree($uid){
		$xml=new XML_Element('userTree');
		$ug=0;
		$xml->{$ug}=array_to_xml(array('gid'=>0,'name'=>'未读消息','remark'=>'未读消息','onlines'=>0,'counts'=>0),'group');
		$without_unread=true;
		foreach(MOD('user.serv')->get_list_by_uid($uid+0) as $id=>$r){
			if(!$xml->{$r['gid']}){
				$g=MOD('user.serv.group')->get($r['gid']);
				$g['onlines']=$g['counts']=0;
				$xml->{$r['gid']}=array_to_xml($g,'group');
			}
			$xml->{$r['gid']}->counts++;
			if($r['isonline']){
				$xml->{$r['gid']}->onlines++;
			}
			$r['avatar']=avatar($id,'small');
			if(empty($r['nickname'])){
				$r['nickname']=$r['username'];
			}
			$xml->{$r['gid']}->$id=array_to_xml($r,'user');
			if($r['unreads']){
				$xml->{$ug}->counts++;
				if($r['isonline']){
					$xml->{$ug}->onlines++;
				}
				$xml->{$ug}->$id=$xml->{$r['gid']}->$id;
				$without_unread=false;
			}
		}
		if($without_unread){
			$xml->{$ug}=null;
			unset($xml->{$ug});
		}
		return $xml;
	}
	function onState($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'index'),'uid');
		$response=new XML_Element('response');
		if(UGK($uid,'consult_reply')){
			$response->type='Consult.State.Succeed';
			$response->state='reply';
			$response->userTree=$this->userTree($uid);
		}elseif(UGK($uid,'consult_ask')){
			$response->type='Consult.State.Succeed';
			$response->state='ask';
			if(($suid=MOD('user.serv')->get($uid,'uid')) && ($u=MOD('user')->get_by_profile($suid))){
				$response->uid=$u['uid'];
				$response->nickname=($u['nickname']?$u['nickname']:$u['username']);
			}else{
				$response->uid=1;
				$response->nickname='管理员';
			}
		}else{
			$response->type='Consult.State.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		if($response->state){
			$response->sendKey=MOD('user.setting')->get($uid,'sendkey');
			if(empty($response->sendKey)){
				$response->sendKey='Click';
			}
		}
		return $response;
	}
	function onClients($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'index'),'uid');
		if(UGK($uid,'consult_reply')){
			$response=$this->userTree($uid);
			$response->type='Consult.Clients.Succeed';
		}else{
			$response=new XML_Element('response');
			$response->type='Consult.Clients.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		return $response;
	}
	function onOpen($request){
		$index=ssp_info($request->ClientId,'index');
		$uid=MOD('user.online')->get_by_client($index,'uid');

		$tuid=(string)($request->params->userId)+0;

		$isReply=$isAsk=false;
		if(UGK($uid,'consult_reply')){
			$isReply=true;
			$where="cuid={$tuid} AND uid={$uid}";
		}elseif(UGK($uid,'consult_ask')){
			$isAsk=true;
			$where="cuid={$uid} AND uid={$tuid}";
		}else{
			$response=new XML_Element('response');
			$response->type='Consult.Open.Failed';
			$response->setText(USER_NOPRIV_MSG);
			return $response;
		}
		if(!($serv=MOD('user.serv')->get_by_where($where))){
			$response=new XML_Element('response');
			$response->type='Consult.Open.Failed';
			$response->setText('您不在服务行列中……');
			return $response;
		}

		$response=new XML_Element('response');

		$messages=MOD('user.consult')->get_list_by_unread($uid,$tuid);
		foreach($messages as $ucid=>$r){
			$r['dateline']=udate('H:i:s',$r['dateline'],$uid);
			$message=$r['message'];
			$r['message']=null;unset($r['message']);
			$response->$ucid=array_to_xml($r,'message');
			$response->$ucid->message=xml_to_object($message);
			$response->$ucid->message->setTag('message');
		}
		MOD('user.consult')->update(array('isread'=>1),'to_uid='.$uid);

		if($isReply){
			$response->type='Consult.Open.Succeed';
			MOD('user.serv')->update(array('isopen'=>1,'unreads'=>0),'cuid='.((string)($request->params->userId)+0).' AND uid='.$uid);
		}elseif($isAsk){
			$response->type='Consult.Open.Succeed';
			MOD('user.online')->edit($index,array('consult'=>1));
		}
		return $response;
	}
	function onDate($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'index'),'uid');

		$tuid=(string)($request->params->userId)+0;

		$isReply=$isAsk=false;
		if(UGK($uid,'consult_reply')){
			$isReply=true;
			$where="cuid={$tuid} AND uid={$uid}";
		}elseif(UGK($uid,'consult_ask')){
			$isAsk=true;
			$where="cuid={$uid} AND uid={$tuid}";
		}else{
			$response=new XML_Element('response');
			$response->type='Consult.Date.Failed';
			$response->setText(USER_NOPRIV_MSG);
			return $response;
		}
		if(!($serv=MOD('user.serv')->get_by_where($where))){
			$response=new XML_Element('response');
			$response->type='Consult.Date.Failed';
			$response->setText('您不在服务行列中……');
			return $response;
		}

		$response=new XML_Element('response');
		$response->type='Consult.Date.Succeed';
		$messages=MOD('user.consult')->get_date_by_list($uid,$tuid);
		foreach($messages as $date=>$r){
			$r['dateday']=udate('Y-m-d',$r['dateday'],$uid);
			$response->$date=array_to_xml($r,'date');
		}
		return $response;
	}
	function onHistory($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'index'),'uid');

		$tuid=(string)($request->params->userId)+0;

		$isReply=$isAsk=false;
		if(UGK($uid,'consult_reply')){
			$isReply=true;
			$where="cuid={$tuid} AND uid={$uid}";
		}elseif(UGK($uid,'consult_ask')){
			$isAsk=true;
			$where="cuid={$uid} AND uid={$tuid}";
		}else{
			$response=new XML_Element('response');
			$response->type='Consult.History.Failed';
			$response->setText(USER_NOPRIV_MSG);
			return $response;
		}
		if(!($serv=MOD('user.serv')->get_by_where($where))){
			$response=new XML_Element('response');
			$response->type='Consult.History.Failed';
			$response->setText('您不在服务行列中……');
			return $response;
		}

		$date=(string)($request->params->date);
		if(empty($date) || !LIB('validate')->date($date)){
			$date='today';
		}
		$response=new XML_Element('response');
		$response->type='Consult.History.Succeed';
		$messages=MOD('user.consult')->get_list_by_date($uid,$tuid,strtotime($date));
		foreach($messages as $ucid=>$r){
			$r['dateline']=udate('H:i:s',$r['dateline'],$uid);
			$message=$r['message'];
			$r['message']=null;unset($r['message']);
			$response->$ucid=array_to_xml($r,'message');
			$response->$ucid->message=xml_to_object($message);
			$response->$ucid->message->setTag('message');
		}
		return $response;
	}
	function onSend($request){
		$index=ssp_info($request->ClientId,'index');

		$uid=MOD('user.online')->get_by_client($index,'uid');
		$profile=MOD('user')->get_by_profile($uid);

		$tuid=(string)($request->params->userId)+0;

		$isReply=$isAsk=false;
		if(UGK($uid,'consult_reply')){
			$isReply=true;
			$where="cuid={$tuid} AND uid={$uid}";
		}elseif(UGK($uid,'consult_ask')){
			$isAsk=true;
			$where="cuid={$uid} AND uid={$tuid}";
		}else{
			$response=new XML_Element('response');
			$response->type='Consult.Send.Failed';
			$response->setText(USER_NOPRIV_MSG);
			return $response;
		}
		if(!($serv=MOD('user.serv')->get_by_where($where))){
			$response=new XML_Element('response');
			$response->type='Consult.Send.Failed';
			$response->setText('您不在服务行列中……');
			return $response;
		}

		$data=array(
			'from_uid'=>$uid,
			'to_uid'=>$tuid,
			'message'=>object_to_xml($request->params->rtf),
			'dateline'=>time(),
			'dateday'=>@strtotime('today'),
		);
		$response=new XML_Element('response');
		if(MOD('user.consult')->add($data)){
			$ucid=DB()->insert_id();
			$isOpen=false;
			$isRecv=false;
			if($online=MOD('user.online')->get_by_user($tuid)){
				if($isAsk && $serv['isopen']){
					$isOpen=true;
				}
				if($isReply && $online['consult']){
					$isOpen=true;
				}
				if($isOpen){
					$sendXML=new XML_Element('response');
					$sendXML->type='Consult.Receive.'.$uid;
					$data['nickname']=($profile['nickname']?$profile['nickname']:$profile['username']);
					$data['ucid']=$ucid;
					$message=$data['message'];
					$data['message']=null;unset($data['message']);
					$sendXML->message=array_to_xml($data,'message');
					$sendXML->message->message=xml_to_object($message);
					$sendXML->message->message->setTag('message');
					$sendXML->message->dateline=udate('H:i:s',$data['dateline'],$tuid);
					if($this->send($online['id'],(string)$sendXML)){
						$isRecv=true;
						$msg='对方收到已消息！';
						MOD('user.consult')->update(array('isread'=>1),$ucid+0);
					}else{
						$msg='发送消息到对方失败！';
					}
				}else{
					$remind=new XML_Element('response');
					$remind->type='Remind.Consult';
					if($this->send($online['id'],(string)$remind)){
						$msg='对方收到提醒！';
					}else{
						$msg='发送提醒失败！';
					}
				}
			}
			if(!$isRecv && $isAsk){
				MOD('user.serv')->update(array('unreads'=>'`unreads`+1'),$uid+0,false);
			}
			$response->type='Consult.Send.Succeed';
			$response->setText('发送成功！'.$msg);
		}elseif($error=MOD('user.consult')->error){
			$response->type='Consult.Send.Failed';
			$response->setText($error);
		}else{
			$response->type='Consult.Send.Failed';
			$response->setText('发送失败！');
		}
		return $response;
	}
	function onClose($request){
		$index=ssp_info($request->ClientId,'index');
		$uid=MOD('user.online')->get_by_client($index,'uid');

		$response=new XML_Element('response');
		if(UGK($uid,'consult_reply')){
			$response->type='Consult.Close.Succeed';
			MOD('user.serv')->update(array('isopen'=>0),'cuid='.((string)($request->params->userId)+0).' AND uid='.$uid);
		}elseif(UGK($uid,'consult_ask')){
			$response->type='Consult.Close.Succeed';
			MOD('user.online')->edit($index,array('consult'=>0));
		}else{
			$response->type='Consult.Close.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		return $response;
	}
}
