<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('mod.base');

class ModThreadField extends ModBase{
	protected $table='v_thread_field';
	protected $priKey='tid';
	protected $order='';
	private $groups=array();
	function get($id){
		if(!isset($groups[$id])){
			$groups[$id]=parent::get($id);
		}
		return $groups[$id];
	}
}
