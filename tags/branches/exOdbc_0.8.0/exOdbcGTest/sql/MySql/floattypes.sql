DROP TABLE IF EXISTS `floattypes`;
CREATE TABLE `floattypes` (
  `idfloattypes` int(11) NOT NULL,
  `tdouble` double DEFAULT NULL,
  `tfloat` float DEFAULT NULL,
  PRIMARY KEY (`idfloattypes`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO `floattypes` VALUES (1,NULL,0),(2,NULL,3.141),(3,NULL,-3.141),(4,0,NULL),(5,3.141592,NULL),(6,-3.141592,NULL);
