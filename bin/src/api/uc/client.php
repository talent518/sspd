<?php
if(!defined('UC_API')) {
	exit('Access denied');
}

define('UC_USER_CHECK_USERNAME_FAILED', -1);
define('UC_USER_USERNAME_BADWORD', -2);
define('UC_USER_USERNAME_EXISTS', -3);
define('UC_USER_EMAIL_FORMAT_ILLEGAL', -4);
define('UC_USER_EMAIL_ACCESS_ILLEGAL', -5);
define('UC_USER_EMAIL_EXISTS', -6);


function uc_api_input($data) {
	$s = urlencode(uc_authcode($data.'&agent='.md5($_SERVER['HTTP_USER_AGENT'])."&time=".time(), 'ENCODE', UC_KEY));
	return $s;
}

function uc_authcode($string, $operation = 'DECODE', $key = '', $expiry = 0) {

	$ckey_length = 4;

	$key = md5($key ? $key : UC_KEY);
	$keya = md5(substr($key, 0, 16));
	$keyb = md5(substr($key, 16, 16));
	$keyc = $ckey_length ? ($operation == 'DECODE' ? substr($string, 0, $ckey_length): substr(md5(microtime()), -$ckey_length)) : '';

	$cryptkey = $keya.md5($keya.$keyc);
	$key_length = strlen($cryptkey);

	$string = $operation == 'DECODE' ? base64_decode(substr($string, $ckey_length)) : sprintf('%010d', $expiry ? $expiry + time() : 0).substr(md5($string.$keyb), 0, 16).$string;
	$string_length = strlen($string);

	$result = '';
	$box = range(0, 255);

	$rndkey = array();
	for($i = 0; $i <= 255; $i++) {
		$rndkey[$i] = ord($cryptkey[$i % $key_length]);
	}

	for($j = $i = 0; $i < 256; $i++) {
		$j = ($j + $box[$i] + $rndkey[$i]) % 256;
		$tmp = $box[$i];
		$box[$i] = $box[$j];
		$box[$j] = $tmp;
	}

	for($a = $j = $i = 0; $i < $string_length; $i++) {
		$a = ($a + 1) % 256;
		$j = ($j + $box[$a]) % 256;
		$tmp = $box[$a];
		$box[$a] = $box[$j];
		$box[$j] = $tmp;
		$result .= chr(ord($string[$i]) ^ ($box[($box[$a] + $box[$j]) % 256]));
	}

	if($operation == 'DECODE') {
		if((substr($result, 0, 10) == 0 || substr($result, 0, 10) - time() > 0) && substr($result, 10, 16) == substr(md5(substr($result, 26).$keyb), 0, 16)) {
			return substr($result, 26);
		} else {
			return '';
		}
	} else {
		return $keyc.str_replace('=', '', base64_encode($result));
	}
}

function uc_fopen2($url, $limit = 0, $post = '', $cookie = '', $bysocket = FALSE, $ip = '', $timeout = 15, $block = TRUE) {
	$__times__ = isset($_GET['__times__']) ? intval($_GET['__times__']) + 1 : 1;
	if($__times__ > 2) {
		return '';
	}
	$url .= (strpos($url, '?') === FALSE ? '?' : '&')."__times__=$__times__";
	return uc_fopen($url, $limit, $post, $cookie, $bysocket, $ip, $timeout, $block);
}

