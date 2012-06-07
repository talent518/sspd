<?PHP
define('IN_SERVER',TRUE);
defined('IS_DEBUG') or define('IS_DEBUG',false);

IS_DEBUG?error_reporting(E_ALL & ~E_WARNING & ~E_NOTICE):error_reporting(0);

date_default_timezone_set('PRC');

define('DIR_SEP',DIRECTORY_SEPARATOR);
define('ROOT',dirname(__FILE__).DIR_SEP);
define('SRC_DIR',ROOT.'src'.DIR_SEP);
define('LIB_DIR',SRC_DIR.'lib'.DIR_SEP);
define('MOD_DIR',SRC_DIR.'mod'.DIR_SEP);
define('CTL_DIR',SRC_DIR.'ctl'.DIR_SEP);

define('LOG_DIR',ROOT.'log'.DIR_SEP);

define('RES_DIR',ROOT.'res'.DIR_SEP);//资源目录
define('RES_FONT_DIR',RES_DIR.'fonts'.DIR_SEP);//字体目录
define('RES_IMAGE_DIR',RES_DIR.'images'.DIR_SEP);//图片目录
define('RES_THUMB_DIR',RES_DIR.'thumb'.DIR_SEP);//缩略图目录

define('IS_AVATAR_REAL',false);

CFG();
defined('SSP_HOST') or define('SSP_HOST','localhost');
defined('SSP_PORT') or define('SSP_PORT',8083);
defined('SSP_KEY') or define('SSP_KEY',LIB('string')->rand(32,STRING_RAND_BOTH));
defined('UC_DIR') or define('UC_DIR','');

function CFG(){
	if(!$cfgs=parse_ini_file(ROOT.'config.ini',true)){
		$error=error_get_last();
		echo $error['message'];
		exit;
	}
	foreach($cfgs as $k=>$v){
		if(is_array($v)){
			foreach($v as $_k=>$_v){
				$d=strtoupper($k.'_'.$_k);
				define($d,is_numeric($_v)?$_v+0:$_v) or die($d);
			}
		}else{
			define(strtoupper($k),$v);
		}
	}
}

function &DB(){
	static $db;
	if(!$db){
		$db=LIB('Db.'.DB_TYPE);
		$db->charset=DB_CHARSET;
		$db->host=DB_HOST;
		$db->user=DB_USER;
		$db->pwd=DB_PWD;
		$db->name=DB_NAME;
		$db->pconnect=DB_PCONNECT;
		$db->tablepre=DB_TABLEPRE;
		$db->connect();
	}
	return $db;
}

function GD($dir){
	return str_replace('.',DIR_SEP,strtolower($dir));
}
function GN($dir){
	$dirs=array();
	foreach(explode('.',strtolower($dir)) as $dir)
		$dirs[]=ucfirst($dir);
	return implode('',$dirs);
}

function import($lib){
	return @include_once(SRC_DIR.GD($lib).'.php');
}

function &LIB($lib){
	static $libs;
	if(!isset($libs[$lib])){
		import('lib.'.$lib);
		$class='Lib'.GN($lib);
		$libs[$lib]=(class_exists($class)?new $class():die('class "'.$class.'" not exists!'));
	}
	return $libs[$lib];
}

function &MOD($mod){
	static $mods;
	if(!isset($mods[$mod])){
		import('mod.'.$mod);
		$class='Mod'.GN($mod);
		$mods[$mod]=(class_exists($class)?new $class():die('class "'.$class.'" not exists!'));
	}
	return $mods[$mod];
}

function &CTL($ctl,$is_new=true){
	static $ctls;
	if(!isset($ctls[$ctl])){
		import('ctl.'.$ctl);
		$class='Ctl'.GN($ctl);
		$ctls[$ctl]=(class_exists($class)?new $class():false);
	}
	return $ctls[$ctl];
}

function saddslashes($string) {
	if(is_array($string)){
		foreach($string as $key=>$val){
			$string[$key]=saddslashes($val);
		}
	}else{
		$string=addslashes($string);
	}
	return $string;
}

