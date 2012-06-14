<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModUserGold extends ModBase{
	protected $table='user_gold';
	protected $priKey='ugid';
	protected $order;
}
