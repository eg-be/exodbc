DROP TABLE IF EXISTS `integertypes_tmp`;
CREATE TABLE `integertypes_tmp` (
  `idintegertypes` int(11) NOT NULL,
  `tsmallint` smallint(6) DEFAULT NULL,
  `tint` int(11) DEFAULT NULL,
  `tbigint` bigint(20) DEFAULT NULL,
  PRIMARY KEY (`idintegertypes`),
  UNIQUE KEY `idQueryTypes_UNIQUE` (`idintegertypes`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