function uc_fopen($url, $limit = 0, $post = '', $cookie = '', $bysocket = FALSE, $ip = '', $timeout = 15, $block = TRUE) {
	$return = '';
	$matches = parse_url($url);
	!isset($matches['host']) && $matches['host'] = '';
	!isset($matches['path']) && $matches['path'] = '';
	!isset($matches['query']) && $matches['query'] = '';
	!isset($matches['port']) && $matches['port'] = '';
	$host = $matches['host'];
	$path = $matches['path'] ? $matches['path'].($matches['query'] ? '?'.$matches['query'] : '') : '/';
	$port = !empty($matches['port']) ? $matches['port'] : 80;
	if($post) {
		$out = "POST $path HTTP/1.0\r\n";
		$out .= "Accept: */*\r\n";
		//$out .= "Referer: $boardurl\r\n";
		$out .= "Accept-Language: zh-cn\r\n";
		$out .= "Content-Type: application/x-www-form-urlencoded\r\n";
		$out .= "User-Agent: $_SERVER[HTTP_USER_AGENT]\r\n";
		$out .= "Host: $host\r\n";
		$out .= 'Content-Length: '.strlen($post)."\r\n";
		$out .= "Connection: Close\r\n";
		$out .= "Cache-Control: no-cache\r\n";
		$out .= "Cookie: $cookie\r\n\r\n";
		$out .= $post;
	} else {
		$out = "GET $path HTTP/1.0\r\n";
		$out .= "Accept: */*\r\n";
		//$out .= "Referer: $boardurl\r\n";
		$out .= "Accept-Language: zh-cn\r\n";
		$out .= "User-Agent: $_SERVER[HTTP_USER_AGENT]\r\n";
		$out .= "Host: $host\r\n";
		$out .= "Connection: Close\r\n";
		$out .= "Cookie: $cookie\r\n\r\n";
	}

	if(function_exists('fsockopen')) {
		$fp = @fsockopen(($ip ? $ip : $host), $port, $errno, $errstr, $timeout);
	} elseif (function_exists('pfsockopen')) {
		$fp = @pfsockopen(($ip ? $ip : $host), $port, $errno, $errstr, $timeout);
	} else {
		$fp = false;
	}

	if(!$fp) {
		return '';
	} else {
		stream_set_blocking($fp, $block);
		stream_set_timeout($fp, $timeout);
		@fwrite($fp, $out);
		$status = stream_get_meta_data($fp);
		if(!$status['timed_out']) {
			while (!feof($fp)) {
				if(($header = @fgets($fp)) && ($header == "\r\n" ||  $header == "\n")) {
					break;
				}
			}

			$stop = false;
			while(!feof($fp) && !$stop) {
				$data = fread($fp, ($limit == 0 || $limit > 8192 ? 8192 : $limit));
				$return .= $data;
				if($limit) {
					$limit -= strlen($data);
					$stop = $limit <= 0;
				}
			}
		}
		@fclose($fp);
		return $return;
	}
}

function _check_username($username) {
	$guestexp = '\xA1\xA1|\xAC\xA3|^Guest|^\xD3\xCE\xBF\xCD|\xB9\x43\xAB\xC8';
	$len = LIB('string')->len($username);
	if($len > 15 || $len < 3 || preg_match("/\s+|^c:\\con\\con|[%,\*\"\s\<\>\&]|$guestexp/is", $username)) {
		return FALSE;
	} else {
		return LIB('validate')->email($username);
	}
}
function _check_usernamecensor($username){
	return true;
}
function _check_emailaccess($email){
	return true;
}
function _quescrypt($questionid, $answer) {
	return $questionid > 0 && $answer != '' ? substr(md5($answer.md5($questionid)), 16, 8) : '';
}

function uc_check_username($username) {
	$username = addslashes(trim(stripslashes($username)));
	if(!_check_username($username)) {
		return UC_USER_CHECK_USERNAME_FAILED;
	} elseif(!_check_usernamecensor($username)) {
		return UC_USER_USERNAME_BADWORD;
	} elseif(MOD('uc.user')->exists_by_where(sprintf('`username`=\'%s\'',$username),'uid')) {
		return UC_USER_USERNAME_EXISTS;
	}
	return 1;
}
function uc_check_email($email, $username = '') {
	if(!LIB('validate')->email($email)) {
		return UC_USER_EMAIL_FORMAT_ILLEGAL;
	} elseif(!_check_emailaccess($email)) {
		return UC_USER_EMAIL_ACCESS_ILLEGAL;
	} elseif(!UC_DOUBLEE && MOD('uc.user')->exists_by_where(sprintf('`email`=\'%s\'',$email).($username?sprintf(' AND `username`=\'%s\'',addslashes($username)):''))) {
		return UC_USER_EMAIL_EXISTS;
	} else {
		return 1;
	}
}

