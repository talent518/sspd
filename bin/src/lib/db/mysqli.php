<?php
import('lib.db.base');

class LibDbMysqli extends LibDbBase {

	private $link;

	function __construct () {
		extension_loaded('mysqli') or die('mysqli:Extension does not exist!');
	}

	function connect ( $silent = FALSE ) {
		list ( $host, $port ) = explode(':', $this->host);
		if (  ! ( $this->link = @mysqli_connect($host, $this->user, $this->pwd, $silent ? null : $this->name, $port, $this->socket) ) &&  ! $silent )
			$this->halt('Can not connect to MySQL server');
		
		if ( $this->version() > '4.1' ) {
			if ( $this->charset )
				@mysqli_query($this->link, "SET character_set_connection=$this->charset,character_set_results=$this->charset,character_set_client=binary");
			
			if ( $this->version() > '5.0.1' )
				@mysqli_query($this->link, "SET sql_mode=''");
		}
		
		if ( $this->name &&  ! $silent &&  ! $this->sdb($this->name) )
			$this->halt('Database does not exist');
			
		return is_object($this->link) ? true : false;
	}

	function ping () {
		return @mysqli_ping($this->link);
	}

	function sdb ( $name ) {
		return @mysqli_select_db($this->link, $name);
	}

	function cdb ( $name ) {
		return $this->query('CREATE DATABASE `' . $name . '` CHARACTER SET ' . $this->charset, TRUE) && $this->sdb($name);
	}

	function query ( $sql, $silent = FALSE, $retry = FALSE ) {
		if ( ( $query = @mysqli_query($this->link, $sql) ) == FALSE &&  ! $silent ) {
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
			$query = $this->query($query);
		}
		
		$row = @mysqli_fetch_assoc($query);

		return $row === NULL ? false : $row;
	}

	function arows () {
		return @mysqli_affected_rows($this->link);
	}

	function result ( $query, $row, $col = null ) {
		if ( is_string($query) ) {
			$query = $this->query($query);
		}
		@mysqli_data_seek($query, $row);
		$row = ( is_int($col) ? $this->row($query) : @mysqli_fetch_row($query) );
		$ret = ( $col === null ? array_shift($row) : $row[$col] );
		$this->clean($query);
		return $ret;
	}

	function clean ( $query ) {
		return @mysqli_free_result($query);
	}

	function insert_id () {
		return ( $id = @mysqli_insert_id($this->link) ) >= 0 ? $id : $this->result($this->query('SELECT last_insertid()'), 0);
	}

	function version () {
		return @mysqli_get_server_info($this->link);
	}

	function close () {
		return @mysqli_close($this->link);
	}

	function error () {
		return ( ( $this->link ) ? @mysqli_error($this->link) : @mysqli_connect_error() );
	}

	function errno () {
		return intval(( $this->link ) ? @mysqli_errno($this->link) : @mysqli_connect_errno());
	}

}
