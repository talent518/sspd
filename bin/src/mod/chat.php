<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModChat extends ModBase{
	protected $table='chat';
	protected $priKey='chat_id';
	protected $order;
}
