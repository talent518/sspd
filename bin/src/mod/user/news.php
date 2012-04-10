<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModUserNews extends ModBase{
	protected $table='user_news';
	protected $priKey='unid';
	protected $order;
	protected $rules=array(
	);
	protected $messages=array(
	);
	private $groups=array();
}
