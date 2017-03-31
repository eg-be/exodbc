DROP TABLE IF EXISTS `unicodetable_tmp`;

CREATE TABLE `unicodetable_tmp` (
  `idunicodetable` int(11) NOT NULL,
  `content` varchar(255) CHARACTER SET utf8 COLLATE utf8_general_ci DEFAULT NULL,
  PRIMARY KEY (`idunicodetable`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
