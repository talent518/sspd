<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModUserServ extends ModBase{
	protected $table='user_serv';
	protected $priKey='cuid';
	protected $forKey='uid';
	protected $order='unreads DESC';
}
