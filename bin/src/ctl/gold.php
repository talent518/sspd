<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('ctl.base');
import('lib.xml');

Class CtlGold extends CtlBase{
	function onState($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');
		$response=new XML_Element('response');
		if(UGK($uid,'gold_add')){
			$response->type='Gold.State.Succeed';
			$response->state='add';
			MOD('user.setting')->set($uid,'gold',1);
		}elseif(UGK($uid,'gold')){
			$response->type='Gold.State.Succeed';
			if(MOD('user.setting')->get($uid,'gold')){
				$response->state='view';
			}else{
				$response->state='normal';
			}
		}else{
			$response->type='Gold.State.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		return $response;
	}
	function onAgree($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');
		$response=new XML_Element('response');

		if(UGK($uid,'gold')){
			MOD('user.setting')->set($uid,'gold',1);
			$response->type='Gold.Agree.Succeed';
			$response->state='view';
		}else{
			$response->type='Gold.Agree.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		return $response;
	}
	function onList($request,$page=1,$size=10,$isToday=0){
		$xml=new XML_Element('response');
		$xml->type='Gold.List';
		if(is_object($request)){
			$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');
			$page=(string)($request->params->page)+0;
			$size=(string)($request->params->size)+0;
			$isToday=(string)($request->params->isToday)+0;
		}else{
			$uid=$request+0;
		}
		if(!UGK($uid,'gold')){
			$xml->setText(USER_NOPRIV_MSG);
			return $xml;
		}
		$xml->counts=MOD('gold')->count($where);
		$limit=get_limit($page,$size,$xml->counts);
		$goldList=MOD('gold')->get_list_by_user($uid,$isToday!=0,$limit);
		foreach($goldList as $r){
			$r['code']=substr('000000'.$r['code'],-6);
			$r['dateline']=udate('m-d H:i',$r['dateline'],$uid);
			$xml->$r['gid']=array_to_xml($r,'gold');
		}
		return $xml;
	}
	function onAdd($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		if(!UGK($uid,'gold_add')){
			$response=new XML_Element('response');
			$response->type='Gold.Add.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$params=&$request->params;
		$data=array(
			'uid'=>$uid,
			'title'=>(string)($params->title),
			'code'=>(string)($params->code),
			'name'=>(string)($params->name),
			'reason'=>(string)($params->reason),
			'prompt'=>(string)($params->prompt),
			'buy_condition'=>(string)($params->buy_condition),
			'sell_condition'=>(string)($params->sell_condition),
			'dateline'=>time(),
		);
		$response=new XML_Element('response');
		if(MOD('gold')->add($data)){
			$response->type='Gold.Add.Succeed';
			$response->setText('提交成功！');

			$data['gid']=DB()->insert_id();

			$remind=new XML_Element('response');
			$remind->type='Remind.Gold';
			foreach(MOD('user.online')->get_list_by_where('uid>0') as $sf=>$r){
				if(UGK($r['uid'],'gold')){
					$this->send($sf,$remind);
				}
			}
		}elseif($error=MOD('gold')->error){
			$response->type='Gold.Add.Failed';
			$response->setText($error);
		}else{
			$response->type='Gold.Add.Failed';
			$response->setText('未知错误!');
		}
		return $response;
	}
	function onView($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		if(!UGK($uid,'gold_add')){
			$response=new XML_Element('response');
			$response->type='Gold.View.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$gid=(string)($request->params->gid)+0;
		$response=new XML_Element('response');
		$gold=MOD('gold')->get($gid);
		if($gold){
			$response->type='Gold.View.Succeed';
			$gold['code']=substr('000000'.$gold['code'],-6);
			$gold['dateline']=udate('Y-m-d H:i',$gold['dateline'],$uid);
			$response->gold=array_to_xml($gold,'gold');
			MOD('user.gold')->add(array('uid'=>$uid,'gid'=>$gid,'isread'=>1,'readtime'=>time()),false,true);
		}else{
			$response->type='Gold.View.Failed';
			$response->setText('优选金股记录不存在!');
		}
		return $response;
	}
}
