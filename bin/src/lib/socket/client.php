<?PHP
if(!defined('IN_SERVER'))
	exit('Access Denied');

define('SOCKET_CLIENT_NAME','SocketClient');
define('SOCKET_CLIENT_VERSION','1.0.0');
define('SOCKET_CLIENT_AUTHOR','ABao <talent518@live.cn>');

/*
 * SocketClient
 * PHP socket client base class
 *
 * @version 1.0.0
 * @author ABao <talent518@live.cn>
 * @package SocketClient
 */
class LibSocketClient{

	/*
	 * connect to server_host:port
	 *
	 * @access readonly
	 * @var string $server_host
	 * @var integer $server_port
	 */
	private $server_host,$server_port;

	/**
	 * connect to client_host:client_port
	 * @access readonly
	 * @var string $client_host
	 * @var integer $client_port
	 */
	private $client_host,$client_port;

	/**
	 * error message
	 * @var string $error
	 */
	var $error = '';

	protected $socket;

	private $connected=false;

	public function __set($name,$value){
		if(isset($this->$name))
			trigger_error("property $name is readonly!",E_USER_NOTICE);
		else
			$this->$name=$value;
	}

	public function __get($name){
		if(isset($this->$name))
			return $this->$name;
		else
			trigger_error("property $name undefined!",E_USER_NOTICE);
	}

	function __construct(){
		extension_loaded('sockets') or die('sockets:Extension does not exist!');
	}

	public function is_connect(){
		return(is_resource($this->socket) && $this->connected);
	}

	/*
	 * connect to server_host and port
	 *
	 * @access public
	 * @param integer   $server_host
	 * @param integer   $server_port
	 */
	function connect($server_host,$server_port){
		$this->server_host = $server_host;
		$this->server_port   = $server_port;
		$this->socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
		if (!is_resource($this->socket)) {
			$this->error= 'Couldn\'t create socket: '.@socket_strerror($this->socket);
			return false;
		}
		$this->connected = @socket_connect($this->socket, $server_host, $server_port);
		if (!$this->connected) {
			$errorcode = socket_last_error();
			$errormsg = socket_strerror($errorcode);
			if(IS_DEBUG)
				$this->error= "Couldn't connect to server for {$server_host}:{$server_port}".PHP_EOL."[$errorcode] $errormsg";
			else
				$this->error= 'Server does not start!';
		} else {
			socket_set_nonblock($this->socket);
			socket_getsockname($this->socket,$this->client_host,$this->client_port);
			if(IS_DEBUG)
				$this->error= "Connected to Server for {$server_host}:{$server_port}.".PHP_EOL."Client connect for {$this->client_host}:{$this->client_port}.";
			else
				$this->error= 'Server is running.';
		}
		return $this->connected;
	}

	/*
	 * read to socket
	 *
	 * @access public
	 * @return string
	 */
	function read(){
		if(!$this->is_connect())
			return;
		$data='';
		while ($buf = @socket_read($this->socket, 2048)) {
			$data.=$buf;
		}
		if( $buf === false ){
			if(socket_last_error($this->socket)==SOCKET_EWOULDBLOCK)
				socket_clear_error($this->socket);
			else
				$this->error= 'Could not read from server!';
		}
		return $data;
	}

	/*
	 * write to socket
	 *
	 * @access public
	 * @param string $in
	 */
	function write($in){
		if(!$this->is_connect()){
			$this->error= 'Could not write to server!';
			return false;
		}
		return @socket_write($this->socket, $in, strlen($in));
	}

	/*
	 * close socket connect
	 *
	 * @access public
	 */
	function close(){
		@socket_close($this->socket);
		$this->socket=null;
	}

	function __destruct(){
		$this->close();
	}
}
