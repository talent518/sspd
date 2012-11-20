<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');
import('lib.cache');

class ModUserOnline extends ModBase{
	protected $table='user_online';
	protected $priKey='onid';
	protected $order;

	protected $ushmid,$cshmid;
	function __construct(){
		echo __CLASS__,':',__FUNCTION__,PHP_EOL;
		$this->ushmid=ssp_attach(ftok(__FILE__,'u'),SSP_MAX_CLIENTS*20,0777);
		$this->cshmid=ssp_attach(ftok(__FILE__,'c'),SSP_MAX_CLIENTS*500,0777);
	}

	function get_by_user($id,$key=''){
		$haved=ssp_has_var($this->ushmid,$id);
		return $haved?$this->get_by_client(ssp_get_var($this->ushmid,$id),$key):null;
	}
	function get_by_client($id,$key=''){
		$client=ssp_get_var($this->cshmid,$id);
		return empty($key)?$client:$client[$key];
	}
	function add($data){
		if(!ssp_set_var($this->cshmid,$data[$this->priKey],$data)){
			echo '$this->cshmid mod.user.online:add id "',$data[$this->priKey],'" error!',PHP_EOL;
		}
		return parent::add($data,false);
	}
	function edit($id,$data){
		$_uid=$this->get_by_client($id,'uid');
		ssp_remove_var($this->ushmid,$_uid);
		$client=array_replace($this->get_by_client($id),$data);
		if(!ssp_set_var($this->cshmid,$id,$client)){
			echo '$this->cshmid mod.user.online:edit id "',$id,'" error!',PHP_EOL;
		}
		$uid=$client['uid'];
		if($uid>0){
			if(!ssp_set_var($this->ushmid,$uid,$id)){
				echo '$this->ushmid mod.user.online:edit id "',$uid,'" error!',PHP_EOL;
			}
		}
		return parent::edit($id,$data,false);
	}
	function drop($id,$isUser=false){
		$uid=$this->get_by_client($id,'uid');
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
			ssp_remove_var($this->ushmid,$uid);
			ssp_remove_var($this->cshmid,$id);
			return parent::drop($id);
		}
	}
	function clean(){
		MOD('user.setting')->clean(0);
		ssp_remove($this->ushmid);
		ssp_remove($this->cshmid);
		$this->delete('1>0');
	}
	function __destruct(){
		echo __CLASS__,':',__FUNCTION__,PHP_EOL;
		ssp_detach($this->ushmid);
		ssp_detach($this->cshmid);
	}
}
