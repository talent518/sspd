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

	protected $mutex;
	function ModUserSetting(){
		$this->mutex=ssp_mutex_create();
	}
	
	function get($uid,$key='',$isCache=true){
		if($isCache || isset($this->users[$uid])){
			if(!isset($this->users[$uid])){
				ssp_mutex_lock($this->mutex);
				if(!isset($this->users[$uid])){
					$this->users[$uid]=parent::get($uid);
					if(empty($this->users[$uid])){
						$this->users[$uid]=null;unset($this->users[$uid]);
					}
				}
				ssp_mutex_unlock($this->mutex);
			}
			return $key?$this->users[$uid][$key]:$this->users[$uid];
		}else{
			return parent::get($uid,$key);
		}
	}
	function set($uid,$key,$value=0){
		if(isset($this->users[$uid])){
			ssp_mutex_lock($this->mutex);
			$this->users[$uid][$key]=$value;
			ssp_mutex_unlock($this->mutex);
		}
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
		ssp_mutex_lock($this->mutex);
		$this->users[$uid]=null;unset($this->users[$uid]);
		ssp_mutex_unlock($this->mutex);
	}
}
