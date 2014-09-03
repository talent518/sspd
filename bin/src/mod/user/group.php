<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

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

	protected $groups=array();
	function get($id,$key=false){
		if(!isset($this->groups[$id])){
			$this->groups[$id]=parent::get($id);
		}
		return $key?$this->groups[$id][$key]:$this->groups[$id];
	}
	function edit($id,$data,$isCheck=true,$isString=true){
		$this->groups[$id]=array_replace($this->groups[$id],$data);
		$this->rules['gname']['query']=array('user_group',sprintf('gname=\'%s\' AND gid!=%d',$data['gname'],$id));
		$this->rules['title']['query']=array('user_group',sprintf('title=\'%s\' AND gid!=%d',$data['title'],$id));
		return parent::edit($id,$data,$isCheck,$isString);
	}
	function drop($id){
		$this->groups[$id]=null;
		if($ret=parent::drop($id)){
			MOD('user')->drops(DB()->select(array('table'=>'user','field'=>'uid','where'=>'gid='.$id),SQL_SELECT_LIST,null,'uid'));
		}
		return $ret;
	}
}
