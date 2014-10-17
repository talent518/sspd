<?php
if (  ! defined('IN_SERVER') )
	exit('Access Denied');

import('mod.uc.base');

class ModUcUser extends ModUcBase {

	protected $table = 'members';

	protected $priKey = 'uid';

	protected $order;

}
