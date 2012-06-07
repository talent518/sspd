<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModInvest extends ModBase{
	protected $table='invest';
	protected $priKey='iid';
	protected $forKey='uid';
	protected $order='dateline DESC,iid DESC';
	protected $rules=array(
		'title'=>array(
			'required'=>true,
		),
	);
	protected $messages=array(
		'title'=>array(
			'required'=>'金股标题不能为空',
		),
	);
}
