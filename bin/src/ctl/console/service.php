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

		$stime=filemtime(ROOT.'log'.DIR_SEP.'ssp.pid');

		$xml->sspTime=udate('Y-m-d H:i:s',$stime,$uid);
		$xml->sspRunTimed=formatsecond(time()-$stime);

		$xml->memoryPeakUsage=formatsize(memory_get_peak_usage(true));
		$xml->memoryUsage=formatsize(memory_get_usage(true));

		$xml->time=udate('Y-m-d H:i:s',time(),$uid);

		$xml->online=DB()->count('user_online','uid>0').'/'.DB()->count('user_online');
/*
 * @array ssp_mallinfo()
 *
 * @desc 内存信息：
 * @key arena 用于malloc 的数据段大小
 * @key ordblks 闲置空间
 *
 * 内存分配调试:
 * @key smblks; 快速垃圾箱数
 * @key hblks; 匿名映射数
 * @key hblkhd; 匿名映射大小
 * @key usmblks; 最大分配大小
 * @key fsmblks; 可用快速垃圾箱的大小
 * @key uordblks; 总分配空间的大小
 * @key fordblks; 可用块的大小
 * @key keepcost; 微调空间的大小
*/
		foreach(ssp_mallinfo() as $k=>$v){
			$xml->$k=formatsize($v);
		}

		return $xml;
	}
}
