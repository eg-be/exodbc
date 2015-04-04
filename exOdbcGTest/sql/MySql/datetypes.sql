DROP TABLE IF EXISTS `datetypes`;
CREATE TABLE `datetypes` (
  `iddatetypes` int(11) NOT NULL,
  `tdate` date DEFAULT NULL,
  `ttime` time DEFAULT NULL,
  `ttimestamp` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`iddatetypes`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO `datetypes` VALUES (1,'1983-01-26','13:55:56','1983-01-26 12:55:56'),(2,NULL,NULL,'1983-01-26 12:55:56'),(3,NULL,NULL,NULL),(4,NULL,NULL,'1983-01-26 12:55:56');
