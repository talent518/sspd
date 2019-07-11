<?php
import('ctl.base');
import('api.uc.client');
import('lib.xml');

class CtlUser extends CtlBase {

	function onLogin ( $request ) {
		$index = ssp_info($request->ClientId, 'index');
		$params = &$request->params;
		$auth = explode("\t", crypt_decode($params->auth, SSP_KEY));
		if ( count($auth) == 2 )
			list ( $uid, $password ) = $auth;
		else {
			$uid = 0;
			$password = '';
		}
		if ( $uid > 0 && preg_match('/^\w{32}$/', $password) ) {
			$isuid = 1;
			$username = $uid;
		} else {
			if ( LIB('validate')->uinteger($params->username) )
				$isuid = 1;
			elseif ( LIB('validate')->email($params->username) )
				$isuid = 2;
			elseif ( LIB('validate')->username($params->username) )
				$isuid = 3;
			else {
				$isuid = 0;
				$response = new XML_Element('response');
				$response->type = 'User.Login.Failed';
				$response->setText('用户名格式不合法！');
				return $response;
			}
			$username = trim($params->username);
			$password = md5($params->password);
		}
		list ( $uid, $username, $password, $email ) = uc_user_login($username, $password, $isuid);
		
		$response = new XML_Element('response');
		if ( $uid < 0 ) {
			MOD('user.online')->edit($index, array(
				'logintimes' => MOD('user.online')->get_by_client($index, 'logintimes') + 1
			));
		}
		if ( $uid ==  - 1 ) {
			$response->type = 'User.Login.Failed';
			$response->setText('用户不存在！');
		} elseif ( $uid ==  - 2 ) {
			$response->type = 'User.Login.Failed';
			$response->setText('用户名或密码不正确！');
		} elseif ( $uid ==  - 3 ) {
			$response->type = 'User.Login.Failed';
			$response->setText('安全问题回答不正确！');
		} else {
			if (  !  ! ( $user = MOD('user')->get($uid) ) ) {
				$profile = MOD('user.profile')->get($uid);
				$data = array(
					'username' => $username, 
					'password' => $password, 
					'email' => $email, 
					'prevlogtime' => $user['logtime'], 
					'logtime' => time()
				);
				MOD('user')->edit($uid, $data, false);
				$user['prevlogtime'] = $data['prevlogtime'];
				$user['logtime'] = $data['logtime'];
			} else {
				$servday = 0;
				$user = array(
					'uid' => $uid, 
					'gid' => USER_REG_GID, 
					'username' => $username, 
					'password' => $password, 
					'email' => $email, 
					'regip' => MOD('user.online')->get_by_client($index, 'host'), 
					'regtime' => time(), 
					'prevlogtime' => time(), 
					'logtime' => time()
				);
				if (  ! MOD('user')->add($user) ) {
					$response->type = 'User.Login.Failed';
					$response->setText(MOD('user')->error ? MOD('user')->error : '未知登录错误！');
					return $response;
				}
			}
			
			$exitLogin = new XML_Element('response');
			$exitLogin->type = 'User.Login.Failed';
			$exitLogin->setText('此用户在另一地点登录，你被迫退出！');
			$exitStrng = $exitLogin;
			
			$exitData=array( 'uid'=>0, 'gid'=>0, 'logintimes'=>0, 'logintime'=>time(), 'timezone'=>0, 'broadcast'=>0, 'consult'=>0 );
			
			$data = array(
				'uid' => $uid, 
				'gid' => $user['gid'], 
				'logintimes' => 0, 
				'logintime' => time(), 
				'timezone' => ( string ) $params->timezone + 0, 
				'broadcast' => ( string ) $params->broadcast + 0, 
				'consult' => ( string ) $params->consult + 0
			);
			
			ssp_lock();
			$list=MOD('user.online')->get_list_by_uid_not_id($uid,$index);
			foreach($list as $exitId){
				MOD('user.online')->edit($exitId,$exitData);
			}
			MOD('user.online')->edit($index, $data);
			ssp_unlock();

			foreach($list as $exitId){
				$exitRes=ssp_resource($exitId);
				if($exitRes) {
					ssp_send($exitRes, $exitStrng); ssp_close($exitRes); ssp_destroy($exitRes);
				}
			}

			if ( UGK($uid, 'consult_reply') ) {
				if (  !  ! ( $consults = ( string ) $params->consults ) ) {
					MOD('user.serv')->update(array(
						'isopen' => 0
					), 'uid=' . $uid . ' AND cuid NOT IN(' . iimplode(explode(',', $consults)) . ')');
					MOD('user.serv')->update(array(
						'isopen' => 1
					), 'uid=' . $uid . ' AND cuid IN(' . iimplode(explode(',', $consults)) . ')');
				} else {
					MOD('user.serv')->update(array(
						'isopen' => 0
					), 'uid=' . $uid);
				}
			}
			if ( UGK($uid, 'use_expiry', false) ) {
				$expiry = MOD('user.setting')->get($uid, 'expiry');
				$servday = round(( $expiry - $user['logtime'] ) / 86400, 1);
				if ( $servday <= 0 ) {
					MOD('user.online')->drop($index, true);
					$response->type = 'User.Login.Failed';
					$response->setText('服务已过期！请即时续费！');
					return $response;
				}
			} else {
				$servday = '无限';
			}
			
			$response->type = 'User.Login.Succeed';
			if ( ( string ) ( $request->is_simple ) != 'true' ) {
				$response->user = new XML_Element('user');
				
				$response->user->auth = crypt_encode($uid . "\t" . $password, SSP_KEY);
				
				$response->user->uid = $uid;
				$response->user->username = $username;
				$response->user->password = $password;
				$response->user->email = $email;
				$response->user->minavatar = avatar($uid, 'small');
				$response->user->midavatar = avatar($uid, 'middle');
				$response->user->maxavatar = avatar($uid, 'big');
				
				$profile['nickname'] = empty($profile['nickname']) ? $username : $profile['nickname'];
				$response->user->profile = array_to_xml($profile, 'profile');
				$response->user->setting = new XML_Element('setting');
				$response->user->setting->servday = $servday;
				$response->user->setting->mute = MOD('user.setting')->get($uid, 'mute');
				$response->user->prevlogtime = udate('Y-m-d H:i:s', $user['prevlogtime'], $uid);
				$response->user->logtime = udate('Y-m-d H:i:s', $user['logtime'], $uid);
				$response->multiConsult = UGK($uid, 'consult_reply');
				$remind = MOD('count')->remind($uid);
				$response->remind = array_to_xml($remind, 'remind');
			} else {
				$response->setText('登录成功！');
			}
		}
		return $response;
	}

