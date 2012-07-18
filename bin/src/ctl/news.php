<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('ctl.base');
import('lib.xml');

Class CtlNews extends CtlBase{
	function url_replace($url){
		return preg_match("/^(https?|ftp|gopher|news|telnet|rtsp|mms|callto|bctp|thunder|qqdl|synacast)\:\/\//",$url)?$url:WEB_URL.$url;
	}
	function onCategory($request){
		$xml=new XML_Element('response');
		$xml->type='News.Category';
		$xml->{0}=new XML_Element('category');
		$xml->{0}->gid=0;
		$xml->{0}->title='所有信息';
		$counts=0;$creads=0;
		$groups=MOD('news.group')->get_list_by_where();
		$reads=MOD('news')->get_user_by_reads(is_object($request)?MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid'):$request+0);
		foreach($groups as $r){
			$xml->$r['gid']=new XML_Element('category');
			$xml->$r['gid']->gid=$r['gid'];
			$xml->$r['gid']->title=$r['gname'];
			$xml->$r['gid']->reads=$reads[$r['gid']]+0;
			$xml->$r['gid']->unreads=$r['counts']-$reads[$r['gid']];
			$xml->$r['gid']->counts=$r['counts']+0;
			$creads+=$reads[$r['gid']];
			$counts+=$r['counts'];
		}
		$xml->{0}->reads=$creads;
		$xml->{0}->unreads=$counts-$creads;
		$xml->{0}->counts=$counts;
		return $xml;
	}
	function onList($request,$gid=0,$page=1,$size=10){
		$xml=new XML_Element('response');
		$xml->type='News.List';
		if(is_object($request)){
			$uid=MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid');
			$gid=(string)$request->params->gid+0;
			$page=(string)$request->params->page+0;
			$size=(string)$request->params->size+0;
		}else{
			$uid=$request+0;
		}
		$gid=($gid<0?0:(int)$gid);
		$size=($size<10)?10:(int)$size;
		$page=($page<1)?1:(int)$page;
		$limit=(($page-1)*$size).','.$size;
		$newsList=MOD('news')->get_list_by_user($uid,$gid,$limit);
		foreach($newsList as $r){
			$r['dateline']=udate('m-d H:i',$r['dateline'],$uid);
			if($r['isread'] && $r['readtime'])
				$r['readtime']=udate('m-d H:i',$r['readtime'],$uid);
			$xml->$r['aid']=array_to_xml($r,'news');
		}
		return $xml;
	}
	function onView($request){
		$response=new XML_Element('response');
		$aid=(string)($request->params->aid)+0;
		if($news=MOD('news')->get($aid)){
			$data=array(
					'uid'=>MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid'),
					'aid'=>$news['aid'],
					'isread'=>1,
					'readtime'=>time(),
				);
			MOD('user.news')->add($data,false,true);
			$cat=MOD('news.group')->get($news['gid']);
			$response->type='News.View.Succeed';
			$response->category=array_to_xml($cat,'category');
			$content=$news['content'];
			$news['content']=null;unset($news['content']);
			$news['dateline']=cdate('m-d H:i',$news['dateline'],$request->ClientId);
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
		$sockfd=ssp_info($request->ClientId,'sockfd');
		$users=MOD('user.online')->get_list_by_where();
		$count=count($users)-1;
		$sends=0;
		$response=new XML_Element('response');
		$response->type='Remind.News';
		$response->news=$request->news;
		$dateline=intval((string)$request->news->dateline);
		foreach($users as $r){
			if($sockfd!==$r['onid']){
				$response->news->dateline=cdate('m-d H:i',$dataline,$request->ClientId);
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
