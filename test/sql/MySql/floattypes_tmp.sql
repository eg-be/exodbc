DROP TABLE IF EXISTS `floattypes_tmp`;
CREATE TABLE `floattypes_tmp` (
  `idfloattypes` int(11) NOT NULL,
  `tdouble` double DEFAULT NULL,
  `tfloat` float DEFAULT NULL,
  PRIMARY KEY (`idfloattypes`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

