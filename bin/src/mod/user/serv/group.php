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
		'name'=>array(
			'required'=>true,
			'chinese'=>true,
			'query'=>'user_serv_group',
		),
	);
	protected $messages=array(
		'name'=>array(
			'required'=>'客户分组名称不能为空！',
			'chinese'=>'客户分组名称只能包括中文和英文、数字和非特殊符号',
			'query'=>'客户分组名称已经存在！',
		),
	);
	private $groups=array();
	
	protected $mutex;
	function ModUserServGroup(){
		$this->mutex=ssp_mutex_create();
	}

	function get($id){
		if(!isset($this->groups[$id])){
			ssp_mutex_lock($this->mutex);
			if(!isset($this->groups[$id])){
				$this->groups[$id]=parent::get($id);
			}
			ssp_mutex_unlock($this->mutex);
		}
		return $this->groups[$id];
	}
	function edit($id,$data,$isCheck=true,$isString=true){
		ssp_mutex_lock($this->mutex);
		$this->groups[$id]=array_replace($this->groups[$id],$data);
		ssp_mutex_unlock($this->mutex);
		return parent::edit($id,$data,$isCheck,$isString);
	}
	function drop($id){
		ssp_mutex_lock($this->mutex);
		$this->groups[$id]=null;
		unset($this->groups[$id]);
		ssp_mutex_unlock($this->mutex);
		return parent::drop($id);
	}
}
