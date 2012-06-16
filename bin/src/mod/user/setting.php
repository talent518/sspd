<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModUserSetting extends ModBase{
	protected $table='user_setting';
	protected $priKey='uid';
	protected $order;
	private $users=array();
	function get($uid,$key=''){
		if(!isset($this->users[$uid])){
			ssp_mutex_lock();
			$this->users[$uid]=parent::get($uid);
			if(empty($this->users[$uid])){
				$this->users[$uid]=null;
				unset($this->users[$uid]);
			}
			ssp_mutex_unlock();
		}
		return $key?$this->users[$uid][$key]:$this->users[$uid];
	}
	function set($uid,$key,$value=0){
		$is_exist=isset($this->users[$uid]);
		ssp_mutex_lock();
		$this->users[$uid][$key]=$value;
		ssp_mutex_unlock();
		$data=array();
		$data[$key]=$value;
		$data[$key.'_dateline']=time();
		if(!$this->exists($uid)){
			$data['uid']=$uid;
			return $this->add($data,false);
		}else{
			return $this->edit($uid,$data,false);
		}
	}
	function clean($uid){
		ssp_mutex_lock();
		$this->users[$uid]=null;
		unset($this->users[$uid]);
		ssp_mutex_unlock();
	}
}
