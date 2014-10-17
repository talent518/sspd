<?PHP
// define('IS_DEBUG',TRUE);
require 'core.php';

$socket = LIB('socket.client');

$exp = true;

commend();
do {
	fputs(STDOUT, ">");
	$exp = commend(trim(fgets(STDIN)));
} while ( $exp );

function commend ( $cmdstr ) {
	global $socket;
	$return = true;
	$cmds = explode(' ', $cmdstr);
	$cmd = array_shift($cmds);
	switch ( $cmd ) {
		case 'connect':
			$socket->close();
			$socket->connect(array_shift($cmds), array_shift($cmds));
			if ( $socket->is_connect() ) {
				echo correct_charset('连接成功！'), PHP_EOL;
			} else {
				echo correct_charset('连接失败！'), PHP_EOL;
			}
			break;
		case 'login':
			if ( $socket->is_connect() ) {
				$request = new XML_Element('request');
				$request->type = 'User.Login';
				$request->is_simple = true;
				$request->params = array_to_xml(array(
					'username' => array_shift($cmds), 
					'password' => array_shift($cmds)
				), 'params');
				$socket->write($request);
				$response = $socket->read();
				if ( $response ) {
					if ( $response->type == 'User.Login.Failed' ) {
						$socket->isLogin = false;
					}
					if ( $response->type == 'User.Login.Succeed' ) {
						$socket->isLogin = true;
					}
					echo correct_charset($response->getText()), PHP_EOL;
				} else {
					echo correct_charset('登录状态未知！'), PHP_EOL;
				}
			} else {
				echo correct_charset('没有连接服务器！'), PHP_EOL;
			}
			break;
		case 'version':
			echo 'Server:1.0.0', PHP_EOL, 'Client:', SOCKET_CLIENT_VERSION, PHP_EOL, 'Console:1.0.0';
			break;
		case 'server':
			print_r($_SERVER);
			break;
		case 'env':
			print_r($_ENV);
			break;
		case 'quit':
		case 'exit':
			$return = false;
			break;
		default:
			echo correct_charset('显示帮助信息:'), PHP_EOL;
			echo correct_charset('connect host port             连接到主机为(host)，端口为(port)'), PHP_EOL;
			echo correct_charset('login user passwd             登录用户到为(user)，密码为(passwd)'), PHP_EOL;
			echo correct_charset('list client-list|client       client-list:显示'), PHP_EOL;
			echo correct_charset('version                       显示版本信息'), PHP_EOL;
			echo correct_charset('server|env                    server:服务器变量,env:环境变量'), PHP_EOL;
			echo correct_charset('quit|exit                     退出控制台'), PHP_EOL;
			break;
	}
	echo PHP_EOL;
	return $return;
}
$socket->close();
$socket = null;