	function newsTree ( $uid ) {
		$xml = new XML_Element('newsTree');
		$groups = MOD('news.group')->get_list_by_where();
		foreach ( $groups as $r ) {
			$xml->{$r['gid']} = new XML_Element('category');
			$xml->{$r['gid']}->gid = $r['gid'];
			$xml->{$r['gid']}->label = $r['gname'];
			$xml->{$r['gid']}->counts = $r['counts'];
		}
		$newsTree = MOD('news')->get_tree_by_user($uid);
		foreach ( $newsTree as $r ) {
			$r['dateline'] = udate('m-d H:i', $r['dateline'], $uid);
			$xml->{$r['gid']}->{$r['aid']} = array_to_xml($r, 'news');
		}
		return $xml;
	}

	function onLogout ( $request ) {
		$response = new XML_Element('response');
		if ( $this->logout($request->ClientId) ) {
			$response->type = 'User.Logout.Succeed';
		} else {
			$response->type = 'User.Logout.Failed';
		}
		return $response;
	}

	function login ( $ClientId ) {
		$response = new XML_Element('response');
		$response->type = 'User.Login';
		return $response;
	}

	function logout ( $ClientId ) {
		$index = ssp_info($ClientId, 'index');
		$uid = MOD('user.online')->get_by_client($index, 'uid');
		if ( $uid > 0 ) {
			$onlinetime = time() - MOD('user.online')->get_by_user($uid, 'logintime');
			$data = array(
				'onlinetime' => '`onlinetime`+' . $onlinetime
			);
			MOD('user')->edit($uid, $data, false, false);
			MOD('user.online')->drop($index, true);
			return true;
		}
		return false;
	}

