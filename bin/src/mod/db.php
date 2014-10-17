<?php
if (  ! defined('IN_SITE') )
	exit('Access Denied');

class ModDb {

	function import ( $fn, $notify = FALSE ) {
		$db = DB();
		if (  ! $fp = fopen($fn, 'rb') )
			return false;
		$tn = null;
		while (  ! feof($fp) ) {
			$sql = '';
			$_sql = '';
			do {
				$_sql = fgets($fp, 4096);
				$sql .= $_sql;
			} while ( $_sql !== false &&  ! in_array(substr($_sql,  - 1), array(
				"\r", 
				"\n"
			)) );
			$sql = trim($sql);
			if (  ! $sql )
				continue;
			if ( substr($sql, 0, 22) == 'DROP TABLE IF EXISTS `' ) {
				if ( $tn && $notify ) {
					echo '<b>OK!</b><br/><br/>';
					ob_flush();
				}
				$tn = substr($sql, 22,  - 2);
				$sql = 'DROP TABLE IF EXISTS `' . $db->tablepre . $tn . '`';
				if ( $notify ) {
					echo '删除表 ' . $db->tablepre . $tn . ' …';
					ob_flush();
				}
			} elseif ( substr($sql, 0, 14) == 'CREATE TABLE `' ) {
				if ( $notify ) {
					echo '<b>OK!</b><br/>';
					ob_flush();
				}
				$sql = 'CREATE TABLE `' . $db->tablepre . substr($sql, 14);
				if ( $notify ) {
					echo '创建表 ' . $db->tablepre . $tn . ' 结构…';
					ob_flush();
				}
				$insert = true;
			} else {
				$sql = 'INSERT INTO `' . $db->tablepre . substr($sql, 13);
				if ( $notify && $insert ) {
					echo '<b>OK!</b><br/>写入表 ' . $db->tablepre . $tn . ' 数据…';
					ob_flush();
					$insert = false;
				}
			}
			if (  ! $db->query($sql, TRUE) ) {
				if ( $notify )
					exit('<p style="font-size:12px;font-family:verdana,arial;background:#F60;padding:0.5em;"><b>SQL</b>: ' . $sql . '<br><b>Error</b>: ' . $db->error() . '<br><b>Errno.</b>: ' . $db->errno() . '</div>');
				else
					return false;
			}
		}
		fclose($fp);
		if ( $tn && $notify ) {
			echo '<b>OK!</b><br/>';
			ob_flush();
		}
		return true;
	}

	function export ( $fn, $notify = FALSE ) {
		$db = DB();
		if (  ! $fp = @fopen($fn, 'wb') )
			return false;
		$q = $db->query('SHOW TABLES FROM `' . $db->name . '` LIKE "' . $db->tablepre . '%"');
		while ( ( $v = $db->row($q) ) !== false ) {
			$n = array_shift($v);
			$tn = substr($n, strlen($db->tablepre));
			if ( $notify ) {
				echo $tn;
				ob_flush();
			}
			if ( $notify ) {
				echo '删除表 ' . $n . ' …';
				ob_flush();
			}
			$sql = "DROP TABLE IF EXISTS `$tn`;\r\n";
			if (  ! fputs($fp, $sql, strlen($sql)) )
				return false;
			if ( $notify ) {
				echo '<b>OK!</b><br/>创建表 ' . $n . ' 结构…';
				ob_flush();
			}
			$sql = str_replace(array(
				'CREATE TABLE `' . $db->tablepre, 
				"\r", 
				"\n"
			), array(
				'CREATE TABLE `', 
				'', 
				''
			), $db->result('SHOW CREATE TABLE `' . $n . '`', 0, 'Create Table')) . ";\r\n";
			if (  ! fputs($fp, $sql, strlen($sql)) )
				return false;
			if ( $notify ) {
				echo '<b>OK!</b><br/>写入表 ' . $n . ' 数据…';
				ob_flush();
			}
			
			$rows = array();
			$fields = null;
			$_q = $db->query('SELECT * FROM `' . $n . '`');
			while ( ( $r = $db->row($_q) ) !== false ) {
				if (  ! $fields )
					$fields = $this->fields(array_keys($r));
				$rows[] = $this->values($r);
				if ( count($rows) == 50 ) {
					$sql = 'INSERT INTO `' . $tn . '` (' . $fields . ')VALUES(' . implode('),(', $rows) . ");\r\n";
					if (  ! fputs($fp, $sql, strlen($sql)) )
						return false;
					$rows = array();
				}
			}
			if ( count($rows) ) {
				$sql = 'INSERT INTO `' . $tn . '` (' . $fields . ')VALUES(' . implode('),(', $rows) . ");\r\n";
				if (  ! fputs($fp, $sql, strlen($sql)) )
					return false;
			}
			$db->clean($_q);
			$sql = "\r\n";
			if (  ! fputs($fp, $sql, strlen($sql)) )
				return false;
		}
		fclose($fp);
		return true;
	}

	private function strs ( $str ) {
		if ( is_array($str) ) {
			foreach ( $str as $k => $v )
				$str[$k] = $this->strs($v);
			return $str;
		}
		return str_replace(array(
			"\r", 
			"\n"
		), array(
			'\r', 
			'\n'
		), addslashes($str));
	}

	private function fields ( $fields ) {
		return '`' . implode('`,`', $this->strs($fields)) . '`';
	}

	private function values ( $values ) {
		$vs = array();
		foreach ( $values as $v )
			$vs[] = ( is_string($v) ? '\'' . $this->strs($v) . '\'' : ( $v === null ? 'null' : $v ) );
		return implode(',', $vs);
	}

}
