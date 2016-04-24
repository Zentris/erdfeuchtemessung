-- MySQL dump 10.13  Distrib 5.6.30, for debian-linux-gnu (x86_64)
--
-- Host: 192.168.178.14    Database: ESPLogger
-- ------------------------------------------------------
-- Server version	5.5.47-0+deb7u1-log

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `ESP_Admin`
--

DROP TABLE IF EXISTS `ESP_Admin`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ESP_Admin` (
  `espid` int(3) NOT NULL,
  `espmac` varchar(45) NOT NULL DEFAULT '0',
  `gpio` int(3) NOT NULL DEFAULT '0',
  `messabstandMS` int(10) NOT NULL DEFAULT '30000' COMMENT 'dauer der feuchtemessung in ms',
  PRIMARY KEY (`espid`,`espmac`,`gpio`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `ESP_Feuchte`
--

DROP TABLE IF EXISTS `ESP_Feuchte`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `ESP_Feuchte` (
  `id` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `espno` int(3) unsigned NOT NULL DEFAULT '0',
  `date` date NOT NULL,
  `time` time NOT NULL,
  `feuchte` int(10) unsigned NOT NULL DEFAULT '0',
  `temperatur` float DEFAULT '-273',
  `luftfeuchte` float DEFAULT '0',
  `nan` smallint(3) NOT NULL DEFAULT '-1' COMMENT 'Anzahl fehlerhafter DTH22 Messungen',
  `espmac` varchar(18) NOT NULL DEFAULT '00:00:00:00:00:00',
  `gpio` varchar(3) NOT NULL DEFAULT '-1',
  PRIMARY KEY (`id`,`espno`),
  UNIQUE KEY `id` (`id`),
  KEY `date` (`date`),
  KEY `espno` (`espno`) USING BTREE,
  KEY `espmac` (`espmac`),
  KEY `gpio` (`gpio`)
) ENGINE=InnoDB AUTO_INCREMENT=164769 DEFAULT CHARSET=utf8 COMMENT='utf8_general_ci';
/*!40101 SET character_set_client = @saved_cs_client */;

--
-- Table structure for table `logging`
--

DROP TABLE IF EXISTS `logging`;
/*!40101 SET @saved_cs_client     = @@character_set_client */;
/*!40101 SET character_set_client = utf8 */;
CREATE TABLE `logging` (
  `lfnr` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `date` date NOT NULL,
  `time` time NOT NULL,
  `espno` int(11) NOT NULL,
  `logtext` text NOT NULL,
  PRIMARY KEY (`lfnr`),
  KEY `timestamp` (`espno`),
  KEY `date` (`date`,`time`)
) ENGINE=InnoDB AUTO_INCREMENT=36 DEFAULT CHARSET=utf8;
/*!40101 SET character_set_client = @saved_cs_client */;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2016-04-24 17:15:34
