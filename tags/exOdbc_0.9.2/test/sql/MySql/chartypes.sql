DROP TABLE IF EXISTS `chartypes`;

CREATE TABLE `chartypes` (
  `idchartypes` int(11) NOT NULL,
  `tvarchar` varchar(128) DEFAULT NULL,
  `tchar` char(128) DEFAULT NULL,
  PRIMARY KEY (`idchartypes`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO `chartypes` VALUES (1,' !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~',NULL);
INSERT INTO `chartypes` VALUES (2,NULL,' !\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~');
INSERT INTO `chartypes` VALUES (3,'abcdef',NULL);
INSERT INTO `chartypes` VALUES (4,NULL,'abcdef');
