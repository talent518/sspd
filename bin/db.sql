/*
SQLyog Ultimate v9.51 
MySQL - 5.1.47 : Database - fenxihui
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
CREATE DATABASE /*!32312 IF NOT EXISTS*/`fenxihui` /*!40100 DEFAULT CHARACTER SET utf8 */;

USE `fenxihui`;

/*Table structure for table `ssp_broadcast` */

DROP TABLE IF EXISTS `ssp_broadcast`;

CREATE TABLE `ssp_broadcast` (
  `bid` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '直播ID',
  `uid` int(10) unsigned NOT NULL COMMENT '用户ID',
  `message` text NOT NULL COMMENT '消息内容',
  `dateline` int(11) NOT NULL COMMENT '发布时间',
  `dateday` int(11) NOT NULL COMMENT '发布日期',
  PRIMARY KEY (`bid`),
  KEY `uid` (`uid`),
  KEY `dateday` (`dateday`)
) ENGINE=MyISAM AUTO_INCREMENT=18 DEFAULT CHARSET=utf8 COMMENT='盘中直播';

/*Data for the table `ssp_broadcast` */

insert  into `ssp_broadcast`(`bid`,`uid`,`message`,`dateline`,`dateday`) values (1,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<textformat leftmargin=\"24px\"><P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"12\" COLOR=\"#000000\" LETTERSPACING=\"0\" KERNING=\"0\">dsfasdfasdfasdf</FONT></P></textformat>]]></htmlText><sprites/></rtf>',1338876108,1338825600),(2,5030,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<textformat leftmargin=\"24px\"><P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"12\" COLOR=\"#000000\" LETTERSPACING=\"0\" KERNING=\"0\">夺压顶地<FONT SIZE=\"24\" COLOR=\"#FFFFFF\" LETTERSPACING=\"4\"></FONT></FONT></P></textformat>]]></htmlText><sprites><sprite src=\"com.fenxihui.desktop.utils::Smileys__smileys_014\" index=\"4\"/></sprites></rtf>',1338876860,1338825600),(3,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"12\" COLOR=\"#000000\" LETTERSPACING=\"0\" KERNING=\"0\">dsfa<FONT SIZE=\"24\" COLOR=\"#FFFFFF\" LETTERSPACING=\"4\"></FONT>sdfasd<FONT SIZE=\"24\" COLOR=\"#FFFFFF\" LETTERSPACING=\"4\"></FONT></FONT></P>]]></htmlText><sprites><sprite src=\"com.fenxihui.desktop.utils::Smileys__smileys_014\" index=\"4\"/><sprite src=\"com.fenxihui.desktop.utils::Smileys__smileys_014\" index=\"10\"/></sprites></rtf>',1338878438,1338825600),(4,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"12\" COLOR=\"#000000\" LETTERSPACING=\"0\" KERNING=\"0\">dsdfasdfasdf</FONT></P>]]></htmlText><sprites/></rtf>',1338878516,1338825600),(5,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"12\" COLOR=\"#000000\" LETTERSPACING=\"0\" KERNING=\"0\">dsfasdfasdf</FONT></P>]]></htmlText><sprites/></rtf>',1338878706,1338825600),(6,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"12\" COLOR=\"#000000\" LETTERSPACING=\"0\" KERNING=\"0\">dfsadfasdfasdf</FONT></P>]]></htmlText><sprites/></rtf>',1338878738,1338825600),(7,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"12\" COLOR=\"#000000\" LETTERSPACING=\"0\" KERNING=\"0\">sdfasdfasdf</FONT></P>]]></htmlText><sprites/></rtf>',1338878747,1338825600),(8,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"12\" COLOR=\"#000000\" LETTERSPACING=\"0\" KERNING=\"0\">dsdfsdfdddddddddddddddddddddddddddddddddddddddddddddd</FONT></P>]]></htmlText><sprites/></rtf>',1338878755,1338825600),(9,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"12\" COLOR=\"#000000\" LETTERSPACING=\"0\" KERNING=\"0\">ddddddd<FONT SIZE=\"24\" COLOR=\"#FFFFFF\" LETTERSPACING=\"4\"></FONT>dd夺夺在奇才</FONT></P>]]></htmlText><sprites><sprite src=\"com.fenxihui.desktop.utils::Smileys__smileys_014\" index=\"7\"/></sprites></rtf>',1338878778,1338825600),(10,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"24\" COLOR=\"#FFFFFF\" LETTERSPACING=\"4\" KERNING=\"0\"></FONT></P>]]></htmlText><sprites><sprite src=\"com.fenxihui.desktop.utils::Smileys__smileys_014\" index=\"0\"/></sprites></rtf>',1338902300,1338825600),(11,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"24\" COLOR=\"#FFFFFF\" LETTERSPACING=\"4\" KERNING=\"0\"><FONT SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"0\"><B>sdfasdfasdfasdfasdf</B></FONT></FONT></P>]]></htmlText><sprites><sprite src=\"com.fenxihui.desktop.utils::Smileys__smileys_014\" index=\"0\"/></sprites></rtf>',1338902356,1338825600),(12,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"0\" KERNING=\"0\"><B>dfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1338943914,1338912000),(13,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"0\" KERNING=\"0\"><B>dsafdfasd</B></FONT></P>]]></htmlText><sprites/></rtf>',1338943922,1338912000),(14,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"24\" COLOR=\"#FFFFFF\" LETTERSPACING=\"4\" KERNING=\"0\"></FONT></P>]]></htmlText><sprites><sprite src=\"com.fenxihui.desktop.utils::Smileys__smileys_014\" index=\"0\"/></sprites></rtf>',1338943953,1338912000),(15,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"0\" KERNING=\"0\"><B>dsf</B><FONT SIZE=\"24\" COLOR=\"#FFFFFF\" LETTERSPACING=\"4\"></FONT><B>sa</B><FONT COLOR=\"#000000\"><B>dfasd</B></FONT><B>fas</B><FONT SIZE=\"24\" COLOR=\"#FFFFFF\" LETTERSPACING=\"4\"></FONT><B>df</B></FONT></P>]]></htmlText><sprites><sprite src=\"com.fenxihui.desktop.utils::Smileys__smileys_001\" index=\"3\"/><sprite src=\"com.fenxihui.desktop.utils::Smileys__smileys_026\" index=\"13\"/></sprites></rtf>',1338943977,1338912000),(16,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#000000\" LETTERSPACING=\"0\" KERNING=\"0\"><B>sdfasdfasdfsadfa</B></FONT></P><P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#000000\" LETTERSPACING=\"0\" KERNING=\"0\"><B>sdfasdf</B></FONT></P><P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#000000\" LETTERSPACING=\"0\" KERNING=\"0\"><B>ds</B></FONT></P>]]></htmlText><sprites/></rtf>',1338943987,1338912000),(17,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"24\" COLOR=\"#FFFFFF\" LETTERSPACING=\"4\" KERNING=\"0\"><FONT SIZE=\"20\" COLOR=\"#000000\" LETTERSPACING=\"0\"><B>中华人民共</B></FONT><FONT SIZE=\"20\" COLOR=\"#000000\" LETTERSPACING=\"0\"><B>和国</B></FONT></FONT></P>]]></htmlText><sprites><sprite src=\"com.fenxihui.desktop.utils::Smileys__smileys_014\" index=\"0\"/><sprite src=\"com.fenxihui.desktop.utils::Smileys__smileys_029\" index=\"5\"/></sprites></rtf>',1338944005,1338912000);

/*Table structure for table `ssp_chat` */

DROP TABLE IF EXISTS `ssp_chat`;

CREATE TABLE `ssp_chat` (
  `chat_id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `from_uid` int(10) unsigned NOT NULL,
  `to_uid` int(10) unsigned NOT NULL,
  `message` text NOT NULL,
  `dateline` int(11) NOT NULL,
  `readed` tinyint(4) NOT NULL,
  PRIMARY KEY (`chat_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `ssp_chat` */

/*Table structure for table `ssp_gold` */

DROP TABLE IF EXISTS `ssp_gold`;

CREATE TABLE `ssp_gold` (
  `gid` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '金股ID',
  `uid` int(10) unsigned NOT NULL COMMENT '用户ID',
  `title` varchar(30) NOT NULL COMMENT '金股标题',
  `code` char(6) NOT NULL COMMENT '股票代码',
  `name` varchar(6) NOT NULL COMMENT '股票名称',
  `reason` varchar(255) NOT NULL COMMENT '关注理由',
  `prompt` varchar(255) NOT NULL COMMENT '风险提示',
  `buy_condition` varchar(255) NOT NULL COMMENT '买入条件',
  `sell_condition` varchar(255) NOT NULL COMMENT '卖出条件',
  `dateline` int(11) NOT NULL COMMENT '发布时间',
  PRIMARY KEY (`gid`)
) ENGINE=MyISAM AUTO_INCREMENT=4 DEFAULT CHARSET=utf8 COMMENT='优选金股';

/*Data for the table `ssp_gold` */

insert  into `ssp_gold`(`gid`,`uid`,`title`,`code`,`name`,`reason`,`prompt`,`buy_condition`,`sell_condition`,`dateline`) values (1,1,'6.6涨停潜力股','002036','宜科科技','1、公司汉麻业务产生突破，将对业绩产生重大影响；\n2、走势突破盘整，近期强势横盘，主力控盘度较高，后市有望继续上涨。','大盘系统性风险','参考买入价17.10——17.20元建仓，仓位三成','短线卖点18.50——19元，向下跌破16.50元止损，出现重大利空消息、拉高出货卖出',1338887125),(2,1,'6.7涨停潜力股','002237','恒邦股份','1、美国可能实行新一轮定量宽松政策，刺激金价上涨，黄金股受益；    \n2、走势放量突破长期底部区域，同时具备高送转题材，后市空间广阔。','大盘系统性风险','借助回调，可在41元一线建仓，仓位三成','短线卖点44——46元，向下跌破39元止损，出现重大利空消息、拉高出货卖出',1338965396),(3,1,'6.5涨停潜力股','002259','升达林业','林业板块补涨品种，走势长期横盘，近期放量突破，上升空间打开','大盘系统性风险','参考买入价4.0——4.05元建仓，仓位三成','短线卖点4.40——4.50元，向下跌破3.80元止损，出现重大利空消息、拉高出货卖出',1338879110);

/*Table structure for table `ssp_invest` */

DROP TABLE IF EXISTS `ssp_invest`;

CREATE TABLE `ssp_invest` (
  `iid` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '金股ID',
  `uid` int(10) unsigned NOT NULL COMMENT '用户ID',
  `title` varchar(30) NOT NULL COMMENT '金股标题',
  `dateline` int(11) NOT NULL COMMENT '发布时间',
  PRIMARY KEY (`iid`)
) ENGINE=MyISAM AUTO_INCREMENT=3 DEFAULT CHARSET=utf8 COMMENT='投资组合';

/*Data for the table `ssp_invest` */

insert  into `ssp_invest`(`iid`,`uid`,`title`,`dateline`) values (1,1,'6月5号最佳投资组合',1339042819),(2,1,'6月7日最佳投资组合',1339043042);

/*Table structure for table `ssp_invest_stock` */

DROP TABLE IF EXISTS `ssp_invest_stock`;

CREATE TABLE `ssp_invest_stock` (
  `isid` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '金股ID',
  `iid` int(10) unsigned NOT NULL COMMENT '用户ID',
  `code` char(6) NOT NULL COMMENT '股票代码',
  `name` varchar(6) NOT NULL COMMENT '股票名称',
  `reason` varchar(255) NOT NULL COMMENT '选入理由',
  `think` varchar(255) NOT NULL COMMENT '操作思路',
  PRIMARY KEY (`isid`)
) ENGINE=MyISAM AUTO_INCREMENT=6 DEFAULT CHARSET=utf8;

/*Data for the table `ssp_invest_stock` */

insert  into `ssp_invest_stock`(`isid`,`iid`,`code`,`name`,`reason`,`think`) values (1,1,'002294','信立泰','短期趋势向上，跟随医药板块走强，后市仍有上升空间','参考买入价26.50元—26.60元，仓位三成，短线目标28.50元，止损25元'),(2,1,'300323','华灿光电','小盘次新股，目前仍处于发行价附近，有望出现补涨机会','参考买入价20.40—20.50元，仓位三成，短期目标22元，止损19.80元。'),(3,1,'002203','海亮股份','近期收购矿产，逆势七连阳，蓄势充分，随时可能向上突破','建议买入价12.60元--12.70元，仓位三成，短期目标14元，止损12元'),(4,2,'002233','塔牌集团','水泥板块强势品种，短线回调到位，可参与反弹行情','参考买入价10.40元—10.50元，仓位三成，短线目标11.50元，止损10元'),(5,2,'600086','东方金钰','主营黄金珠宝，受益金价上涨，日线两阳夹一阴面临突破','参考买入价14.80—14.90元，仓位三成，短期目标16元，止损14元。');

/*Table structure for table `ssp_user` */

DROP TABLE IF EXISTS `ssp_user`;

CREATE TABLE `ssp_user` (
  `uid` int(10) unsigned NOT NULL,
  `gid` int(10) unsigned NOT NULL,
  `username` varchar(20) NOT NULL,
  `password` varchar(32) NOT NULL,
  `email` varchar(128) NOT NULL,
  `onlinetime` int(10) unsigned NOT NULL,
  `regip` char(15) DEFAULT NULL,
  `regtime` int(11) NOT NULL,
  `prevlogtime` int(11) NOT NULL,
  `logtime` int(11) NOT NULL,
  PRIMARY KEY (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `ssp_user` */

insert  into `ssp_user`(`uid`,`gid`,`username`,`password`,`email`,`onlinetime`,`regip`,`regtime`,`prevlogtime`,`logtime`) values (1,1,'admin','8762eb814817cc8dcbb3fb5c5fcd52e0','admin@admin.com',7836,'127.0.0.1',1331613922,1339071692,1339071906),(8,3,'remind','ca7bd8607488cbdae7a55c2bc603eb63','remind@remind.com',0,'127.0.0.1',1331636171,0,0),(5030,3,'abao','4c05797d74c03a0f06da096299960a5f','talent518@live.cn',201,'192.168.80.1',1338744830,1338988584,1338991290);

/*Table structure for table `ssp_user_group` */

DROP TABLE IF EXISTS `ssp_user_group`;

CREATE TABLE `ssp_user_group` (
  `gid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `gname` varchar(10) NOT NULL,
  `title` varchar(20) NOT NULL,
  `counts` int(10) unsigned NOT NULL,
  `userlistgroup` varchar(30) NOT NULL,
  `stock` tinyint(1) unsigned NOT NULL,
  `stock_add` tinyint(1) unsigned NOT NULL,
  `stock_eval` tinyint(1) unsigned NOT NULL,
  `broadcast` tinyint(1) unsigned NOT NULL,
  `broadcast_add` tinyint(1) unsigned NOT NULL,
  `gold` tinyint(1) unsigned NOT NULL,
  `gold_add` tinyint(1) unsigned NOT NULL,
  `invest` tinyint(1) unsigned NOT NULL,
  `invest_add` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`gid`)
) ENGINE=MyISAM AUTO_INCREMENT=5 DEFAULT CHARSET=utf8;

/*Data for the table `ssp_user_group` */

insert  into `ssp_user_group`(`gid`,`gname`,`title`,`counts`,`userlistgroup`,`stock`,`stock_add`,`stock_eval`,`broadcast`,`broadcast_add`,`gold`,`gold_add`,`invest`,`invest_add`) values (1,'admin','管理员',0,'1,2,3',1,0,1,1,1,1,1,1,1),(2,'analyst','分析师',0,'3',1,0,1,1,1,1,1,1,1),(3,'investor','股民',0,'2',1,1,0,1,0,1,0,1,0),(4,'guest','来宾',0,'0',0,0,0,0,0,0,0,0,0);

/*Table structure for table `ssp_user_news` */

DROP TABLE IF EXISTS `ssp_user_news`;

CREATE TABLE `ssp_user_news` (
  `unid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `uid` int(10) unsigned NOT NULL,
  `aid` int(10) unsigned NOT NULL,
  `isread` tinyint(3) unsigned NOT NULL,
  `readtime` int(11) NOT NULL,
  PRIMARY KEY (`unid`),
  UNIQUE KEY `NewIndex1` (`uid`,`aid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `ssp_user_news` */

/*Table structure for table `ssp_user_online` */

DROP TABLE IF EXISTS `ssp_user_online`;

CREATE TABLE `ssp_user_online` (
  `onid` int(10) unsigned NOT NULL,
  `host` char(15) NOT NULL DEFAULT '0.0.0.0',
  `port` smallint(5) unsigned NOT NULL,
  `time` int(11) NOT NULL,
  `receiveKey` char(128) DEFAULT NULL,
  `sendkey` char(128) DEFAULT NULL,
  `uid` int(10) unsigned NOT NULL,
  `gid` int(10) unsigned NOT NULL,
  `logintimes` tinyint(3) unsigned NOT NULL,
  `logintime` int(11) NOT NULL,
  `timezone` smallint(6) NOT NULL,
  `broadcast` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`onid`)
) ENGINE=MEMORY DEFAULT CHARSET=utf8;

/*Data for the table `ssp_user_online` */

/*Table structure for table `ssp_user_profile` */

DROP TABLE IF EXISTS `ssp_user_profile`;

CREATE TABLE `ssp_user_profile` (
  `uid` int(10) unsigned NOT NULL,
  `nickname` varchar(20) DEFAULT NULL,
  `sex` tinyint(1) unsigned NOT NULL,
  `signature` varchar(100) NOT NULL,
  PRIMARY KEY (`uid`,`sex`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `ssp_user_profile` */

/*Table structure for table `ssp_user_setting` */

DROP TABLE IF EXISTS `ssp_user_setting`;

CREATE TABLE `ssp_user_setting` (
  `uid` int(10) unsigned NOT NULL COMMENT '用户ID',
  `gold` tinyint(1) unsigned NOT NULL COMMENT '是否同意条款（优选金股）',
  `gold_dateline` int(11) NOT NULL COMMENT '同意时间（优选金股）',
  `invest` tinyint(1) unsigned NOT NULL COMMENT '是否同意条款（投资组合）',
  `invest_dateline` int(11) NOT NULL COMMENT '同意时间（投资组合）',
  PRIMARY KEY (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `ssp_user_setting` */

insert  into `ssp_user_setting`(`uid`,`gold`,`gold_dateline`,`invest`,`invest_dateline`) values (1,1,1339070126,1,1339050562),(5030,1,1338988561,0,0);

/*Table structure for table `ssp_user_stock` */

DROP TABLE IF EXISTS `ssp_user_stock`;

CREATE TABLE `ssp_user_stock` (
  `sid` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '股票ID',
  `uid` int(10) unsigned NOT NULL COMMENT '发布者ID',
  `code` char(6) NOT NULL COMMENT '股票代码',
  `name` varchar(6) NOT NULL COMMENT '股票名称',
  `type` tinyint(1) unsigned NOT NULL COMMENT '操作类型：1为建仓，2为补仓，3为减仓，4为清仓',
  `dealdate` int(11) NOT NULL COMMENT '交易日期',
  `amount` int(11) NOT NULL COMMENT '数量',
  `price` float NOT NULL COMMENT '交易价格',
  `location` float NOT NULL COMMENT '仓位',
  `profitloss` float NOT NULL COMMENT '盈亏',
  `stoploss` varchar(255) NOT NULL COMMENT '止损',
  `reason` varchar(255) NOT NULL COMMENT '交易理由',
  `dateline` int(11) NOT NULL COMMENT '发布时间',
  `evaluid` int(10) NOT NULL COMMENT '点评者ID',
  `evaluation` varchar(255) DEFAULT NULL COMMENT '交易评价',
  `evaldate` int(11) NOT NULL COMMENT '评价日期',
  PRIMARY KEY (`sid`)
) ENGINE=MyISAM AUTO_INCREMENT=11 DEFAULT CHARSET=utf8 COMMENT='股票交易逐笔登记表';

/*Data for the table `ssp_user_stock` */

insert  into `ssp_user_stock`(`sid`,`uid`,`code`,`name`,`type`,`dealdate`,`amount`,`price`,`location`,`profitloss`,`stoploss`,`reason`,`dateline`,`evaluid`,`evaluation`,`evaldate`) values (1,1,'600326','西藏天路',1,1337702400,2300,12.12,50,0,'无','反弹不过前高可以卖出，等回调之后可以逢低回补，反复操作',0,8,'sdfasdf2324',0),(2,1,'600979','广安爱众',4,1333209600,4860,4.1,50,0,'要设好止损，每笔损失不超过总资金3%','走势破位，建议尽快止损',0,1,'增发下调，未及时减仓',0),(5,1,'000123','df压顶',3,1337011200,4850,56.5,60,0,'d的发生地方','适当方式打法',0,1,'的发生地方',0),(6,1,'012456','1dfsd5',1,1337097600,456,254.12,32.1,0,'雨辰下标下大雨苦','dfsdfasdfasdfsd',0,1,'sdf132ds5f43s5d1f3sdfsdgvvasdf',1338756057),(7,1,'484654','dfasdf',3,1337097600,4566,125,12.3,12,'是男是女是是别别加盟','2451321df',0,1,'8sdf6sd13f1ew',1338756087),(8,1,'012540','顶替城',4,1338566400,5000,50.12,89.1,3.1,'大规模于','夺压顶 压标奇才',0,-1,'',0),(9,5030,'012345','12345s',1,1338652800,32,32,323,0,'ddddddddddddddddd','sdfasd',0,0,'',0),(10,5030,'154630','dsfsdf',1,1339430400,45123,0.125,12.1,0,'fsdfsdfasdfasdfsdsdfsdfdssdfsdfasdfsadf','dsfsdfsd',0,0,NULL,0);

/*Table structure for table `ssp_user_thread` */

DROP TABLE IF EXISTS `ssp_user_thread`;

CREATE TABLE `ssp_user_thread` (
  `utid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `uid` int(10) unsigned NOT NULL,
  `tid` int(10) unsigned NOT NULL,
  `isread` tinyint(3) unsigned NOT NULL,
  `readtime` int(11) NOT NULL,
  PRIMARY KEY (`utid`),
  UNIQUE KEY `NewIndex1` (`uid`,`tid`)
) ENGINE=MyISAM AUTO_INCREMENT=12 DEFAULT CHARSET=utf8;

/*Data for the table `ssp_user_thread` */

insert  into `ssp_user_thread`(`utid`,`uid`,`tid`,`isread`,`readtime`) values (1,1,254,1,1331615484),(2,1,246,1,1331615488),(5,1,238,1,1331616127),(4,1,240,1,1331616121),(6,1,265,1,1331641724),(9,1,317,1,1331641727),(8,1,318,1,1331641726),(10,1,250,1,1331641729),(11,1,245,1,1332213266);

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
