<?PHP
define('IN_SERVER', TRUE);
defined('IS_DEBUG') or define('IS_DEBUG', 0);

IS_DEBUG ? error_reporting(E_ALL &  ~ E_WARNING &  ~ E_NOTICE) : error_reporting(0);

date_default_timezone_set('PRC');

define('DIR_SEP', DIRECTORY_SEPARATOR);
define('ROOT', dirname(__FILE__) . DIR_SEP);
define('SRC_DIR', ROOT . 'src' . DIR_SEP);
define('LIB_DIR', SRC_DIR . 'lib' . DIR_SEP);
define('MOD_DIR', SRC_DIR . 'mod' . DIR_SEP);
define('CTL_DIR', SRC_DIR . 'ctl' . DIR_SEP);

define('LOG_DIR', ROOT . 'log' . DIR_SEP);

define('RES_DIR', ROOT . 'res' . DIR_SEP); // 资源目录
define('RES_FONT_DIR', RES_DIR . 'fonts' . DIR_SEP); // 字体目录
define('RES_IMAGE_DIR', RES_DIR . 'images' . DIR_SEP); // 图片目录
define('RES_THUMB_DIR', RES_DIR . 'thumb' . DIR_SEP); // 缩略图目录

define('IS_AVATAR_REAL', false);

CFG();
defined('UC_DIR') or define('UC_DIR', '');

function CFG () {
	if (  ! $cfgs = parse_ini_file(ROOT . 'config.ini', true) ) {
		$error = error_get_last();
		echo $error['message'];
		exit();
	}
	foreach ( $cfgs as $k => $v ) {
		if ( is_array($v) ) {
			foreach ( $v as $_k => $_v ) {
				$d = strtoupper($k . '_' . $_k);
				define($d, is_numeric($_v) ? $_v + 0 : $_v) or die($d);
			}
		} else {
			define(strtoupper($k), $v);
		}
	}
}

function &DB () {
	static $db;
	if (  ! is_object($db) ) {
		$db = LIB('Db.' . DB_TYPE);
		$db->charset = DB_CHARSET;
		$db->host = DB_HOST;
		$db->user = DB_USER;
		$db->pwd = DB_PWD;
		$db->name = DB_NAME;
		$db->pconnect = DB_PCONNECT;
		$db->tablepre = DB_TABLEPRE;
		$db->connect();
	}
	return $db;
}

function GD ( $dir ) {
	return str_replace('.', DIR_SEP, strtolower($dir));
}

function GN ( $dir ) {
	$dirs = array();
	foreach ( explode('.', strtolower($dir)) as $dir )
		$dirs[] = ucfirst($dir);
	$return = implode('', $dirs);
	$dirs = null;
	return $return;
}

function import ( $lib ) {
	include_once ( SRC_DIR . GD($lib) . '.php' );
}

function &LIB ( $lib ) {
	static $libs;
	if (  ! is_object($libs[$lib]) ) {
		import('lib.' . $lib);
		$class = 'Lib' . GN($lib);
		$libs[$lib] = ( class_exists($class) ? new $class() : die('class "' . $class . '" not exists!') );
	}
	return $libs[$lib];
}

function &MOD ( $mod ) {
	static $mods;
	if (  ! is_object($mods[$mod]) ) {
		import('mod.' . $mod);
		$class = 'Mod' . GN($mod);
		$mods[$mod] = ( class_exists($class) ? new $class() : die('class "' . $class . '" not exists!') );
	}
	return $mods[$mod];
}

function &CTL ( $ctl ) {
	static $ctls;
	if (  ! is_object($ctls[$ctl]) ) {
		import('ctl.' . $ctl);
		$class = 'Ctl' . GN($ctl);
		$ctls[$ctl] = ( class_exists($class) ? new $class() : die('class "' . $class . '" not exists!') );
	}
	return $ctls[$ctl];
}

function saddslashes ( $string ) {
	if ( is_array($string) ) {
		foreach ( $string as $key => $val ) {
			$string[$key] = saddslashes($val);
		}
	} else {
		$string = addslashes($string);
	}
	return $string;
}

// 去掉slassh
function sstripslashes ( $string ) {
	if ( is_array($string) ) {
		foreach ( $string as $key => $val ) {
			$string[$key] = sstripslashes($val);
		}
	} else {
		$string = stripslashes($string);
	}
	return $string;
}

// 连接整数
function iimplode ( &$ints ) {
	foreach ( $ints as $k => $v )
		$ints[$k] = intval($v);
	return implode(',', $ints);
}

// 连接字符
function simplode ( &$strs ) {
	return "'" . implode("','", $strs) . "'";
}

