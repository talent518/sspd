<?php
if (  ! defined('IN_SERVER') )
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModBroadcast extends ModBase {

	protected $table = 'broadcast';

	protected $priKey = 'bid';

	protected $order = 'bid ASC';

	protected $rules = array(
		'message' => array(
			'required' => true
		)
	);

	protected $messages = array(
		'message' => array(
			'required' => '消息内容不能为空！'
		)
	);

	function get_list_by_date ( $date ) {
		return DB()->select(array(
			'table' => $this->table . ' b', 
			'field' => 'b.*,IF(LENGTH(up.nickname),up.nickname,u.username) as `nickname`', 
			'join_split' => 1, 
			'join' => array(
				'user u' => 'b.uid=u.uid', 
				'user_profile up' => 'b.uid=up.uid'
			), 
			'where' => 'dateline>' . $date . ' AND dateline<86400+' . $date, 
			'order' => $this->order
		), SQL_SELECT_LIST, $this->priKey);
	}

	function get_date_by_list () {
		return DB()->select(array(
			'table' => $this->table, 
			'field' => 'dateday,count(' . $this->priKey . ') as `count`', 
			'group' => 'dateday', 
			'order' => 'dateday DESC'
		), SQL_SELECT_LIST, 'dateday');
	}

}
