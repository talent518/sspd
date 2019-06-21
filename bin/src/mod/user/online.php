<?php
import('api.uc.client');
import('mod.base');

class ModUserOnline extends ModBase {

	protected $table = 'user_online';

	protected $priKey = 'id';

	protected $order;

	function get_list_by_uid_not_id ( $uid, $id ) {
		return DB()->select(array(
			'table' => $this->table, 
			'field' => '*', 
			'where' => 'uid=' . $uid . ' AND ' . $this->priKey . '<>' . $id, 
		), SQL_SELECT_LIST, '', $this->priKey);
	}

	function get_by_user ( $id, $key = '' ) {
		if (  ! isset($_SSP['UOR'][$id]) ) {
			$data = $this->get_by_where('uid=' . $id);
			$_SSP['UO'][$data[$this->priKey]] = $data;
			$_SSP['UOR'][$id] = $data[$this->priKey];
		}
		return $this->get_by_client($_SSP['UOR'][$id], $key);
	}

	function get_by_client ( $id, $key = '' ) {
		if (  ! isset($_SSP['UO'][$id]) ) {
			$_SSP['UO'][$id] = $this->get($id);
			$uid = $_SSP['UO'][$id]['uid'];
			if ( $uid ) {
				$_SSP['UOR'][$uid] = $id;
			}
		}
		return empty($key) ? $_SSP['UO'][$id] : $_SSP['UO'][$id][$key];
	}

	function add ( $data ) {
		$id = $data[$this->priKey];
		$_SSP['UO'][$id] = $data;
		if ( $data['uid'] ) {
			$_SSP['UOR'][$data['uid']] = $id;
		}
		return parent::add($data, false);
	}

	function edit ( $id, $data ) {
		$_uid = $this->get_by_client($id, 'uid');
		$_SSP['UOR'][$_uid] = null;
		$_SSP['UO'][$id] = $client = array_replace($_SSP['UO'][$id], $data);
		$uid = $client['uid'];
		if ( $uid ) {
			$_SSP['UOR'][$uid] = $id;
		}
		return parent::edit($id, $data, false);
	}

	function drop ( $id, $isUser = false ) {
		$uid = $this->get_by_client($id, 'uid');
		if ( $isUser ) {
			$data = array(
				'uid' => 0, 
				'gid' => 0, 
				'logintimes' => 0, 
				'logintime' => 0, 
				'timezone' => 0
			);
			$_SSP['UO'][$id] = $client = array_replace($_SSP['UO'][$id], $data);
			$_SSP['UOR'][$uid] == null;
			return $this->edit($id, $data);
		} else {
			$_SSP['UO'][$id] = null;
			$_SSP['UOR'][$uid] == null;
			return parent::drop($id);
		}
	}

	function clean () {
		$this->delete('1>0');
	}

}
