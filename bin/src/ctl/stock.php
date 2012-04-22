<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('ctl.base');
import('lib.xml');

Class CtlStock extends CtlBase{
	function onRemind($request){
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$users=MOD('user.online')->get_list_by_where();
		$count=count($users)-1;
		$sends=0;
		$response=new XML_Element('response');
		$response->type='Remind.Stock';
		$response->stock=$request->stock;
		$dateline=intval((string)$request->stock->dateline);
		foreach($users as $r){
			if($sockfd!==$r['onid']){
				$response->stock->dateline=gmdate('m-d H:i',$dataline-$r['timezone']);
				$this->send($r['onid'],$response);
				$sends++;
			}
		}
		$response=new XML_Element('response');
		$response->type='Stock.Remind.Succeed';
		$response->setText('提醒成功！有'.sprintf('%s/%s',$sends,$count).'收到了通知！');
		return $response;
	}
}
