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
