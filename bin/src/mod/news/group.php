<?php
import('api.uc.client');
import('mod.base');

class ModNewsGroup extends ModBase {

	protected $table = 'v_news_group';

	protected $priKey = 'gid';

	protected $order;

	protected $rules = array();

	protected $messages = array();

	function &get ( $id ) {
		if (  ! isset($_SSP['NG'][$id]) ) {
			$_SSP['NG'][$id] = parent::get($id);
		}
		return $_SSP['NG'][$id];
	}

}
