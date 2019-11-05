<?php
define('I', (SSP_PORT-8082)/2);

error_reporting(E_ALL);

$db = new PDO('mysql:dbname=test;host=127.0.0.1', 'root', '1qazXSW23edc');
$db->setAttribute(PDO::ATTR_AUTOCOMMIT, 1);

$db->exec('CREATE TABLE IF NOT EXISTS ssp_conv(sid INT(11) UNSIGNED NOT NULL,id INT(11) UNSIGNED NOT NULL,recvs BIGINT(20) UNSIGNED NOT NULL DEFAULT 0,sends BIGINT(20) UNSIGNED NOT NULL DEFAULT 0,PRIMARY KEY(sid,id)) ENGINE=memory');

$connStmt = $db->prepare('INSERT INTO ssp_conv (sid,id)VALUES(?,?)');
$closeStmt = $db->prepare('DELETE FROM ssp_conv WHERE sid=? AND id=?');
$recvStmt = $db->prepare('UPDATE ssp_conv SET recvs=recvs+? WHERE sid=? and id=?');
$sendStmt = $db->prepare('UPDATE ssp_conv SET sends=sends+? WHERE sid=? and id=?');
$fetchStmt = $db->prepare('SELECT sid, id FROM ssp_conv');

$names = ['scpu', 'pcpu', 'smem', 'pmem', 'threads', 'etime', 'args'];

function ssp_monitor_handler(array $scpu, array $pcpu, array $smem, array $pmem, int $threads, int $etime, array $args) {
	global $db, $names, $monitorStmt;
	if(!$monitorStmt) {
		$fieldstr = null;
		$namestr = null;
		$valuestr = null;
		foreach(func_get_args() as $i=>$v) {
			if(!isset($names[$i])) break;
			$field = $names[$i];
			if(is_array($v)) {
				if(isset($v[0])) {
					$fieldstr .= ',' . $field . ' VARCHAR(2048) NOT NULL';
					$namestr .= ',' . $field;
					$valuestr .= ',?';
				} else {
					foreach($v as $n=>$v2) {
						$fieldstr .= ',' . $field . ucfirst($n) . ' ' . gettype($v2) . ' UNSIGNED NOT NULL';
						$namestr .= ',' . $field . ucfirst($n);
						$valuestr .= ',?';
					}
				}
			} else {
				$fieldstr .= ',' . $field . ' ' . gettype($v) . ' UNSIGNED NOT NULL';
				$namestr .= ',' . $field;
				$valuestr .= ',?';
			}
		}
		$db->exec('CREATE TABLE IF NOT EXISTS ssp_monitor(id INT(11) UNSIGNED NOT NULL AUTO_INCREMENT,sid INT(11) UNSIGNED NOT NULL' . $fieldstr . ',createTime TIMESTAMP,PRIMARY KEY (id)) ENGINE=Innodb CHARSET=utf8');
		$monitorStmt = $db->prepare('INSERT INTO ssp_monitor (sid' . $namestr . ',createTime)VALUES(?' . $valuestr . ',NOW())');
	}
	$params = [I];
	foreach(func_get_args() as $i=>$v) {
		if(!isset($names[$i])) break;
		if(is_array($v)) {
			if(isset($v[0])) {
				$params[] = implode(' ', $v);
			} else {
				foreach($v as $v2) {
					$params[] = $v2;
				}
			}
		} else {
			$params[] = $v;
		}
	}
	$monitorStmt->execute($params);
}

function ssp_start_handler () {
	global $db;
	
	ssp_conv_setup(I, 3);
	for($i=0; $i<3; $i++) {
		ssp_conv_connect('127.0.0.1', 8083+$i*2, $i);
	}
	
	$db->prepare('DELETE FROM ssp_conv WHERE sid=?')->execute([I]);

	ssp_msg_queue_init(10000, 1);
}

function ssp_connect_handler ( $ClientId ) {
	global $connStmt;
	
	$connStmt->execute([I,ssp_info($ClientId, 'index')]);
	
	ssp_send($ClientId, date('H:i:s'));
}

function ssp_connect_denied_handler ( $ClientId ) {
}

function broadcast_handler($I, $xml, $arg1, $arg2, $arg3, $arg4, $arg5) {
	global $recvStmt, $fetchStmt;

	// echo 'broadcast_handler', PHP_EOL;

	// $recvStmt->execute([strlen($xml),I,$I]);
	// $recvStmt->closeCursor();
	
	$fetchStmt->execute([I,$I]);
	
	$i = 0;
	while($row = $fetchStmt->fetch(PDO::FETCH_NUM)) {
		list($sid,$id) = $row;
		if($sid == I && $id == $I) continue;
		// echo $sid, ' ', $id, PHP_EOL;
		$row[] = $xml;
		if(ssp_send($id, $row, $sid)) $i++;
	}
	// echo $I, ', ', $xml, ', ', $i+1, PHP_EOL;

	$fetchStmt->closeCursor();
}

function ssp_receive_handler ( $ClientId, $xml ) {
	global $recvStmt, $fetchStmt;
	
	$I = ssp_info($ClientId, 'index');

	ssp_msg_queue_push('broadcast_handler', $I, $xml);
	
	return [I, $I, $xml];
}

function ssp_send_handler ( $ClientId, $xml ) {
	global $sendStmt;

	$I = ssp_info($ClientId, 'index');

	if(!is_string($xml)) $xml = serialize($xml);

	// $sendStmt->execute([strlen($xml),I,$I]);
	// $sendStmt->closeCursor();

	return $xml;
}

function ssp_close_handler ( $ClientId ) {
	global $closeStmt;

	$I = ssp_info($ClientId, 'index');

	$closeStmt->execute([I,$I]);
}

function ssp_stop_handler () {
	ssp_conv_disconnect();
	ssp_msg_queue_destory();
}