//去掉slassh
function sstripslashes($string){
	if(is_array($string)){
		foreach($string as $key=>$val){
			$string[$key]=sstripslashes($val);
		}
	}else{
		$string=stripslashes($string);
	}
	return $string;
}

//连接整数
function iimplode(&$ints){
	foreach($ints as $k=>$v)
		$ints[$k]=intval($v);
	return implode(',',$ints);
}

//连接字符
function simplode(&$strs) {
	return "'".implode("','", $strs)."'";
}

//计算两个时间之差的函数(年,月,周,日,小时,分钟,秒数) 
function timediff($aTime,$bTime){
	$td=array();
	$td['second']=$aTime-$bTime;
	$td['mintue']=round($td ['second']/60);
	$td['hour']=round($td ['mintue']/60);
	$td['day']=round($td ['hour']/24);
	$td['week']=round($td ['day']/7);
	$td['month']=round($td ['day']/30);
	$td['year']=round($td ['day']/365);
	return $td;
}

//最小时间之差
function lowtimediff($aTime,$bTime){
	$td=timediff($aTime,$bTime);
	if($td['year']>0){
		return $td['year'].'年';
	}elseif($td['month']>0){
		return $td['month'].'月';
	}elseif($td['week']>0){
		return $td['week'].'周';
	}elseif($td['day']>0){
		return $td['day'].'天';
	}elseif($td['hour']>0){
		return $td['hour'].'小时';
	}elseif($td['mintue']>0){
		return $td['mintue'].'分钟';
	}elseif($td['second']>0){
		return $td['second'].'秒';
	}else{
		return '现在';
	}
}

//取消HTML代码
function shtmlspecialchars($string){
	if(is_array($string)){
		foreach($string as $key => $val){
			$string[$key]=shtmlspecialchars($val);
		}
	} else {
		$string=preg_replace('/&amp;((#(\d{3,5}|x[a-fA-F0-9]{4})|[a-zA-Z][a-z0-9]{2,5});)/', '&\\1',
			str_replace(array('&', '"', '<', '>'), array('&amp;', '&quot;', '&lt;', '&gt;'), $string));
	}
	return $string;
}

//格式化大小函数
function formatsize($size){
	$size=round(abs($size));
	$units=array(0=>'B',1=>'KB',2=>'MB',3=>'GB',4=>'TB');
	$unit=($size<1024?0:min(4,floor(log($size,1024))));
	$size=($unit==0?$size:round($size/pow(1024,$unit),3));
	return $size.$units[$unit];
}

function smicrotime(){
    list($usec,$sec)=explode(' ',microtime());
    return bcadd($usec,$sec,8);
}

function correct_charset($string){
	if(STD_CHARSET!='utf-8' && LIB('string')->is_utf8($string))
		return LIB('string')->charset($string,'utf-8',STD_CHARSET);
	else
		return $string;
}

function str_encode($string,$key=SSP_KEY,$expiry=0){
	return LIB('crypt')->encode((string)$string,$key,$expiry);
}

function str_decode($string,$key=SSP_KEY,$expiry=0){
	return LIB('crypt')->decode((string)$string,$key,$expiry);
}

function udate($format,$time,$uid){
	return gmdate($format,$time+MOD('user.online')->get_by_user($uid,'timezone'));
}

function cdate($format,$time,$cid){
	return gmdate($format,$time+MOD('user.online')->get_by_client(ssp_info($cid,'sockfd'),'timezone'));
}

function UGK($uid,$key=''){
	$gid=MOD('user.online')->get_by_user($uid,'gid');
	$group=MOD('user.group')->get($gid);
	return empty($key)?$group:$group[$key];
}

function get_limit($page,$size,$count){
	$size=($size<10?10:(int)$size);
	$page=($page<1?1:(int)$page);
	$pages=(int)($count/$size)+($count%$size?1:0);
	if($pages>1 && $page>$pages){
		$page=$pages;
	}
	return (($page-1)*$size).','.$size;
}