	function onRegister ( $request ) {
		$response = new XML_Element('response');
		$data = array(
			'username' => ( string ) ( $request->params->username ), 
			'password' => ( string ) ( $request->params->password ), 
			'email' => ( string ) ( $request->params->email ), 
			'regip' => MOD('user.online')->get_by_client(ssp_info($request->ClientId, 'index'), 'host'), 
			'regtime' => time()
		);
		if (  !  ! ( $uid = MOD('user')->register($data) ) ) {
			$response->type = 'User.Register.Succeed';
			$response->uid = $uid;
			$response->username = $data['username'];
			$response->email = $data['email'];
			$response->setText('注册成功！');
		} else {
			$response->type = 'User.Register.Failed';
			$response->setText(MOD('user')->error . $uid);
		}
		return $response;
	}

	function onLostpasswd ( $request ) {
		$response = new XML_Element('response');
		
		$username = ( string ) ( $request->params->username );
		$email = ( string ) ( $request->params->email );
		
		list ( $uid, $_username, $_email ) = uc_get_user($username, 0);
		if ( $username != $_username || $email != $_email ) {
			$response->type = 'User.Lostpasswd.Succeed';
			$response->setText('您填写的账户资料不匹配，不能使用取回密码功能，如有疑问请与管理员联系');
		} else {
			$password = LIB('string')->rand(8, STRING_RAND_BOTH);
			$status = uc_user_edit($username, null, $password, $email, 1);
			switch ( $status ) {
				case 1:
					$title = WEB_TITLE;
					if ( MOD('mail')->send($email, '找回密码已成功！', "恭喜您！<br/>
　　您在“{$title}”注册的帐户，找回密码已成功！<br/>
<br/>
帐户信息如下：<br/>
　　用户ID：$uid<br/>
　　用户名：$username<br/>
　　密码：$password<br/>
　　邮箱地址：$email") ) {
						$message = '你的密码发送E-Mail到“' . $email . '”成功！';
						break;
					} else {
						$status =  - 1;
						$message = '你的密码发送E-Mail到“' . $email . '”失败！';
						break;
					}
					$response->username = $username;
				case 0:
					$message = '没有做任何修改！';
					break;
				case  - 1:
					$message = '旧密码不正确！';
					break;
				case  - 4:
					$message = 'Email 格式有误！';
					break;
				case  - 5:
					$message = 'Email 不允许注册！';
					break;
				case  - 6:
					$message = '该 Email 已经被注册！';
					break;
				case  - 7:
					$message = '没有做任何修改！';
					break;
				case  - 8:
					$message = '该用户受保护无权限更改！';
					break;
			}
			if ( $status < 0 ) {
				$response->type = 'User.Lostpasswd.Failed';
			} else {
				$response->type = 'User.Lostpasswd.Succeed';
			}
			$response->setText($message);
		}
		return $response;
	}

	function onProfile ( $request ) {
		$keys = array(
			'nickname', 
			'sex', 
			'signature'
		);
		$data = array();
		foreach ( $request->params as $key => $value ) {
			if ( in_array($key, $keys) ) {
				$data[$key] = ( string ) $value;
			}
		}
		$uid = MOD('user.online')->get_by_client(ssp_info($request->ClientId, 'index'), 'uid');
		if (  !  ! ( $profile = MOD('user.profile')->get($uid) ) ) {
			MOD('user.profile')->edit($uid, $data);
		} else {
			$data['uid'] = $uid;
			MOD('user.profile')->add($data);
		}
		$response = new XML_Element('response');
		$response->type = 'User.Profile.Succeed';
		$response->setText('保存成功！');
		return $response;
	}

	function onMute ( $request ) {
		$uid = MOD('user.online')->get_by_client(ssp_info($request->ClientId, 'index'), 'uid');
		MOD('user.setting')->set($uid, 'mute', ( string ) ( $request->params->mute ));
	}

	function onSendKey ( $request ) {
		$uid = MOD('user.online')->get_by_client(ssp_info($request->ClientId, 'index'), 'uid');
		MOD('user.setting')->set($uid, 'sendkey', ( string ) ( $request->params->key ));
	}

}
