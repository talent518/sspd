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
) ENGINE=MyISAM AUTO_INCREMENT=70 DEFAULT CHARSET=utf8 COMMENT='盘中直播';

/*Data for the table `ssp_broadcast` */

insert  into `ssp_broadcast`(`bid`,`uid`,`message`,`dateline`,`dateday`) values (1,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339567319,1339516800),(2,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dfdasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339567354,1339516800),(3,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dsfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339567489,1339516800),(4,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dsfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339567596,1339516800),(5,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dfdsfsd</B></FONT></P>]]></htmlText><sprites/></rtf>',1339567652,1339516800),(6,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339567785,1339516800),(7,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>fdgsdfgsdfg</B></FONT></P>]]></htmlText><sprites/></rtf>',1339567851,1339516800),(8,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dsfasdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339567912,1339516800),(9,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"24\" COLOR=\"#FFFFFF\" LETTERSPACING=\"4\" KERNING=\"0\"></FONT></P>]]></htmlText><sprites><sprite src=\"com.fenxihui.desktop.utils::Smileys__smileys_014\" index=\"0\"/></sprites></rtf>',1339568358,1339516800),(10,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dfgsadfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339572463,1339516800),(11,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dsfasdfasd</B></FONT></P>]]></htmlText><sprites/></rtf>',1339752117,1339689600),(12,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"24\" COLOR=\"#FFFFFF\" LETTERSPACING=\"4\" KERNING=\"0\"></FONT></P>]]></htmlText><sprites><sprite src=\"com.fenxihui.desktop.utils::Smileys__smileys_014\" index=\"0\"/></sprites></rtf>',1339752120,1339689600),(13,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfas</B></FONT></P>]]></htmlText><sprites/></rtf>',1339752669,1339689600),(14,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dsfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339752673,1339689600),(15,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339754619,1339689600),(16,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dfgasdfasd</B></FONT></P>]]></htmlText><sprites/></rtf>',1339754626,1339689600),(17,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dadsfasdfsd</B></FONT></P>]]></htmlText><sprites/></rtf>',1339821245,1339776000),(18,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339836452,1339776000),(19,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339836524,1339776000),(20,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasd</B></FONT></P>]]></htmlText><sprites/></rtf>',1339836538,1339776000),(21,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasfasd</B></FONT></P>]]></htmlText><sprites/></rtf>',1339836568,1339776000),(22,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>fsdfgsdfg</B></FONT></P>]]></htmlText><sprites/></rtf>',1339836571,1339776000),(23,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339836705,1339776000),(24,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339836713,1339776000),(25,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339836845,1339776000),(26,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339836848,1339776000),(27,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339837782,1339776000),(28,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339837858,1339776000),(29,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dsfasdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838599,1339776000),(30,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838608,1339776000),(31,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838612,1339776000),(32,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838618,1339776000),(33,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838623,1339776000),(34,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838628,1339776000),(35,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838695,1339776000),(36,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dsfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838754,1339776000),(37,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838760,1339776000),(38,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838762,1339776000),(39,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838764,1339776000),(40,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838765,1339776000),(41,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838766,1339776000),(42,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sadf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838767,1339776000),(43,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838768,1339776000),(44,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838768,1339776000),(45,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838769,1339776000),(46,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838769,1339776000),(47,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838770,1339776000),(48,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838770,1339776000),(49,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838770,1339776000),(50,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838771,1339776000),(51,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838771,1339776000),(52,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838772,1339776000),(53,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838772,1339776000),(54,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838773,1339776000),(55,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838773,1339776000),(56,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asd</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838775,1339776000),(57,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>fasd</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838775,1339776000),(58,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838775,1339776000),(59,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838776,1339776000),(60,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasd</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838776,1339776000),(61,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>fasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838777,1339776000),(62,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838777,1339776000),(63,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasd</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838777,1339776000),(64,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>asdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339838778,1339776000),(65,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339841204,1339776000),(66,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339841269,1339776000),(67,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339841275,1339776000),(68,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dsfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339841783,1339776000),(69,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1339841785,1339776000);

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
) ENGINE=MyISAM AUTO_INCREMENT=14 DEFAULT CHARSET=utf8 COMMENT='优选金股';

