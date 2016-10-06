DROP TABLE IF EXISTS `chartable`;
CREATE TABLE `chartable` (
  `idchartable` int(11) NOT NULL,
  `col2` varchar(128) DEFAULT NULL,
  `col3` varchar(128) DEFAULT NULL,
  `col4` varchar(128) DEFAULT NULL,
  PRIMARY KEY (`idchartable`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO `chartable` VALUES (1,'r1_c2','r1_c3','r1_c4');
