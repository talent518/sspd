<?php
import('mod.uc.base');

class ModUcUser extends ModUcBase {

	protected $table = 'members';

	protected $priKey = 'uid';

	protected $order;

}