/*Data for the table `ssp_gold` */

insert  into `ssp_gold`(`gid`,`uid`,`title`,`code`,`name`,`reason`,`prompt`,`buy_condition`,`sell_condition`,`dateline`) values (4,44,'6.11优选金股','300108','双龙股份','中报预增，主力筹码集中，低位涨停启动，有望走出连续上攻行情','大盘系统性风险','参考买入价10.40——10.60元建仓，仓位三成','短线卖点11.50——12元，向下跌破10元止损，出现重大利空消息、拉高出货卖出',1339230281),(5,1,'312312300012301','101231','123dsf','sdfasdfasdf120','`sdf321as3d2f1a0sd3f21','`sdf1a3s2d1fa0s3d2f10','sdfasd32fsadfasdfasdf',1339752721),(6,1,'sdfasdf','012345','123d1f','sdfasdfasdfasdf`','sdf2sd3f1a3sdf1','sdfasd3f21asd3f1as3df','sdfsdf1as3d2g1s3drg1q3wer1t32fasdfgasdf',1339753219),(7,1,'sdfasdf','102131','1ds2f3','sd1f3a2sd1f3ad1f0','a1df32asd13f2a1sdf','321df2a3s1df32a1sdf31','asdf21sa3df3awe454r3asd1g.zsdf1gwerfes',1339753261),(8,1,'sdfasdf','012354','321d3f','sdfasdfasdf','sdfasdfdsfasdf','fasdfasdfasdf','dsadfasd',1339753351),(9,1,'012345641302131','123132','321sd3','1sd1f3a1sd3f1','a1sd32f1a3f1','3131ds3f213','3131sd32f1as3',1339753452),(10,1,'0123sdf1a3sd2f1a3s2df13','133213','31s32d','31s32d1f3as1df32a1sdf','321a3sd21f','313asd1f3','131f3ds13f1',1339753564),(11,1,'01321321','231321','21321s','10','2310','321','321',1339753744),(12,1,'123sdf1a0','123100','132123','13201321','032132','1321','321321',1339753873),(13,1,'321','213213','321301','3132130','31','32132','1320',1339754423);

/*Table structure for table `ssp_invest` */

DROP TABLE IF EXISTS `ssp_invest`;

CREATE TABLE `ssp_invest` (
  `iid` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '金股ID',
  `uid` int(10) unsigned NOT NULL COMMENT '用户ID',
  `title` varchar(30) NOT NULL COMMENT '金股标题',
  `dateline` int(11) NOT NULL COMMENT '发布时间',
  PRIMARY KEY (`iid`)
) ENGINE=MyISAM AUTO_INCREMENT=7 DEFAULT CHARSET=utf8 COMMENT='投资组合';

/*Data for the table `ssp_invest` */

insert  into `ssp_invest`(`iid`,`uid`,`title`,`dateline`) values (5,44,'6月11日最佳投资组合（周一）',1339230148),(6,1,'d1f2a3sdf',1339754479);

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
) ENGINE=MyISAM AUTO_INCREMENT=16 DEFAULT CHARSET=utf8;

/*Data for the table `ssp_invest_stock` */

insert  into `ssp_invest_stock`(`isid`,`iid`,`code`,`name`,`reason`,`think`) values (11,5,'300282','汇冠股份','小盘触摸屏概念股，走势完成突破，具备黑马潜质，回调可重点关注','建议买入价17-17.20元，仓位三成，短期目标19元，止损16元'),(12,5,'002344','英威腾','低位放量启动，主力筹码集中，上涨潜力巨大','建议买入价14.50-14.60元，仓位三成，短期目标16元，止损14元'),(13,5,'002499','科林环保','环保概念是近期主流热点，放量突破之后强势横盘，后市仍有上升空间','建议买入价17.50-17.70元，仓位三成，短期目标19元，止损16.80元'),(14,6,'012313','313210','3213132013','32130132103'),(15,6,'012345','sdfasd','asdfasdf','asdfadsf');

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

