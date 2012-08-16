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

	protected $shmid;
	function ModUserGroup(){
		$this->shmid=ssp_attach(ftok(__FILE__,'g'),SSP_MAX_CLIENTS*1000);
	}

	function get($id,$key=false){
		if(!ssp_has_var($this->shmid,$id)){
			if(!ssp_set_var($this->shmid,$id,parent::get($id))){
				echo '$this->shmid mod.user.group:get id "',$id,'" error!',PHP_EOL;
			}
		}
		$group=ssp_get_var($this->shmid,$id);
		return $key?$group[$key]:$group;
	}
	function edit($id,$data,$isCheck=true,$isString=true){
		$group=array_replace($this->get($id),$data);
		if(!ssp_set_var($this->shmid,$id,$group)){
			echo '$this->shmid mod.user.group:edit id "',$id,'" error!',PHP_EOL;
		}
		return parent::edit($id,$data,$isCheck,$isString);
	}
	function drop($id){
		ssp_remove_var($this->shmid,$id);
		return parent::drop($id);
	}
	function clean(){
		ssp_remove($this->shmid);
	}
	function __destruct(){
		ssp_detach($this->shmid);
	}
}
