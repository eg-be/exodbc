DROP TABLE blobtypes;

CREATE TABLE blobtypes (
		idblobtypes Long PRIMARY KEY,
		tblob BINARY(16),
		tvarblob_20 VARBINARY(20)
	);

INSERT INTO blobtypes (idblobtypes, tblob, tvarblob_20) VALUES(1, 0x00000000000000000000000000000000, NULL);
INSERT INTO blobtypes (idblobtypes, tblob, tvarblob_20) VALUES(2, 0xffffffffffffffffffffffffffffffff, NULL);
INSERT INTO blobtypes (idblobtypes, tblob, tvarblob_20) VALUES(3, 0xabcdeff01234567890abcdef01234567, NULL);
INSERT INTO blobtypes (idblobtypes, tblob, tvarblob_20) VALUES(4, NULL, 0xabcdeff01234567890abcdef01234567);
INSERT INTO blobtypes (idblobtypes, tblob, tvarblob_20) VALUES(5, NULL, 0xabcdeff01234567890abcdef01234567ffffffff);

