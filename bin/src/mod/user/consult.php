<?php
import('api.uc.client');
import('mod.base');

class ModUserConsult extends ModBase {

	protected $table = 'user_consult';

	protected $priKey = 'ucid';

	protected $order = 'ucid ASC';

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

	function get_list ( $where ) {
		return DB()->select(
				array(
					'table' => $this->table . ' uc', 
					'field' => 'uc.*,IF(LENGTH(us.nickname),us.nickname,IF(LENGTH(up.nickname),up.nickname,u.username)) as `nickname`', 
					'join_split' => 1, 
					'join' => array(
						'user_serv us' => 'uc.from_uid=us.cuid', 
						'user u' => 'uc.from_uid=u.uid', 
						'user_profile up' => 'uc.from_uid=up.uid'
					), 
					'where' => $where, 
					'order' => $this->order
				), SQL_SELECT_LIST, $this->priKey);
	}

	function get_list_by_unread ( $uid, $tuid ) {
		return $this->get_list('uc.isread=0 AND uc.from_uid=' . $tuid . ' AND uc.to_uid=' . $uid);
	}

	function get_list_by_date ( $uid, $tuid, $date ) {
		return $this->get_list('uc.dateline>' . $date . ' AND uc.dateline<86400+' . $date . ' AND ((uc.from_uid=' . $uid . ' AND uc.to_uid=' . $tuid . ') OR (uc.from_uid=' . $tuid . ' AND uc.to_uid=' . $uid . '))');
	}

	function get_date_by_list ( $uid, $tuid ) {
		return DB()->select(array(
			'table' => $this->table, 
			'field' => 'dateday,count(' . $this->priKey . ') as `count`', 
			'group' => 'dateday', 
			'where' => '(from_uid=' . $uid . ' AND to_uid=' . $tuid . ') OR (from_uid=' . $tuid . ' AND to_uid=' . $uid . ')', 
			'order' => 'dateday DESC'
		), SQL_SELECT_LIST, 'dateday');
	}

}
