<?php
import('ctl.base');
import('lib.xml');

class CtlConsoleServGroup extends CtlBase {

	function onState ( $request ) {
		$uid = MOD('user.online')->get_by_client(ssp_info($request->ClientId, 'index'), 'uid');
		$response = new XML_Element('response');
		if ( MUK($uid, 'user_serv_group') ) {
			$response->type = 'Console.ServGroup.State.Succeed';
		} else {
			$response->type = 'Console.ServGroup.State.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		return $response;
	}

	function onList ( $request ) {
		$uid = MOD('user.online')->get_by_client(ssp_info($request->ClientId, 'index'), 'uid');
		$page = ( string ) ( $request->params->page ) + 0;
		$size = ( string ) ( $request->params->size ) + 0;
		
		$xml = new XML_Element('response');
		if (  ! MUK($uid, 'user_serv_group') ) {
			$xml->type = 'Console.ServGroup.List.Failed';
			$xml->setText(USER_NOPRIV_MSG);
			return $xml;
		}
		
		$xml->type = 'Console.ServGroup.List.Succeed';
		$groups = MOD('user.serv.group')->get_list_by_where();
		foreach ( $groups as $gid => $r ) {
			$xml->$gid = array_to_xml($r, 'group');
		}
		return $xml;
	}

	function onAdd ( $request ) {
		$index = ssp_info($request->ClientId, 'index');
		$uid = MOD('user.online')->get_by_client($index, 'uid');
		if (  ! MUK($uid, 'user_serv_group') ) {
			$response = new XML_Element('response');
			$response->type = 'Console.ServGroup.Add.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$data = array(
			'name' => ( string ) ( $request->params->name ), 
			'remark' => ( string ) ( $request->params->remark )
		);
		$response = new XML_Element('response');
		if ( MOD('user.serv.group')->add($data) ) {
			$response->type = 'Console.ServGroup.Add.Succeed';
			$response->setText('提交成功！');
		} elseif (  !  ! ( $error = MOD('user.serv.group')->error ) ) {
			$response->type = 'Console.ServGroup.Add.Failed';
			$response->setText($error);
		} else {
			$response->type = 'Console.ServGroup.Add.Failed';
			$response->setText('未知错误!');
		}
		return $response;
	}

	function onEdit ( $request ) {
		$index = ssp_info($request->ClientId, 'index');
		$uid = MOD('user.online')->get_by_client($index, 'uid');
		if (  ! MUK($uid, 'user_serv_group') ) {
			$response = new XML_Element('response');
			$response->type = 'Console.ServGroup.Edit.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$gid = ( string ) ( $request->params->gid ) + 0;
		$response = new XML_Element('response');
		if (  !  ! ( $group = MOD('user.serv.group')->get($gid) ) ) {
			$response->type = 'Console.ServGroup.Edit.Succeed';
			$response->group = array_to_xml($group, 'group');
		} else {
			$response->type = 'Console.ServGroup.Edit.Failed';
			$response->setText('用户组不存在!');
		}
		return $response;
	}

	function onEditSave ( $request ) {
		$index = ssp_info($request->ClientId, 'index');
		$uid = MOD('user.online')->get_by_client($index, 'uid');
		if (  ! MUK($uid, 'user_serv_group') ) {
			$response = new XML_Element('response');
			$response->type = 'Console.ServGroup.EditSave.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$gid = ( string ) ( $request->params->gid ) + 0;
		$response = new XML_Element('response');
		if (  !  ! ( $group = MOD('user.serv.group')->get($gid) ) ) {
			$data = array(
				'name' => ( string ) ( $request->params->name ), 
				'remark' => ( string ) ( $request->params->remark )
			);
			if (  ! MOD('user.serv.group')->edit($gid, $data) ) {
				$response->type = 'Console.ServGroup.EditSave.Failed';
				$response->setText(MOD('user.serv.group')->error ? MOD('user.serv.group')->error : '未知错误！');
			} else {
				$response->type = 'Console.ServGroup.EditSave.Succeed';
				$response->setText('保存成功！');
			}
		} else {
			$response->type = 'Console.ServGroup.EditSave.Failed';
			$response->setText('客户分组不存在!');
		}
		return $response;
	}

	function onDrop ( $request ) {
		$index = ssp_info($request->ClientId, 'index');
		$uid = MOD('user.online')->get_by_client($index, 'uid');
		$response = new XML_Element('response');
		if (  ! MUK($uid, 'user_serv_group') ) {
			$response->type = 'Console.ServGroup.Drop.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$gid = ( string ) ( $request->params->gid ) + 0;
		if ( MOD('user.serv.group')->drop($gid) ) {
			$response->type = 'Console.ServGroup.Drop.Succeed';
			$response->setText('删除成功！');
		} else {
			$response->type = 'Console.ServGroup.Drop.Failed';
			$response->setText(MOD('user.serv.group')->error ? MOD('user.serv.group')->error : '未知错误！');
		}
		return $response;
	}

}