insert  into `ssp_user`(`uid`,`gid`,`username`,`password`,`email`,`onlinetime`,`regip`,`regtime`,`prevlogtime`,`logtime`) values (1,1,'admin','ece71ca58ba5c5f4aa394580af5c20c1','admin@admin.com',415,'182.119.76.244',1338798574,1340183028,1340184883),(44,1,'蓝绸','e10adc3949ba59abbe56e057f20f883e','lanchou@126.com',0,'p/src/function_',1338798740,1339210935,1339229995),(73,3,'玉泉山','e10adc3949ba59abbe56e057f20f883e','65545@163.com',0,'p/src/function_',1338798960,1339231617,1339231745),(4801,3,'黑色郁金香','9bc3b521ebbc0f2bb12f3d8672ca2ede','sishuiliunian1225@163.com',0,'182.119.76.226',1338801937,1338803175,1339207468),(64,3,'梦回三国','f8f25e1a06132e9d51b43c480af94b00','1051455549@qq.com',0,'182.119.76.244',1338802092,1340182409,1340183918),(4850,3,'浮夸人生','57a74fa0f5ee0b60cea9021ceee26315','fuzhz1989@sina.com',0,'182.119.76.244',1338802141,1339219244,1339220251),(123,3,'职场煮夫','26cd664e0b78b81947ca2b3508461248','lckj090@163.com',0,'182.119.76.226',1338802350,1338802350,1338802350),(4905,3,'王者归来','48620ee79f2275a8170dd1bc4f59b666','1141581749@qq.com',0,'182.119.76.226',1338802356,1339380837,1339399631),(4883,3,'夏尔特蓝','023f1bc744b414d380987aaddf0af8bf','313462081@qq.com',0,'182.119.76.244',1338802360,1338802360,1338881741),(5031,3,'qiubo31','290e92c6bc06f28c355c0c4122589de2','15188322171@163.com',0,'182.119.76.244',1338802376,1339207096,1339210130),(5088,3,'贺雪','6c72bbd1882804ba32bd24c7bfcd6542','285734027@qq.com',0,'182.119.76.226',1338802404,1339207170,1339210262),(65,3,'红豆','5111cc2c17763a06742dce5d0e8baeb4','983071973@qq.com',0,'182.119.76.226',1338802420,1338802420,1338802944),(42,3,'深蓝','26cd664e0b78b81947ca2b3508461248','zhangyhdpp@163.com',0,'182.119.76.226',1338802453,1338802453,1338802453),(4838,3,'大森林','e10adc3949ba59abbe56e057f20f883e','dongsheng01025@163.com',0,'',1338802567,1339207254,1339207317),(5059,3,'云中漫步','8689f86384567d60c62ebe0b692f9974','457896547@qq.com',0,'182.119.76.244',1338802572,1340181871,1340184934),(5120,3,'爱姗','a637fedbd0396734e1639cdd8137cef2','gtrgozpvicopjebm@qq.com',0,'182.119.76.244',1338802768,1339219441,1339376787),(4866,3,'半步含烟','9bc3b521ebbc0f2bb12f3d8672ca2ede','fhdtjszmuo662592@qq.com',0,'182.119.76.226',1338802779,1338879333,1339374009),(4747,3,'飞侠','db5669184f031595c533fcb823e4f09f','feixia@qq.com',0,'',1338802792,1339207246,1339375289),(5122,3,'libinfeng','c46d587818087dce268c30acd17ddcd1','601285008@qq.com',0,'182.119.76.226',1338803153,1339218583,1339219245),(5124,3,'楚风','f85f505a754967fc1fb4bf6c3f3b56e4','1120381887@qq.com',0,'182.119.76.244',1338804004,1339236004,1339384449),(2,3,'rukyst','97989c32b28a9aa6f643cf8325990ba6','rukyst@vip.qq.com',61,'',1338804050,1339423460,1339426221),(5123,3,'白狐','ef9402387de35a420f5f31a5aa50ba07','kjkljo@11.com',0,'182.119.76.244',1338809414,1338809414,1338809414),(4993,3,'玉麟','6c72bbd1882804ba32bd24c7bfcd6542','ganyu13137635006@163.com',0,'182.119.76.244',1338812135,1339215366,1339219324),(5030,3,'abao','4c05797d74c03a0f06da096299960a5f','talent518@live.cn',2444,'61.163.84.100',1338814107,1340183157,1340183934),(5119,3,'竹林听雨','2df2e8be49adac3c4891ad5976e08a0f','598283188@qq.com',0,'182.119.79.9',1338856501,1339216491,1339216640),(5126,3,'热豆腐','c3e3657ce6e3e87ff00bb1c9b44ffdcd','1547470662@qq.com',0,'182.119.79.9',1338876912,1338888046,1338971204),(5127,3,'野驴','b63b8f89d55d47e82bd6059cc53d93db','www.735877905@qq.com',0,'182.119.79.65',1338877102,1339216682,1339400976),(5128,3,'妞妞','a3c897faca29d89b960ec9714d688032','627664045@qq.com',0,'182.119.76.180',1338877874,1339376214,1339399258),(5145,3,'樱花烂漫','1f56377a14674755067d9552a1377574','1152163344@qq.com',0,'123.14.249.81',1338962283,1338962283,1338962283),(5147,3,'天外来客','1f56377a14674755067d9552a1377574','317121305@qq.com',0,'123.6.170.58',1338972413,1339052888,1339053061),(5092,3,'大猩猩二号','3c4bbb4a97cac64e94cda0b01aabfb36','daxingxing@dxshr.com',0,'125.46.244.150',1338989765,1338990778,1339049320),(4728,3,'BC-3','a45958517604f5cd90d6ee51ad9cfdb6','bc-3@qq.com',0,'123.14.249.81',1338993199,1338993425,1338993662),(5140,3,'丹尼','c3e3657ce6e3e87ff00bb1c9b44ffdcd','1749197174@qq.com',5,'123.14.25.180',1339028419,1339407506,1339407514),(5044,3,'申成','023f1bc744b414d380987aaddf0af8bf','98899766@qq.com',0,'123.14.25.52',1339051015,1339051833,1339051958),(5141,3,'苏格拉没有底','5111cc2c17763a06742dce5d0e8baeb4','358683381@qq.com',0,'123.14.25.180',1339051602,1339383899,1339384535),(5161,3,'夕阳无限好','57a74fa0f5ee0b60cea9021ceee26315','891395827@qq.com',0,'123.14.25.52',1339051656,1339051656,1339051656),(5133,3,'滴水看世界','7bd8ed6366a1bd9c251271cd245fc369','1132334111@qq.com',0,'123.14.25.52',1339052712,1339217100,1339377842),(5138,3,'木槿花开','c46d587818087dce268c30acd17ddcd1','1275335090@qq.com',0,'61.52.55.200',1339205547,1339205906,1339205954),(5060,3,'马宇','023f1bc744b414d380987aaddf0af8bf','3456787@qq.com',0,'182.119.76.123',1339207357,1339207357,1339207357),(5042,3,'股民老刘','023f1bc744b414d380987aaddf0af8bf','4567987989@qq.com',0,'182.119.76.123',1339207591,1339207591,1339207591),(5053,3,'海洋','023f1bc744b414d380987aaddf0af8bf','45677979@qq.com',0,'182.119.76.123',1339207692,1339207692,1339207692),(5129,3,'珊瑚海','6b40c3432a6da5851fbdd71027c27880','499054981@qq.com',0,'182.119.76.86',1339384790,1340182409,1340182860),(5167,4,'辰风','47ec20928bd484090efb38ceac63200c','369640911@qq.com',0,'192.168.1.128',1340179881,1340179881,1340179982);

