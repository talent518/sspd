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
			ssp_mutex_lock();
			if($user){
				$this->users[$id]=$user;
				if($this->users[$id][$this->priKey]>0){
					$this->clients[$this->users[$id][$this->priKey]]=$this->users[$id];
				}
			}else{
				$this->users[$id]=array();
			}
			ssp_mutex_unlock();
		}
		return empty($key)?$this->users[$id]:$this->users[$id][$key];
	}
	function get_by_client($id,$key=''){
		if(!isset($this->clients[$id])){
			ssp_mutex_lock();
			$this->clients[$id]=$this->get($id);
			if($this->clients[$id]['uid']>0){
				$this->users[$this->clients[$id]['uid']]=$this->clients[$id];
			}
			ssp_mutex_unlock();
		}
		return empty($key)?$this->clients[$id]:$this->clients[$id][$key];
	}
	function add($data){
		$this->clients[$data['onid']]=$data;
		return parent::add($data,false);
	}
	function edit($id,$data){
		ssp_mutex_lock();
		$this->clients[$id]=array_replace($this->clients[$id],$data);
		$uid=$this->clients[$id]['uid'];
		if($uid>0){
			$this->users[$uid]=$this->clients[$id];
		}
		ssp_mutex_unlock();
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
			ssp_mutex_lock();
			$this->clients[$id]=$this->users[$uid]=null;
			unset($this->clients[$id],$this->users[$uid]);
			ssp_mutex_unlock();
			return parent::drop($id);
		}
	}
	function clean(){
		$this->clients=$this->users=array();
		DB()->delete($this->table,'1>0');
	}/*
	function get_list_by_where($where='',$limit=0){
		$query=DB()->select(array(
				'table'=>$this->table,
				'field'=>'*',
				'where'=>$where,
				'order'=>$this->order,
				'limit'=>$limit
			),SQL_SELECT_QUERY);
		while($row=DB()->row($query)){
			$list[$row[$this->priKey]]=$this->users[$row['uid']]=$this->clients[$row[$this->priKey]]=$row;
		}
		return $list;
	}*/
}
