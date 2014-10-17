<?php
if (  ! defined('IN_SERVER') )
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModInvestStock extends ModBase {

	protected $table = 'invest_stock';

	protected $priKey = 'isid';

	protected $forKey = 'iid';

	protected $order = 'isid';

	protected $rules = array(
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
		'think' => array(
			'required' => true
		)
	);

	protected $messages = array(
		'code' => array(
			'required' => '股票代码不能为空', 
			'uinteger' => '股票代码只包括0-9的数字', 
			'length' => '股票代码长度只能为{0}'
		), 
		'name' => array(
			'required' => '股票名称不能为空'
		), 
		'reason' => array(
			'required' => '选入理由不能为空'
		), 
		'think' => array(
			'required' => '操作思路不能为空'
		)
	);

}
