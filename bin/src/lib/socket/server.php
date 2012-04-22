<?PHP
if(!defined('IN_SERVER'))
	exit('Access Denied');

define('SERVER_NAME','SocketServer');
define('SERVER_VERSION','1.1.2');
define('SERVER_AUTHOR','ABao <talent518@live.cn>');

define('SCK_WRITE_PACKET_SIZE',8192);
define('SCK_READ_PACKET_SIZE',4096);
define('SCK_READ_SELECT_TIMEOUT',1);

if(strtoupper(substr(PHP_OS, 3)) == 'WIN'){ 
	extension_loaded('sockets') or dl('php_sockets.dll');
}else{
	extension_loaded('sockets') or dl('sockets.so'); 
	extension_loaded('posix') or dl('posix.so'); 
	extension_loaded('pcntl') or dl('pcntl.so'); 
	extension_loaded('sysvsem') or dl('sysvsem.so'); 
	extension_loaded('sysvshm') or dl('sysvshm.so'); 
	extension_loaded('sysvmsg') or dl('sysvmsg.so'); 
}

define('EXT_POSIX',SERVER_THREAD_ENABLE && extension_loaded('posix'));
define('EXT_PCNTL',SERVER_THREAD_ENABLE && extension_loaded('pcntl'));

/*
 * SocketServer
 * PHP socket server base class
 * Events that can be handled:
 *   start()
 *   connect($clientId)
 *   connectDenied($newClientId)
 *   close($clientId)
 *   stop()
 *   receive($clientId,$data)
 *   send($clientId,$data)
 *
 * @version 1.1.2
 * @author ABao <talent518@live.cn>
 * @package LibSocketServer
 */
class LibSocketServer{
	/*
	 * domain to bind to
	 * @var string $domain
	 */
	private $domain;

	/*
	 * port to listen
	 * @var integer   $port
	 */
	private $port;

	/*
	 * use $user set the user run the script.
	 * @var string $user
	 */
	private $user,$gid,$uid,$userHome;

	/*
	 * maximum amount of clients
	 * @var integer $maxClients
	 */
	var $maxClients = -1;

	/*
	 * maximum of backlog in queue
	 * @var integer $maxQueues
	 */
	var $maxQueues = 500;

	/*
	 * empty array, used for socket_select
	 * @var array $null
	 */
	private $null   = array();

	/*
	 * socket server resource
	 * @var resource $initFD
	 */
	private $initFD;

	/*
	 * all file descriptors are stored here
	 * @var array $clientFD
	 */
	private $clientFD = array();

	/*
	 * amount of clients
	 * @var integer   $clients
	 */
	private $clients = 0;

	/*
	 * bind to socket for event
	 * @var array $events
	 */
	private $events = array('start'=>null,'connect'=>null,'connectdenied'=>null,'close'=>null,'stop'=>null,'receive'=>null,'send'=>null);

	/*
	 * server listened
	 */
	private $listened=false;

	/*
	 * if inited server then true else false
	 */
	private $inited=false;

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

	/*
	 * create a new socket server
	 */
	function __construct(){
		if(!extension_loaded('sockets')){
			server_log('sockets:Extension does not exist!');
			exit(-1);
		}
	}
    function __destruct(){
        //$this->stop();
    }

	/*
	 * bind a event
	 */
	function bind($event,$callback){
		$this->events[strtolower($event)]=$callback;
	}

	/*
	 * trigger a event
	 */
	function trigger($event,$data=null){
		$event=strtolower($event);
		if(!isset($this->events[$event])){
			if($event=='receive')
				$this->send($data[1]);
		}elseif($this->events[$event]===null){
			server_log('Event "'.$event.'" not exists!');
		}elseif(is_array($data)){
			return call_user_func_array($this->events[$event],$data);
		}else{
			return call_user_func($this->events[$event],$data);
		}
	}

