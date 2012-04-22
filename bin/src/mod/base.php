<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

class ModBase{
	var $key,$error;
	protected $table;
	protected $priKey;
	protected $order;
	protected $rules;
	protected $messages;

	function check($data){
		$valid=LIB('validate');
		if($valid->check($data,$this->rules,$this->messages)){
			return true;
		}else{
			$this->key=$valid->key;
			$this->error=$valid->error;
			return false;
		}
	}
	function &exists($id){
		return $id!==0?DB()->select(array(
				'table'=>$this->table,
				'field'=>$this->priKey,
				'where'=>$this->priKey.'='.($id+0)
			),SQL_SELECT_ONLY,$this->priKey):0;
	}
	function &get($id){
		return $id!==0?$this->get_by_where($this->priKey.'='.($id+0)):false;
	}
	function &get_by_where($where=''){
		return DB()->select(array(
				'table'=>$this->table,
				'field'=>'*',
				'where'=>$where
			),SQL_SELECT_ONLY);
	}
	function get_list_by_where($where='',$limit=0){
		return DB()->select(array(
				'table'=>$this->table,
				'field'=>'*',
				'where'=>$where,
				'order'=>$this->order,
				'limit'=>$limit
			),SQL_SELECT_LIST);
	}
	function add(&$data,$isCheck=true,$isReplace=false){
		if(!$isCheck || $this->check($data)){
			DB()->insert($this->table,saddslashes($data),$isReplace);
			return true;
		}
		return false;
	}
	function edit($id,&$data,$isCheck=true,$isString=true){
		if($id<=0)
			return false;
		if(!$isCheck || $this->check($data)){
			DB()->update($this->table,saddslashes($data),($id+0>0)?'`'.$this->priKey.'`='.($id+0):'1>0',$isString);
			return true;
		}
		return false;
	}
	function drop($id){
		if($id<=0)
			return false;
		if($this->exists($id)==$id){
			DB()->delete($this->table,$this->priKey.'='.($id+0));
			return true;
		}
		return false;
	}
}
