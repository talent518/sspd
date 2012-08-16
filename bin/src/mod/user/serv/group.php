<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModUserServGroup extends ModBase{
	protected $table='user_serv_group';
	protected $priKey='gid';
	protected $order;
	protected $rules=array(
		'name'=>array(
			'required'=>true,
			'chinese'=>true,
			'query'=>'user_serv_group',
		),
	);
	protected $messages=array(
		'name'=>array(
			'required'=>'客户分组名称不能为空！',
			'chinese'=>'客户分组名称只能包括中文和英文、数字和非特殊符号',
			'query'=>'客户分组名称已经存在！',
		),
	);
}
