<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModThread extends ModBase{
	protected $table='v_thread';
	protected $priKey='tid';
	protected $order;
	function get_list_by_user($uid,$gid=0,$limit=null){
		return DB()->select(array(
			'table'=>$this->table.' t',
			'field'=>'t.tid,t.gid,t.author,t.title,t.content,t.dateline,IF(ut.`isread`=1,1,0) AS `isread`,ut.readtime',
			'join'=>array(
				'user_thread ut'=>'t.tid=ut.tid AND ut.uid='.$uid,
			),
			'where'=>($gid>0?'t.gid='.$gid:''),
			'order'=>'ut.isread,t.dateline DESC',
			'limit'=>$limit,
		),SQL_SELECT_LIST);
	}
	function get_user_by_reads($uid){
		return DB()->select(array(
			'table'=>$this->table.' t',
			'field'=>'t.gid,count(t.tid) as counts',
			'join'=>array(
				'user_thread ut'=>'t.tid=ut.tid',
			),
			'where'=>'ut.uid='.$uid,
			'group'=>'t.gid',
		),SQL_SELECT_LIST,'gid','counts');
	}
}
