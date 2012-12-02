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

	protected $shmid;
	function ModUserSetting(){
		$this->shmid=ssp_attach(ftok(__FILE__,'j'),SSP_MAX_CLIENTS*512,0777);
	}

	function get($uid,$key='',$isCache=true){
		if($isCache){
			if(!ssp_has_var($this->shmid,$uid)){
				if(!ssp_set_var($this->shmid,$uid,parent::get($uid))){
					echo '$this->shmid mod.user.group:get uid "',$uid,'" error!',PHP_EOL;
				}
			}
			$group=ssp_get_var($this->shmid,$uid);
			return $key?$group[$key]:$group;
		}else{
			return parent::get($uid,$key);
		}
	}
	function set($uid,$key,$value=0){
		$data=array();
		$data[$key]=$value;
		$data[$key.'_dateline']=time();

		$setting=array_replace($this->get($uid),$data);
		if(!ssp_set_var($this->shmid,$uid,$setting)){
			echo '$this->shmid mod.user.setting:set uid "',$uid,'" error!',PHP_EOL;
		}
		
		if(!$this->exists($uid)){
			$data['uid']=$uid;
			return $this->add($data,false);
		}else{
			return $this->edit($uid,$data,false);
		}
	}
	function clean($uid=0){
		if($uid>0){
			ssp_remove_var($this->shmid,$uid);
		}else{
			ssp_remove($this->shmid);
		}
	}
	function __destruct(){
		ssp_detach($this->shmid);
	}
}
