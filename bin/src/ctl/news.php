<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('ctl.base');
import('lib.xml');

Class CtlNews extends CtlBase{
	function url_replace($url){
		return eregi("/^https?\:\/\//",$url)?$url:'http://www.wodome.com/'.$url;
	}
	function onView($request){
		$response=new XML_Element('response');
		$aid=(string)($request->params->aid)+0;
		if($news=MOD('news')->get($aid)){
			$data=array(
					'uid'=>MOD('user.online')->get_by_client($request->ClientId,'uid'),
					'aid'=>$news['aid'],
					'isread'=>1,
					'readtime'=>time(),
				);
			MOD('user.news')->add($data,false,true);
			$cat=MOD('news.group')->get($news['gid']);
			$response->type='News.View.Succeed';
			$response->category=array_to_xml($cat,'category');
			$content=$news['content'];
			$news['content']=null;
			unset($news['content']);
			$news['dateline']=gmdate('m-d H:i',$news['dateline']-MOD('user.online')->get_by_client($request->ClientId,'timezone'));
			$response->news=array_to_xml($news,'news');
			$content=preg_replace("/\<img\s+src\=\"(.+?)\"/ie","'<img src=\"'.\$this->url_replace('\\1').'\"'",$content);
			$response->news->setText($content);
		}else{
			$response->type='News.View.Failed';
			$response->setText('信息不存在！');
		}
		return $response;
	}
	function onRemind($request){
		$users=MOD('user.online')->get_list_by_where();
		$count=count($users)-1;
		$sends=0;
		$response=new XML_Element('response');
		$response->type='Remind.News';
		$response->news=$request->news;
		$dateline=intval((string)$request->news->dateline);
		foreach($users as $r){
			if($request->ClientId!==$r['onid']){
				$response->news->dateline=gmdate('m-d H:i',$dataline-$r['timezone']);
				$this->send($r['onid'],$response);
				$sends++;
			}
		}
		$response=new XML_Element('response');
		$response->type='News.Remind.Succeed';
		$response->setText('提醒成功！有'.sprintf('%s/%s',$sends,$count).'收到了通知！');
		return $response;
	}
}
