<?php
if (  ! defined('IN_SERVER') )
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModUserSetting extends ModBase {

	protected $table = 'user_setting';

	protected $priKey = 'uid';

	protected $order;

	protected $users = array();

	function get ( $uid, $key = '' ) {
		if (  ! isset($_SSP['US'][$uid]) ) {
			$_SSP['US'][$uid] = parent::get($uid);
		}
		return $key ? $_SSP['US'][$uid][$key] : $_SSP['US'][$uid];
	}

	function set ( $uid, $key, $value = 0 ) {
		$data = array();
		$data[$key] = $value;
		$data[$key . '_dateline'] = time();
		
		$_SSP['US'][$uid] = array_replace($this->get($uid), $data);
		
		if (  ! $this->exists($uid) ) {
			$data['uid'] = $uid;
			return $this->add($data, false);
		} else {
			return $this->edit($uid, $data, false);
		}
	}

}
