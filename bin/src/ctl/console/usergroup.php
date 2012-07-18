<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('ctl.base');
import('lib.xml');

Class CtlConsoleUserGroup extends CtlBase{
	function onState($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');
		$response=new XML_Element('response');
		if(MUK($uid,'user_group')){
			$response->type='Console.UserGroup.State.Succeed';
		}else{
			$response->type='Console.UserGroup.State.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		return $response;
	}
	function onList($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');
		$page=(string)($request->params->page)+0;
		$size=(string)($request->params->size)+0;

		$xml=new XML_Element('response');
		if(!MUK($uid,'user_group')){
			$response->type='Console.UserGroup.List.Failed';
			$xml->setText(USER_NOPRIV_MSG);
			return $xml;
		}

		$xml->type='Console.UserGroup.List.Succeed';
		$groups=MOD('user.group')->get_list_by_where();
		foreach($groups as $gid=>$r){
			$xml->$gid=new XML_Element('group');
			$xml->$gid->gid=$gid;
			$xml->$gid->gname=$r['gname'];
			$xml->$gid->title=$r['title'];
		}
		return $xml;
	}
	function onAdd($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		if(!MUK($uid,'user_group')){
			$response=new XML_Element('response');
			$response->type='Console.UserGroup.Add.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$params=&$request->params;
		$data=array(
			'gname'=>(string)($params->gname),
			'title'=>(string)($params->title),
		);
		$response=new XML_Element('response');
		if(MOD('user.group')->add($data)){
			$response->type='Console.UserGroup.Add.Succeed';
			$response->setText('提交成功！');
		}elseif($error=MOD('user.group')->error){
			$response->type='Console.UserGroup.Add.Failed';
			$response->setText($error);
		}else{
			$response->type='Console.UserGroup.Add.Failed';
			$response->setText('未知错误!');
		}
		return $response;
	}
	function onEdit($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		if(!MUK($uid,'user_group')){
			$response=new XML_Element('response');
			$response->type='Console.UserGroup.Edit.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$gid=(string)($request->params->gid)+0;
		$response=new XML_Element('response');
		if($group=MOD('user.group')->get($gid)){
			$response->type='Console.UserGroup.Edit.Succeed';
			$response->group=array_to_xml($group,'group');
		}else{
			$response->type='Console.UserGroup.Edit.Failed';
			$response->setText('用户组不存在!');
		}
		return $response;
	}
	function onEditSave($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		if(!MUK($uid,'user_group')){
			$response=new XML_Element('response');
			$response->type='Console.UserGroup.EditSave.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$gid=(string)($request->params->gid)+0;
		$response=new XML_Element('response');
		if($group=MOD('user.group')->get($gid)){
			$data=array(
				'gname'=>(string)($request->params->gname),
				'title'=>(string)($request->params->title),
				'manage'=>(string)($request->params->manage)+0,
				'manage_user'=>(string)($request->params->manage_user)+0,
				'manage_user_group'=>(string)($request->params->manage_user_group)+0,
				'manage_serv_group'=>(string)($request->params->manage_serv_group)+0,
				'manage_service'=>(string)($request->params->manage_service)+0,
				'manage_count'=>(string)($request->params->manage_count)+0,
				'use_expiry'=>(string)($request->params->use_expiry)+0,
				'stock'=>(string)($request->params->stock)+0,
				'stock_add'=>(string)($request->params->stock_add)+0,
				'stock_eval'=>(string)($request->params->stock_eval)+0,
				'broadcast'=>(string)($request->params->broadcast)+0,
				'broadcast_add'=>(string)($request->params->broadcast_add)+0,
				'gold'=>(string)($request->params->gold)+0,
				'gold_add'=>(string)($request->params->gold_add)+0,
				'invest'=>(string)($request->params->invest)+0,
				'invest_add'=>(string)($request->params->invest_add)+0,
				'consult_ask'=>(string)($request->params->consult_ask)+0,
				'consult_reply'=>(string)($request->params->consult_reply)+0,
			);
			if($group['gname']==$data['gname']){
				$data['gname']=null;unset($data['gname']);
			}
			if($group['title']==$data['title']){
				$data['title']=null;unset($data['title']);
			}
			if(!MOD('user.group')->edit($gid,$data)){
				$response->type='Console.UserGroup.EditSave.Failed';
				$response->setText(MOD('user.group')->error?MOD('user.group')->error:'未知错误！');
			}else{
				$response->type='Console.UserGroup.EditSave.Succeed';
				$response->setText('保存成功！');
			}
		}else{
			$response->type='Console.UserGroup.EditSave.Failed';
			$response->setText('用户组不存在!');
		}
		return $response;
	}
	function onDrop($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		$response=new XML_Element('response');
		if(!MUK($uid,'user_group')){
			$response->type='Console.UserGroup.Drop.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$gid=(string)($request->params->gid)+0;
		if(MOD('user.group')->drop($gid)){
			$response->type='Console.UserGroup.Drop.Succeed';
			$response->setText('删除成功！');
		}else{
			$response->type='Console.UserGroup.Drop.Failed';
			$response->setText(MOD('user.group')->error?MOD('user.group')->error:'未知错误！');
		}
		return $response;
	}
}
