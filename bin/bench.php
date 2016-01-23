<?php
@list(,$host,$port,$nthreads,$nconns,$ntimes,$pid) = $_SERVER['argv'];

if($host === null) {
	$host = '127.0.0.1';
}
if($port === null) {
	$port = 8083;
}
if($nthreads === null) {
	$nthreads = 8;
}
if($nconns === null) {
	$nconns = 10;
}
if($ntimes === null) {
	$ntimes = 1000;
}

if($pid===null) {
	$pipes = array();
	$conns = $connErrors = $sendLoginErrors = $recvLoginErrors = $loginFails = $logins = $sendRequestErrors = $recvRequestErrors = $recvRequestFails = $requests = $closes = 0;

	for($pid=0; $pid<$nthreads; $pid++) {
		$cmdString = sprintf("/opt/ssp/bin/ssp -f %s -s script %s %s %s %s %s %s", __FILE__,$host,$port,$nthreads,$nconns,$ntimes,$pid);
		$fp = popen($cmdString, 'r');
		stream_set_blocking($fp, 1);
		$pipes[] = $fp;
	}

	$beginTime=microtime(true);
	while ( count($pipes) )
	{
		$reads = $pipes;
		$writes = null;
		$excepts = null;
		$ret = stream_select($reads, $writes, $excepts, 0);
		if ( $ret > 0 && count($reads) )
		{
			foreach ( $reads as $fp )
			{
				$buffer = fread($fp, 8192);
				for($i=0; $i<strlen($buffer); $i++) {
					switch($buffer{$i}) {
						case 'c':
							$conns++;
							break;
						case 's':
							$sendLoginErrors++;
							break;
						case 'r':
							$recvLoginErrors++;
							break;
						case 'f':
							$loginFails++;
							break;
						case 'l':
							$logins++;
							break;
						case 'e':
							$connErrors++;
							break;
						case 'S':
							$sendRequestErrors++;
							$conns--;
							break;
						case 'R':
							$recvRequestErrors++;
							$conns--;
							break;
						case 'F':
							$recvRequestFails++;
							$conns--;
							break;
						case 'q':
							$requests++;
							break;
						case 'C':
							$id = array_search($fp, $pipes);
							unset($pipes[$id]);
							pclose($fp);
							break;
					}
				}
			}
		}
		//echo 'conns(',$conns,'), connErrors(',$connErrors,'), sendLoginErrors(',$sendLoginErrors,'), recvLoginErrors(',$recvLoginErrors,'), logins(',$logins,'), sendRequestErrors(',$sendRequestErrors,'), recvRequestErrors(',$recvRequestErrors,'), requests(',$requests,'), nthreads(',$nthreads,'), closeThreads(',$nthreads-count($pipes),')', PHP_EOL;
		if($beginRequestTime) {
			echo 'logins: ', $logins, ', avg logins(',$logins/($beginRequestTime - $beginTime),'), avg requests: ', $requests/(microtime(true) - $beginRequestTime), PHP_EOL;
		}elseif($requests) {
			$beginRequestTime = microtime(true);
			echo 'logins: ', $logins, ', requests: ', $requests, PHP_EOL;
		}else{
			echo 'logins: ', $logins, ', avg logins: ', $logins/(microtime(true) - $beginTime), PHP_EOL;
		}
		sleep(1);
	}

	exit;
}

define('IS_DEBUG', 0);

require 'core.php';

import('lib.xml');

$request=new XML_Element('request');
$request->type='User.Login';
$request->is_simple=true;
$request->params=array_to_xml(array('username'=>'abao','password'=>'123456'),'params');

$list = MOD('uc.user')->get_list_by_where('', ($pid*$nconns).','.$nconns, 'uid');

$sockets=array();
for($i=0; $i<$nconns; $i++) {
	$socket=clone LIB('socket.client');
	$socket->connect($host, $port);

	if($socket->is_connect()){
		echo 'c'; // connect

		$request->pid=$pid;
		$request->i=$i;

		$user = array_shift($list);
		$request->params->username = $user['username'];
		if($socket->write($request) === false) {
			echo 's'; // send login request error and close connection
			continue;
		}
		if(($recv=$socket->read()) === false) {
			echo 'r'; // recv login response error and close connection
			continue;
		}
		if($recv->type === 'User.Login.Failed') {
			echo 'f'; // recv login fail and close connection
			continue;
		}
		echo 'l'; // login success

		$sockets[$i]=$socket;
	}else{
		echo 'e'; // connection error
	}
}

$request=new XML_Element('request');
$request->type='Gold.State';
$request->is_simple=true;
$request->params=array_to_xml(array('page'=>1,'size'=>10,'isToday'=>0),'params');

for($i=0; $i<$ntimes; $i++) {
	$request->pid=$pid;
	$request->i=$i;
	foreach($sockets as $j=>$socket) {
		if($socket->write($request) === false) {
			unset($sockets[$j]);
			echo 'S'; // send request error and close connection
			continue;
		}
		$response=$socket->read();
		if($response === false) {
			echo 'R'; // recv request error and close connection
			unset($sockets[$j]);
			continue;
		}
		if($response->type === 'Gold.State.Failed') {
			echo 'F'; // fail request
		}
		echo 'q'; // ok request
	}
}

foreach($sockets as $socket) {
	$socket->close();
	echo 'C'; // close connection
}
?>