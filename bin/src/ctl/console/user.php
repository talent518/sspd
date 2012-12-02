<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('ctl.base');
import('lib.xml');

Class CtlConsoleUser extends CtlBase{
	function onState($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');
		$response=new XML_Element('response');
		if(MUK($uid,'user')){
			$response->type='Console.User.State.Succeed';
		}else{
			$response->type='Console.User.State.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		return $response;
	}
	function onGroup($request){
		$xml=new XML_Element('response');
		$xml->type='Console.User.Group';
		$groups=MOD('user.group')->get_list_by_where();
		$gids=explode(',',USER_CLIENT_GIDS);
		foreach($groups as $gid=>$r){
			$xml->$gid=new XML_Element('group');
			$xml->$gid->gid=$gid;
			$xml->$gid->title=$r['title'];
			$xml->$gid->useExpiry=$r['use_expiry'];
		}
		return $xml;
	}
	function onServ($request){
		$xml=new XML_Element('response');
		$xml->type='Console.User.Serv';
		$servs=MOD('user')->get_list_by_where('gid IN('.iimplode(explode(',',USER_SERV_GIDS)).')');
		foreach($servs as $uid=>$r){
			$xml->$uid=new XML_Element('serv');
			$xml->$uid->uid=$uid;
			$xml->$uid->title=$r['username'];
		}
		return $xml;
	}
	function onServGroup($request){
		$xml=new XML_Element('response');
		$xml->type='Console.User.ServGroup';
		$groups=MOD('user.serv.group')->get_list_by_where();
		foreach($groups as $gid=>$r){
			$xml->$gid=new XML_Element('group');
			$xml->$gid->gid=$gid;
			$xml->$gid->title=$r['name'];
		}
		return $xml;
	}
	function onList($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');

		if(!MUK($uid,'user')){
			$response=new XML_Element('response');
			$response->type='Console.User.List.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		
		$page=(string)($request->params->page)+0;
		$size=(string)($request->params->size)+0;

		$userId=(string)($request->params->uid)+0;
		$username=(string)($request->params->username);
		$email=(string)($request->params->email);
		$userGid=(string)($request->params->gid)+0;
		$fromExpiry=(string)($request->params->from_expiry);
		$fromExpiry=(LIB('validate')->date($fromExpiry)?@strtotime($fromExpiry):0);
		$toExpiry=(string)($request->params->to_expiry);
		$toExpiry=(LIB('validate')->date($toExpiry)?@strtotime($toExpiry):0);
		$status=(string)($request->params->status)+0;

		$xml=new XML_Element('response');
		$xml->type='Console.User.List.Succeed';
		
		$where='1>0';

		if($userId){
			$where.=' AND uid='.$userId;
		}

		if($username && LIB('validate')->username($username)){
			$where.=' AND `username`=\''.addslashes($username).'\'';
		}

		if($email && LIB('validate')->email($email)){
			$where.=' AND `email`=\''.addslashes($email).'\'';
		}

		if($userGid){
			$where.=' AND `gid`='.$userGid;
		}

		if($fromExpiry){
			$where.=' AND uid IN(SELECT uid FROM '.DB()->tname('user_setting').' WHERE expiry>'.$fromExpiry.')';
		}
		if($toExpiry){
			$where.=' AND uid IN(SELECT uid FROM '.DB()->tname('user_setting').' WHERE expiry<'.$toExpiry.')';
		}

		if($status){
			$where.=' AND uid'.($status<0?' NOT ':' ').'IN(SELECT uid FROM '.DB()->tname('user_online').' WHERE uid>0)';
		}

		if($xml->counts=MOD('user')->count($where)){
			$limit=get_limit($page,$size,$xml->counts);
			$users=MOD('user')->get_list_by_where($where,$limit);
			foreach($users as $r){
				$r['group']=MOD('user.group')->get($r['gid'],'title');
				$r['online']=(MOD('user.online')->get_by_user($r['uid'])?'在线':'离线');
				$r['onlinetime']=formatsecond($r['onlinetime'],'未登录过');
				$xml->$r['uid']=array_to_xml($r,'user');
			}
		}else{
			if($userId){
				$isuid=1;
			}elseif($username){
				$isuid=0;
			}else{
				return $xml;
			}
			list($suid,$username,$email)=uc_get_user($username,$isuid);
			if($suid<=0){
				return $xml;
			}
			$user=array(
				'uid'=>$suid,
				'gid'=>USER_REG_GID,
				'username'=>$username,
				'password'=>md5('123456'),
				'email'=>$email,
				'regip'=>MOD('user.online')->get_by_client($sockfd,'host'),
				'regtime'=>time(),
				'prevlogtime'=>time(),
				'logtime'=>time(),
			);
			if(!MOD('user')->add($user)){
				$xml->type='Console.User.List.Failed';
				$xml->setText(MOD('user')->error?MOD('user')->error:'未知错误！');
				return $xml;
			}else{
				$user['regtime']=udate('Y-m-d H:i',$user['regtime'],$uid);
				$xml->$suid=array_to_xml($user,'user');
			}
		}
		return $xml;
	}
	function onEdit($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		if(!MUK($uid,'user')){
			$response=new XML_Element('response');
			$response->type='Console.User.Edit.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$userId=(string)($request->params->uid)+0;
		$response=new XML_Element('response');
		$user=MOD('user')->get($userId);
		if($user){
			$response->type='Console.User.Edit.Succeed';
			$user['regtime']=udate('Y-m-d H:i',$user['regtime'],$uid);
			$user['prevlogtime']=udate('Y-m-d H:i',$user['prevlogtime'],$uid);
			$user['logtime']=udate('Y-m-d H:i',$user['logtime'],$uid);
			if($expiry=MOD('user.setting')->get($userId,'expiry',false)){
				$user['expiry']=date('Y-m-d',$expiry);
				$user['servday']=round(($expiry-time())/86400,1);
			}else{
				$user['servday']=0;
			}
			$response->user=array_to_xml($user,'user');
			if($serv=MOD('user.serv')->get($userId)){
				$response->serv=array_to_xml($serv,'serv');
			}else{
				$response->serv=new XML_Element('serv');
			}
		}else{
			$response->type='Console.User.Edit.Failed';
			$response->setText('用户记录不存在!');
		}
		return $response;
	}
	function onEditSave($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		if(!MUK($uid,'user')){
			$response=new XML_Element('response');
			$response->type='Console.User.EditSave.Failed';
			$response->setText(USER_NOPRIV_MSG);
			return $response;
		}
		$userId=(string)($request->params->uid)+0;
		$response=new XML_Element('response');
		if($user=MOD('user')->get($userId)){
			$email=$password=false;
			if(property_exists($request->params,'email')){
				$email=(string)($request->params->email);
				if(!LIB('validate')->email($email)){
					$response->type='Console.User.EditSave.Failed';
					$response->setText('EMail格式不合法!');
					return $response;
				}
			}
			if(property_exists($request->params,'password')){
				$password=(string)($request->params->password);
			}
			if(property_exists($request->params,'gid')){
				$gid=(string)($request->params->gid)+0;
			}else{
				$gid=0;
			}
			$data=array();
			if(($email && $email!=$user['email']) || $password){
				import('api.uc.client');
				$ret=uc_user_edit($user['username'],'',$password,$email,1);
				switch($ret){
					case -1:
						$msg='密码错误';
						break;
					case UC_USER_EMAIL_FORMAT_ILLEGAL:
						$msg='Email 地址无效';
						break;
					case UC_USER_EMAIL_ACCESS_ILLEGAL:
						$msg='抱歉，Email 包含不可使用的邮箱域名';
						break;
					case UC_USER_EMAIL_EXISTS:
						$msg='该 Email 地址已被注册';
						break;
					default:
						$msg=false;
						break;
				}
				if($msg){
					$response->type='Console.User.EditSave.Failed';
					$response->setText($msg);
					return $response;
				}else{
					if($password){
						$data['password']=md5($password);
					}
					if($email){
						$data['email']=$email;
					}
				}
			}
			if($gid>0){
				$data['gid']=$gid;
			}
			MOD('user')->edit($userId,$data,false);

			if(CGK($gid>0?$gid:$user['gid'],'use_expiry')){
				$expiry=(string)($request->params->expiry);
				if(property_exists($request->params,'expiry')){
					if(LIB('validate')->date($expiry)){
						MOD('user.setting')->set($userId,'expiry',@strtotime($expiry));
					}else{
						$response->type='Console.User.EditSave.Failed';
						$response->setText('过期时间格式不正确！');
						return $response;
					}
				}
				$akeys=array('emailserv'=>'email','suid'=>'uid','sgid'=>'gid');
				$data=array();
				foreach(array('suid','sgid','nickname','phone','emailserv','qq','funds','address','remark') as $key){
					if(property_exists($request->params,$key)){
						$data[$akeys[$key]?$akeys[$key]:$key]=(string)($request->params->$key);
					}
				}
				if(count($data)){
					if(MOD('user.serv')->exists($userId)){
						MOD('user.serv')->edit($userId,$data);
					}else{
						$data['cuid']=$userId;
						MOD('user.serv')->add($data);
					}
				}
			}elseif($gid>0){
				MOD('user.setting')->set($userId,'expiry',0);
			}
			$response->type='Console.User.EditSave.Succeed';
			$response->setText('修改用户“'.$user['username'].'”保存成功!');
		}else{
			$response->type='Console.User.EditSave.Failed';
			$response->setText('用户记录不存在!');
		}
		return $response;
	}
	function onDrop($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		if(!MUK($uid,'user')){
			$response=new XML_Element('response');
			$response->type='Console.User.Drop.Failed';
			$response->setText(USER_NOPRIV_MSG);
			return $response;
		}
		$userId=(string)($request->params->uid)+0;
		$response=new XML_Element('response');
		if(MOD('user')->drop($userId)){
			$response->type='Console.User.Drop.Succeed';
			$response->setText('删除成功！');
		}else{
			$response->type='Console.User.Drop.Failed';
			$response->setText('删除失败!');
		}
		return $response;
	}
	function onView($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$uid=MOD('user.online')->get_by_client($sockfd,'uid');
		if(!MUK($uid,'user')){
			$response=new XML_Element('response');
			$response->type='Console.User.View.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$userId=(string)($request->params->uid)+0;
		$response=new XML_Element('response');
		$user=MOD('user')->get($userId);
		if($user){
			$response->type='Console.User.View.Succeed';
			$user['online']=(MOD('user.online')->get_by_user($userId)?'在线':'离线');
			$user['onlinetime']=formatsecond($user['onlinetime'],'从未登录过');
			$user['regtime']=udate('Y-m-d H:i',$user['regtime'],$uid);
			$user['group']=MOD('user.group')->get($user['gid'],'title');
			$response->user=array_to_xml($user,'user');
			$response->user->minavatar=avatar($userId,'small');
			$response->user->midavatar=avatar($userId,'middle');
			$response->user->maxavatar=avatar($userId,'big');
			$response->user->prevlogtime=udate('Y-m-d H:i:s',$user['prevlogtime'],$uid);
			$response->user->logtime=udate('Y-m-d H:i:s',$user['logtime'],$uid);
			$response->profile=array_to_xml(MOD('user.profile')->get($userId),'profile');
			$serv=MOD('user.serv')->get($userId);
			$response->serv=array_to_xml($serv,'serv');
			$group=MOD('user.serv.group')->get($serv['gid']);
			$response->serv->group=$group['name'];
			if(UGK($userId,'use_expiry',false)){
				$expiry=MOD('user.setting')->get($userId,'expiry');
				$response->serv->servday=round(($expiry-time())/86400,1);
			}else{
				$response->serv->servday='无限';
			}
		}else{
			$response->type='Console.User.View.Failed';
			$response->setText('用户记录不存在!');
		}
		return $response;
	}
}
