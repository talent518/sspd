<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('ctl.base');
import('lib.xml');

Class CtlStock extends CtlBase{
	function onState($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');
		$response=new XML_Element('response');
		if(UGK($uid,'stock_add')){
			$response->type='Stock.State.Succeed';
			$response->state='add';
		}elseif(UGK($uid,'stock_eval')){
			$response->type='Stock.State.Succeed';
			$response->state='eval';
		}elseif(UGK($uid,'stock')){
			$response->type='Stock.State.Succeed';
			$response->state='normal';
		}else{
			$response->type='Stock.State.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		return $response;
	}
	function onList($request,$page=1,$size=10,$isEval=0){
		$xml=new XML_Element('response');
		$xml->type='Stock.List';
		if(is_object($request)){
			$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');
			$page=(string)($request->params->page)+0;
			$size=(string)($request->params->size)+0;
			$isEval=(string)($request->params->isEval)+0;
		}else{
			$uid=$request+0;
		}
		if($iseval=UGK($uid,'stock_eval')){
			if($isEval){
				$where='evaluid=0 OR evaluid=-'.$uid;
			}else{
				$where='evaluid='.$uid;
				$iseval=0;
				$order='evaldate DESC';
			}
		}else{
			$where='uid='.$uid;
			if($isEval){
				$where.=' AND evaluid>0';
				$order='evaldate DESC';
			}else{
				$order='sid ASC,evaldate ASC';
			}
		}
		$xml->counts=MOD('user.stock')->count($where);
		$limit=get_limit($page,$size,$xml->counts);
		$stockList=MOD('user.stock')->get_list_by_where($where,$limit,$order);
		foreach($stockList as $r){
			$r['code']=substr('000000'.$r['code'],-6);
			$r['dateline']=udate('m-d H:i',$r['dateline'],$uid);
			$r['dealdate']=udate('Y-m-d',$r['dealdate'],$uid);
			if($r['evaluid']>0){
				$r['evaldate']=udate('m-d H:i',$r['evaldate'],$uid);
			}else{
				$r['iseval']=$iseval;
			}
			$xml->$r['sid']=array_to_xml($r,'stock');
		}
		return $xml;
	}
	function onAdd($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		if(!UGK($uid,'stock_add')){
			$response=new XML_Element('response');
			$response->type='Stock.Add.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$params=&$request->params;
		$data=array(
			'uid'=>$uid,
			'code'=>(string)($params->code),
			'name'=>(string)($params->name),
			'type'=>(string)($params->type),
			'dealdate'=>(string)($params->dealdate),
			'amount'=>(string)($params->amount),
			'location'=>(string)($params->location),
			'price'=>(string)($params->price),
			'stoploss'=>(string)($params->stoploss),
			'reason'=>(string)($params->reason),
			'profitloss'=>(string)($params->profitloss),
		);
		$response=new XML_Element('response');
		if(MOD('user.stock')->add($data)){
			$response->type='Stock.Add.Succeed';
			$response->setText('提交成功！');
		}elseif($error=MOD('user.stock')->error){
			$response->type='Stock.Add.Failed';
			$response->setText($error);
		}else{
			$response->type='Stock.Add.Failed';
			$response->setText('未知错误!');
		}
		return $response;
	}
	function onView($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		$sid=(string)($request->params->sid)+0;
		$response=new XML_Element('response');
		$stock=MOD('user.stock')->get($sid);
		if($stock){
			$response->type='Stock.View.Succeed';
			$stock['code']=substr('000000'.$stock['code'],-6);
			$stock['dateline']=udate('m-d H:i',$stock['dateline'],$uid);
			$stock['dealdate']=udate('Y-m-d',$stock['dealdate'],$uid);
			if($stock['evaluid']>0){
				$user=MOD('user')->get($stock['evaluid']);
				$stock['evalusername']=$user['username'];
				$stock['evaldate']=udate('Y-m-d H:i',$stock['evaldate'],$uid);
				$stock['evalavatar']=avatar($stock['evaluid'],'small');
				$response->state='view';
			}elseif(UGK($uid,'stock_eval')){
				$response->state='eval';
				MOD('user.stock')->update(array('evaluid'=>-$uid),$sid,false);
			}else{
				$response->state='none';
			}
			$response->stock=array_to_xml($stock,'stock');
		}else{
			$response->type='Stock.View.Failed';
			$response->setText('股票操作记录不存在!');
		}
		return $response;
	}
	function onEval($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		if(!UGK($uid,'stock_eval')){
			$response=new XML_Element('response');
			$response->type='Stock.Eval.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$params=&$request->params;
		$sid=(string)($params->sid)+0;
		$data=array(
			'evaluid'=>$uid,
			'evaluation'=>(string)($params->evaluation),
			'evaldate'=>time(),
		);
		$response=new XML_Element('response');
		if(empty($data['evaluation'])){
			$response->type='Stock.Eval.Failed';
			$response->setText('请输入评语！');
		}elseif(MOD('user.stock')->edit($sid,$data,false)){
			$response->type='Stock.Eval.Succeed';
			$response->setText('提交成功！');
		}elseif($error=MOD('user.stock')->error){
			$response->type='Stock.Eval.Failed';
			$response->setText($error);
		}else{
			$response->type='Stock.Eval.Failed';
			$response->setText('未知错误!');
		}
		return $response;
	}
}
