<?php
if(!defined('IN_SERVER'))
exit('Access Denied');

class LibIoFile{
	function __construct(){
		$this->LibIoFile();
	}
	function LibIoFile(){
	}

	//得到文件名.
	function ext($filename) {
		return strtolower(pathinfo($filename,PATHINFO_EXTENSION));
	}

	function move($source,$target){
		if(@rename($source,$target)){
			return true;
		}elseif(function_exists('move_uploaded_file') && @move_uploaded_file($source,$target)){
			return true;
		}elseif(@copy($source,$target)){
			@unlink($source);
			return true;
		}else{
			return false;
		}
	}

	function read($file){
		if($fp=@fopen($file, 'rb')){
			$content=@fread($fp, filesize($file));
			@fclose($fp);
		}
		return($content);
	}

	function write($file,$content){
		$path=pathinfo($file,PATHINFO_DIRNAME);
		if(!is_dir($path)){
			mkdir($path,777,true) or die('Create directory failed'.(IS_DEBUG?':'.$path.'.':'!'));
		}

		if($fp=@fopen($file,'wb')){
			flock($fp,2);
			fwrite($fp,$content);
			fclose($fp);
			return true;
		}else
			return false;
	}
}
