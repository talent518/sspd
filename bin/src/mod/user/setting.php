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
		if(!$this->users[$uid]){
			$this->users[$uid]=parent::get($uid);
		}
		return $key?$this->users[$uid][$key]:$this->users[$uid];
	}
	function set($uid,$key,$value=0){
		$this->users[$uid][$key]=$value;
		$data=array();
		$data[$key]=$value;
		$data[$key.'_dateline']=time();
		if(empty($this->users[$uid])){
			$data['uid']=$uid;
			return $this->add($data,false);
		}else{
			return $this->edit($uid,$data,false,false);
		}
	}
}
