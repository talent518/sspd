<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModUserThread extends ModBase{
	protected $table='user_thread';
	protected $priKey='utid';
	protected $order;
	protected $rules=array(
	);
	protected $messages=array(
	);
}
