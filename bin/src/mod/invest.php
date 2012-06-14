<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModInvest extends ModBase{
	protected $table='invest';
	protected $priKey='iid';
	protected $forKey='uid';
	protected $order='iid DESC';
	protected $rules=array(
		'title'=>array(
			'required'=>true,
		),
	);
	protected $messages=array(
		'title'=>array(
			'required'=>'金股标题不能为空',
		),
	);
	function get_list_by_user($uid,$isToday=true,$limit=null){
		return DB()->select(array(
			'table'=>$this->table.' i',
			'field'=>'i.*,IF(ui.`isread`=1,1,0) AS `isread`,ui.readtime',
			'join'=>array(
				'user_invest ui'=>'i.iid=ui.iid AND ui.uid='.$uid,
			),
			'where'=>($isToday?'i.dateline>'.@strtotime('today'):''),
			'order'=>'ui.isread,i.iid DESC',
			'limit'=>$limit,
		),SQL_SELECT_LIST);
	}
}
