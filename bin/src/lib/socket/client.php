<?PHP
define('SOCKET_CLIENT_NAME', 'SocketClient');
define('SOCKET_CLIENT_VERSION', '1.0.0');
define('SOCKET_CLIENT_AUTHOR', 'ABao <talent518@live.cn>');

/*
 * SocketClient PHP socket client base class @version 1.0.0 @author ABao <talent518@live.cn> @package class_socket_client
 */
class LibSocketClient {
	
	/*
	 * connect to server_host:port @access readonly @var string $server_host @var integer $server_port
	 */
	private $server_host, $server_port;

	/**
	 * connect to client_host:client_port
	 *
	 * @access readonly
	 * @var string $client_host
	 * @var integer $client_port
	 */
	private $client_host, $client_port;

	private $sendKey, $receiveKey;

	/**
	 * error message
	 *
	 * @var string $error
	 */
	var $isLogin = false, $error = '';

	protected $socket;

	private $connected = false;

	public function __set ( $name, $value ) {
		if ( isset($this->$name) )
			trigger_error("property $name is readonly!", E_USER_NOTICE);
		else
			$this->$name = $value;
	}

	public function __get ( $name ) {
		if ( isset($this->$name) )
			return $this->$name;
		else
			trigger_error("property $name undefined!", E_USER_NOTICE);
	}

	function __construct () {
		extension_loaded('sockets') or die('sockets:Extension does not exist!');
		import('lib.xml');
	}
	
	/*
	 * connect to server_host and port @access public @param integer $server_host @param integer $server_port
	 */
	function connect ( $server_host, $server_port ) {
		$this->server_host = $server_host;
		$this->server_port = $server_port;
		$this->socket = @socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
		if (  ! is_resource($this->socket) ) {
			$this->error = 'Couldn\'t create socket: ' . @socket_strerror($this->socket);
			return false;
		}
		$this->connected = @socket_connect($this->socket, $server_host, $server_port);
		if (  ! $this->connected ) {
			$errorcode = socket_last_error();
			$errormsg = socket_strerror($errorcode);
			if ( IS_DEBUG )
				$this->error = "Couldn't connect to server for {$server_host}:{$server_port}" . PHP_EOL . "[$errorcode] $errormsg";
			else
				$this->error = 'Server does not start!';
		} else {
			// socket_set_nonblock($this->socket);
			socket_getsockname($this->socket, $this->client_host, $this->client_port);
			if ( IS_DEBUG )
				$this->error = "Connected to Server for {$server_host}:{$server_port}." . PHP_EOL . "Client connect for {$this->client_host}:{$this->client_port}.";
			else {
				$this->error = 'Server is running.';
			}
			
			$sendKey = $this->randstr(128);
			$request = new XML_Element('request');
			$request->type = 'Connect.Key';
			$request->setText($sendKey);
			$this->write($request);
			$this->sendKey = $sendKey;
			
			$response = $this->read();
			if ( $response->type == 'Connect.Key' ) {
				$this->receiveKey = $response->getText();
			}
		}
		return $this->connected;
	}
	
	function isconnected() {
		return $this->connected;
	}

	function randstr ( $len = 6 ) {
		$chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789';
		$max = strlen($chars);
		// 中文随机字
		$str = '';
		for ( $i = 0; $i < $len; $i ++  ) {
			$str .= $chars{rand(0, $max)};
		}
		return $str;
	}

	function int4_to_str ( $num = 0 ) {
		$return = '';
		for ( $i = 0; $i < 4; $i ++  ) {
			$return .= chr(( $num >> ( ( 3 - $i ) * 8 ) ) & 0xff);
		}
		return $return;
	}

	function str_to_int4 ( $str = '' ) {
		$num = 0;
		for ( $i = 0; $i < 4; $i ++  ) {
			$num += ( ord($str{$i}) & 0xff ) << ( ( 3 - $i ) * 8 );
		}
		return $num;
	}
	
	function isreadable($timeout=0) {
		if(!$this->connected) return false;
	
		$reads = [$this->socket];
		$writes = null;
		$excepts = null;
		return socket_select($reads, $writes, $excepts, $timeout, 100) > 0;
	}
	
	/*
	 * read to socket @access public @return string
	 */
	function read () {
		if (  ! $this->connected ) {
			$this->error = 'Could not read from server!';
			return false;
		}
		$buf = '';
		$len = @socket_recv($this->socket, $buf, 4, 0);
		$recv_len = $this->str_to_int4($buf);
		$recved_len = 0;
		$data = '';
		if ( $len > 0 && $recv_len > 0 ) {
			while ( ( $len = @socket_recv($this->socket, $buf, $recv_len - $recved_len, 0) ) !== false ) {
				if ( $len > 0 )
					$recved_len += $len;
				$data .= $buf;
				if ( $recved_len == $recv_len ) {
					break;
				}
			}
		}
		if (  ! $len ) {
			return false;
		}
		if ( IS_DEBUG ) {
			echo 'recv_len:', $recv_len, ',read:', $data, PHP_EOL;
		}
		if ( $data ) {
			$response = xml_to_object($data);
			if ( $this->receiveKey && $response->type == 'Connect.Data' ) {
				$data = str_decode($response->getText(), $this->receiveKey);
				$response = xml_to_object($data);
			}
			return $response;
		}
	}

	public function iswritable($timeout=0) {
		if(!$this->connected) return false;
		
		$writes = [$this->socket];
		$excepts = null;

		return $this->connected = socket_select($reads, $writes, $excepts, $timeout, 100) > 0;
	}
	
	/*
	 * write to socket @access public @param string $in
	 */
	function write ( XML_Element $in ) {
		if (! $this->iswritable()) {
			$this->error = 'Could not write to server!';
			return false;
		}
		if ( IS_DEBUG ) {
			echo 'write:', $in, PHP_EOL;
		}
		if ( $this->sendKey &&  ! in_array($in->type, array(
			'Connect.Key', 
			'Connect.Ping'
		)) ) {
			$request = new XML_Element('request');
			$request->type = 'Connect.Data';
			$request->setText(str_encode(( string ) $in, $this->sendKey));
			$in = ( string ) $request;
		} else {
			$in = ( string ) $in;
		}
		$ln = strlen($in);
		return @socket_send($this->socket, $this->int4_to_str($ln) . $in, $ln + 4, 0);
	}
	
	/*
	 * close socket connect @access public
	 */
	function close () {
		@socket_close($this->socket);
		$this->socket = null;
		$this->connected = false;
	}

	function __destruct () {
		$this->close();
	}

}
