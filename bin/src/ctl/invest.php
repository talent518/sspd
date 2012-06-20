<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('ctl.base');
import('lib.xml');

Class CtlInvest extends CtlBase{
	function onState($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');
		$response=new XML_Element('response');
		if(UGK($uid,'invest_add')){
			$response->type='Invest.State.Succeed';
			$response->state='add';
			MOD('user.setting')->set($uid,'invest',1);
		}elseif(UGK($uid,'invest')){
			$response->type='Invest.State.Succeed';
			if(MOD('user.setting')->get($uid,'invest')){
				$response->state='view';
			}else{
				$response->state='normal';
			}
		}else{
			$response->type='Invest.State.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		return $response;
	}
	function onAgree($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');
		$response=new XML_Element('response');

		if(UGK($uid,'invest')){
			MOD('user.setting')->set($uid,'invest',1);
			$response->type='Invest.Agree.Succeed';
			$response->state='view';
		}else{
			$response->type='Invest.Agree.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		return $response;
	}
	function onList($request,$page=1,$size=10,$isToday=0){
		$xml=new XML_Element('response');
		$xml->type='Invest.List';
		if(is_object($request)){
			$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');
			$page=(string)($request->params->page)+0;
			$size=(string)($request->params->size)+0;
			$isToday=(string)($request->params->isToday)+0;
		}else{
			$uid=$request+0;
		}
		if(!UGK($uid,'invest')){
			$xml->setText(USER_NOPRIV_MSG);
			return $xml;
		}
		if($isToday){
			$time=@strtotime('today');
			$where='dateline>'.$time.' AND dateline<86400+'.$time;
		}
		$xml->counts=MOD('invest')->count($where);
		$limit=get_limit($page,$size,$xml->counts);
		$investList=MOD('invest')->get_list_by_user($uid,$isToday!=0,$limit);
		foreach($investList as $r){
			$r['code']=substr('000000'.$r['code'],-6);
			$r['dateline']=udate('m-d H:i',$r['dateline'],$uid);
			$xml->$r['iid']=array_to_xml($r,'invest');
		}
		return $xml;
	}
	function onAdd($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		if(!UGK($uid,'invest_add')){
			$response=new XML_Element('response');
			$response->type='Invest.Add.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$params=&$request->params;
		$isids=array();
		if($_isids=explode(',',$params->isids)){
			foreach($_isids as $isid){
				if(LIB('validate')->integer($isid)){
					$isids[]=(int)$isid;
				}
			}
		}
		if(count($isids)==0){
			$response=new XML_Element('response');
			$response->type='Invest.Add.Failed';
			$response->setText('您还没有添加关联的股票！');
		}
		$data=array(
			'uid'=>$uid,
			'title'=>(string)($params->title),
			'dateline'=>time(),
		);
		$response=new XML_Element('response');
		if(MOD('invest')->add($data)){
			MOD('invest.stock')->update(array('iid'=>DB()->insert_id()),'isid IN('.iimplode($isids).')');
			$response->type='Invest.Add.Succeed';
			$response->setText('保存成功！');

			$data['gid']=DB()->insert_id();

			$remind=new XML_Element('response');
			$remind->type='Remind.Invest';
			foreach(MOD('user.online')->get_list_by_where('uid>0') as $sf=>$r){
				if(UGK($r['uid'],'invest')){
					$this->send($sf,$remind);
				}
			}
		}elseif($error=MOD('invest')->error){
			$response->type='Invest.Add.Failed';
			$response->setText($error);
		}else{
			$response->type='Invest.Add.Failed';
			$response->setText('未知错误!');
		}
		return $response;
	}
	function onAddStock($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		if(!UGK($uid,'invest_add')){
			$response=new XML_Element('response');
			$response->type='Invest.AddStock.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$params=&$request->params;
		$data=array(
			'code'=>(string)($params->code),
			'name'=>(string)($params->name),
			'reason'=>(string)($params->reason),
			'think'=>(string)($params->think),
		);
		$response=new XML_Element('response');
		if(MOD('invest.stock')->add($data)){
			$response->type='Invest.AddStock.Succeed';
			$response->isid=DB()->insert_id();
			$response->setText('添加股票成功！<br/><font color="#960003" size="18"><b>随后别忘了保存幺！</b></font>');
		}elseif($error=MOD('invest')->error){
			$response->type='Invest.AddStock.Failed';
			$response->setText($error);
		}else{
			$response->type='Invest.AddStock.Failed';
			$response->setText('未知错误!');
		}
		return $response;
	}
	function onView($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		if(!UGK($uid,'invest_add')){
			$response=new XML_Element('response');
			$response->type='Invest.View.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$iid=(string)($request->params->iid)+0;
		$response=new XML_Element('response');
		$invest=MOD('invest')->get($iid);
		if($invest){
			$response->type='Invest.View.Succeed';
			$invest['dateline']=udate('Y-m-d H:i',$invest['dateline'],$uid);
			$response->invest=array_to_xml($invest,'invest');
			foreach(MOD('invest.stock')->get_list_by_where($iid) as $isid=>$r){
				$r['code']=substr('000000'.$r['code'],-6);
				$response->$isid=array_to_xml($r,'stock');
			}
			MOD('user.invest')->add(array('uid'=>$uid,'iid'=>$iid,'isread'=>1,'readtime'=>time()),false,true);
		}else{
			$response->type='Invest.View.Failed';
			$response->setText('投资组合记录不存在!');
		}
		return $response;
	}
}
