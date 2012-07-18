<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModUserOnline extends ModBase{
	protected $table='user_online';
	protected $priKey='onid';
	protected $order;
	private $clients=array(),$users=array();
	
	protected $mutex;
	function ModUserOnline(){
		$this->mutex=ssp_mutex_create();
	}

	function get_by_user($id,$key=''){
		return $this->users[$id]?$this->get_by_client($this->users[$id],$key):null;
	}
	function get_by_client($id,$key=''){
		return empty($key)?$this->clients[$id]:$this->clients[$id][$key];
	}
	function add($data){
		ssp_mutex_lock($this->mutex);
		$this->clients[$data[$this->priKey]]=$data;
		ssp_mutex_unlock($this->mutex);
		return parent::add($data,false);
	}
	function edit($id,$data){
		ssp_mutex_lock($this->mutex);
		$_uid=$this->clients[$id]['uid'];
		$this->users[$_uid]=null;unset($this->users[$_uid]);
		$this->clients[$id]=array_replace($this->clients[$id],$data);
		$uid=$this->clients[$id]['uid'];
		if($uid>0){
			$this->users[$uid]=$id;
		}
		ssp_mutex_unlock($this->mutex);
		return parent::edit($id,$data,false);
	}
	function drop($id,$isUser=false){
		$uid=$this->clients[$id]['uid'];
		MOD('user.setting')->clean($uid);
		if($isUser){
			$data=array(
				'uid'=>0,
				'gid'=>0,
				'logintimes'=>0,
				'logintime'=>0,
				'timezone'=>0,
			);
			return $this->edit($id,$data);
		}else{
			ssp_mutex_lock($this->mutex);
			$this->clients[$id]=$this->users[$uid]=null;unset($this->clients[$id],$this->users[$uid]);
			ssp_mutex_unlock($this->mutex);
			return parent::drop($id);
		}
	}
	function clean(){
		foreach(array_keys($this->clients) as $id){
			$this->drop($id);
		}
		ssp_mutex_lock($this->mutex);
		$this->clients=$this->users=array();
		ssp_mutex_unlock($this->mutex);
		$this->delete('1>0');
	}
}
