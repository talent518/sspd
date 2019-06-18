<?php
import('ctl.base');
import('lib.xml');

class CtlStock extends CtlBase {

	function onState ( $request ) {
		$uid = MOD('user.online')->get_by_client(ssp_info($request->ClientId, 'index'), 'uid');
		$response = new XML_Element('response');
		if ( UGK($uid, 'stock_add') ) {
			$response->type = 'Stock.State.Succeed';
			$response->state = 'add';
		} elseif ( UGK($uid, 'stock_eval') ) {
			$response->type = 'Stock.State.Succeed';
			$response->state = 'eval';
		} elseif ( UGK($uid, 'stock') ) {
			$response->type = 'Stock.State.Succeed';
			$response->state = 'normal';
		} else {
			$response->type = 'Stock.State.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		return $response;
	}

	function onList ( $request, $page = 1, $size = 10, $isEval = 0 ) {
		$xml = new XML_Element('response');
		$xml->type = 'Stock.List';
		if ( is_object($request) ) {
			$uid = MOD('user.online')->get_by_client(ssp_info($request->ClientId, 'index'), 'uid');
			$page = ( string ) ( $request->params->page ) + 0;
			$size = ( string ) ( $request->params->size ) + 0;
			$isEval = ( string ) ( $request->params->isEval ) + 0;
		} else {
			$uid = $request + 0;
		}
		if (  !  ! ( $iseval = UGK($uid, 'stock_eval') ) ) {
			if ( $isEval ) {
				$where = 'evaluid=0 OR evaluid=-' . $uid;
			} else {
				$where = 'evaluid=' . $uid;
				$iseval = 0;
				$order = 'evaldate DESC';
			}
		} else {
			$where = 'uid=' . $uid;
			if ( $isEval ) {
				$where .= ' AND evaluid>0';
				$order = 'evaldate DESC';
			} else {
				$order = 'sid ASC,evaldate ASC';
			}
		}
		$xml->counts = MOD('user.stock')->count($where);
		$limit = get_limit($page, $size, $xml->counts);
		$stockList = MOD('user.stock')->get_list_by_where($where, $limit, $order);
		foreach ( $stockList as $r ) {
			$r['code'] = substr('000000' . $r['code'],  - 6);
			$r['dateline'] = udate('m-d H:i', $r['dateline'], $uid);
			$r['dealdate'] = udate('Y-m-d', $r['dealdate'], $uid);
			if ( $r['evaluid'] > 0 ) {
				$r['evaldate'] = udate('m-d H:i', $r['evaldate'], $uid);
			} else {
				$r['iseval'] = $iseval;
			}
			$xml->$r['sid'] = array_to_xml($r, 'stock');
		}
		return $xml;
	}

	function onAdd ( $request ) {
		$index = ssp_info($request->ClientId, 'index');
		$uid = MOD('user.online')->get_by_client($index, 'uid');
		if (  ! UGK($uid, 'stock_add') ) {
			$response = new XML_Element('response');
			$response->type = 'Stock.Add.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$params = &$request->params;
		$data = array(
			'uid' => $uid, 
			'code' => ( string ) ( $params->code ), 
			'name' => ( string ) ( $params->name ), 
			'type' => ( string ) ( $params->type ), 
			'dealdate' => ( string ) ( $params->dealdate ), 
			'amount' => ( string ) ( $params->amount ), 
			'location' => ( string ) ( $params->location ), 
			'price' => ( string ) ( $params->price ), 
			'stoploss' => ( string ) ( $params->stoploss ), 
			'reason' => ( string ) ( $params->reason ), 
			'profitloss' => ( string ) ( $params->profitloss ), 
			'dateline' => time()
		);
		$response = new XML_Element('response');
		if ( MOD('user.stock')->add($data) ) {
			$response->type = 'Stock.Add.Succeed';
			$response->setText('提交成功！');
			
			$data['sid'] = DB()->insert_id();
			
			$remind = new XML_Element('response');
			$remind->type = 'Remind.OS';
			foreach ( MOD('user.online')->get_list_by_where('uid>0') as $sf => $r ) {
				if ( UGK($r['uid'], 'stock_eval') ) {
					$res = ssp_resource($sf);
					ssp_send($res, ( string ) $remind);
					ssp_destroy($res);
				}
			}
		} elseif (  !  ! ( $error = MOD('user.stock')->error ) ) {
			$response->type = 'Stock.Add.Failed';
			$response->setText($error);
		} else {
			$response->type = 'Stock.Add.Failed';
			$response->setText('未知错误!');
		}
		return $response;
	}

	function onEdit ( $request ) {
		$index = ssp_info($request->ClientId, 'index');
		$uid = MOD('user.online')->get_by_client($index, 'uid');
		if (  ! UGK($uid, 'stock_add') ) {
			$response = new XML_Element('response');
			$response->type = 'Stock.Edit.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$params = &$request->params;
		$sid = ( string ) ( $params->sid );
		$data = array(
			'uid' => $uid, 
			'code' => ( string ) ( $params->code ), 
			'name' => ( string ) ( $params->name ), 
			'type' => ( string ) ( $params->type ), 
			'dealdate' => ( string ) ( $params->dealdate ), 
			'amount' => ( string ) ( $params->amount ), 
			'location' => ( string ) ( $params->location ), 
			'price' => ( string ) ( $params->price ), 
			'stoploss' => ( string ) ( $params->stoploss ), 
			'reason' => ( string ) ( $params->reason ), 
			'profitloss' => ( string ) ( $params->profitloss ), 
			'dateline' => time(), 
			'evaluid' => 0, 
			'evaluation' => '', 
			'evaldate' => 0, 
			'isread' => 0, 
			'readtime' => 0
		);
		$response = new XML_Element('response');
		if ( MOD('user.stock')->edit($sid, $data) ) {
			$response->type = 'Stock.Edit.Succeed';
			$response->setText('提交成功！');
			
			$remind = new XML_Element('response');
			$remind->type = 'Remind.OS';
			foreach ( MOD('user.online')->get_list_by_where('uid>0') as $sf => $r ) {
				if ( UGK($r['uid'], 'stock_eval') ) {
					$res = ssp_resource($sf);
					ssp_send($res, ( string ) $remind);
					ssp_destroy($res);
				}
			}
		} elseif (  !  ! ( $error = MOD('user.stock')->error ) ) {
			$response->type = 'Stock.Edit.Failed';
			$response->setText($error);
		} else {
			$response->type = 'Stock.Edit.Failed';
			$response->setText('未知错误!');
		}
		return $response;
	}

	function onDrop ( $request ) {
		$index = ssp_info($request->ClientId, 'index');
		$uid = MOD('user.online')->get_by_client($index, 'uid');
		if (  ! UGK($uid, 'stock_add') ) {
			$response = new XML_Element('response');
			$response->type = 'Stock.Drop.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$sid = ( string ) ( $request->params->sid ) + 0;
		$response = new XML_Element('response');
		if ( MOD('user.stock')->drop($sid) ) {
			$response->type = 'Stock.Drop.Succeed';
			$response->setText('删除成功！');
		} else {
			$response->type = 'Stock.Drop.Failed';
			$response->setText('删除失败!');
		}
		return $response;
	}

	function onView ( $request ) {
		$index = ssp_info($request->ClientId, 'index');
		$uid = MOD('user.online')->get_by_client($index, 'uid');
		$sid = ( string ) ( $request->params->sid ) + 0;
		$response = new XML_Element('response');
		$stock = MOD('user.stock')->get($sid);
		if ( $stock ) {
			$response->type = 'Stock.View.Succeed';
			$stock['code'] = substr('000000' . $stock['code'],  - 6);
			$stock['dateline'] = udate('m-d H:i', $stock['dateline'], $uid);
			$stock['dealdate'] = udate('Y-m-d', $stock['dealdate'], $uid);
			if ( $stock['evaluid'] > 0 ) {
				$user = MOD('user')->get($stock['evaluid']);
				$stock['evalusername'] = $user['username'];
				$stock['evaldate'] = udate('Y-m-d H:i', $stock['evaldate'], $uid);
				$stock['evalavatar'] = avatar($stock['evaluid'], 'small');
				if (  ! $stock['isread'] ) {
					MOD('user.stock')->update(array(
						'isread' => 1, 
						'readtime' => time()
					), $sid, false);
				}
				$response->state = 'view';
			} elseif ( UGK($uid, 'stock_eval') ) {
				$response->state = 'eval';
				MOD('user.stock')->update(array(
					'evaluid' =>  - $uid
				), $sid, false);
			} else {
				$response->state = 'none';
			}
			$response->stock = array_to_xml($stock, 'stock');
		} else {
			$response->type = 'Stock.View.Failed';
			$response->setText('股票操作记录不存在!');
		}
		return $response;
	}

	function onEval ( $request ) {
		$index = ssp_info($request->ClientId, 'index');
		$uid = MOD('user.online')->get_by_client($index, 'uid');
		if (  ! UGK($uid, 'stock_eval') ) {
			$response = new XML_Element('response');
			$response->type = 'Stock.Eval.Failed';
			$response->setText(USER_NOPRIV_MSG);
		}
		$params = &$request->params;
		$sid = ( string ) ( $params->sid ) + 0;
		$data = array(
			'evaluid' => $uid, 
			'evaluation' => ( string ) ( $params->evaluation ), 
			'evaldate' => time()
		);
		$response = new XML_Element('response');
		if ( empty($data['evaluation']) ) {
			$response->type = 'Stock.Eval.Failed';
			$response->setText('请输入评语！');
		} elseif ( MOD('user.stock')->edit($sid, $data, false) ) {
			$response->type = 'Stock.Eval.Succeed';
			$response->setText('提交成功！');
			$stock = MOD('user.stock')->get($sid);
			if (  !  ! ( $sf = MOD('user.online')->get_by_user($stock['uid'], 'id') ) ) {
				$remind = new XML_Element('response');
				$remind->type = 'Remind.OS';
				if ( UGK($stock['uid'], 'stock_add') ) {
					$res = ssp_resource($sf);
					ssp_send($res, ( string ) $remind);
					ssp_destroy($res);
				}
			}
		} elseif (  !  ! ( $error = MOD('user.stock')->error ) ) {
			$response->type = 'Stock.Eval.Failed';
			$response->setText($error);
		} else {
			$response->type = 'Stock.Eval.Failed';
			$response->setText('未知错误!');
		}
		return $response;
	}

}
