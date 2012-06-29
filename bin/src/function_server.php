<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

function data_log($message){
	if(empty($message))
		return false;
	if(IS_DEBUG){
		echo PHP_EOL,correct_charset($message),PHP_EOL;
		flush();
	}else{
		return write_log('data',$message);
	}
}

function server_log($message){
	if(empty($message))
		return false;
	if(IS_DEBUG){
		echo PHP_EOL,correct_charset($message),PHP_EOL;
		flush();
	}else{
		return write_log('server',$message);
	}
}

function write_log($type,$message){
	static $types,$mutex;
	if(!$types){
		$types=array();
	}
	if(!is_resource($mutex)){
		$mutex=ssp_mutex_create();
	}
	if(!isset($types[$type]) && file_exists(LOG_DIR.$type.'_size.log')){
		ssp_mutex_lock($mutex);
		$types[$type]=LIB('io.file')->read(LOG_DIR.$type.'_size.log')+0;
		ssp_mutex_unlock($mutex);
	}

	$n=(isset($types[$type])?$types[$type]:1);

	$file=LOG_DIR.$type.sprintf('-%05s',$n).'.log';
	while(file_exists($file) && filesize($file)>2097152){
		$n++;
		$file=LOG_DIR.$type.sprintf('-%05s',$n).'.log';
	}

	if($types[$type]!=$n){
		ssp_mutex_lock($mutex);
		$types[$type]=$n;
		ssp_mutex_unlock($mutex);
		LIB('io.file')->write(LOG_DIR.$type.'-size.log',$n);
	}
	return LIB('io.file')->write($file,$message.PHP_EOL.PHP_EOL,true);
}

//检查头像是否上传
function ckavatar($uid) {
	if(UC_DIR){
		$file = UC_DIR.'./data/avatar/'.avatar_dir($uid, 'middle');
		return file_exists($file)?1:0;
	}else{
		import('api.uc.client');
		$type = (IS_AVATAR_REAL?'real':'virtual');
		return uc_check_avatar($uid, 'middle', $type);
	}
}

function avatar_file($uid,$size){
	$afile='';
	$dfile='';
	if(UC_DIR){
		$file=UC_DIR.str_replace('/',DIR_SEP,'data/avatar/'.atatar_dir($uid,$size));
		if(file_exists($file))
			return file_get_contents($file);
		else
			return file_get_contents(UC_DIR.'images'.DIR_SEP.'noavatar_'.$size.'.gif');
	}else{
		$content=file_get_contents(avatar_url($uid,$size));
		if($content===false)
			return file_get_contents(UC_API.'/images/noavatar_'.$size.'.gif');
		else
			return $content;
	}
}

function avatar_url($uid,$size){
	return UC_API.(ckavatar($uid)?'/data/avatar/'.avatar_dir($uid, $size):'/images/noavatar_'.$size.'.gif');
}

//得到头像
function avatar_dir($uid, $size) {
	static $avatars;
	if(!$avatars)
		$avatars=array();
	$type = (IS_AVATAR_REAL?'real':'virtual');
	$var = "{$uid}_{$size}_{$type}";
	if(!isset($avatars[$var])) {
		$uid = abs(intval($uid));
		$uid = sprintf("%09d", $uid);
		$dir1 = substr($uid, 0, 3);
		$dir2 = substr($uid, 3, 2);
		$dir3 = substr($uid, 5, 2);
		$typeadd = $type == 'real' ? '_real' : '';
		$avatars[$var] = $dir1.'/'.$dir2.'/'.$dir3.'/'.substr($uid, -2).$typeadd."_avatar_$size.jpg";
	}
	return $avatars[$var];
}

//处理头像
function avatar($uid, $size='small', $isfile=FALSE) {
	return $isfile?avatar_file($uid,$size):avatar_url($uid,$size);
}
