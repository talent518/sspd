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

$benchTimes = 10;
$benchFile = dirname(__FILE__) . '/bench.lock';

@unlink($benchFile);

if($pid===null) {
	$pipes = array();

	for($pid=0; $pid<$nthreads; $pid++) {
		$cmdString = sprintf("/opt/ssp/bin/ssp -f %s -s script %s %s %s %s %s %s", __FILE__,$host,$port,$nthreads,$nconns,$ntimes,$pid);
		$fp = popen($cmdString, 'r');
		stream_set_blocking($fp, 1);
		$pipes[] = $fp;
	}

	$conns = $connErrors = $sendLoginErrors = $recvLoginErrors = $loginFails = $logins = 0;
	$logines = $pipes;
	$beginTime=microtime(true);
	while ( count($logines) && count($pipes) )
	{
		sleep(1);
		
		$reads = $logines;
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
						case 'W':
							$id = array_search($fp, $logines);
							unset($logines[$id]);
							break;
						case 'C':
							$id = array_search($fp, $pipes);
							unset($pipes[$id]);
							@pclose($fp);
							break;
					}
				}
			}
		}
		
		$tmpTime = microtime(true);
		echo 'logins: ', $logins, ', avg logins: ', $logins/($tmpTime - $beginTime), PHP_EOL;
	}
	
	echo 'conns: ', $conns, ', connErrors: ', $connErrors, ', logins: ', $logins, ', loginFails: ', $loginFails, ', sendLoginErrors: ', $sendLoginErrors, ', recvLoginErrors: ', $recvLoginErrors, PHP_EOL;
	$loginBenchString = 'logins: ' . $logins . ', avg logins: ' . ($logins/($tmpTime - $beginTime));
	
	if(empty($pipes)) {
		exit;
	}
	
	touch($benchFile);
	
	$conns = $logins;
	$sendRequestErrors = $recvRequestErrors = $recvRequestFails = $requests = 0;
	$requestStack = $requestStackTime = array();
	$requestStackIndex = 0;
	$requestStackSum = 0;
	$beginTime=microtime(true);
	while ( count($pipes) )
	{
		$tmpTime = microtime(true);
		sleep(1);
		$_requests = $requests;
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
							@pclose($fp);
							break;
					}
				}
			}
		}

		echo $loginBenchString;
		$tmpTime2 = microtime(true);
		if(count($requestStack) == $benchTimes) {
			if($requestStackIndex == $benchTimes-1) {
				$requestStackIndex = 0;
			} else {
				$requestStackIndex++;
			}
			
			$requestStackTime[$requestStackIndex] = $tmpTime;
			$requestStackSum -= $requestStack[$requestStackIndex];
			$requestStackSum += ($requestStack[$requestStackIndex] = ($requests - $_requests));
			echo ', total avg requests: ', $requestStackSum/($tmpTime2 - ($requestStackIndex>0 ? $requestStackTime[$requestStackIndex-1] : $requestStackTime[$benchTimes-1]));
		} else {
			$requestStackTime[$requestStackIndex] = $tmpTime;
			$requestStackSum += ($requestStack[$requestStackIndex] = ($requests - $_requests));
			echo ', total avg requests: ', $requestStackSum/($tmpTime2 - $requestStackTime[0]);
			$requestStackIndex++;
			if($requestStackIndex == $benchTimes) {
				$requestStackIndex--;
			}
		}
		
		echo ', total avg requests: ', $requests/($tmpTime2 - $beginTime), PHP_EOL;
	}

	exit;
}

define('IS_DEBUG', 0);

require 'core.php';

import('lib.xml');
import('mod.uc.base');

$request=new XML_Element('request');
$request->type='User.Login';
$request->is_simple=true;
$request->params=array_to_xml(array('username'=>'abao','password'=>'123456'),'params');

$list = UDB()->select(array(
	'table' => 'members',
	'field' => 'username',
	'order' => 'uid',
	'limit' => ($pid*$nconns).','.$nconns
), SQL_SELECT_LIST);

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
			echo 'sC'; // send login request error and close connection
			continue;
		}
		if(($recv=$socket->read()) === false) {
			echo 'rC'; // recv login response error and close connection
			continue;
		}
		if($recv->type === 'User.Login.Failed') {
			UDB()->delete('members', 'username=\''.$user['username'].'\'');
			echo 'fC'; // recv login fail and close connection
			continue;
		}
		echo 'l'; // login success

		$sockets[$i]=$socket;
	}else{
		echo 'e'; // connection error
	}
}

echo 'W';

while(!file_exists($benchFile)) {
	usleep(100);
}

$request=new XML_Element('request');
$request->type='Gold.State';
$request->is_simple=true;
$request->params=array_to_xml(array('page'=>1,'size'=>10,'isToday'=>0),'params');

for($i=0; $i<$ntimes; $i++) {
	if(empty($sockets)) {
		exit;
	}
	$request->pid=$pid;
	$request->i=$i;
	foreach($sockets as $j=>$socket) {
		if($socket->write($request) === false) {
			unset($sockets[$j]);
			echo 'SC'; // send request error and close connection
			continue;
		}
		$response=$socket->read();
		if($response === false) {
			echo 'RC'; // recv request error and close connection
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