/*Table structure for table `ssp_user_consult` */

DROP TABLE IF EXISTS `ssp_user_consult`;

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
) ENGINE=MyISAM AUTO_INCREMENT=9 DEFAULT CHARSET=utf8;

/*Data for the table `ssp_user_consult` */

insert  into `ssp_user_consult`(`ucid`,`from_uid`,`to_uid`,`message`,`dateline`,`dateday`,`isread`) values (1,5059,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>可以了</B></FONT></P>]]></htmlText><sprites/></rtf>',1340180894,1340121600,1),(2,5059,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"24\" COLOR=\"#FFFFFF\" LETTERSPACING=\"4\" KERNING=\"0\"></FONT></P>]]></htmlText><sprites><sprite src=\"com.fenxihui.desktop.utils::Smileys__smileys_061\" index=\"0\"/></sprites></rtf>',1340180909,1340121600,1),(3,5129,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"24\" COLOR=\"#FFFFFF\" LETTERSPACING=\"4\" KERNING=\"0\"></FONT></P>]]></htmlText><sprites><sprite src=\"com.fenxihui.desktop.utils::Smileys__smileys_013\" index=\"0\"/></sprites></rtf>',1340180924,1340121600,1),(4,5030,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dfasdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1340181932,1340121600,1),(5,5030,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>dsf423532</B></FONT></P>]]></htmlText><sprites/></rtf>',1340181988,1340121600,1),(6,5030,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>edfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1340182010,1340121600,1),(7,5030,1,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1340182016,1340121600,1),(8,1,5030,'<?xml version=\"1.0\" encoding=\"utf-8\"?><rtf><htmlText><![CDATA[<P ALIGN=\"LEFT\"><FONT FACE=\"Arial\" SIZE=\"20\" COLOR=\"#FF0000\" LETTERSPACING=\"1\" KERNING=\"0\"><B>sdfasdfasdf</B></FONT></P>]]></htmlText><sprites/></rtf>',1340182184,1340121600,1);

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
) ENGINE=MyISAM AUTO_INCREMENT=46 DEFAULT CHARSET=utf8;

