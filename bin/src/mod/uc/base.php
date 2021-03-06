<?php
function UDB () {
	if (  ! isset($_SSP['UDB']) ) {
		$isConn = true;
		if ( DB_TYPE != UC_TYPE ) {
			$_SSP['UDB'] = LIB('db.' . UC_TYPE);
		} else {
			if ( DB_CHARSET == UC_CHARSET && DB_HOST == UC_HOST && DB_USER == UC_USER && DB_PWD == UC_PWD && DB_NAME == UC_NAME ) {
				$_SSP['UDB'] = clone DB();
				$isConn = false;
			} else {
				$_SSP['UDB'] = clone LIB('db.' . UC_TYPE);
			}
		}
		$_SSP['UDB']->charset = UC_CHARSET;
		$_SSP['UDB']->host = UC_HOST;
		$_SSP['UDB']->user = UC_USER;
		$_SSP['UDB']->pwd = UC_PWD;
		$_SSP['UDB']->socket = UC_SOCKET;
		$_SSP['UDB']->name = UC_NAME;
		$_SSP['UDB']->pconnect = UC_PCONNECT;
		$_SSP['UDB']->tablepre = UC_TABLEPRE;
		if ( $isConn ) {
			$_SSP['UDB']->connect();
		}
	}
	return $_SSP['UDB'];
}

class ModUcBase {

	protected $table;

	protected $priKey;

	protected $forKey;

	protected $order;

	function exists ( $id ) {
		return $id !== 0 ? $this->get($id + 0, $this->priKey) : 0;
	}

	function exists_by_where ( $where, $key = '' ) {
		return $where ? $this->get_by_where($where, $key ? $key : $this->priKey) : 0;
	}

	function get ( $id, $key = '' ) {
		return $id !== 0 ? $this->get_by_where($this->priKey . '=' . ( $id + 0 ), $key) : false;
	}

	function get_by_where ( $where, $key = '' ) {
		return UDB()->select(array(
			'table' => $this->table, 
			'field' => $key ? "`{$key}`" : '*', 
			'where' => $where
		), SQL_SELECT_ONLY, $key);
	}

	function get_list_by_where ( $where = '', $limit = 0, $order = '' ) {
		return UDB()->select(array(
			'table' => $this->table, 
			'field' => '*', 
			'where' => is_int($where) ? $this->forKey . '=' . ( $where + 0 ) : $where, 
			'order' => $order ? $order : $this->order, 
			'limit' => $limit
		), SQL_SELECT_LIST, $this->priKey);
	}

	function count ( $where ) {
		return UDB()->count($this->table, is_int($where) ? $this->forKey . '=' . ( $where + 0 ) : $where);
	}

	function add ( $data, $isCheck = true, $isReplace = false ) {
		UDB()->insert($this->table, saddslashes($data), $isReplace);
	}

	function edit ( $id, $data, $isCheck = true, $isString = true ) {
		if ( $id <= 0 )
			return false;
		UDB()->update($this->table, saddslashes($data), ( $id + 0 > 0 ) ? '`' . $this->priKey . '`=' . ( $id + 0 ) : '1>0', $isString);
		return true;
	}

	function drop ( $id ) {
		if ( $id <= 0 )
			return false;
		if ( $this->exists($id) == $id ) {
			$this->delete($id + 0);
			return true;
		}
		return false;
	}

	function update ( $data, $where, $isString = true ) {
		UDB()->update($this->table, $data, is_int($where) ? $this->priKey . '=' . $where : $where, $isString);
	}

	function delete ( $where ) {
		UDB()->delete($this->table, is_int($where) ? $this->priKey . '=' . $where : $where);
	}

}
