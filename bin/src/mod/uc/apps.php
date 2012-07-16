<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('mod.uc.base');

class ModUcApps extends ModUcBase{
	protected $table='applications';
	protected $priKey='appid';
	protected $order;
	function gets(){
		static $apps;
		if(!$apps){
			$apps=$this->get_list_by_where();
		}
		return $apps;
	}
}
