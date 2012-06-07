<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

define('SQL_SELECT_QUERY',0);
define('SQL_SELECT_ONLY',1);
define('SQL_SELECT_LIST',2);
define('SQL_SELECT_STRING',3);

class LibDbBase{
	var $querys=array();
	var $host,$user,$pwd,$name,$pconnect,$tablepre,$charset;

	protected function tidy($data){
		if(is_array($data)){
			foreach($data as $k=>$v)
				$data[$k]=$this->tidy($v);
			return $data;
		}elseif(is_numeric($data)){
			return $data+0;
		}else{
			return $data;
		}
	}

	function connect($silent=FALSE){exit('connect method no define');}

	function sdb($name){exit('sdb method no define');}

	function cdb($name){exit('cdb method no define');}

	function tname($name){
		return "`{$this->name}`.`{$this->tablepre}{$name}`";
	}

	/*
	*array('table','field','join','having','group','order','limit')
	*/
	function select($sqls=array(),$return=0,$key='',$vkey=''){
		extract($sqls);
		list($table,$alias)=explode(' ',$table);
		$sql='SELECT '.$field.' FROM '.$this->tname($table).($alias?" as `$alias`":'');
		if($join){
			if($join_split){
				foreach($join as $k=>$v){
					list($t,$a)=explode(' ',$k);
					$t=$this->tname($t);
					$sql.=' LEFT JOIN('.$t.' AS `'.$a.'`)ON('.$v.')';
				}
			}else{
				$joins=$ons=array();
				foreach($join as $k=>$v){
					list($t,$a)=explode(' ',$k);
					$t=$this->tname($t);
					$joins[]="$t as `$a`";
					$ons[]=$v;
				}
				$sql.=' LEFT JOIN('.implode(',',$joins).')ON('.implode(' AND ',$ons).')';
				$joins=$ons=null;
			}
		}
		if($where)
			$sql.=' WHERE '.$where;
		if($group)
			$sql.=' GROUP BY '.$group;
		if($having)
			$sql.=' HAVING '.$having;
		if($order)
			$sql.=' ORDER BY '.$order;
		if($limit && SQL_SELECT_ONLY!=$return){
			$sql.=' LIMIT '.$limit;
		}
		switch($return){
			case SQL_SELECT_QUERY:
				$return=$this->query($sql);
				break;
			case SQL_SELECT_ONLY:
				$q=$this->query($sql.' LIMIT 1');
				$value=$this->row($q);
				$this->clean($q);
				$return=($key?$value[$key]:$value);
				break;
			case SQL_SELECT_LIST:
				$q=$this->query($sql);
				$list=array();
				while($v=$this->row($q)){
					$value=($vkey?$v[$vkey]:$v);
					if($key)
						$list[$v[$key]]=$value;
					else
						$list[]=$value;
				}
				$this->clean($q);
				$return=&$list;
				break;
			default:
				$return=$sql;
				break;
		}
		return($return);
	}

	function count($table,$where=''){
		return $this->select(array(
			'table'=>$table,
			'field'=>'count(*) as `count`',
			'where'=>$where
		),SQL_SELECT_ONLY,'count')+0;
	}

	function insert($table,$data,$replace=false){
		$this->query(($replace?'REPLACE':'INSERT').' INTO '.$this->tname($table).' (`'.implode('`,`',array_keys($data)).'`)VALUES('.simplode($data).')');
	}

	function inserts($table,$fields,$datas,$replace=false){
		$data=array();
		foreach($datas as $value)
			$data[]=simplode($value);
		$this->query(($replace?'REPLACE':'INSERT').' INTO '.$this->tname($table).' (`'.implode('`,`',$fields).'`)VALUES('.implode('),(',$data).')');
	}

	function update($table,$data=array(),$where='1>0',$isString=true){
		$sets=array();
		foreach($data as $field=>$value)
			$sets[]=($isString?"`$field`='$value'":"`$field`=$value");
		$this->query('UPDATE '.$this->tname($table).' SET '.implode(',',$sets).' WHERE '.$where);
	}

	function delete($table,$where=''){
		$this->query('DELETE FROM '.$this->tname($table).' WHERE '.$where);
	}

	function query($query,$silent=FALSE){exit('query method no define');}

	function row($query){exit('row method no define');}

	function arows(){exit('arows method no define');}

	function result($query,$row,$field=null){exit('result method no define');}

	function clean($query){exit('clean method no define');}

	function insert_id(){exit('insert_id method no define');}

	function version(){exit('version method no define');}

	function close(){exit('close method no define');}

	function error(){exit('error method no define');}

	function errno(){exit('errno method no define');}

	function halt($message='',$sql=''){
		if(!IS_DEBUG)
			exit($message);
		$dberror=$this->error();
		$dberrno=$this->errno();
		echo PHP_EOL,PHP_EOL,"MySQL Error",PHP_EOL;
		echo "\tMessage: $message",PHP_EOL;
		echo "\tSQL: $sql",PHP_EOL;
		echo "\tError: $dberror",PHP_EOL;
		echo "\tErrno.: $dberrno",PHP_EOL,PHP_EOL;
		exit;
	}

	function __destruct(){
		$this->close();
	}
}
