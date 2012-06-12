<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('ctl.base');
import('api.uc.client');
import('lib.xml');

Class CtlUser extends CtlBase{
	function onLogin($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
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
				$response=new XML_Element('response');
				$response->type='User.Login.Failed';
				$response->setText('用户名格式不合法！');
				return $response;
			}
			$username=trim($params->username);
			$password=md5($params->password);
		}
		list($uid,$username,$password,$email)=uc_user_login($username,$password,$isuid);

		$response=new XML_Element('response');
		if($uid<0){
			MOD('user.online')->edit($sockfd,array('logintimes'=>MOD('user.online')->get_by_client($sockfd,'logintimes')+1));
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
			if($user=MOD('user')->get($uid)){
				$profile=MOD('user.profile')->get($uid);
				$data=array(
					'username'=>$username,
					'password'=>$password,
					'email'=>$email,
					'prevlogtime'=>$user['logtime'],
					'logtime'=>time(),
				);
				MOD('user')->edit($uid,$data,false);
				$user['prevlogtime']=$data['prevlogtime'];
				$user['logtime']=$data['logtime'];
			}else{
				$servday=0;
				$user=array(
					'uid'=>$uid,
					'gid'=>USER_REG_GID,
					'username'=>$username,
					'password'=>$password,
					'email'=>$email,
					'regip'=>MOD('user.online')->get_by_client($sockfd,'host'),
					'regtime'=>time(),
					'prevlogtime'=>time(),
					'logtime'=>time(),
				);
				if(!MOD('user')->add($user)){
					$response->type='User.Login.Failed';
					$response->setText(MOD('user')->error?MOD('user')->error:'未知登录错误！');
					return $response;
				}
			}
			$data=array(
				'uid'=>$uid,
				'gid'=>$user['gid'],
				'logintimes'=>0,
				'logintime'=>time(),
				'timezone'=>(string)$params->timezone+0,
			);
			MOD('user.online')->edit($sockfd,$data);
			if(UGK($uid,'use_expiry',false)){
				$expiry=MOD('user.setting')->get($uid,'expiry');
				$servday=ceil(($expiry-$user['logtime'])/86400);
				if($servday<=0){
					MOD('user.online')->drop($sockfd,true);
					$response->type='User.Login.Failed';
					$response->setText('服务已过期！请即时续费！');
					return $response;
				}
			}else{
				$servday='无限';
			}

			$response->type='User.Login.Succeed';
			if((string)($request->is_simple)!='true'){
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
				$response->user->setting=new XML_Element('setting');
				$response->user->setting->servday=$servday;
				$response->user->prevlogtime=udate('Y-m-d H:i:s',$user['prevlogtime'],$uid);
				$response->user->logtime=udate('Y-m-d H:i:s',$user['logtime'],$uid);
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
			$r['dateline']=udate('m-d H:i',$r['dateline'],$uid);
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
		$sockfd=ssp_info($ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		if($uid>0){
			$onlinetime=time()-MOD('user.online')->get_by_user($uid,'logintime');
			$data=array(
				'onlinetime'=>'`onlinetime`+'.$onlinetime,
			);
			MOD('user')->edit($uid,$data,false,false);
			MOD('user.online')->drop($sockfd,true);
			return true;
		}
		return false;
	}
	function onRegister($request){
		$params=&$request->params;
		$response=new XML_Element('response');
		$data=array(
			'username'=>$params->username,
			'password'=>$params->password,
			'email'=>$params->email,
			'regip'=>MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'host'),
			'regtime'=>time(),
		);
		if(MOD('user')->register($data)){
			$response->type='User.Register.Succeed';
			$response->username=(string)$data['username'];
			$response->setText('注册成功！');
		}else{
			$response->type='User.Register.Failed';
			$response->setText(MOD('user')->error);
		}
		return $response;
	}
	function onLostpasswd($request){
		$params=&$request->params;
		$response=new XML_Element('response');

		$username=(string)($params->username);
		$email=(string)($params->email);

		list($uid,$_username,$_email)=uc_get_user($username,0);
		if($username!=$_username || $email!=$_email){
			$response->type='User.Lostpasswd.Succeed';
			$response->setText('您填写的账户资料不匹配，不能使用取回密码功能，如有疑问请与管理员联系');
		}else{
			$password=LIB('string')->rand(8,STRING_RAND_BOTH);
			$status=uc_user_edit($username,null,$password,$email,1);
			switch($status){
				case 1:
					$title=WEB_TITLE;
					if(MOD('mail')->send($params->email,'找回密码已成功！',"恭喜您！<br/>
　　您在“{$title}”注册的帐户，找回密码已成功！<br/>
<br/>
帐户信息如下：<br/>
　　用户ID：$uid<br/>
　　用户名：$username<br/>
　　密码：$password<br/>
　　邮箱地址：$email")){
						$message='你的密码发送E-Mail到“'.$email.'”成功！';break;
					}else{
						$status=-1;
						$message='你的密码发送E-Mail到“'.$email.'”失败！';break;
					}
					$response->username=$username;
				case 0:
					$message='没有做任何修改！';break;
				case -1:
					$message='旧密码不正确！';break;
				case -4:
					$message='Email 格式有误！';break;
				case -5:
					$message='Email 不允许注册！';break;
				case -6:
					$message='该 Email 已经被注册！';break;
				case -7:
					$message='没有做任何修改！';break;
				case -8:
					$message='该用户受保护无权限更改！';break;
			}
			if($status<0){
				$response->type='User.Lostpasswd.Failed';
			}else{
				$response->type='User.Lostpasswd.Succeed';
			}
			$response->setText($message);
		}
		return $response;
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
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');
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
	function onAvatar($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		$response=new XML_Element('response');
		$response->type='User.Avatar';
		$response->avatar=new XML_Element('avatar');
		$response->avatar->min=avatar($uid,'small');
		$response->avatar->mid=avatar($uid,'middle');
		$response->avatar->max=avatar($uid,'big');
		$args=uc_avatar($uid,'virtual', 0);
		$response->args=new XML_Element('args');
		for($i=0;$i<count($args);$i+=2){
			$response->args->{$args[$i]}=$args[$i+1];
		}
		return $response;
	}
	function onPriv($request){
		$response=new XML_Element('response');
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');
		if(UGK($uid,(string)($request->params->key))){
			$response->type='User.Priv.Succeed';
		}else{
			$response->type='User.Priv.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		return $response;
	}
}
