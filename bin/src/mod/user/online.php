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
	function get_by_user($id,$key=''){
		if(!isset($this->users[$id])){
			$user=$this->get_by_where('`uid`='.($id+0));
			if($user){
				$this->users[$id]=$user;
				if($this->users[$id][$this->priKey]>0){
					$this->clients[$this->users[$id][$this->priKey]]=&$this->users[$id];
				}
			}elseif(!extension_loaded('pcntl')){
				$this->users[$id]=array();
			}
		}
		return empty($key)?$this->users[$id]:$this->users[$id][$key];
	}
	function get_by_client($id,$key=''){
		if(!isset($this->clients[$id])){
			$this->clients[$id]=$this->get($id);
			if($this->clients[$id]['uid']>0){
				$this->users[$this->clients[$id]['uid']]=&$this->clients[$id];
			}
		}
		return empty($key)?$this->clients[$id]:$this->clients[$id][$key];
	}
	function add($data){
		$data[$this->priKey]=$data['port'];
		$this->clients[$data['port']]=$data;
		return parent::add($data,false);
	}
	function edit($id,$data){
		$uid=$this->clients[$id]['uid'];
		$this->clients[$id]=array_merge($this->clients[$id],$data);
		if($this->clients[$id]['uid']>0){
			$this->users[$this->clients[$id]['uid']]=&$this->clients[$id];
		}
		if($uid>0 && $this->clients[$id]['uid']!=$uid){
			unset($this->users[$uid]);
		}
		return parent::edit($id,$data,false);
	}
	function drop($id,$isUser=false){
		if($isUser){
			$data=array(
				'uid'=>0,
				'logintimes'=>0,
				'logintime'=>0,
				'timezone'=>0,
			);
			return $this->edit($id,$data);
		}else{
			return parent::drop($id);
		}
	}
	function clean(){
		DB()->delete($this->table,'1>0');
	}
}
