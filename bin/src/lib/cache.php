<?php
if(!defined('IN_SERVER')) {
	exit('Access Denied');
}

class LibCache{
	private $caches;
    public $dir,$name,$callback;

	function __construct(){
		$this->LibCache();
	}

	public function LibCache(){
		$this->caches=array();
	}

	//设置
    public function set($data){
		$this->caches[$this->key()]=$data;
		if(empty($data)){
			$this->drop();
			return false;
		}
		$data="<?php if(!defined('IN_SERVER')) exit('Access Denied');\r\nreturn ".var_export($data,true).";";
		return L('io.file')->write($this->file(),$data);
    }
    //获取
    public function &get($minute=0){
		$key=$this->key();
		$file=$this->file();
		if(isset($this->caches[$key])){
			return($this->caches[$key]);
		}
		if(!file_exists($file)){
			$this->set($this->getData($this->callback));
		}elseif($minute>0 && filemtime($file)+$minute*60<TIMESTAMP){
			$this->set($this->getData($this->callback));
		}else{
			$this->caches[$key]=@include($file);
		}
		return $this->caches[$key];
    }
    //删除
    public function drop(){
		$key=$this->key();
		$this->caches[$key]=null;unset($this->caches[$key]);
        return @unlink($this->file());
    }
	//获取数据
	public function getData(&$callback){
		list($object,$name,$param)=$callback;
		return call_user_method_array($name,$object,$param);
	}
	//文件
	private function file(){
		if(!(CFG()->isEncrypt && is_dir(DATA_DIR.$this->dir))){
			@mkdir(DATA_DIR.$this->dir,777,true);
			@chmod(DATA_DIR.$this->dir,777);
		}
		$file=($this->dir?$this->dir.'/':null).$this->name;
		return DATA_DIR.(CFG()->isEncrypt?md5($file):$file).'.php';
	}
	private function key(){
		return md5($this->dir.'_'.$this->name);
	}
}
