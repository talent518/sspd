<?php
if (  ! defined('IN_SERVER') )
	exit('Access Denied');

import('lib.db.base');

class LibDbMysql extends LibDbBase {

	private $link;

	function __construct () {
		extension_loaded('mysql') or die('mysql:Extension does not exist!');
	}

	function connect ( $silent = FALSE ) {
		$func = ( $this->pconnect ? 'mysql_pconnect' : 'mysql_connect' );
		if (  ! ( $this->link = @$func($this->host, $this->user, $this->pwd) ) &&  ! $silent )
			$this->halt('Can not connect to MySQL server');
		
		if ( $this->version() > '4.1' ) {
			if ( $this->charset )
				@mysql_query("SET character_set_connection=$this->charset,character_set_results=$this->charset,character_set_client=binary", $this->link);
			
			if ( $this->version() > '5.0.1' )
				@mysql_query("SET sql_mode=''", $this->link);
		}
		
		if ( $this->name &&  ! $silent &&  ! $this->sdb($this->name) )
			$this->halt('Database does not exist');
		
		return is_resource($this->link) ? true : false;
	}

	function ping () {
		return @mysql_ping($this->link);
	}

	function sdb ( $name ) {
		return @mysql_select_db($name, $this->link);
	}

	function cdb ( $name ) {
		return $this->query('CREATE DATABASE `' . $name . '` CHARACTER SET ' . $this->charset, TRUE) && $this->sdb($name);
	}

	function query ( $sql, $silent = FALSE, $retry = FALSE ) {
		if ( ( $query = @mysql_query($sql, $this->link) ) == FALSE &&  ! $silent ) {
			if ( in_array($this->errno(), array(
				2006, 
				2013
			)) && $retry === FALSE ) {
				if (  ! $this->ping() ) {
					$this->connect();
					return $this->query($sql, $silent, TRUE);
				}
			}
			$this->halt('MySQL Query Error', $sql);
		}
		return $query;
	}

	function row ( $query ) {
		if ( is_string($query) ) {
			$query = $this->query($query, 1);
		}
		return mysql_fetch_assoc($query);
	}

	function arows () {
		return @mysql_affected_rows($this->link);
	}

	function result ( $query, $row, $col = null ) {
		if ( is_string($query) ) {
			$query = $this->query($query);
		}
		$ret = @mysql_result($query, $row, $col);
		$this->clean($query);
		return $ret;
	}

	function clean ( $query ) {
		return @mysql_free_result($query);
	}

	function insert_id () {
		return ( $id = @mysql_insert_id($this->link) ) >= 0 ? $id : $this->result($this->query('SELECT last_insertid()'), 0);
	}

	function version () {
		return @mysql_get_server_info($this->link);
	}

	function close () {
		return @mysql_close($this->link);
	}

	function error () {
		return ( ( $this->link ) ? @mysql_error($this->link) : @mysql_error() );
	}

	function errno () {
		return intval(( $this->link ) ? @mysql_errno($this->link) : @mysql_errno());
	}

}
