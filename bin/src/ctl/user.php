<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('ctl.base');
import('api.uc.client');
import('lib.xml');

Class CtlUser extends CtlBase{
	function onLogin($request){
		$params=&$request->params;
		$auth=explode("\t",str_decode($params->auth));
		if(count($auth)==2)
			list($uid,$password)=$auth;
		else{
			$uid=0;
			$password='';
		}
		if($uid>0 && preg_match('/^\w{32}$/', $password)){
			$isuid=1;
			$username=$uid;
		}else{
			if(LIB('validate')->uinteger($params->username))
				$isuid=1;
			elseif(LIB('validate')->email($params->username))
				$isuid=2;
			elseif(LIB('validate')->username($params->username))
				$isuid=3;
			else{
				$isuid=0;
				return;
			}
			$username=trim($params->username);
			$password=md5($params->password);
		}
		list($uid,$username,$password,$email)=uc_user_login($username,$password,$isuid);

		$response=new XML_Element('response');
		if($uid<0){
			MOD('user.online')->edit($request->ClientId,array('logintimes'=>MOD('user.online')->get_by_client($request->ClientId,'logintimes')+1));
		}
		if($uid==-1){
			$response->type='User.Login.Failed';
			$response->setText('用户不存在！');
		}elseif($uid==-2){
			$response->type='User.Login.Failed';
			$response->setText('用户名或密码不正确！');
		}elseif($uid==-3){
			$response->type='User.Login.Failed';
			$response->setText('安全问题回答不正确！');
		}else{
			if($exitId=MOD('user.online')->get_by_user($uid,'onid')){
				$exitLogin=new XML_Element('response');
				$exitLogin->type='User.Login.Failed';
				$exitLogin->setText('此用户在另一地点登录，你被迫退出！');
				$this->send($exitId,$exitLogin);
				$this->close($exitId);
			}
			$data=array(
				'uid'=>$uid,
				'logintimes'=>0,
				'logintime'=>time(),
				'timezone'=>(string)$params->timezone+0,
			);
			MOD('user.online')->edit($request->ClientId,$data);
			if($user=MOD('user')->get($uid)){
				$profile=MOD('user.profile')->get($uid);
				$setting=MOD('user.setting')->get($uid);
				$data=array(
					'username'=>$username,
					'password'=>$password,
					'email'=>$email,
				);
				MOD('user')->edit($uid,$data,false);
			}else{
				$user=array(
					'uid'=>$uid,
					'gid'=>3,
					'username'=>$username,
					'password'=>$password,
					'email'=>$email,
					'regip'=>MOD('user.online')->get_by_client($request->ClientId,'host'),
					'regtime'=>time(),
				);
				MOD('user')->add($user);
			}
			$response->type='User.Login.Succeed';
			if((string)($params->is_simple)!='true'){
				$response->user=new XML_Element('user');

				$response->user->auth=str_encode($uid."\t".$password);

				$response->user->uid=$uid;
				$response->user->username=$username;
				$response->user->password=$password;
				$response->user->email=$email;
				$response->user->minavatar=avatar($uid,'small');
				$response->user->midavatar=avatar($uid,'middle');
				$response->user->maxavatar=avatar($uid,'big');

				$profile['nickname']=empty($profile['nickname'])?$username:$profile['nickname'];
				$response->user->profile=array_to_xml($profile,'profile');
				$response->user->setting=array_to_xml($setting,'setting');

				if($userTree=$this->userTree($user['gid'])){
					$response->userTree=$userTree;
				}

				$response->newsTree=$this->newsTree($uid);
			}else{
				$response->setText('登录成功！');
			}
		}
		return $response;
	}
	function userTree($gid){
		if($userTree=MOD('user')->get_tree_by_group($gid)){
			$group=MOD('user.group')->get($gid);
			$isTree=strpos($group['userlistgroup'],',')!==false;
			$xml=($isTree?new XML_Element('userTree'):array());
			foreach($userTree as $r){
				$g=MOD('user.group')->get($r['gid']);
				$r['nickname']=empty($r['nickname'])?$r['username']:$r['nickname'];
				unset($r['gid'],$r['username']);
				$r['avatar']=avatar($r['uid'],'small');
				if($isTree){
					$tags=$g['gname'].'s';
					if(!$xml->$tags){
						$xml->$tags=new XML_Element($tags);
						$xml->$tags->gid=$g['gid'];
						$xml->$tags->name=$g['gname'];
						$xml->$tags->label=$g['title'];
					}
					$xml->$tags->$r['uid']=array_to_xml($r,$g['gname']);
				}else{
					$xml[$r['uid']]=array_to_xml($r,$g['gname']);
				}
			}
			return $xml;
		}
		return false;
	}
	function newsTree($uid){
		$xml=new XML_Element('newsTree');
		$groups=MOD('news.group')->get_list_by_where();
		foreach($groups as $r){
			$xml->$r['gid']=new XML_Element('category');
			$xml->$r['gid']->gid=$r['gid'];
			$xml->$r['gid']->label=$r['gname'];
			$xml->$r['gid']->counts=$r['counts'];
		}
		$newsTree=MOD('news')->get_tree_by_user($uid);
		foreach($newsTree as $r){
			$r['dateline']=gmdate('m-d H:i',$r['dateline']-MOD('user.online')->get_by_user($uid,'timezone'));
			$xml->$r['gid']->$r['aid']=array_to_xml($r,'news');
		}
		return $xml;
	}
	function onLogout($request){
		$response=new XML_Element('response');
		if($this->logout($request->ClientId)){
			$response->type='User.Logout.Succeed';
		}else{
			$response->type='User.Logout.Failed';
		}
		return $response;
		//return $this->login($request->ClientId);
	}
	function login($ClientId){
		$response=new XML_Element('response');
		$response->type='User.Login';
/*
		$response->secode=new XML_Element('secode');
		$response->secode->type='jpg';
		$response->secode->mime='image/jpg';
		$response->secode->encoding='Base64';
		$response->secode->setText=base64_encode('Base64');
*/
		return $response;
	}
	function logout($ClientId){
		$uid=MOD('user.online')->get_by_client($ClientId,'uid');
		if($uid>0){
			$onlinetime=time()-MOD('user.online')->get_by_user($uid,'logintime');
			$data=array(
				'onlinetime'=>'`onlinetime`+'.$onlinetime,
			);
			MOD('user')->edit($uid,$data,false,false);
			MOD('user.online')->drop($ClientId,true);
			return true;
		}
		return false;
	}
	function onRegister($request){
		$params=&$request->params;
		$response=new XML_Element('response');
		$data=array(
			'username'=>trim($params->username),
			'password'=>trim($params->password),
			'email'=>trim($params->email),
			'regip'=>MOD('user.online')->get_by_client($request->ClientId,'host'),
			'regtime'=>time(),
		);
		if(MOD('user')->register($data)){
			$response->type='User.Register.Succeed';
			$response->username=$data['username'];
			$response->setText('注册成功！');
		}else{
			$response->type='User.Register.Failed';
			$response->setText(MOD('user')->error);
		}
		return $response;
	}
	function onEdit($request){
	}
	function onSetting($request){
		$keys=array();
		$data=array();
		foreach($request->params as $key=>$value){
			if(in_array($key,$keys)){
				$data[$key]=(string)$value;
			}
		}
		if($profile=MOD('user.profile')->get($uid)){
			MOD('user.profile')->edit($uid,$data);
		}else{
			$data['uid']=$uid;
			MOD('user.profile')->add($data);
		}
		$response=new XML_Element('response');
		$response->type='User.Setting.Succeed';
		$response->setText('保存成功！');
		return $response;
	}
	function onProfile($request){
		$keys=array('nickname','sex','signature');
		$data=array();
		foreach($request->params as $key=>$value){
			if(in_array($key,$keys)){
				$data[$key]=(string)$value;
			}
		}
		$uid=MOD('user.online')->get_by_client($request->ClientId,'uid');
		if($profile=MOD('user.profile')->get($uid)){
			MOD('user.profile')->edit($uid,$data);
		}else{
			$data['uid']=$uid;
			MOD('user.profile')->add($data);
		}
		$response=new XML_Element('response');
		$response->type='User.Profile.Succeed';
		$response->setText('保存成功！');
		return $response;
	}
}