/*Data for the table `ssp_user_gold` */

insert  into `ssp_user_gold`(`ugid`,`uid`,`gid`,`isread`,`readtime`) values (4,1,4,1,1339752700),(6,1,5,1,1339752798),(7,1,6,1,1339753231),(8,1,9,1,1339753548),(9,1,8,1,1339753549),(10,1,7,1,1339753551),(11,1,13,1,1339754638),(12,1,12,1,1339754638),(13,1,11,1,1339754639),(14,1,10,1,1339754639),(22,5059,13,1,1340180187),(16,5129,13,1,1340180080),(17,5129,12,1,1340180088),(18,5129,4,1,1340180095),(19,5129,11,1,1340180176),(20,5129,9,1,1340180179),(21,5129,10,1,1340180186),(23,5129,8,1,1340180189),(24,5129,7,1,1340180192),(25,5129,6,1,1340180194),(26,5129,5,1,1340180197),(27,5059,12,1,1340181167),(28,5059,11,1,1340181171),(29,5059,10,1,1340181175),(30,5059,9,1,1340181178),(31,5059,8,1,1340181181),(32,5059,7,1,1340181185),(33,5059,6,1,1340181188),(34,5059,5,1,1340181193),(35,5059,4,1,1340181197),(36,5030,4,1,1340183945),(37,5030,5,1,1340183945),(38,5030,6,1,1340183945),(39,5030,7,1,1340183947),(40,5030,8,1,1340183947),(41,5030,9,1,1340183947),(42,5030,10,1,1340183947),(43,5030,12,1,1340183948),(44,5030,13,1,1340183948),(45,5030,11,1,1340183950);

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
  `consult_ask` tinyint(1) unsigned NOT NULL,
  `consult_reply` tinyint(1) unsigned NOT NULL,
  PRIMARY KEY (`gid`)
) ENGINE=MyISAM AUTO_INCREMENT=5 DEFAULT CHARSET=utf8;

/*Data for the table `ssp_user_group` */

insert  into `ssp_user_group`(`gid`,`gname`,`title`,`counts`,`userlistgroup`,`use_expiry`,`stock`,`stock_add`,`stock_eval`,`broadcast`,`broadcast_add`,`gold`,`gold_add`,`invest`,`invest_add`,`consult_ask`,`consult_reply`) values (1,'admin','管理员',0,'1,2,3',0,1,0,1,1,1,1,1,1,1,0,1),(2,'analyst','分析师',0,'3',0,1,0,1,1,1,1,1,1,1,0,1),(3,'investor','股民',0,'2',1,1,1,0,1,0,1,0,1,0,1,0),(4,'guest','来宾',0,'0',0,0,0,0,0,0,0,0,0,0,0,0);

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
) ENGINE=MyISAM AUTO_INCREMENT=10 DEFAULT CHARSET=utf8;