function formatsecond ( $second, $default = '现在' ) {
	$td = array();
	$td['second'] = $second;
	$td['mintue'] = $td['second'] / 60;
	$td['hour'] = $td['mintue'] / 60;
	$td['day'] = $td['hour'] / 24;
	$td['week'] = $td['day'] / 7;
	$td['month'] = $td['day'] / 30;
	$td['year'] = $td['day'] / 365;
	
	if ( $td['year'] > 1 ) {
		return round($td['year'], 2) . '年';
	} elseif ( $td['month'] > 1 ) {
		return round($td['month'], 2) . '月';
	} elseif ( $td['week'] > 1 ) {
		return round($td['week'], 2) . '周';
	} elseif ( $td['day'] > 1 ) {
		return round($td['day'], 2) . '天';
	} elseif ( $td['hour'] > 1 ) {
		return round($td['hour'], 2) . '小时';
	} elseif ( $td['mintue'] > 1 ) {
		return round($td['mintue'], 2) . '分钟';
	} elseif ( $td['second'] > 1 ) {
		return round($td['second'], 2) . '秒';
	} else {
		return $default;
	}
}

// 取消HTML代码
function shtmlspecialchars ( $string ) {
	if ( is_array($string) ) {
		foreach ( $string as $key => $val ) {
			$string[$key] = shtmlspecialchars($val);
		}
	} else {
		$string = preg_replace('/&amp;((#(\d{3,5}|x[a-fA-F0-9]{4})|[a-zA-Z][a-z0-9]{2,5});)/', '&\\1', str_replace(array(
			'&', 
			'"', 
			'<', 
			'>'
		), array(
			'&amp;', 
			'&quot;', 
			'&lt;', 
			'&gt;'
		), $string));
	}
	return $string;
}

// 格式化大小函数
function formatsize ( $size ) {
	$size = round(abs($size));
	$units = array(
		0 => 'B', 
		1 => 'KB', 
		2 => 'MB', 
		3 => 'GB', 
		4 => 'TB'
	);
	$unit = ( $size < 1024 ? 0 : min(4, floor(log($size, 1024))) );
	$size = ( $unit == 0 ? $size : round($size / pow(1024, $unit), 3) );
	return $size . $units[$unit];
}

function smicrotime () {
	list ( $usec, $sec ) = explode(' ', microtime());
	return bcadd($usec, $sec, 8);
}

function correct_charset ( $string ) {
	if ( STD_CHARSET != 'utf-8' && LIB('string')->is_utf8($string) )
		return LIB('string')->charset($string, 'utf-8', STD_CHARSET);
	else
		return $string;
}

function str_encode ( $string, $key = SSP_KEY, $expiry = 0 ) {
	return LIB('crypt')->encode(( string ) $string, $key, $expiry);
}

function str_decode ( $string, $key = SSP_KEY, $expiry = 0 ) {
	return LIB('crypt')->decode(( string ) $string, $key, $expiry);
}

// 字符截取...
function strcut ( $string, $length, $isHTML = false, $suffix = '…' ) {
	if ( $isHTML ) {
		$string = strip_tags($string);
		$string = str_replace(array(
			'&nbsp;', 
			'&quot;', 
			'&lt;', 
			'&gt;', 
			'&ldquo;', 
			'&rdquo;'
		), array(
			' ', 
			'"', 
			'<', 
			'>', 
			'“', 
			'”'
		), $string);
		$string = preg_replace("/\s{2,}/", " ", trim($string));
	}
	return LIB('string')->cut($string, 0, $length, 'utf-8', $suffix);
}

function udate ( $format, $time, $uid ) {
	return gmdate($format, $time + MOD('user.online')->get_by_user($uid, 'timezone'));
}

function cdate ( $format, $time, $cid ) {
	return gmdate($format, $time + MOD('user.online')->get_by_client(ssp_info($cid, 'index'), 'timezone'));
}

function CGK ( $gid, $key = false ) {
	return MOD('user.group')->get($gid, $key);
}

function UGK ( $uid, $key, $isCheckExpiry = true ) {
	$gid = MOD('user.online')->get_by_user($uid, 'gid');
	if ( $isCheckExpiry && CGK($gid, 'use_expiry') && MOD('user.setting')->get($uid, 'expiry') < time() ) {
		return 0;
	}
	return CGK($gid, $key);
}

function MUK ( $uid, $key ) {
	$ret = UGK($uid, 'manage', false);
	if ( $ret ) {
		return $ret;
	} else {
		return UGK($uid, 'manage_' . $key, false);
	}
}

function get_limit ( $page, $size, $count ) {
	$size = abs($size < 10 ? 10 : ( int ) $size);
	$page = abs($page < 1 ? 1 : ( int ) $page);
	$count = abs(( int ) $count);
	$pages = ( int ) ( $count / $size ) + ( $count % $size ? 1 : 0 );
	if ( $pages > 1 && $page > $pages ) {
		$page = $pages;
	}
	$start = ( $page - 1 ) * $size;
	return ( $start < 0 ? 0 : $start ) . ',' . $size;
}