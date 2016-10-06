DROP TABLE IF EXISTS `chartypes_tmp`;

CREATE TABLE `chartypes_tmp` (
  `idchartypes` int(11) NOT NULL,
  `tvarchar` varchar(128) DEFAULT NULL,
  `tchar` char(128) DEFAULT NULL,
  `tvarchar_10` varchar(10) DEFAULT NULL,
  `tchar_10` char(10) DEFAULT NULL,
  PRIMARY KEY (`idchartypes`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

