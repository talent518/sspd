USE `fenxihui`;

CREATE TABLE `cs_broadcast` (
  `bid` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `uid` INT(10) UNSIGNED NOT NULL,
  `message` TEXT NOT NULL,
  `dateline` INT(11) NOT NULL,
  `dateday` INT(11) NOT NULL,
  PRIMARY KEY (`bid`),
  KEY `uid` (`uid`),
  KEY `dateday` (`dateday`)
) ENGINE=MYISAM DEFAULT CHARSET=utf8;
ALTER TABLE `cs_user_group`
  ADD COLUMN `broadcast` TINYINT(1) UNSIGNED NOT NULL AFTER `stock_eval`,
  ADD COLUMN `broadcast_add` TINYINT(1) UNSIGNED NOT NULL AFTER `broadcast`; 
ALTER TABLE `cs_user_online` ADD COLUMN `broadcast` TINYINT(1) UNSIGNED NOT NULL AFTER `timezone`; 

ALTER TABLE `cs_broadcast`   
  CHANGE `bid` `bid` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT  COMMENT '直播ID',
  CHANGE `uid` `uid` INT(10) UNSIGNED NOT NULL  COMMENT '用户ID',
  CHANGE `message` `message` TEXT CHARSET utf8 COLLATE utf8_general_ci NOT NULL  COMMENT '消息内容',
  CHANGE `dateline` `dateline` INT(11) NOT NULL  COMMENT '发布时间',
  CHANGE `dateday` `dateday` INT(11) NOT NULL  COMMENT '发布日期',
COMMENT='盘中直播';

CREATE TABLE `cs_gold` (
  `gid` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '金股ID',
  `uid` INT(10) UNSIGNED NOT NULL COMMENT '用户ID',
  `title` VARCHAR(30) NOT NULL COMMENT '金股标题',
  `code` CHAR(6) NOT NULL COMMENT '股票代码',
  `name` VARCHAR(6) NOT NULL COMMENT '股票名称',
  `reason` VARCHAR(255) NOT NULL COMMENT '关注理由',
  `prompt` VARCHAR(255) NOT NULL COMMENT '风险提示',
  `buy_condition` VARCHAR(255) NOT NULL COMMENT '买入条件',
  `sell_condition` VARCHAR(255) NOT NULL COMMENT '卖出条件',
  `dateline` INT(11) NOT NULL COMMENT '发布时间',
  PRIMARY KEY (`gid`)
) ENGINE=MYISAM DEFAULT CHARSET=utf8 COMMENT='优选金股';
CREATE TABLE `cs_inverst` (
  `iid` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '金股ID',
  `uid` INT(10) UNSIGNED NOT NULL COMMENT '用户ID',
  `title` VARCHAR(30) NOT NULL COMMENT '金股标题',
  `code` CHAR(6) NOT NULL COMMENT '股票代码',
  `name` VARCHAR(6) NOT NULL COMMENT '股票名称',
  `reason` VARCHAR(255) NOT NULL COMMENT '选入理由',
  `think` VARCHAR(255) NOT NULL COMMENT '操作思路',
  `dateline` INT(11) NOT NULL COMMENT '发布时间',
  PRIMARY KEY (`iid`)
) ENGINE=MYISAM DEFAULT CHARSET=utf8 COMMENT='投资组合';
ALTER TABLE `cs_user_group` ADD COLUMN `gold` TINYINT(1) UNSIGNED NOT NULL AFTER `broadcast_add`, ADD COLUMN `gold_add` TINYINT(1) UNSIGNED NOT NULL AFTER `gold`, ADD COLUMN `inverst` TINYINT(1) UNSIGNED NOT NULL AFTER `gold_add`, ADD COLUMN `inverst_add` TINYINT(1) UNSIGNED NOT NULL AFTER `inverst`; 
ALTER TABLE `cs_user_setting` CHANGE `uid` `uid` INT(10) UNSIGNED NOT NULL COMMENT '用户ID', ADD COLUMN `gold` TINYINT(1) UNSIGNED NOT NULL COMMENT '是否同意条款（优选金股）' AFTER `uid`, ADD COLUMN `gold_dateline` INT NOT NULL COMMENT '同意时间（优选金股）' AFTER `gold`, ADD COLUMN `inverst` TINYINT(1) UNSIGNED NOT NULL COMMENT '是否同意条款（投资组合）' AFTER `gold_dateline`, ADD COLUMN `inverst_dateline` INT NOT NULL COMMENT '同意时间（投资组合）' AFTER `inverst`; 

