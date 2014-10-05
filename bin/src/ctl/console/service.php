<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('ctl.base');
import('lib.xml');

Class CtlConsoleService extends CtlBase{
	function onState($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'index'),'uid');
		$response=new XML_Element('response');
		if(MUK($uid,'service')){
			$response->type='Console.Service.State.Succeed';
		}else{
			$response->type='Console.Service.State.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		return $response;
	}
	function onInfo($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'index'),'uid');

		$xml=new XML_Element('response');
		if(!MUK($uid,'service')){
			$xml->type='Console.Service.Info.Failed';
			$xml->setText(USER_NOPRIV_MSG);
			return $xml;
		}

		$xml->type='Console.Service.Info.Succeed';

		$xml->runtime=array_to_xml(MOD('count')->runtime(),'runtime');

		$xml->sspVersion='SSP '.SSP_VERSION;
		$xml->phpVersion='PHP '.PHP_VERSION;

		$stime=filemtime(SSP_PIDFILE);

		$xml->sspTime=udate('Y-m-d H:i:s',$stime,$uid);
		$xml->sspRunTimed=formatsecond(time()-$stime);

		$xml->time=udate('Y-m-d H:i:s',time(),$uid);

		$xml->online=DB()->count('user_online','uid>0').'/'.DB()->count('user_online');

		$stats=ssp_stats();
		foreach($stats as $k=>$v) {
			$xml->$k=$v;
		}

		$xml->sysCPUus=round($stats['sysinfo']['us'],3).'%';
		$xml->sysCPUsy=round($stats['sysinfo']['sy'],3).'%';
		$xml->sysCPUid=round($stats['sysinfo']['id'],3).'%';
		$xml->sysCPUni=round($stats['sysinfo']['ni'],3).'%';
		$xml->sysMemTotal=formatsize($stats['sysinfo']['memTotal']);
		$xml->sysMemUsed=formatsize($stats['sysinfo']['memUsed']);
		$xml->procCPU=round($stats['procinfo']['pcpu'],3).'%';
		$xml->procMem=formatsize($stats['procinfo']['rss']);

		return $xml;
	}
}