	/*
	 * create a new socket server
	 *
	 * @access public
	 * @param string   $domain   domain to bind to
	 * @param integer   $port   port to listen to
	 */
	function init($domain='localhost',$port=8083,$user='daemon'){
		$this->inited=true;
		$this->domain = $domain;
		$this->port   = $port;
		$this->user   = $user;

		if(EXT_POSIX){
			$uid_name = posix_getpwnam($this->user);
			$this->uid = $uid_name['uid'];
			$this->gid = $uid_name['gid'];
			$this->userHome = $uid_name['dir'];
			posix_setuid($this->uid) or server_log("Unable to setuid to " . $this->uid);
		}
	}
	function restart(){
		$this->stop();
		return $this->start();
	}

    /**
     * Signal handler
     */
    function signalHandler($sig){
        switch($sig){
			case SIGHUP:
            case SIGTERM:
            case SIGINT:
            case SIGKILL:
            case SIGSTOP:
			case SIGTSTP:
                $this->stop();
				exit(0);
				break;
			default:
				echo 'main signal:',$sig,PHP_EOL;
				flush();
        }
    }

	private $mutex;

	/*
	 * start the server
	 *
	 * @access public
	 * @param int $maxClients
	 */
	function start(){
		if($this->inited===false){
			echo 'Please run method init.';
			return false;
		}
		if($this->listened){
			return false;
		}
        declare(ticks=1);
		$this->clientFD=array();
		$this->clients=0;
		if(EXT_PCNTL){
			if(file_exists(PIDFILE)){
				echo 'Started wodome server.';
				flush();
				$cols=exec('tput cols');
				echo str_repeat('.',$cols-34);
				flush();
				system('echo -e "\\E[33m"[Keeped]');
				system('tput sgr0');
				return false;
			}else{
				$pid=pcntl_fork();
			}
			if($pid==-1){
				/* fork failed */
				echo "fork failure!",PHP_EOL;
				return false;
			}elseif($pid>0){
				/* close the parent */
				echo 'Starting wodome server.';
				usleep(100);
				$cols=exec('tput cols');
				echo str_repeat('.',$cols-35);
				flush();
				if($status=file_exists(PIDFILE)){
					system('echo -e "\\E[32m"[Succeed]');
				}else{
					system('echo -e "\\E[31m".[Failed]');
				}
				system('tput sgr0');
				return $status;
			}else{
				/* child becomes our daemon */
				chdir($this->userHome);
				umask(0);
				$sid=posix_setsid();
				if($sid<1)
					exit(-1);
			}
		}
		$this->initFD = @socket_create( AF_INET, SOCK_STREAM, SOL_TCP );
		if( !$this->initFD ){
			server_log('Could not create socket.');
			exit(-1);
		}

		socket_set_option($this->initFD, SOL_SOCKET, SO_REUSEADDR, 1);

		$sbuf=socket_get_option($this->initFD, SOL_SOCKET, SO_SNDBUF);
		if($sbuf<(SCK_WRITE_PACKET_SIZE*32))
			socket_set_option($this->initFD, SOL_SOCKET, SO_SNDBUF, SCK_WRITE_PACKET_SIZE*32);
		$rbuf=socket_get_option($this->initFD, SOL_SOCKET, SO_RCVBUF);
		if($rbuf<(SCK_READ_PACKET_SIZE*32))
			socket_set_option($this->initFD, SOL_SOCKET, SO_RCVBUF, SCK_READ_PACKET_SIZE*32);

		// adress may be reused
		socket_set_option( $this->initFD, SOL_SOCKET, SO_REUSEADDR, 1 );
		socket_set_nonblock($this->initFD);

		// bind the socket
		if( !@socket_bind( $this->initFD, $this->domain, $this->port ) ){
			@socket_close( $this->initFD );
			server_log('Could not bind socket to '.$this->domain.' on port '.$this->port);
			exit(-1);
		}

		// listen on selected port
		if( !@socket_listen( $this->initFD, $this->maxQueues ) ){
			server_log( 'Could not listen ( '.$this->getLastSocketError( $this->initFD ).' ).' );
			exit(-1);
		}

		if(EXT_POSIX && ($pid=posix_getpid())){
			LIB('io.file')->write(PIDFILE,$pid);
			pcntl_signal(SIGHUP,array(&$this, 'signalHandler'));
			pcntl_signal(SIGTERM,array(&$this, 'signalHandler'));
			pcntl_signal(SIGINT,array(&$this, 'signalHandler'));
			pcntl_signal(SIGKILL,array(&$this, 'signalHandler'));
			pcntl_signal(SIGSTOP,array(&$this, 'signalHandler'));
			pcntl_signal(SIGTSTP,array(&$this, 'signalHandler'));
		}

		$this->trigger('start');

		$this->listen();

		exit(0);
	}

