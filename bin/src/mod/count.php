<?php
class ModCount {

	function get () {
		return array(
			'user' => DB()->count('user'), 
			'stock' => DB()->count('user_stock'), 
			'gold' => DB()->count('gold'), 
			'invest' => DB()->count('invest'), 
			'broadcast' => DB()->count('broadcast'), 
			'consult' => DB()->count('user_consult'), 
			'client' => DB()->count('user_serv', 'uid!=1'), 
			'online' => DB()->count('user_online', 'uid>0') . '/' . DB()->count('user_online')
		);
	}

	function today () {
		$time = strtotime('today');
		return array(
			'regUser' => DB()->count('user', 'regtime>' . $time), 
			'activeUser' => DB()->count('user', 'logtime>' . $time), 
			'stock' => DB()->count('user_stock', 'dateline>' . $time), 
			'gold' => DB()->count('gold', 'dateline>' . $time), 
			'invest' => DB()->count('invest', 'dateline>' . $time), 
			'broadcast' => DB()->count('broadcast', 'dateline>' . $time), 
			'consult' => DB()->count('user_consult', 'dateline>' . $time)
		);
	}

	function week () {
		$time = strtotime('-1 week');
		return array(
			'regUser' => DB()->count('user', 'regtime>' . $time), 
			'activeUser' => DB()->count('user', 'logtime>' . $time), 
			'stock' => DB()->count('user_stock', 'dateline>' . $time), 
			'gold' => DB()->count('gold', 'dateline>' . $time), 
			'invest' => DB()->count('invest', 'dateline>' . $time), 
			'broadcast' => DB()->count('broadcast', 'dateline>' . $time), 
			'consult' => DB()->count('user_consult', 'dateline>' . $time)
		);
	}

	private function gold ( $uid ) {
		return DB()->select(array(
			'table' => 'gold g', 
			'field' => 'count(g.gid) as `count`', 
			'join' => array(
				'user_gold ug' => 'g.gid=ug.gid AND ug.uid=' . $uid
			), 
			'where' => 'ug.isread IS NULL'
		), SQL_SELECT_ONLY, 'count') + 0;
	}

	private function invest ( $uid ) {
		return DB()->select(array(
			'table' => 'invest i', 
			'field' => 'count(i.iid) as `count`', 
			'join' => array(
				'user_invest ui' => 'i.iid=ui.iid AND ui.uid=' . $uid
			), 
			'where' => 'ui.isread IS NULL'
		), SQL_SELECT_ONLY, 'count') + 0;
	}

	function remind ( $uid ) {
		return array(
			'os' => DB()->count('user_stock', UGK($uid, 'stock_eval') ? 'evaluid=0 OR evaluid=-' . $uid : 'evaluid>0 AND isread=0 AND uid=' . $uid), 
			'gold' => $this->gold($uid), 
			'invest' => $this->invest($uid), 
			'consult' => ( UGK($uid, 'consult_reply') || UGK($uid, 'consult_ask') ) ? MOD('user.consult')->count('isread=0 AND to_uid=' . $uid) : 0
		);
	}

	function runtime () {
		$dbsize = 0;
		$query = DB()->query('SHOW TABLE STATUS LIKE \'' . DB()->tablepre . '%\'');
		while ( ( $table = DB()->row($query) ) !== false ) {
			$dbsize += $table['Data_length'] + $table['Index_length'];
		}
		return array(
			'mysql' => 'MySQL ' . DB()->version(), 
			'dbsize' => formatsize($dbsize)
		);
	}

}
