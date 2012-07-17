<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

function UDB(){
	static $db;
	if(!is_object($db)){
		ssp_mutex_lock();
		$isConn=true;
		if(DB_TYPE!=UC_TYPE){
			$db=LIB('db.'.UC_TYPE);
		}else{
			if(DB_CHARSET==UC_CHARSET && DB_HOST==UC_HOST && DB_USER==UC_USER && DB_PWD==UC_PWD && DB_NAME==UC_NAME){
				$db=clone DB();
				$isConn=false;
			}else{
				$db=clone LIB('db.'.UC_TYPE);
			}
		}
		$db->charset=UC_CHARSET;
		$db->host=UC_HOST;
		$db->user=UC_USER;
		$db->pwd=UC_PWD;
		$db->name=UC_NAME;
		$db->pconnect=UC_PCONNECT;
		$db->tablepre=UC_TABLEPRE;
		if($isConn){
			$db->connect();
		}
		ssp_mutex_unlock();
	}
	return $db;
}

class ModUcBase{
	protected $table;
	protected $priKey;
	protected $forKey;
	protected $order;

	function exists($id){
		return $id!==0?$this->get($id+0,$this->priKey):0;
	}
	function exists_by_where($where,$key=''){
		return $where?$this->get_by_where($where,$key?$key:$this->priKey):0;
	}
	function get($id,$key=''){
		return $id!==0?$this->get_by_where($this->priKey.'='.($id+0),$key):false;
	}
	function get_by_where($where,$key=''){
		return UDB()->select(array(
				'table'=>$this->table,
				'field'=>$key?"`{$key}`":'*',
				'where'=>$where
			),SQL_SELECT_ONLY,$key);
	}
	function get_list_by_where($where='',$limit=0,$order=''){
		return UDB()->select(array(
				'table'=>$this->table,
				'field'=>'*',
				'where'=>is_int($where)?$this->forKey.'='.($where+0):$where,
				'order'=>$order?$order:$this->order,
				'limit'=>$limit
			),SQL_SELECT_LIST,$this->priKey);
	}
	function count($where){
		return UDB()->count($this->table,is_int($where)?$this->forKey.'='.($where+0):$where);
	}
	function add($data,$isCheck=true,$isReplace=false){
		if(!$isCheck || $this->check($data)){
			UDB()->insert($this->table,saddslashes($data),$isReplace);
			return true;
		}
		return false;
	}
	function edit($id,$data,$isCheck=true,$isString=true){
		if($id<=0)
			return false;
		if(!$isCheck || $this->check($data)){
			UDB()->update($this->table,saddslashes($data),($id+0>0)?'`'.$this->priKey.'`='.($id+0):'1>0',$isString);
			return true;
		}
		return false;
	}
	function drop($id){
		if($id<=0)
			return false;
		if($this->exists($id)==$id){
			$this->delete($id+0);
			return true;
		}
		return false;
	}
	function update($data,$where,$isString=true){
		UDB()->update($this->table,$data,is_int($where)?$this->priKey.'='.$where:$where,$isString);
	}
	function delete($where){
		UDB()->delete($this->table,is_int($where)?$this->priKey.'='.$where:$where);
	}
}