function uc_user_register($username, $password, $email, $questionid = '', $answer = '', $regip = '') {
	if(($status = uc_check_username($username)) < 0) {
		return $status;
	}
	if(($status = uc_check_email($email)) < 0) {
		return $status;
	}
	$salt = substr(uniqid(rand()), -6);
	$password = md5(md5($password).$salt);
	MOD('uc.user')->add(array(
		'uid'=>$uid,
		'username'=>$username,
		'password'=>$password,
		'email'=>$email,
		'regip'=>(empty($regip)?'unknown':$regip),
		'regdate'=>time(),
		'salt'=>$salt,
		'secques'=>($questionid>0?_quescrypt($questionid, $answer):''),
	));
	$uid = UDB()->insert_id();
	UDB()->insert('memberfields',array('uid'=>$uid));
	return $uid;
}
function uc_user_login($username, $password, $isuid = 0, $checkques = 0, $questionid = '', $answer = '') {
	switch($isuid){
		case 1:
			$key='uid';
			break;
		case 2:
			$key='email';
			break;
		default:
			$key='username';
			break;
	}
	$user=MOD('uc.user')->get_by_where(sprintf('`%s`=\'%s\'',$key,addslashes($username)));

	$passwordmd5 = preg_match('/^\w{32}$/', $password) ? $password : md5($password);

	if(empty($user)) {
		$status = -1;
	} elseif($user['password'] != md5($passwordmd5.$user['salt'])) {
		$status = -2;
	} elseif($checkques && $user['secques'] != '' && $user['secques'] != _quescrypt($questionid, $answer)) {
		$status = -3;
	} else {
		$status = $user['uid'];
	}
	return array($status, $user['username'], $password, $user['email']);
}
function uc_user_edit($username, $oldpw, $newpw, $email, $ignoreoldpw = 0, $questionid = '', $answer = '') {
	if(!$ignoreoldpw && $email && ($status = uc_check_email($email, $username)) < 0) {
		return $status;
	}

	$where=sprintf('`username`=\'%s\'',addslashes($username));

	$user = MOD('uc.user')->get_by_where($where);

	if($ignoreoldpw) {
		$isprotected = $this->db->result_first("SELECT COUNT(*) FROM ".UC_DBTABLEPRE."protectedmembers WHERE uid = '$data[uid]'");
		if($isprotected) {
			return -8;
		}
	}

	if(!$ignoreoldpw && $user['password'] != md5(md5($oldpw).$user['salt'])) {
		return -1;
	}

	$data=array();

	if($newpw){
		$data['password']=md5(md5($newpw).$data['salt']);
	}
	if($email){
		$data['email']=$email;
	}

	if($questionid!==''){
		if($questionid>0){
			$data['secques']=_quescrypt($questionid, $answer);
		}else{
			$data['secques']='';
		}
	}

	if(count($data)) {
		MOD('uc.user')->update($data,$where);
		return $this->db->affected_rows();
	} else {
		return -7;
	}
	
	$status = $_ENV['user']->edit_user($username, $oldpw, $newpw, $email, $ignoreoldpw, $questionid, $answer);

	if($newpw && $status > 0) {
		$this->load('note');
		MOD('uc.user')->add('updatepw', 'username='.urlencode($username).'&password=');
	}
	return $status;
}
function uc_user_delete($uid) {
}
function uc_get_user($username, $isuid=0) {
	switch($isuid){
		case 1:
			$key='uid';
			break;
		default:
			$key='username';
			break;
	}
	$status=MOD('uc.user')->get_by_where(sprintf('`%s`=\'%s\'',$isuid?'uid':'username',addslashes($username)));
	return $status?array($status['uid'],$status['username'],$status['email']):0;
}

function uc_avatar($uid, $type = 'virtual', $returnhtml = 1) {
	$uid = intval($uid);
	$uc_input = uc_api_input("uid=$uid");
	$uc_avatarflash = UC_API.'/images/camera.swf?inajax=1&appid='.UC_APPID.'&input='.$uc_input.'&agent='.md5($_SERVER['HTTP_USER_AGENT']).'&ucapi='.urlencode(str_replace('http://', '', UC_API)).'&avatartype='.$type.'&uploadSize=2048';
	if($returnhtml) {
		return '<object classid="clsid:d27cdb6e-ae6d-11cf-96b8-444553540000" codebase="http://download.macromedia.com/pub/shockwave/cabs/flash/swflash.cab#version=9,0,0,0" width="450" height="253" id="mycamera" align="middle">
			<param name="allowScriptAccess" value="always" />
			<param name="scale" value="exactfit" />
			<param name="wmode" value="transparent" />
			<param name="quality" value="high" />
			<param name="bgcolor" value="#ffffff" />
			<param name="movie" value="'.$uc_avatarflash.'" />
			<param name="menu" value="false" />
			<embed src="'.$uc_avatarflash.'" quality="high" bgcolor="#ffffff" width="450" height="253" name="mycamera" align="middle" allowScriptAccess="always" allowFullScreen="false" scale="exactfit"  wmode="transparent" type="application/x-shockwave-flash" pluginspage="http://www.macromedia.com/go/getflashplayer" />
		</object>';
	} else {
		return array(
			'width', '450',
			'height', '253',
			'scale', 'exactfit',
			'src', $uc_avatarflash,
			'id', 'mycamera',
			'name', 'mycamera',
			'quality','high',
			'bgcolor','#ffffff',
			'menu', 'false',
			'swLiveConnect', 'true',
			'allowScriptAccess', 'always'
		);
	}
}
function uc_check_avatar($uid, $size = 'middle', $type = 'virtual') {
	$url = UC_API."/avatar.php?uid=$uid&size=$size&type=$type&check_file_exists=1";
	$res = uc_fopen2($url, 500000, '', '', TRUE, UC_IP, 20);
	if($res == 1) {
		return 1;
	} else {
		return 0;
	}
}
