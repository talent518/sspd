<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModUserServGroup extends ModBase{
	protected $table='user_serv_group';
	protected $priKey='gid';
	protected $order;
	protected $rules=array(
	);
	protected $messages=array(
	);
	private $groups=array();
	function get($id){
		if(!isset($this->groups[$id])){
			ssp_mutex_lock();
			$this->groups[$id]=parent::get($id);
			ssp_mutex_unlock();
		}
		return $this->groups[$id];
	}
	function edit($id,$data,$isCheck=true,$isString=true){
		ssp_mutex_lock();
		$this->groups[$id]=array_replace($this->groups[$id],$data);
		ssp_mutex_unlock();
		return parent::edit($id,$data,$isCheck,$isString);
	}
	function drop($id){
		ssp_mutex_lock();
		$this->groups[$id]=null;
		unset($this->groups[$id]);
		ssp_mutex_unlock();
		return parent::drop($id);
	}
}
