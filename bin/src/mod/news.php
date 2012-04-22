<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModNews extends ModBase{
	protected $table='v_news';
	protected $priKey='aid';
	protected $order;
	function get_list_by_user($uid,$gid=0,$limit=null){
		return DB()->select(array(
			'table'=>$this->table.' n',
			'field'=>'n.aid,n.gid,n.author,n.title,summary,n.dateline,IF(un.`isread`=1,1,0) AS `isread`,un.readtime',
			'join'=>array(
				'user_news un'=>'n.aid=un.aid AND un.uid='.$uid,
			),
			'where'=>($gid>0?'n.gid='.$gid:''),
			'order'=>'un.isread,n.dateline DESC',
			'limit'=>$limit,
		),SQL_SELECT_LIST);
	}
	function get_user_by_reads($uid){
		return DB()->select(array(
			'table'=>$this->table.' n',
			'field'=>'n.gid,count(n.aid) as counts',
			'join'=>array(
				'user_news un'=>'n.aid=un.aid',
			),
			'where'=>'un.uid='.$uid,
			'group'=>'n.gid',
		),SQL_SELECT_LIST,'gid','counts');
	}
}
