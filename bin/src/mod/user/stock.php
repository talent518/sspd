<?php
import('api.uc.client');
import('mod.base');

class ModUserStock extends ModBase {

	protected $table = 'user_stock';

	protected $priKey = 'sid';

	protected $forKey = 'uid';

	protected $order = 'sid,evaldate';

	protected $rules = array(
		'code' => array(
			'required' => true, 
			'uinteger' => true, 
			'length' => 6
		), 
		'name' => array(
			'required' => true
		), 
		'type' => array(
			'required' => true, 
			'uinteger' => true
		), 
		'dealdate' => array(
			'required' => true, 
			'date' => true
		), 
		'amount' => array(
			'required' => true, 
			'uinteger' => true
		), 
		'location' => array(
			'required' => true, 
			'ufloat' => true
		), 
		'price' => array(
			'required' => true, 
			'ufloat' => true
		), 
		'stoploss' => array(
			'required' => true
		), 
		'reason' => array(
			'required' => true
		), 
		'profitloss' => array(
			'required' => true, 
			'float' => true, 
			'max' => 100, 
			'min' =>  - 100
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
		'type' => array(
			'required' => '请选择操作类型', 
			'uinteger' => '请选择操作类型'
		), 
		'dealdate' => array(
			'required' => '交易日期不能为空', 
			'date' => '交易日期不是合法的日期格式'
		), 
		'amount' => array(
			'required' => '数量不能为空', 
			'uinteger' => '数量只能为正整数'
		), 
		'location' => array(
			'required' => '仓位不能为空', 
			'ufloat' => '仓位只能为无符号小数'
		), 
		'price' => array(
			'required' => '交易价位不能为空', 
			'ufloat' => '交易价位只能为无符号小数'
		), 
		'stoploss' => array(
			'required' => '止损不能为空'
		), 
		'reason' => array(
			'required' => '交易理由不能为空'
		), 
		'profitloss' => array(
			'required' => '盈亏不能为空', 
			'float' => '盈亏只能为小数', 
			'max' => '盈亏最大{0}%', 
			'min' => '盈亏最小{0}%'
		)
	);

	function add ( $data ) {
		$this->rules['profitloss']['required'] = $data['type'] > 2;
		if ( $this->check($data) ) {
			$data['dealdate'] = strtotime($data['dealdate']);
			$data['dateline'] = strtotime($data['dateline']);
			parent::add($data, false);
			return true;
		}
		return false;
	}

	function edit ( $id, $data, $isCheck = true, $isString = true ) {
		$this->rules['profitloss']['required'] = $data['type'] > 2;
		if (  ! $isCheck || $this->check($data) ) {
			$data['dealdate'] = strtotime($data['dealdate']);
			$data['dateline'] = strtotime($data['dateline']);
			parent::edit($id, $data, false, $isString);
			return true;
		}
		return false;
	}

}
