<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('mod.uc.base');

class ModUcApps extends ModUcBase{
	protected $table='applications';
	protected $priKey='appid';
	protected $order;
	function gets(){
		if(!$_SSP['UC_API']){
			$_SSP['UC_API']=$this->get_list_by_where();
		}
		return $_SSP['UC_API'];
	}
}