ALTER TABLE `cs_inverst` DROP COLUMN `code`, DROP COLUMN `name`, DROP COLUMN `reason`, DROP COLUMN `think`;
CREATE TABLE `cs_inverst_stock` (
  `isid` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '金股ID',
  `iid` INT(10) UNSIGNED NOT NULL COMMENT '用户ID',
  `code` CHAR(6) NOT NULL COMMENT '股票代码',
  `name` VARCHAR(6) NOT NULL COMMENT '股票名称',
  `reason` VARCHAR(255) NOT NULL COMMENT '选入理由',
  `think` VARCHAR(255) NOT NULL COMMENT '操作思路',
  PRIMARY KEY (`isid`),
  KEY `iid_code` (`iid`,`code`)
) ENGINE=MYISAM DEFAULT CHARSET=utf8;
RENAME TABLE `fenxihui`.`cs_inverst` TO `fenxihui`.`cs_invest`;
RENAME TABLE `fenxihui`.`cs_inverst_stock` TO `fenxihui`.`cs_invest_stock`;
ALTER TABLE `fenxihui`.`cs_invest_stock` DROP INDEX `iid_code`;
ALTER TABLE `fenxihui`.`cs_user_group` CHANGE `inverst` `invest` TINYINT(1) UNSIGNED NOT NULL, CHANGE `inverst_add` `invest_add` TINYINT(1) UNSIGNED NOT NULL;
ALTER TABLE `fenxihui`.`cs_user_setting`   
  CHANGE `inverst` `invest` TINYINT(1) UNSIGNED NOT NULL  COMMENT '是否同意条款（投资组合）',
  CHANGE `inverst_dateline` `invest_dateline` INT(11) NOT NULL  COMMENT '同意时间（投资组合）';

ALTER TABLE `fenxihui`.`ssp_user_group`   
  ADD COLUMN `use_expiry` TINYINT(1) UNSIGNED NOT NULL AFTER `userlistgroup`;
ALTER TABLE `fenxihui`.`ssp_user_setting`   
  ADD COLUMN `expiry` INT NOT NULL  COMMENT '过期时间' AFTER `uid`,
  ADD COLUMN `expiry_dateline` INT NOT NULL  COMMENT '开通时间' AFTER `expiry`;

CREATE TABLE `ssp_user_gold` (
  `ugid` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `uid` INT(10) UNSIGNED NOT NULL,
  `gid` INT(10) UNSIGNED NOT NULL,
  `isread` TINYINT(3) UNSIGNED NOT NULL,
  `readtime` INT(11) NOT NULL,
  PRIMARY KEY (`ugid`),
  UNIQUE KEY `uid_gid` (`uid`,`gid`)
) ENGINE=MYISAM DEFAULT CHARSET=utf8;
CREATE TABLE `ssp_user_invest` (
  `uiid` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `uid` INT(10) UNSIGNED NOT NULL,
  `iid` INT(10) UNSIGNED NOT NULL,
  `isread` TINYINT(3) UNSIGNED NOT NULL,
  `readtime` INT(11) NOT NULL,
  PRIMARY KEY (`uiid`),
  UNIQUE KEY `uid_iid` (`uid`,`iid`)
) ENGINE=MYISAM DEFAULT CHARSET=utf8;
ALTER TABLE `fenxihui`.`ssp_user_stock`   
  ADD COLUMN `isread` TINYINT(1) UNSIGNED NOT NULL  COMMENT '是否已读' AFTER `evaldate`,
  ADD COLUMN `readtime` INT NOT NULL  COMMENT '阅读时间' AFTER `isread`;

