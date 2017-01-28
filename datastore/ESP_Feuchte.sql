-- phpMyAdmin SQL Dump
-- version 3.4.11.1deb2+deb7u7
-- http://www.phpmyadmin.net
--
-- Host: localhost
-- Erstellungszeit: 28. Jan 2017 um 22:00
-- Server Version: 5.5.53
-- PHP-Version: 5.4.45-0+deb7u6

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

--
-- Datenbank: `ESPLogger`
--

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ESP_Admin`
--
-- Erzeugt am: 23. Jan 2017 um 17:49
--

CREATE TABLE IF NOT EXISTS `ESP_Admin` (
  `espid` int(3) NOT NULL,
  `espmac` varchar(45) NOT NULL DEFAULT '0',
  `gpio` int(3) NOT NULL DEFAULT '0',
  `messabstandMS` int(10) NOT NULL DEFAULT '30000' COMMENT 'dauer der feuchtemessung in ms',
  PRIMARY KEY (`espid`,`espmac`,`gpio`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `ESP_Feuchte`
--
-- Erzeugt am: 23. Jan 2017 um 17:49
--

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
  `sensorid` int(4) DEFAULT '-1',
  `topfgewicht` float NOT NULL DEFAULT '-1',
  PRIMARY KEY (`id`,`espno`),
  UNIQUE KEY `id` (`id`),
  KEY `date` (`date`),
  KEY `espno` (`espno`) USING BTREE,
  KEY `espmac` (`espmac`),
  KEY `gpio` (`gpio`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 COMMENT='utf8_general_ci' AUTO_INCREMENT=871796 ;

-- --------------------------------------------------------

--
-- Tabellenstruktur für Tabelle `logging`
--
-- Erzeugt am: 23. Jan 2017 um 17:49
--

CREATE TABLE IF NOT EXISTS `logging` (
  `lfnr` bigint(20) unsigned NOT NULL AUTO_INCREMENT,
  `date` date NOT NULL,
  `time` time NOT NULL,
  `espno` int(11) NOT NULL,
  `logtext` text NOT NULL,
  PRIMARY KEY (`lfnr`),
  KEY `timestamp` (`espno`),
  KEY `date` (`date`,`time`)
) ENGINE=InnoDB  DEFAULT CHARSET=utf8 AUTO_INCREMENT=36 ;

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
