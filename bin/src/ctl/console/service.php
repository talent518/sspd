<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('ctl.base');
import('lib.xml');

Class CtlConsoleService extends CtlBase{
	function onState($request){
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');
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
		$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');

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
		$xml->sspTime=udate('Y-m-d H:i:s',SSP_TIME,$uid);
		$xml->sspRunTimed=timediff(time(),SSP_TIME);

		$xml->memoryPeakUsage=formatsize(memory_get_peak_usage(true));
		$xml->memoryUsage=formatsize(memory_get_usage(true));

		$xml->time=udate('Y-m-d H:i:s',time(),$uid);

		return $xml;
	}
}
