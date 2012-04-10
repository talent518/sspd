<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModUserSetting extends ModBase{
	protected $table='user_setting';
	protected $priKey='uid';
	protected $order;
	protected $rules=array(
	);
	protected $messages=array(
	);
}
