DROP TABLE IF EXISTS `datetypes`;
CREATE TABLE `datetypes` (
  `iddatetypes` int(11) NOT NULL,
  `tdate` date DEFAULT NULL,
  `ttime` time DEFAULT NULL,
  `ttimestamp` timestamp NULL DEFAULT NULL,
  PRIMARY KEY (`iddatetypes`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
