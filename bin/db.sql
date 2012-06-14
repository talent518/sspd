/*
SQLyog Ultimate v9.51 
MySQL - 5.1.52 : Database - fenxihui
*********************************************************************
*/

/*!40101 SET NAMES utf8 */;

/*!40101 SET SQL_MODE=''*/;

/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;
CREATE DATABASE /*!32312 IF NOT EXISTS*/`fenxihui` /*!40100 DEFAULT CHARACTER SET latin1 */;

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
) ENGINE=MyISAM AUTO_INCREMENT=11 DEFAULT CHARSET=utf8 COMMENT='盘中直播';

/*Data for the table `ssp_broadcast` */

insert  into `ssp_broadcast`(`bid`,`uid`,`message`,`dateline`,`dateday`) values (1,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339567319,1339516800),(2,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dfdasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339567354,1339516800),(3,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dsfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339567489,1339516800),(4,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dsfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339567596,1339516800),(5,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dfdsfsd</B></FONT></P>]]></htmlText><sprites/></rtf>',1339567652,1339516800),(6,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339567785,1339516800),(7,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>fdgsdfgsdfg</B></FONT></P>]]></htmlText><sprites/></rtf>',1339567851,1339516800),(8,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dsfasdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339567912,1339516800),(9,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"24\" COLOR=\"#FFFFFF\" LETTERSPACING=\"4\" KERNING=\"0\"></FONT></P>]]></htmlText><sprites><sprite src=\"com.fenxihui.desktop.utils::Smileys__smileys_014\" index=\"0\"/></sprites></rtf>',1339568358,1339516800),(10,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dfgsadfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339572463,1339516800);

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
) ENGINE=MyISAM AUTO_INCREMENT=5 DEFAULT CHARSET=utf8 COMMENT='优选金股';

/*Data for the table `ssp_gold` */

insert  into `ssp_gold`(`gid`,`uid`,`title`,`code`,`name`,`reason`,`prompt`,`buy_condition`,`sell_condition`,`dateline`) values (4,44,'6.11优选金股','300108','双龙股份','中报预增，主力筹码集中，低位涨停启动，有望走出连续上攻行情','大盘系统性风险','参考买入价10.40——10.60元建仓，仓位三成','短线卖点11.50——12元，向下跌破10元止损，出现重大利空消息、拉高出货卖出',1339230281);

/*Table structure for table `ssp_invest` */

DROP TABLE IF EXISTS `ssp_invest`;

CREATE TABLE `ssp_invest` (
  `iid` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '金股ID',
  `uid` int(10) unsigned NOT NULL COMMENT '用户ID',
  `title` varchar(30) NOT NULL COMMENT '金股标题',
  `dateline` int(11) NOT NULL COMMENT '发布时间',
  PRIMARY KEY (`iid`)
) ENGINE=MyISAM AUTO_INCREMENT=6 DEFAULT CHARSET=utf8 COMMENT='投资组合';

/*Data for the table `ssp_invest` */

insert  into `ssp_invest`(`iid`,`uid`,`title`,`dateline`) values (5,44,'6月11日最佳投资组合（周一）',1339230148);

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
) ENGINE=MyISAM AUTO_INCREMENT=14 DEFAULT CHARSET=utf8;

/*Data for the table `ssp_invest_stock` */

insert  into `ssp_invest_stock`(`isid`,`iid`,`code`,`name`,`reason`,`think`) values (11,5,'300282','汇冠股份','小盘触摸屏概念股，走势完成突破，具备黑马潜质，回调可重点关注','建议买入价17-17.20元，仓位三成，短期目标19元，止损16元'),(12,5,'002344','英威腾','低位放量启动，主力筹码集中，上涨潜力巨大','建议买入价14.50-14.60元，仓位三成，短期目标16元，止损14元'),(13,5,'002499','科林环保','环保概念是近期主流热点，放量突破之后强势横盘，后市仍有上升空间','建议买入价17.50-17.70元，仓位三成，短期目标19元，止损16.80元');

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

