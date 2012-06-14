<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModUserInvest extends ModBase{
	protected $table='user_invest';
	protected $priKey='uiid';
	protected $order;
}
