<?php
define('I', (SSP_PORT-8082)/2);

$db = new PDO('mysql:dbname=test;host=127.0.0.1', 'root', '1qazXSW23edc');
$db->setAttribute(PDO::ATTR_AUTOCOMMIT, 1);

function ssp_start_handler () {
	global $db;
	
	ssp_conv_setup(I, 3);
	for($i=0; $i<3; $i++) {
		ssp_conv_connect('127.0.0.1', 8083+$i*2, $i);
	}
	
	$db->exec('CREATE TABLE IF NOT EXISTS ssp_conv(sid INT(11) UNSIGNED NOT NULL,id INT(11) UNSIGNED NOT NULL,PRIMARY KEY(sid,id)) ENGINE=memory');
	$db->prepare('DELETE FROM ssp_conv WHERE sid=?')->execute([I]);
}

function ssp_connect_handler ( $ClientId ) {
	global $db;
	
	$db->prepare('INSERT INTO ssp_conv (sid,id)VALUES(?,?)')->execute([I,ssp_info($ClientId, 'index')]);
}

function ssp_connect_denied_handler ( $ClientId ) {
}

function ssp_receive_handler ( $ClientId, $xml ) {
	global $db;
	
	$I = ssp_info($ClientId, 'index');
	
	$stmt = $db->prepare('SELECT sid, id FROM ssp_conv WHERE not (sid=? and id=?)');
	$stmt->execute([I,$I]);
	
	// echo ssp_info($ClientId, 'index'), ', ', $xml, ', ', $stmt->rowCount(), PHP_EOL;
	
	while($row = $stmt->fetch(PDO::FETCH_NUM)) {
		list($sid,$id) = $row;
		// echo $sid, ' ', $id, PHP_EOL;
		$row[] = $xml;
		ssp_send($id, $row, $sid);
	}
	
	return $xml;
}

function ssp_send_handler ( $ClientId, $xml ) {
	return serialize($xml);
}

function ssp_close_handler ( $ClientId ) {
	global $db;

	$db->prepare('DELETE FROM ssp_conv WHERE sid=? AND id=?')->execute([I,ssp_info($ClientId, 'index')]);
}

function ssp_stop_handler () {
	ssp_conv_disconnect();
}

