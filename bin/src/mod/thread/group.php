<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('mod.base');

class ModThreadGroup extends ModBase{
	protected $table='v_thread_group';
	protected $priKey='gid';
	protected $order;
	protected $rules=array(
	);
	protected $messages=array(
	);
	private $groups=array();
	function &get($id){
		if(!isset($groups[$id])){
			$groups[$id]=parent::get($id);
		}
		return $groups[$id];
	}
}
