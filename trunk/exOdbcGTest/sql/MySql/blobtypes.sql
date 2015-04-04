DROP TABLE IF EXISTS `blobtypes`;

CREATE TABLE `blobtypes` (
  `idblobtypes` int(11) NOT NULL,
  `tblob` binary(16) DEFAULT NULL,
  `tvarblob_20` varbinary(20) DEFAULT NULL,
  PRIMARY KEY (`idblobtypes`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

INSERT INTO blobtypes (idblobtypes, tblob, tvarblob_20) VALUES(1, 0x00000000000000000000000000000000, NULL);
INSERT INTO blobtypes (idblobtypes, tblob, tvarblob_20) VALUES(2, 0xffffffffffffffffffffffffffffffff, NULL);
INSERT INTO blobtypes (idblobtypes, tblob, tvarblob_20) VALUES(3, 0xabcdeff01234567890abcdef01234567, NULL);
INSERT INTO blobtypes (idblobtypes, tblob, tvarblob_20) VALUES(4, NULL, 0xabcdeff01234567890abcdef01234567);
INSERT INTO blobtypes (idblobtypes, tblob, tvarblob_20) VALUES(5, NULL, 0xabcdeff01234567890abcdef01234567ffffffff);
