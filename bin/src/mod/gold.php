<?php
if (  ! defined('IN_SERVER') )
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModGold extends ModBase {

	protected $table = 'gold';

	protected $priKey = 'gid';

	protected $forKey = 'uid';

	protected $order = 'gid DESC';

	protected $rules = array(
		'title' => array(
			'required' => true
		), 
		'code' => array(
			'required' => true, 
			'uinteger' => true, 
			'length' => 6
		), 
		'name' => array(
			'required' => true
		), 
		'reason' => array(
			'required' => true
		), 
		'prompt' => array(
			'required' => true
		), 
		'buy_condition' => array(
			'required' => true
		), 
		'sell_condition' => array(
			'required' => true
		)
	);

	protected $messages = array(
		'title' => array(
			'required' => '金股标题不能为空'
		), 
		'code' => array(
			'required' => '股票代码不能为空', 
			'uinteger' => '股票代码只包括0-9的数字', 
			'length' => '股票代码长度只能为{0}'
		), 
		'name' => array(
			'required' => '股票名称不能为空'
		), 
		'reason' => array(
			'required' => '关注理由不能为空'
		), 
		'prompt' => array(
			'required' => '风险提示不能为空'
		), 
		'buy_condition' => array(
			'required' => '买入条件不能为空'
		), 
		'sell_condition' => array(
			'required' => '卖出条件由不能为空'
		)
	);

	function get_list_by_user ( $uid, $isToday = true, $limit = null ) {
		return DB()->select(array(
			'table' => $this->table . ' g', 
			'field' => 'g.*,IF(ug.`isread`=1,1,0) AS `isread`,ug.readtime', 
			'join' => array(
				'user_gold ug' => 'g.gid=ug.gid AND ug.uid=' . $uid
			), 
			'where' => ( $isToday ? 'g.dateline>' . @strtotime('today') : '' ), 
			'order' => 'ug.isread,ug.gid DESC', 
			'limit' => $limit
		), SQL_SELECT_LIST);
	}

}
