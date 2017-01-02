DROP TABLE IF EXISTS `integertypes`;
CREATE TABLE `integertypes` (
  `idintegertypes` int(11) NOT NULL,
  `tsmallint` smallint(6) DEFAULT NULL,
  `tint` int(11) DEFAULT NULL,
  `tbigint` bigint(20) DEFAULT NULL,
  PRIMARY KEY (`idintegertypes`),
  UNIQUE KEY `idQueryTypes_UNIQUE` (`idintegertypes`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO `integertypes` VALUES (1,-32768,NULL,NULL),(2,32767,NULL,NULL),(3,NULL,-2147483648,NULL),(4,NULL,2147483647,NULL),(5,NULL,NULL,-9223372036854775808),(6,NULL,NULL,9223372036854775807),(7,-13,26,10502);
