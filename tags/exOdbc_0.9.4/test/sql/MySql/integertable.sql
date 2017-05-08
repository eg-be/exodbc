DROP TABLE IF EXISTS `integertable`;
CREATE TABLE `integertable` (
  `idintegertable` int(11) NOT NULL,
  `tint1` int(11) DEFAULT NULL,
  `tint2` int(11) DEFAULT NULL,
  `tint3` int(11) DEFAULT NULL,
  PRIMARY KEY (`idintegertable`),
  UNIQUE KEY `idQueryTypes_UNIQUE` (`idintegertable`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO `integertable` VALUES (1,2,3,4),(2,20,NULL,40);
