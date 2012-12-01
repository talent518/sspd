<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

class ModBase{
	var $key,$error;
	protected $table;
	protected $priKey;
	protected $forKey;
	protected $order;
	protected $rules;
	protected $messages;

	function check($data){
		if(empty($this->rules)){
			return true;
		}
		$valid=clone LIB('validate');
		if($valid->check($data,$this->rules,$this->messages)){
			return true;
		}else{
			$this->key=$valid->key;
			$this->error=$valid->error;
			return false;
		}
	}
	function exists($id){
		return $id!==0?$this->exists_by_where($this->priKey.'='.($id+0)):0;
	}
	function exists_by_where($where){
		return $where?DB()->select(array(
				'table'=>$this->table,
				'field'=>$this->priKey,
				'where'=>$where,
			),SQL_SELECT_ONLY,$this->priKey):0;
	}
	function get($id,$key=''){
		return $id!==0?$this->get_by_where($this->priKey.'='.($id+0),$key):false;
	}
	function get_by_where($where='',$key=''){
		return DB()->select(array(
				'table'=>$this->table,
				'field'=>$key?"`{$key}`":'*',
				'where'=>$where
			),SQL_SELECT_ONLY,$key);
	}
	function get_list_by_where($where='',$limit=0,$order=''){
		return DB()->select(array(
				'table'=>$this->table,
				'field'=>'*',
				'where'=>is_int($where)?$this->forKey.'='.($where+0):$where,
				'order'=>$order?$order:$this->order,
				'limit'=>$limit
			),SQL_SELECT_LIST,$this->priKey);
	}
	function count($where){
		return DB()->count($this->table,is_int($where)?$this->forKey.'='.($where+0):$where);
	}
	function add($data,$isCheck=true,$isReplace=false){
		if(!$isCheck || $this->check($data)){
			DB()->insert($this->table,saddslashes($data),$isReplace);
			return true;
		}
		return false;
	}
	function edit($id,$data,$isCheck=true,$isString=true){
		if($id<=0){
			$this->error='信息ID号不合法';
			return false;
		}
		if(!$isCheck || $this->check($data)){
			DB()->update($this->table,saddslashes($data),($id+0>0)?'`'.$this->priKey.'`='.($id+0):'1>0',$isString);
			return true;
		}
		return false;
	}
	function drop($id){
		if($id<=0){
			$this->error='信息ID号不合法';
			return false;
		}
		if($this->exists($id)==$id){
			$this->delete($id+0);
			return true;
		}
		return false;
	}
	function update($data,$where,$isString=true){
		DB()->update($this->table,$data,is_int($where)?$this->priKey.'='.$where:$where,$isString);
	}
	function delete($where){
		DB()->delete($this->table,is_int($where)?$this->priKey.'='.$where:$where);
	}
}
