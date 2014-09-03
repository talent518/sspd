<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModUserOnline extends ModBase{
	protected $table='user_online';
	protected $priKey='id';
	protected $order;

	protected $onlines=array();
	protected $relations=array();

	function get_by_user($id,$key=''){
		if(!array_key_exists($this->relations,$id)) {
			$data=$this->get_by_where('uid='.$id);
			$this->onlines[$data[$this->priKey]]=$data;
			$this->relations[$id]=$data[$this->priKey];
		}
		return $this->get_by_client($this->relations[$id],$key);
	}
	function get_by_client($id,$key=''){
		if(!array_key_exists($this->onlines,$id)) {
			$this->onlines[$id]=$this->get($id);
			$uid = $this->onlines[$id]['uid'];
			if($uid) {
				$this->relations[$uid]=$id;
			}
		}
		return empty($key)?$this->onlines[$id]:$this->onlines[$id][$key];
	}
	function add($data){
		$id=$data[$this->priKey];
		$this->onlines[$id]=$data;
		if($data['uid']) {
			$this->relations[$data['uid']]=$id;
		}
		return parent::add($data,false);
	}
	function edit($id,$data){
		$_uid=$this->get_by_client($id,'uid');
		$this->relations[$_uid]=null;
		$this->onlines[$id]=$client=array_replace($this->onlines[$id],$data);
		$uid=$client['uid'];
		if($uid){
			$this->relations[$uid]=$id;
		}
		return parent::edit($id,$data,false);
	}
	function drop($id,$isUser=false){
		$uid=$this->get_by_client($id,'uid');
		if($isUser){
			$data=array(
				'uid'=>0,
				'gid'=>0,
				'logintimes'=>0,
				'logintime'=>0,
				'timezone'=>0,
			);
			$this->onlines[$id]=$client=array_replace($this->onlines[$id],$data);
			$this->relations[$uid]==null;
			return $this->edit($id,$data);
		}else{
			$this->onlines[$id]=null;
			$this->relations[$uid]==null;
			return parent::drop($id);
		}
	}
	function clean(){
		$this->delete('1>0');
	}
}
