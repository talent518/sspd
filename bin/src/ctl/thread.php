<?php
if(!defined('IN_SERVER'))
	exit('Access Denied');

import('ctl.base');
import('lib.xml');

Class CtlThread extends CtlBase{
	function url_replace($url){
		return preg_match("/^(https?|ftp|gopher|news|telnet|rtsp|mms|callto|bctp|thunder|qqdl|synacast)\:\/\//",$url)?$url:WEB_URL.$url;
	}
	function onCategory($request){
		$xml=new XML_Element('response');
		$xml->type='Thread.Category';
		$xml->{0}=new XML_Element('category');
		$xml->{0}->gid=0;
		$xml->{0}->title='所有信息';
		$counts=0;$creads=0;
		$groups=MOD('thread.group')->get_list_by_where();
		$reads=MOD('thread')->get_user_by_reads(is_object($request)?MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid'):$request+0);
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
	function _get_summary_by_content($content){
		$content = preg_replace(array("/\[attach\].*?\[\/attach\]/", "/\&[a-z]+\;/i", "/\<script.*?\<\/script\>/"), '', $content);
		$content = preg_replace("/\[.*?\]/", '', $content);
		$content = preg_replace("/[\r\n\t ]{2,}/", ' ', strip_tags($content));
		$content = LIB('string')->cut(trim($content),0,200);
		return $content;
	}
	function onList($request,$gid=0,$page=1,$size=10){
		$xml=new XML_Element('response');
		$xml->type='Thread.List';
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
		$threadList=MOD('thread')->get_list_by_user($uid,$gid,$limit);
		foreach($threadList as $r){
			$r['summary']=$this->_get_summary_by_content($r['content']);
			$r['content']=null;
			unset($r['content']);
			$r['dateline']=udate('m-d H:i',$r['dateline'],$uid);
			if($r['isread'] && $r['readtime'])
				$r['readtime']=udate('m-d H:i',$r['readtime'],$uid);
			$xml->$r['tid']=array_to_xml($r,'thread');
		}
		return $xml;
	}
	function onView($request){
		$response=new XML_Element('response');
		$tid=(string)($request->params->tid)+0;
		if($thread=MOD('thread')->get($tid)){
			$data=array(
					'uid'=>MOD('user.online')->get_by_client(ssp_info($request->ClientId,'sockfd'),'uid'),
					'tid'=>$thread['tid'],
					'isread'=>1,
					'readtime'=>time(),
				);
			MOD('user.thread')->add($data,false,true);
			$cat=MOD('thread.group')->get($thread['gid']);
			$response->type='Thread.View.Succeed';
			$response->category=array_to_xml($cat,'category');
			$content=MOD('thread.coder')->coder($thread['tid'],$data['uid'],$thread['uid'],$thread['content']);
			$thread['content']=null;
			unset($thread['content']);
			$thread['dateline']=cdate('m-d H:i',$thread['dateline'],$request->ClientId);
			$response->thread=array_to_xml($thread,'thread');
			$content=preg_replace("/\<img\s+src\=\"(.+?)\"/ie","'<img src=\"'.\$this->url_replace('\\1').'\"'",$content);
			$response->thread->setText($content);
		}else{
			$response->type='Thread.View.Failed';
			$response->setText('信息不存在！');
		}
		return $response;
	}
	function onRemind($request){
		$users=MOD('user.online')->get_list_by_where('onid!='.ssp_info($request->ClientId,'sockfd'));
		$count=count($users);
		$sends=0;
		$response=new XML_Element('response');
		$response->type='Remind.Thread';
		$response->thread=$request->thread;
		$dateline=intval((string)$request->thread->dateline);
		foreach($users as $r){
			$response->thread->dateline=cdate('m-d H:i',$dataline,$request->ClientId);
			if($this->send($r['onid'],$response))
				$sends++;
		}
		$response=new XML_Element('response');
		$response->type='Thread.Remind.Succeed';
		$response->setText('提醒成功！有'.sprintf('%s/%s',$sends,$count).'收到了通知！');
		return $response;
	}
}