	function listen(){
		$this->listened=true;
		while($this->listened){
			$newClient=$this->acceptConnection($this->initFD);
			if($newClient!==false && $this->maxClients>0 && $this->clients>$this->maxClients){
				$this->trigger('connectDenied',$newClient);
				$this->closeConnect($newClient);
			}
			if(EXT_PCNTL){
				if($newClient===false){
					usleep(100);
				}
				continue;
			}
			$readFDs=array_values($this->clientFD);
			$ready=@socket_select($readFDs,$this->null,$this->null,SCK_READ_SELECT_TIMEOUT);
			// check all clients for incoming data
			foreach($readFDs as $csock){
				socket_getpeername($csock,$peer_host,$peer_port);
				$this->readFromSocket($peer_port);
			}
		}
		$this->listened=false;
	}

	/*
	 * read from a socket
	 *
	 * @access private
	 * @param integer $clientId internal id of the client to read from
	 * @return string $data   data that was read
	 */
	function readFromSocket($clientId){
		$data='';
		while($buf=@socket_read($this->clientFD[$clientId],SCK_READ_PACKET_SIZE)){
			$data.=$buf;
		}
		if($buf===false){
			if(socket_last_error($this->clientFD[$clientId])==SOCKET_EWOULDBLOCK){
				socket_clear_error($this->clientFD[$clientId]);
			}else{
				server_log('Could not read from client '.$clientId.' ( '.$this->getLastSocketError($this->clientFD[$clientId]).' ).' );
				$this->closeConnect($clientId);
				return false;
			}
		}
		if($data===''){
			$this->closeConnect($clientId);
			return false;
		}
		$this->trigger('receive',array($clientId,$data));
		return true;
	}

	/*
	 * accept a new connection
	 *
	 * @access public
	 * @param resource &$socket socket that received the new connection
	 * @return int    $clientID internal ID of the client
	 */
	function acceptConnection(&$socket){
		$csock = @socket_accept($socket);
		if($csock===false){
			return false;
		}
		//set socket option
		socket_set_option($csock,SOL_SOCKET,SO_REUSEADDR,1);
		$sbuf=socket_get_option($csock,SOL_SOCKET,SO_SNDBUF);
		if($sbuf<(SCK_WRITE_PACKET_SIZE*32))
			socket_set_option($csock,SOL_SOCKET,SO_SNDBUF,SCK_WRITE_PACKET_SIZE*32);
		$rbuf=socket_get_option($csock,SOL_SOCKET,SO_RCVBUF);
		if($rbuf<(SCK_READ_PACKET_SIZE*32))
			socket_set_option($csock, SOL_SOCKET, SO_RCVBUF, SCK_READ_PACKET_SIZE*32);
		socket_set_nonblock($csock);

		$this->clients++;

		socket_getpeername($csock,$host,$port);
		$this->clientFD[$port]=&$csock;
		$this->trigger('connect',array($port,$host,$port));

		if(EXT_PCNTL){
			$pid=pcntl_fork();
			if($pid===0){
				while(is_resource($this->clientFD[$port]) && (posix_getppid()>1)){
					$readFD=array($csock);
					if(socket_select($readFD,$null,$null,SCK_READ_SELECT_TIMEOUT)>0){
						$this->readFromSocket($port);
					}
				}
				exit(0);
			}elseif($pid===-1){
				echo "fork failure!",PHP_EOL;
			}else{
				return $port;
			}
		}else{
			return $port;
		}
	}

