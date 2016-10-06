DROP TABLE IF EXISTS `chartypes`;

CREATE TABLE `chartypes` (
  `idchartypes` int(11) NOT NULL,
  `tvarchar` varchar(128) DEFAULT NULL,
  `tchar` char(128) DEFAULT NULL,
  `tvarchar_10` varchar(10) DEFAULT NULL,
  `tchar_10` char(10) DEFAULT NULL,
  PRIMARY KEY (`idchartypes`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO `chartypes` VALUES (1,' !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~',NULL,NULL,NULL),(2,NULL,' !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~',NULL,NULL),(3,'äöüàéè',NULL,NULL,NULL),(4,NULL,'äöüàéè',NULL,NULL),(5,NULL,NULL,'abc','abc'),(6,NULL,NULL,'abcde12345','abcde12345');