/*Data for the table `ssp_user_invest` */

insert  into `ssp_user_invest`(`uiid`,`uid`,`iid`,`isread`,`readtime`) values (2,1,5,1,1339724169),(3,1,6,1,1339754493),(4,5129,5,1,1340180113),(5,5129,6,1,1340180118),(6,5059,6,1,1340180225),(7,5059,5,1,1340181256),(8,5030,5,1,1340183959),(9,5030,6,1,1340183959);

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
  `consult` tinyint(1) unsigned NOT NULL,
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

/*Table structure for table `ssp_user_serv` */

DROP TABLE IF EXISTS `ssp_user_serv`;

CREATE TABLE `ssp_user_serv` (
  `cuid` int(10) unsigned NOT NULL COMMENT '客户用户ID',
  `uid` int(10) unsigned NOT NULL COMMENT '分析师用户ID',
  `gid` int(10) unsigned NOT NULL COMMENT '客户分组ID',
  `nickname` varchar(20) NOT NULL COMMENT '客户名称',
  `remark` varchar(100) NOT NULL COMMENT '客户备注',
  `isopen` tinyint(1) unsigned NOT NULL COMMENT '是否打开对话窗口',
  `unreads` int(10) unsigned NOT NULL COMMENT '未读数',
  PRIMARY KEY (`cuid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `ssp_user_serv` */

insert  into `ssp_user_serv`(`cuid`,`uid`,`gid`,`nickname`,`remark`,`isopen`,`unreads`) values (5030,1,1,'阿宝','一位好人',1,0),(2,1,2,'水哥','一位牛人',0,0),(73,1,1,'玉泉山','玉泉山',0,0),(4801,1,3,'黑色郁金香','黑色郁金香',0,0),(64,1,2,'梦回三国','梦回三国',0,0),(4850,1,1,'浮夸人生','浮夸人生',0,0),(123,1,2,'职场煮夫','职场煮夫',0,0),(4905,1,3,'王者归来','王者归来',0,0),(4883,1,2,'夏尔特蓝','夏尔特蓝',0,0),(5031,1,3,'qiubo31','qiubo31',0,0),(5088,1,2,'贺雪','贺雪',0,0),(65,1,1,'红豆','红豆',0,0),(42,1,1,'深蓝','深蓝',0,0),(4838,1,2,'大森林','大森林',0,0),(5059,1,2,'云中漫步','云中漫步',0,2),(5120,1,2,'爱姗','爱姗',0,0),(4866,1,1,'半步含烟','半步含烟',0,0),(4747,1,2,'飞侠','飞侠',0,0),(5122,1,2,'libinfeng','libinfeng',0,0),(5124,1,3,'楚风','楚风',0,0),(5123,1,2,'白狐','白狐',0,0),(4993,1,1,'玉麟','玉麟',0,0),(5119,1,1,'竹林听雨','竹林听雨',0,0),(5126,1,2,'热豆腐','热豆腐',0,0),(5127,1,2,'野驴','野驴',0,0),(5128,1,1,'妞妞','妞妞',1,0),(5145,1,2,'樱花烂漫','樱花烂漫',0,0),(5147,1,3,'天外来客','天外来客',0,0),(5092,1,2,'大猩猩二号','大猩猩二号',0,0),(4728,1,1,'BC-3','BC-3',0,0),(5140,1,2,'丹尼','丹尼',0,0),(5044,1,1,'申成','申成',0,0),(5141,1,2,'苏格拉没有底','苏格拉没有底',0,0),(5161,1,1,'夕阳无限好','夕阳无限好',0,0),(5133,1,3,'滴水看世界','滴水看世界',0,0),(5138,1,2,'木槿花开','木槿花开',0,0),(5060,1,2,'马宇','马宇',0,0),(5042,1,2,'股民老刘','股民老刘',0,0),(5053,1,1,'海洋','海洋',0,0),(5129,1,3,'珊瑚海','珊瑚海',0,1);

/*Table structure for table `ssp_user_serv_group` */

DROP TABLE IF EXISTS `ssp_user_serv_group`;

CREATE TABLE `ssp_user_serv_group` (
  `gid` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '客户分组ID',
  `name` varchar(20) NOT NULL COMMENT '客户分组名',
  `remark` varchar(100) NOT NULL COMMENT '客户分组备注',
  PRIMARY KEY (`gid`)
) ENGINE=MyISAM AUTO_INCREMENT=4 DEFAULT CHARSET=utf8;

/*Data for the table `ssp_user_serv_group` */

insert  into `ssp_user_serv_group`(`gid`,`name`,`remark`) values (1,'季度','季度(1800)'),(2,'半年','半年(3600)'),(3,'全年','全年(5800)');

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
  `sendkey` varchar(20) NOT NULL COMMENT '直播发送键',
  `sendkey_dateline` int(11) NOT NULL COMMENT '直播发送键设置时间',
  PRIMARY KEY (`uid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

/*Data for the table `ssp_user_setting` */

insert  into `ssp_user_setting`(`uid`,`expiry`,`expiry_dateline`,`gold`,`gold_dateline`,`invest`,`invest_dateline`,`sendkey`,`sendkey_dateline`) values (1,0,0,1,1339822011,1,1339822014,'Click',1340182184),(73,1355277212,1339563116,1,1339212867,1,1339231759,'',0),(5030,1355277212,1339563116,1,1339212934,1,1339212970,'',0),(4993,1355277212,1339563116,1,1339215465,1,1339215446,'',0),(4850,1355277212,1339563116,1,1339216477,1,1339216484,'',0),(5119,1355277212,1339563116,1,1339216516,1,1339216542,'',0),(5122,1355277212,1339563116,1,1339216643,1,1339217073,'',0),(5140,1355277212,1339563116,1,1339216972,1,1339217041,'',0),(4905,1355277212,1339563116,1,1339217122,1,1339217129,'',0),(64,1355277212,1339563116,1,1339217510,1,1339217558,'',0),(5128,1355277212,1339563116,1,1339218063,1,1339399278,'',0),(5059,1355277212,1339563116,1,1339218433,1,1339218523,'',0),(44,0,0,1,1339230332,1,1339230172,'',0),(5120,1355277212,1339563116,0,0,1,1339376992,'',0),(5133,1355277212,1339563116,1,1339377851,1,1339377846,'',0),(5141,1355277212,1339563116,1,1339383904,1,1339384108,'',0),(2,1355277212,1339563116,0,0,1,1339419645,'',0),(4801,1355277212,1339563044,0,0,0,0,'',0),(123,1355277212,1339563044,0,0,0,0,'',0),(4883,1355277212,1339563044,0,0,0,0,'',0),(5031,1355277212,1339563044,0,0,0,0,'',0),(5088,1355277212,1339563044,0,0,0,0,'',0),(65,1355277212,1339563044,0,0,0,0,'',0),(42,1355277212,1339563044,0,0,0,0,'',0),(4838,1355277212,1339563044,0,0,0,0,'',0),(4866,1355277212,1339563044,0,0,0,0,'',0),(4747,1355277212,1339563044,0,0,0,0,'',0),(5124,1355277212,1339563044,0,0,0,0,'',0),(5123,1355277212,1339563044,0,0,0,0,'',0),(5126,1355277212,1339563044,0,0,0,0,'',0),(5127,1355277212,1339563044,0,0,0,0,'',0),(5145,1355277212,1339563044,0,0,0,0,'',0),(5147,1355277212,1339563044,0,0,0,0,'',0),(5092,1355277212,1339563044,0,0,0,0,'',0),(4728,1355277212,1339563044,0,0,0,0,'',0),(5044,1355277212,1339563044,0,0,0,0,'',0),(5161,1355277212,1339563044,0,0,0,0,'',0),(5138,1355277212,1339563044,0,0,0,0,'',0),(5060,1355277212,1339563044,0,0,0,0,'',0),(5042,1355277212,1339563044,0,0,0,0,'',0),(5053,1355277212,1339563044,0,0,0,0,'',0),(5129,1355277212,1339563044,1,1340180064,1,1340180105,'',0);

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
