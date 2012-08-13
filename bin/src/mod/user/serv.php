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
	function get_list_by_uid($uid){
		return DB()->select(array(
				'table'=>$this->table.' us',
				'field'=>'us.*,IF(uo.uid,1,0) as isonline',
				'join_split'=>true,
				'join'=>array('user u'=>'u.uid=us.cuid','user_online uo'=>'uo.uid=us.cuid'),
				'where'=>'u.gid=3 AND us.uid='.$uid,
				'order'=>'IF(uo.uid,1,0) DESC,unreads DESC,CONVERT(nickname using gb2312) ASC'
			),SQL_SELECT_LIST,'cuid');
	}
}
