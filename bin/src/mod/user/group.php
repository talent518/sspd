<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModUserGroup extends ModBase{
	protected $table='user_group';
	protected $priKey='gid';
	protected $order;
	protected $rules=array(
		'gname'=>array(
			'required'=>true,
			'english'=>true,
			'query'=>'user_group',
		),
		'title'=>array(
			'required'=>true,
			'chinese'=>true,
			'query'=>'user_group',
		),
	);
	protected $messages=array(
		'gname'=>array(
			'required'=>'用户组名不能为空！',
			'english'=>'用户组名只能包括英文字母、数字和非特殊符号！',
			'query'=>'用户组名已经存在！',
		),
		'title'=>array(
			'required'=>'用户组标题不能为空！',
			'chinese'=>'用户组标题只能包括中文和英文、数字和非特殊符号',
			'query'=>'用户组标题已经存在！',
		),
	);
	private $groups=array();

	protected $mutex;
	function ModUserGroup(){
		$this->mutex=ssp_mutex_create();
	}

	function get($id,$key=false){
		if(!isset($this->groups[$id])){
			ssp_mutex_lock($this->mutex);
			$this->groups[$id]=parent::get($id);
			ssp_mutex_unlock($this->mutex);
		}
		return $key?$this->groups[$id][$key]:$this->groups[$id];
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
