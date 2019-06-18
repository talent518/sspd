<?php
import('api.uc.client');
import('mod.base');

class ModUserProfile extends ModBase {

	protected $table = 'user_profile';

	protected $priKey = 'uid';

	protected $order;

	protected $rules = array();

	protected $messages = array();

}
