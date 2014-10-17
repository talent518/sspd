<?php
if (  ! defined('IN_SERVER') )
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModUser extends ModBase {

	protected $table = 'user';

	protected $priKey = 'uid';

	protected $order;

	protected $rules = array(
		'gid' => array(
			'required' => true, 
			'uinteger' => true, 
			'min' => 1
		), 
		'username' => array(
			'required' => true, 
			'username' => true, 
			'query' => 'user'
		), 
		'password' => array(
			'required' => true, 
			'password' => true
		), 
		'email' => array(
			'required' => true, 
			'email' => true, 
			'query' => 'user'
		)
	);

	protected $messages = array(
		'gid' => array(
			'required' => '用户组不能为空', 
			'uinteger' => '用户组不是一个整数', 
			'min' => '请选择用户组'
		), 
		'username' => array(
			'required' => '用户名不能为空', 
			'query' => '用户名“{0}”已被注册'
		), 
		'password' => array(
			'required' => '密码不能为空'
		), 
		'email' => array(
			'required' => '电子邮件不能为空', 
			'query' => '电子邮件“{0}”已被注册'
		)
	);

	function register ( $data ) {
		$data['gid'] = USER_REG_GID;
		if ( $this->check($data) ) {
			$uid = uc_user_register($data['username'], $data['password'], $data['email'], null, null, $data['regip']);
			if ( $uid > 0 ) {
				$data['uid'] = $uid;
				$data['password'] = md5($data['password']);
				DB()->insert($this->table, $data);
				return $uid;
			} else {
				switch ( $uid ) {
					case UC_USER_CHECK_USERNAME_FAILED:
						$this->error = '用户名包含敏感字符';
						break;
					case UC_USER_USERNAME_BADWORD:
						$this->error = '用户名包含被系统屏蔽的字符';
						break;
					case UC_USER_USERNAME_EXISTS:
						$this->error = '该用户名已被注册';
						break;
					case UC_USER_EMAIL_FORMAT_ILLEGAL:
						$this->error = '电子邮件不合法';
						break;
					case UC_USER_EMAIL_ACCESS_ILLEGAL:
						$this->error = '抱歉，电子邮件包含不可使用的邮箱域名';
						break;
					case UC_USER_EMAIL_EXISTS:
						$this->error = '该 Email 地址已被注册';
						break;
				}
			}
		}
		return false;
	}

	function get_list_by_group ( $gid ) {
		$group = MOD('user.group')->get($gid);
		return DB()->select(array(
			'table' => $this->table . ' u', 
			'field' => 'u.uid,u.gid,u.username,up.nickname,up.signature', 
			'join' => array(
				// 'user_group ug'=>'u.gid=ug.gid',
				'user_profile up' => 'u.uid=up.uid'
			), 
			'where' => 'u.gid in(' . $group['userlistgroup'] . ')', 
			'order' => 'u.gid,u.onlinetime DESC'
		), SQL_SELECT_LIST);
	}

	function get_by_profile ( $uid ) {
		return DB()->select(array(
			'table' => $this->table . ' u', 
			'field' => 'u.uid,u.gid,u.username,up.nickname,up.signature',  // ,IF(uo.uid>0,1,0) as `isonline`
			'join' => array(
				// 'user_online uo'=>'u.uid=uo.uid',
				'user_profile up' => 'u.uid=up.uid'
			), 
			'where' => 'u.uid=' . ( $uid + 0 )
		), SQL_SELECT_ONLY);
	}

	function drop ( $id ) {
		$this->error = '删除用户失败！';
		if ( parent::exists($id) ) {
			return $this->drops(array(
				$id
			));
		}
		return false;
	}

	function drops ( $uids ) {
		$this->error = '批量删除用户失败！';
		$uids = ( array ) $uids;
		if (  ! $uids ) {
			return false;
		}
		$uids = DB()->select(array(
			'table' => 'user',
			'field' => 'uid',
			'where' => 'uid IN(' . iimplode($uids) . ')'
		), SQL_SELECT_LIST, null, 'uid');
		if ( $uids ) {
			uc_user_delete($uids);
			$iuids = iimplode($uids);
			// 直播
			DB()->delete('broadcast', "uid IN($iuids)");
			// 优选金股
			$ids = DB()->select(array(
				'table' => 'gold',
				'field' => 'gid',
				'where' => "uid IN($iuids)"
			), SQL_SELECT_LIST, null, 'gid');
			if ( $ids ) {
				$ids = iimplode($ids);
				DB()->delete('gold', "uid IN($iuids)");
				DB()->delete('user_gold', "uid IN($iuids) OR gid IN($ids)");
			}
			// 投资组合
			$ids = DB()->select(array(
				'table' => 'invest',
				'field' => 'iid',
				'where' => "uid IN($iuids)"
			), SQL_SELECT_LIST, null, 'iid');
			if ( $ids ) {
				$ids = iimplode($ids);
				DB()->delete('invest_stock', 'iid IN($ids)');
				DB()->delete('invest', "uid IN($iuids)");
				DB()->delete('user_invest', "uid IN($iuids) OR iid IN($ids)");
			}
			// 个股会诊
			DB()->delete('user_consult', "from_uid IN($iuids) OR to_uid IN($iuids)");
			// 用户相关
			DB()->delete('user', "uid IN($iuids)");
			DB()->delete('user_profile', "uid IN($iuids)");
			DB()->delete('user_serv', "cuid IN($iuids) OR uid IN($iuids)");
			DB()->delete('user_setting', "uid IN($iuids)");
			// 操作系统
			DB()->delete('user_stock', "uid IN($iuids)");
			return count($uids);
		} else {
			return false;
		}
	}

}
