<?php
if(!defined('IN_SERVER'))
exit('Access Denied');

class ModCount{
	function &get(){
		return array(
			'user'=>DB()->count('user'),
		);
	}

	function &week(){
		return array(
			'regUser'=>DB()->count('user','regtime>'.strtotime('-1 week')),
			'activeUser'=>DB()->count('user','lastactivetime>'.strtotime('-1 week')),
		);
	}

	private function gold($uid){
		return DB()->select(array(
			'table'=>'gold g',
			'field'=>'count(g.gid) as `count`',
			'join'=>array('user_gold ug'=>'g.gid=ug.gid AND ug.uid='.$uid),
			'where'=>'ug.isread IS NULL',
		),SQL_SELECT_ONLY,'count')+0;
	}

	private function invest($uid){
		return DB()->select(array(
			'table'=>'invest i',
			'field'=>'count(i.iid) as `count`',
			'join'=>array('user_invest ui'=>'i.iid=ui.iid AND ui.uid='.$uid),
			'where'=>'ui.isread IS NULL',
		),SQL_SELECT_ONLY,'count')+0;
	}

	function &remind($uid){
		return array(
			'os'=>DB()->count('user_stock',UGK($uid,'stock_eval')?'evaluid=0 OR evaluid=-'.$uid:'evaluid>0 AND isread=0 AND uid='.$uid),
			'gold'=>$this->gold($uid),
			'invest'=>$this->invest($uid),
			'consult'=>(UGK($uid,'consult_reply') || UGK($uid,'consult_ask'))?MOD('user.consult')->count('isread=0 AND to_uid='.$uid):0,
		);
	}

	function &runtime(){
		$dbsize = 0;
		$query=DB()->query("SHOW TABLE STATUS LIKE '{DB()->tablepre}%'");
		while($table=DB()->row($query)){
			$dbsize+=$table['Data_length']+$table['Index_length'];
		}
		return array(
			'mysql'=>DB()->version(),
			'dbsize'=>formatsize($dbsize),
		);
	}
}
