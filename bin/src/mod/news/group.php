<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModNewsGroup extends ModBase{
	protected $table='v_news_group';
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
