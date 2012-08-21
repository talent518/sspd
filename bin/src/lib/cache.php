<?php
if(!defined('IN_SERVER')) {
	exit('Access Denied');
}

function &ssp_attach($key,$memsize=2097152,$perm=0777){
	return array(shm_attach($key,$memsize,$perm),sem_get($key,1,$perm));
}
function ssp_has_var(&$shmid,$key){
	return shm_has_var($shmid[0],$key);
}
function ssp_get_var($shmid,$key){
	return shm_get_var($shmid[0],$key);
}
function ssp_set_var(&$shmid,$key,$value){
	$sret=sem_acquire($shmid[1]);
	$ret=shm_put_var($shmid[0],$key,$value);
	if($sret){
		sem_release($shmid[1]);
	}
	return $ret;
}
function ssp_remove_var(&$shmid,$key){
	$sret=sem_acquire($shmid[1]);
	$ret=shm_remove_var($shmid[0],$key);
	if($sret){
		sem_release($shmid[1]);
	}
	return $ret;
}
function ssp_remove(&$shmid){
	return shm_remove($shmid[0]);
}
function ssp_detach(&$shmid){
	shm_detach($shmid[0]);
	sem_remove($shmid[1]);
	return true;
}
