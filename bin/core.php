<?php
defined('IS_DEBUG') or define('IS_DEBUG', 1);

error_reporting(E_ALL & ~ E_WARNING &  ~ E_NOTICE & ~ E_STRICT);

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
	if (  ! isset($_SSP['DB']) ) {
		$_SSP['DB'] = LIB('Db.' . DB_TYPE);
		$_SSP['DB']->charset = DB_CHARSET;
		$_SSP['DB']->host = DB_HOST;
		$_SSP['DB']->user = DB_USER;
		$_SSP['DB']->pwd = DB_PWD;
		$_SSP['DB']->socket = DB_SOCKET;
		$_SSP['DB']->name = DB_NAME;
		$_SSP['DB']->pconnect = DB_PCONNECT;
		$_SSP['DB']->tablepre = DB_TABLEPRE;
		$_SSP['DB']->connect();
	}
	return $_SSP['DB'];
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

function LIB ( $lib ) {
	if (  ! isset($_SSP['LIB'][$lib]) ) {
		import('lib.' . $lib);
		$class = 'Lib' . GN($lib);
		$_SSP['LIB'][$lib] = ( class_exists($class) ? new $class() : die('class "' . $class . '" not exists!') );
	}
	return $_SSP['LIB'][$lib];
}

function MOD ( $mod ) {
	if (  ! isset($_SSP['MOD'][$mod]) ) {
		import('mod.' . $mod);
		$class = 'Mod' . GN($mod);
		$_SSP['MOD'][$mod] = ( class_exists($class) ? new $class() : die('class "' . $class . '" not exists!') );
	}
	return $_SSP['MOD'][$mod];
}

function CTL ( $ctl ) {
	if (  ! isset($_SSP['CTL'][$ctl]) ) {
		import('ctl.' . $ctl);
		$class = 'Ctl' . GN($ctl);
		$_SSP['CTL'][$ctl] = ( class_exists($class) ? new $class() : die('class "' . $class . '" not exists!') );
	}
	return $_SSP['CTL'][$ctl];
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

// 检查头像是否上传
function ckavatar ( $uid ) {
	if ( UC_DIR ) {
		$file = UC_DIR . './data/avatar/' . avatar_dir($uid, 'middle');
		return file_exists($file) ? 1 : 0;
	} else {
		import('api.uc.client');
		$type = ( IS_AVATAR_REAL ? 'real' : 'virtual' );
		return uc_check_avatar($uid, 'middle', $type);
	}
}

function avatar_file ( $uid, $size ) {
	$afile = '';
	$dfile = '';
	if ( UC_DIR ) {
		$file = UC_DIR . str_replace('/', DIR_SEP, 'data/avatar/' . avatar_dir($uid, $size));
		if ( file_exists($file) )
			return file_get_contents($file);
		else
			return file_get_contents(UC_DIR . 'images' . DIR_SEP . 'noavatar_' . $size . '.gif');
	} else {
		$content = file_get_contents(avatar_url($uid, $size));
		if ( $content === false )
			return file_get_contents(UC_API . '/images/noavatar_' . $size . '.gif');
		else
			return $content;
	}
}

function avatar_url ( $uid, $size ) {
	return UC_API . ( ckavatar($uid) ? '/data/avatar/' . avatar_dir($uid, $size) : '/images/noavatar_' . $size . '.gif' );
}

// 得到头像
function avatar_dir ( $uid, $size ) {
	$type = ( IS_AVATAR_REAL ? 'real' : 'virtual' );
	$var = "{$uid}_{$size}_{$type}";
	if (  ! isset($_SSP['AVR'][$var]) ) {
		$uid = abs(intval($uid));
		$uid = sprintf("%09d", $uid);
		$dir1 = substr($uid, 0, 3);
		$dir2 = substr($uid, 3, 2);
		$dir3 = substr($uid, 5, 2);
		$typeadd = $type == 'real' ? '_real' : '';
		$_SSP['AVR'][$var] = $dir1 . '/' . $dir2 . '/' . $dir3 . '/' . substr($uid,  - 2) . $typeadd . "_avatar_$size.jpg";
	}
	return $_SSP['AVR'][$var];
}

// 处理头像
function avatar ( $uid, $size = 'small', $isfile = FALSE ) {
	return $isfile ? avatar_file($uid, $size) : avatar_url($uid, $size);
}