	/*
	 * check, whether a client is still connected
	 *
	 * @access public
	 * @param integer $id client id
	 * @return boolean $connected true if client is connected, false otherwise
	 */
	function isConnected( $id ){
		return isset($this->clientFD[$id]);
	}

	/*
	 * close connection to a client
	 *
	 * @access public
	 * @param int $clientID internal ID of the client
	 */
	function closeConnect( $id ){
		if( !$this->isConnected($id))
			return false;

		$this->trigger('close', $id );

		@socket_shutdown($this->clientFD[$id]);
		@socket_close($this->clientFD[$id]);

		$this->clientFD[$id]=null;
		unset($this->clientFD[$id]);

		$this->clients--;
	}

	/*
	 * stop server
	 *
	 * @access public
	 */
	function stop(){
		if(!$this->listened){
			$result=false;
			if(EXT_POSIX){
				echo 'Stopping WoDoMe server';
				flush();
				if(file_exists(PIDFILE) && ($pid=intval(LIB('io.file')->read(PIDFILE))) && posix_getsid($pid)){
					$result=posix_kill($pid,SIGTERM);
					$i=22;
					while($result && ($pid==posix_getsid($pid))){
						echo '.';
						sleep(1);
						$i++;
					}
					$cols=exec('tput cols');
					echo str_repeat('.',$cols-12-$i);
					flush();
					system('echo -e "\\E[32m"[Succeed]');
					system('tput sgr0');
				}else{
					$cols=exec('tput cols');
					echo str_repeat('.',$cols-34);
					flush();
					system('echo -e "\\E[31m"[Failed]');
					system('tput sgr0');
				}
				unlink(PIDFILE);
			}else{
				echo 'The wodome server is not started.',PHP_EOL;
			}
			return $result;
		}

		$this->listened=false;

		foreach($this->clientFD as $i=>$csock){
			$this->closeConnect($i);
		}
		@socket_close($this->initFD);
		if(EXT_POSIX && EXT_PCNTL){
			file_exists(PIDFILE) and unlink(PIDFILE);
		}else{
			echo 'Stopped WoDoMe server.';
		}

		$this->trigger('stop');
		exit(0);
	}

	/*
	 * return string for last socket error
	 *
	 * @access public
	 * @return string $error last error
	 */
	private function getLastSocketError( &$fd ){
		$lastError = socket_last_error( $fd );
		return 'msg: ' . socket_strerror( $lastError ) . ' / Code: '.$lastError;
	}

	/*
	 * send data to a client
	 *
	 * @access public
	 * @param int   $clientId ID of the client
	 * @param string $data   data to send
	 */
	function sendTo( $clientId, $data){
		if( !isset( $this->clientFD[$clientId] ) || $this->clientFD[$clientId] == null )
			return false;
		if($this->events['send']!==null)
			$data=$this->trigger('send',array($clientId,$data));
		$sent_len=0;
		$send_len=strlen($data);
		while($sent_len<$send_len){
			$size=($sent_len+SCK_WRITE_PACKET_SIZE>$send_len?$send_len-$sent_len:SCK_WRITE_PACKET_SIZE);
			$ret=@socket_write($this->clientFD[$clientId],substr($data, $sent_len, $size),$size);
			if($ret===false){
				server_log('Could not write "'.$data.'" client '.$clientId.' ( '.$this->getLastSocketError($this->clientFD[$clientId]).' ).');
				return false;
			}
			$sent_len+=$ret;
		}
		return $sent_len==$send_len;
	}
	function sendToClient($clientId, $data){
		return sendTo($clientId, $data);
	}

	/*
	 * send data to all clients
	 *
	 * @access public
	 * @param string $data   data to send
	 * @param array $exclude client ids to exclude
	 */
	function send( $data, $exclude = array()){
		if( !empty( $exclude ) && !is_array( $exclude ) )
		$exclude = array( $exclude );

		foreach(array_keys($this->clientFD) as $key){
			if( !in_array( $i, $exclude ) ){
				$this->sendTo($i,$data);
			}
		}
	}
	function sendToAll($data, $exclude = array()){
		return $this->send($data, $exclude);
	}
}
