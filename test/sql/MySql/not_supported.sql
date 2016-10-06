DROP TABLE IF EXISTS `not_supported`;

CREATE TABLE `not_supported` (
  `idnot_supported` int(11) NOT NULL,
  `int1` int(11) DEFAULT NULL,
  `geomcoll` point DEFAULT NULL,
  `int2` int(11) DEFAULT NULL,
  PRIMARY KEY (`idnot_supported`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
