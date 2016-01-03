DROP TABLE IF EXISTS `numerictypes_tmp`;
CREATE TABLE `numerictypes_tmp` (
  `idnumerictypes` int(11) NOT NULL,
  `tdecimal_18_0` decimal(18,0) DEFAULT NULL,
  `tdecimal_18_10` decimal(18,10) DEFAULT NULL,
  `tdecimal_5_3` decimal(5,3) DEFAULT NULL,
  PRIMARY KEY (`idnumerictypes`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

