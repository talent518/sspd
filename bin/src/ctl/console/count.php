<?php
if (  ! defined('IN_SERVER') )
	exit('Access Denied');

import('ctl.base');
import('lib.xml');

class CtlConsoleCount extends CtlBase {

	function onState ( $request ) {
		$uid = MOD('user.online')->get_by_client(ssp_info($request->ClientId, 'index'), 'uid');
		$response = new XML_Element('response');
		if ( MUK($uid, 'count') ) {
			$response->type = 'Console.Count.State.Succeed';
		} else {
			$response->type = 'Console.Count.State.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		return $response;
	}

	function onInfo ( $request ) {
		$uid = MOD('user.online')->get_by_client(ssp_info($request->ClientId, 'index'), 'uid');
		
		$xml = new XML_Element('response');
		if (  ! MUK($uid, 'count') ) {
			$xml->type = 'Console.Count.Info.Failed';
			$xml->setText(USER_NOPRIV_MSG);
			return $xml;
		}
		
		$xml->type = 'Console.Count.Info.Succeed';
		
		$xml->all = array_to_xml(MOD('count')->get(), 'all');
		$xml->today = array_to_xml(MOD('count')->today(), 'today');
		$xml->week = array_to_xml(MOD('count')->week(), 'week');
		
		return $xml;
	}

}
