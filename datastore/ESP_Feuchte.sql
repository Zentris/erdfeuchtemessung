-- phpMyAdmin SQL Dump
-- version 3.5.8.1
-- http://www.phpmyadmin.net
--
-- Host: dd21600
-- Erstellungszeit: 11. Okt 2016 um 21:51
-- Server Version: 5.5.52-nmm1-log
-- PHP-Version: 5.2.17-nmm5

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Datenbank: `d021e3ce`
--

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ESP_Feuchte`
--

DROP TABLE IF EXISTS `ESP_Feuchte`;
CREATE TABLE IF NOT EXISTS `ESP_Feuchte` (
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
  `sensorid` int(4) NOT NULL DEFAULT '-1',
  PRIMARY KEY (`id`),
  KEY `date` (`date`),
  KEY `espno` (`espno`) USING BTREE,
  KEY `espmac` (`espmac`),
  KEY `gpio` (`gpio`),
  KEY `sensorid` (`sensorid`),
  KEY `espmac_2` (`espmac`,`sensorid`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COMMENT='utf8_general_ci' AUTO_INCREMENT=260344 ;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
