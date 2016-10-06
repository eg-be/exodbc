DROP TABLE IF EXISTS `blobtypes_tmp`;

CREATE TABLE `blobtypes_tmp` (
  `idblobtypes` int(11) NOT NULL,
  `tblob` binary(16) DEFAULT NULL,
  `tvarblob_20` varbinary(20) DEFAULT NULL,
  PRIMARY KEY (`idblobtypes`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

