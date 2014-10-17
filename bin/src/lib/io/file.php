<?php
if (  ! defined('IN_SERVER') )
	exit('Access Denied');

class LibIoFile {

	function __construct () {
		$this->LibIoFile();
	}

	function LibIoFile () {
	}
	
	// 得到文件名.
	function ext ( $filename ) {
		return strtolower(pathinfo($filename, PATHINFO_EXTENSION));
	}

	function move ( $source, $target ) {
		if ( @rename($source, $target) ) {
			return true;
		} elseif ( function_exists('move_uploaded_file') && @move_uploaded_file($source, $target) ) {
			return true;
		} elseif ( @copy($source, $target) ) {
			@unlink($source);
			return true;
		} else {
			return false;
		}
	}

	function read ( $file ) {
		if ( ( $fp = @fopen($file, 'rb') ) !== false ) {
			flock($fp, LOCK_SH);
			$content = @fread($fp, filesize($file));
			flock($fp, LOCK_UN);
			@fclose($fp);
		}
		return ( $content );
	}

	function write ( $file, $content, $isAppend = false ) {
		$path = pathinfo($file, PATHINFO_DIRNAME);
		if (  ! is_dir($path) ) {
			mkdir($path, 777, true) or die('Create directory failed' . ( IS_DEBUG ? ':' . $path . '.' : '!' ));
		}
		
		if ( ( $fp = @fopen($file, $isAppend ? 'ab' : 'wb') ) !== false ) {
			flock($fp, LOCK_EX);
			fwrite($fp, $content);
			flock($fp, LOCK_UN);
			fclose($fp);
			return true;
		} else
			return false;
	}

}