insert  into `ssp_user`(`uid`,`gid`,`username`,`password`,`email`,`onlinetime`,`regip`,`regtime`,`prevlogtime`,`logtime`) values (1,1,'admin','ece71ca58ba5c5f4aa394580af5c20c1','admin@admin.com',127,'182.119.76.244',1338798574,1339590526,1339591460),(44,1,'蓝绸','e10adc3949ba59abbe56e057f20f883e','lanchou@126.com',0,'p/src/function_',1338798740,1339210935,1339229995),(73,3,'玉泉山','e10adc3949ba59abbe56e057f20f883e','65545@163.com',0,'p/src/function_',1338798960,1339231617,1339231745),(4801,3,'黑色郁金香','9bc3b521ebbc0f2bb12f3d8672ca2ede','sishuiliunian1225@163.com',0,'182.119.76.226',1338801937,1338803175,1339207468),(64,3,'梦回三国','f8f25e1a06132e9d51b43c480af94b00','1051455549@qq.com',0,'182.119.76.244',1338802092,1339379739,1339410331),(4850,3,'浮夸人生','57a74fa0f5ee0b60cea9021ceee26315','fuzhz1989@sina.com',0,'182.119.76.244',1338802141,1339219244,1339220251),(123,3,'职场煮夫','26cd664e0b78b81947ca2b3508461248','lckj090@163.com',0,'182.119.76.226',1338802350,1338802350,1338802350),(4905,3,'王者归来','48620ee79f2275a8170dd1bc4f59b666','1141581749@qq.com',0,'182.119.76.226',1338802356,1339380837,1339399631),(4883,3,'夏尔特蓝','023f1bc744b414d380987aaddf0af8bf','313462081@qq.com',0,'182.119.76.244',1338802360,1338802360,1338881741),(5031,3,'qiubo31','290e92c6bc06f28c355c0c4122589de2','15188322171@163.com',0,'182.119.76.244',1338802376,1339207096,1339210130),(5088,3,'贺雪','6c72bbd1882804ba32bd24c7bfcd6542','285734027@qq.com',0,'182.119.76.226',1338802404,1339207170,1339210262),(65,3,'红豆','5111cc2c17763a06742dce5d0e8baeb4','983071973@qq.com',0,'182.119.76.226',1338802420,1338802420,1338802944),(42,3,'深蓝','26cd664e0b78b81947ca2b3508461248','zhangyhdpp@163.com',0,'182.119.76.226',1338802453,1338802453,1338802453),(4838,3,'大森林','e10adc3949ba59abbe56e057f20f883e','dongsheng01025@163.com',0,'',1338802567,1339207254,1339207317),(5059,3,'云中漫步','8689f86384567d60c62ebe0b692f9974','457896547@qq.com',0,'182.119.76.244',1338802572,1339219207,1339219244),(5120,3,'爱姗','a637fedbd0396734e1639cdd8137cef2','gtrgozpvicopjebm@qq.com',0,'182.119.76.244',1338802768,1339219441,1339376787),(4866,3,'半步含烟','9bc3b521ebbc0f2bb12f3d8672ca2ede','fhdtjszmuo662592@qq.com',0,'182.119.76.226',1338802779,1338879333,1339374009),(4747,3,'飞侠','db5669184f031595c533fcb823e4f09f','feixia@qq.com',0,'',1338802792,1339207246,1339375289),(5122,3,'libinfeng','c46d587818087dce268c30acd17ddcd1','601285008@qq.com',0,'182.119.76.226',1338803153,1339218583,1339219245),(5124,3,'楚风','f85f505a754967fc1fb4bf6c3f3b56e4','1120381887@qq.com',0,'182.119.76.244',1338804004,1339236004,1339384449),(2,3,'rukyst','97989c32b28a9aa6f643cf8325990ba6','rukyst@vip.qq.com',61,'',1338804050,1339423460,1339426221),(5123,3,'白狐','ef9402387de35a420f5f31a5aa50ba07','kjkljo@11.com',0,'182.119.76.244',1338809414,1338809414,1338809414),(4993,3,'玉麟','6c72bbd1882804ba32bd24c7bfcd6542','ganyu13137635006@163.com',0,'182.119.76.244',1338812135,1339215366,1339219324),(5030,3,'abao','4c05797d74c03a0f06da096299960a5f','talent518@live.cn',249,'61.163.84.100',1338814107,1339642961,1339654306),(5119,3,'竹林听雨','2df2e8be49adac3c4891ad5976e08a0f','598283188@qq.com',0,'182.119.79.9',1338856501,1339216491,1339216640),(5126,3,'热豆腐','c3e3657ce6e3e87ff00bb1c9b44ffdcd','1547470662@qq.com',0,'182.119.79.9',1338876912,1338888046,1338971204),(5127,3,'野驴','b63b8f89d55d47e82bd6059cc53d93db','www.735877905@qq.com',0,'182.119.79.65',1338877102,1339216682,1339400976),(5128,3,'妞妞','a3c897faca29d89b960ec9714d688032','627664045@qq.com',0,'182.119.76.180',1338877874,1339376214,1339399258),(5145,3,'樱花烂漫','1f56377a14674755067d9552a1377574','1152163344@qq.com',0,'123.14.249.81',1338962283,1338962283,1338962283),(5147,3,'天外来客','1f56377a14674755067d9552a1377574','317121305@qq.com',0,'123.6.170.58',1338972413,1339052888,1339053061),(5092,3,'大猩猩二号','3c4bbb4a97cac64e94cda0b01aabfb36','daxingxing@dxshr.com',0,'125.46.244.150',1338989765,1338990778,1339049320),(4728,3,'BC-3','a45958517604f5cd90d6ee51ad9cfdb6','bc-3@qq.com',0,'123.14.249.81',1338993199,1338993425,1338993662),(5140,3,'丹尼','c3e3657ce6e3e87ff00bb1c9b44ffdcd','1749197174@qq.com',5,'123.14.25.180',1339028419,1339407506,1339407514),(5044,3,'申成','023f1bc744b414d380987aaddf0af8bf','98899766@qq.com',0,'123.14.25.52',1339051015,1339051833,1339051958),(5141,3,'苏格拉没有底','5111cc2c17763a06742dce5d0e8baeb4','358683381@qq.com',0,'123.14.25.180',1339051602,1339383899,1339384535),(5161,3,'夕阳无限好','57a74fa0f5ee0b60cea9021ceee26315','891395827@qq.com',0,'123.14.25.52',1339051656,1339051656,1339051656),(5133,3,'滴水看世界','7bd8ed6366a1bd9c251271cd245fc369','1132334111@qq.com',0,'123.14.25.52',1339052712,1339217100,1339377842),(5138,3,'木槿花开','c46d587818087dce268c30acd17ddcd1','1275335090@qq.com',0,'61.52.55.200',1339205547,1339205906,1339205954),(5060,3,'马宇','023f1bc744b414d380987aaddf0af8bf','3456787@qq.com',0,'182.119.76.123',1339207357,1339207357,1339207357),(5042,3,'股民老刘','023f1bc744b414d380987aaddf0af8bf','4567987989@qq.com',0,'182.119.76.123',1339207591,1339207591,1339207591),(5053,3,'海洋','023f1bc744b414d380987aaddf0af8bf','45677979@qq.com',0,'182.119.76.123',1339207692,1339207692,1339207692),(5129,3,'珊瑚海','6b40c3432a6da5851fbdd71027c27880','499054981@qq.com',0,'182.119.76.86',1339384790,1339384790,1339384790);

