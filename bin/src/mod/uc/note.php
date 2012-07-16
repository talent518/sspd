<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('mod.uc.base');

class ModUcNote extends ModUcBase{
	protected $table='notelist';
	protected $priKey='noteid';
	protected $order;

	function add($operation, $getdata='', $postdata='', $appids=array(), $pri = 0) {
		$data=array(
			'operation'=>$operation,
			'getdata'=>$getdata,
			'postdata'=>$postdata,
			'pri'=>$pri,
			'dateline'=>time(),
			'closed'=>1,
		);
		foreach(MOD('uc.apps')->gets() as $appid => $app) {
			$data['app'.$appid]=1;
		}
		parent::add($data);
		if($insert_id=UDB()->insert_id()){
			$this->edit($insert_id,array('totalnum'=>'totalnum+1','succeednum'=>'succeednum+1'),false,false);
		}
		return $insert_id;
	}
}
