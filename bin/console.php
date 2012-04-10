<?PHP
require 'core.php';

$socket=LIB('socket.client');
$socket->connect('localhost',SERVER_PORT);
echo $socket->error,PHP_EOL;

$exp=true;

do{
	fputs(STDOUT,">");
	$exp=commend(trim(fgets(STDIN)));
}while($exp);

function commend($cmdstr){
	global $socket;
	$return=true;
	$cmds=explode(' ',$cmdstr);
	$cmd=array_shift($cmds);
	switch($cmd){
		case 'help':
			echo 'help		显示帮助信息';
			break;
		case 'start':
		case 'stop':
		case 'restart':
			break;
		case 'show';
			$param=array_shift($cmds);
			if(in_array($param,array('client-list','client','')) && $socket->write('show:'.$param))
				echo $socket->read();
			break;
		case 'write';
			$msg=implode(' ',$cmds);
			if(!$socket->write(iconv(STD_CHARSET,'UTF-8',$msg)))
				echo($socket->error);
			break;
		case 'read';
			$msg=$socket->read();
			if($msg===false)
				$msg=$socket->error;
			echo correct_charset($msg);
			break;
		case 'version';
			echo 'Server:1.0.0',PHP_EOL,'Client:',SOCKET_CLIENT_VERSION,PHP_EOL,'Console:1.0.0';
			break;
		case 'server':
			var_dump($_SERVER);
			break;
		case 'env':
			var_dump($_ENV);
			break;
		case 'quit':
		case 'exit':
			$return=false;
			break;
		default:
			echo 'invalid command!';
	}
	echo PHP_EOL;
	return $return;
}
$socket->close();
$socket=null;
