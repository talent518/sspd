<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('api.uc.client');
import('mod.base');

class ModNews extends ModBase{
	protected $table='v_news';
	protected $priKey='aid';
	protected $order;
	function get_tree_by_user($uid){
		return DB()->select(array(
			'table'=>$this->table.' n',
			'field'=>'n.aid,n.gid,n.title,summary,n.dateline,IF(un.`isread`=1,1,0) AS `isread`',
			'join'=>array(
				'v_news_group ng'=>'n.gid=ng.gid',
				'user_news un'=>'n.aid=un.aid AND un.uid='.$uid,
			),
			'where'=>'EXISTS (SELECT COUNT(*) FROM '.DB()->tname('v_news').' WHERE aid=n.aid AND dateline<n.dateline HAVING COUNT(*) < 12)',
			'order'=>'ng.gid,un.isread DESC,ng.gorder,n.dateline DESC',
		),SQL_SELECT_LIST);
	}
}