/*Table structure for table `ssp_user_gold` */

DROP TABLE IF EXISTS `ssp_user_gold`;

CREATE TABLE `ssp_user_gold` (
  `ugid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `uid` int(10) unsigned NOT NULL,
  `gid` int(10) unsigned NOT NULL,
  `isread` tinyint(3) unsigned NOT NULL,
  `readtime` int(11) NOT NULL,
  PRIMARY KEY (`ugid`),
  UNIQUE KEY `uid_gid` (`uid`,`gid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `ssp_user_gold` */

/*Table structure for table `ssp_user_group` */

DROP TABLE IF EXISTS `ssp_user_group`;

CREATE TABLE `ssp_user_group` (
  `gid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `gname` varchar(10) NOT NULL,
  `title` varchar(20) NOT NULL,
  `counts` int(10) unsigned NOT NULL,
  `userlistgroup` varchar(30) NOT NULL,
  `use_expiry` tinyint(1) unsigned NOT NULL,
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

insert  into `ssp_user_group`(`gid`,`gname`,`title`,`counts`,`userlistgroup`,`use_expiry`,`stock`,`stock_add`,`stock_eval`,`broadcast`,`broadcast_add`,`gold`,`gold_add`,`invest`,`invest_add`) values (1,'admin','管理员',0,'1,2,3',0,1,0,1,1,1,1,1,1,1),(2,'analyst','分析师',0,'3',0,1,0,1,1,1,1,1,1,1),(3,'investor','股民',0,'2',1,1,1,0,1,0,1,0,1,0),(4,'guest','来宾',0,'0',0,0,0,0,0,0,0,0,0,0);

/*Table structure for table `ssp_user_invest` */

DROP TABLE IF EXISTS `ssp_user_invest`;

CREATE TABLE `ssp_user_invest` (
  `uiid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `uid` int(10) unsigned NOT NULL,
  `iid` int(10) unsigned NOT NULL,
  `isread` tinyint(3) unsigned NOT NULL,
  `readtime` int(11) NOT NULL,
  PRIMARY KEY (`uiid`),
  UNIQUE KEY `uid_iid` (`uid`,`iid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `ssp_user_invest` */

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
  `expiry` int(11) NOT NULL COMMENT '过期时间',
  `expiry_dateline` int(11) NOT NULL COMMENT '开通时间',
  `gold` tinyint(1) unsigned NOT NULL COMMENT '是否同意条款（优选金股）',
  `gold_dateline` int(11) NOT NULL COMMENT '同意时间（优选金股）',
  `invest` tinyint(1) unsigned NOT NULL COMMENT '是否同意条款（投资组合）',
  `invest_dateline` int(11) NOT NULL COMMENT '同意时间（投资组合）',
  PRIMARY KEY (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `ssp_user_setting` */

insert  into `ssp_user_setting`(`uid`,`expiry`,`expiry_dateline`,`gold`,`gold_dateline`,`invest`,`invest_dateline`) values (1,0,0,1,1339407392,1,1339407395),(73,1355277212,1339563116,1,1339212867,1,1339231759),(5030,1355277212,1339563116,1,1339212934,1,1339212970),(4993,1355277212,1339563116,1,1339215465,1,1339215446),(4850,1355277212,1339563116,1,1339216477,1,1339216484),(5119,1355277212,1339563116,1,1339216516,1,1339216542),(5122,1355277212,1339563116,1,1339216643,1,1339217073),(5140,1355277212,1339563116,1,1339216972,1,1339217041),(4905,1355277212,1339563116,1,1339217122,1,1339217129),(64,1355277212,1339563116,1,1339217510,1,1339217558),(5128,1355277212,1339563116,1,1339218063,1,1339399278),(5059,1355277212,1339563116,1,1339218433,1,1339218523),(44,0,0,1,1339230332,1,1339230172),(5120,1355277212,1339563116,0,0,1,1339376992),(5133,1355277212,1339563116,1,1339377851,1,1339377846),(5141,1355277212,1339563116,1,1339383904,1,1339384108),(2,1355277212,1339563116,0,0,1,1339419645),(4801,1355277212,1339563044,0,0,0,0),(123,1355277212,1339563044,0,0,0,0),(4883,1355277212,1339563044,0,0,0,0),(5031,1355277212,1339563044,0,0,0,0),(5088,1355277212,1339563044,0,0,0,0),(65,1355277212,1339563044,0,0,0,0),(42,1355277212,1339563044,0,0,0,0),(4838,1355277212,1339563044,0,0,0,0),(4866,1355277212,1339563044,0,0,0,0),(4747,1355277212,1339563044,0,0,0,0),(5124,1355277212,1339563044,0,0,0,0),(5123,1355277212,1339563044,0,0,0,0),(5126,1355277212,1339563044,0,0,0,0),(5127,1355277212,1339563044,0,0,0,0),(5145,1355277212,1339563044,0,0,0,0),(5147,1355277212,1339563044,0,0,0,0),(5092,1355277212,1339563044,0,0,0,0),(4728,1355277212,1339563044,0,0,0,0),(5044,1355277212,1339563044,0,0,0,0),(5161,1355277212,1339563044,0,0,0,0),(5138,1355277212,1339563044,0,0,0,0),(5060,1355277212,1339563044,0,0,0,0),(5042,1355277212,1339563044,0,0,0,0),(5053,1355277212,1339563044,0,0,0,0),(5129,1355277212,1339563044,0,0,0,0);

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
  `isread` tinyint(1) unsigned NOT NULL COMMENT '是否已读',
  `readtime` int(11) NOT NULL COMMENT '阅读时间',
  PRIMARY KEY (`sid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='股票交易逐笔登记表';

/*Data for the table `ssp_user_stock` */

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;