ALTER TABLE `fenxihui`.`ssp_user_setting` ADD COLUMN `sendkey` VARCHAR(20) NOT NULL COMMENT '直播发送键' AFTER `invest_dateline`, ADD COLUMN `sendkey_dateline` INT NOT NULL COMMENT '直播发送键设置时间' AFTER `sendkey`;

ALTER TABLE `fenxihui`.`ssp_user_online` ADD COLUMN `consult` TINYINT(1) UNSIGNED NOT NULL AFTER `broadcast`;
ALTER TABLE `fenxihui`.`ssp_user_group` ADD COLUMN `consult` TINYINT(1) UNSIGNED NOT NULL AFTER `invest_add`, ADD COLUMN `consult_ask` TINYINT(1) UNSIGNED NOT NULL AFTER `consult`, ADD COLUMN `consult_reply` TINYINT(1) UNSIGNED NOT NULL AFTER `consult_ask`;
ALTER TABLE `fenxihui`.`ssp_user_group` DROP COLUMN `consult_ask`;
ALTER TABLE `fenxihui`.`ssp_user_group` CHANGE `consult` `consult_ask` TINYINT(1) UNSIGNED NOT NULL; 
CREATE TABLE `ssp_user_consult` (
  `ucid` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '直播ID',
  `from_uid` int(10) unsigned NOT NULL COMMENT '发送者用户ID',
  `to_uid` int(10) unsigned NOT NULL COMMENT '接收者用户ID',
  `message` text NOT NULL COMMENT '消息内容',
  `dateline` int(11) NOT NULL COMMENT '发布时间',
  `dateday` int(11) NOT NULL COMMENT '发布日期',
  `isread` tinyint(1) unsigned NOT NULL COMMENT '是否已读',
  PRIMARY KEY (`ucid`),
  KEY `uid` (`from_uid`),
  KEY `dateday` (`dateday`),
  KEY `ruid` (`to_uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
CREATE TABLE `ssp_user_serv` (
  `cuid` INT(10) UNSIGNED NOT NULL COMMENT '客户用户ID',
  `uid` INT(10) UNSIGNED NOT NULL COMMENT '分析师用户ID',
  `gid` INT(10) UNSIGNED NOT NULL COMMENT '客户分组ID',
  `nickname` VARCHAR(20) NOT NULL COMMENT '客户名称',
  `remark` VARCHAR(100) NOT NULL COMMENT '客户备注',
  `isopen` TINYINT(1) UNSIGNED NOT NULL COMMENT '是否打开对话窗口',
  PRIMARY KEY (`cuid`)
) ENGINE=MYISAM DEFAULT CHARSET=utf8;
CREATE TABLE `ssp_user_serv_group` (
  `gid` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '客户分组ID',
  `name` VARCHAR(20) NOT NULL COMMENT '客户分组名',
  `remark` VARCHAR(100) NOT NULL COMMENT '客户分组备注',
  PRIMARY KEY (`gid`)
) ENGINE=MYISAM DEFAULT CHARSET=utf8;
INSERT INTO `ssp_user_serv_group` (`gid`, `name`, `remark`) VALUES('1','季度','季度(1800)');
INSERT INTO `ssp_user_serv_group` (`gid`, `name`, `remark`) VALUES('2','半年','半年(3600)');
INSERT INTO `ssp_user_serv_group` (`gid`, `name`, `remark`) VALUES('3','全年','全年(5800)');
ALTER TABLE `fenxihui`.`ssp_user_serv` ADD COLUMN `unreads` INT UNSIGNED NOT NULL COMMENT '未读数' AFTER `isopen`;
ALTER TABLE `fenxihui`.`ssp_user_consult` CHANGE `uid` `from_uid` INT(10) UNSIGNED NOT NULL COMMENT '发送者用户ID', CHANGE `ruid` `to_uid` INT(10) UNSIGNED NOT NULL COMMENT '接收者用户ID';

ALTER TABLE `fenxihui`.`ssp_user_group` ADD COLUMN `manage` TINYINT(1) UNSIGNED NOT NULL AFTER `userlistgroup`, ADD COLUMN `manage_user` TINYINT(1) UNSIGNED NOT NULL AFTER `manage`, ADD COLUMN `manage_group` TINYINT(1) UNSIGNED NOT NULL AFTER `manage_user`, ADD COLUMN `manage_serv` TINYINT(1) UNSIGNED NOT NULL AFTER `manage_group`, ADD COLUMN `manage_serv_group` TINYINT(1) UNSIGNED NOT NULL AFTER `manage_serv`, ADD COLUMN `manage_count` TINYINT(1) UNSIGNED NOT NULL AFTER `manage_serv_group`;
ALTER TABLE `fenxihui`.`ssp_user_serv` ADD COLUMN `phone` VARCHAR(50) NOT NULL COMMENT '客户电话' AFTER `nickname`, ADD COLUMN `email` VARCHAR(100) NULL COMMENT '客户E-Mail' AFTER `phone`, ADD COLUMN `qq` VARCHAR(30) NOT NULL COMMENT '客户QQ' AFTER `email`, ADD COLUMN `funds` INT UNSIGNED NOT NULL COMMENT '客户资金量' AFTER `qq`, ADD COLUMN `address` VARCHAR(40) NOT NULL COMMENT '客户住址' AFTER `funds`;
ALTER TABLE `fenxihui`.`ssp_user_group` ADD COLUMN `manage_service` TINYINT(1) UNSIGNED NOT NULL AFTER `manage_serv_group`;
ALTER TABLE `fenxihui`.`ssp_user_group` CHANGE `manage_group` `manage_user_group` TINYINT(1) UNSIGNED NOT NULL;
ALTER TABLE `fenxihui`.`ssp_user_group` DROP `counts`;
ALTER TABLE `fenxihui`.`ssp_user_group` DROP `userlistgroup`;
ALTER TABLE `fenxihui`.`ssp_user_group` DROP PRIMARY KEY, ADD PRIMARY KEY (`gid`, `gname`, `title`);
ALTER TABLE `fenxihui`.`ssp_user_serv_group` DROP PRIMARY KEY, ADD PRIMARY KEY (`gid`, `name`);

CREATE VIEW `客户信息列表` AS (select  `s`.`cuid` AS `客户用户ID`,  `u`.`username` AS `客户用户名`,  `s`.`uid` AS `客服用户ID`,  `us`.`username` AS `客服用户名`,  `s`.`gid` AS `所有客服组ID`,  `sg`.`name` AS `所有客服组名`,  `s`.`nickname` AS `客户真实姓名`,  `s`.`phone` AS `客户电话`,  `s`.`email` AS `客户E-Mail`,  `s`.`qq` AS `客户QQ`,  `s`.`funds` AS `客户资金量`,  `s`.`address` AS `客户地址`,  `s`.`remark` AS `备注` from (((`ssp_user_serv` `s`  left join `ssp_user` `u`  on ((`u`.`uid` = `s`.`cuid`)))  left join `ssp_user` `us`  on ((`us`.`uid` = `s`.`uid`)))  left join `ssp_user_serv_group` `sg`  on ((`sg`.`gid` = `s`.`gid`))) where (`s`.`uid` > 1));
CREATE VIEW `在线用户列表` AS (select  `o`.`uid` AS `用户ID`,  `u`.`username` AS `用户名`,  `u`.`email` AS `邮箱地址`,  `ug`.`gid` AS `所属用户组ID`,  `ug`.`title` AS `所属用户组标题`,  if((`us`.`uid` > 1),'是','否') AS `是否客户`,  `us`.`nickname` AS `客户真实姓名`,  `us`.`phone` AS `客户电话`,  `us`.`email` AS `客户E-Mail`,  `us`.`qq` AS `客户QQ`,  `us`.`funds` AS `客户资金量`,  `us`.`address` AS `客户地址`,  `us`.`remark` AS `备注` from (((`ssp_user_online` `o`  left join `ssp_user` `u`  on ((`u`.`uid` = `o`.`uid`)))  left join `ssp_user_group` `ug`  on ((`ug`.`gid` = `o`.`gid`)))  left join `ssp_user_serv` `us`  on ((`us`.`cuid` = `o`.`uid`))) where (`o`.`uid` > 0) order by `u`.`gid`,if((`us`.`uid` > 1),0,1));
