DROP TABLE exodbc.blobtypes

CREATE TABLE exodbc.blobtypes (
		idblobtypes integer NOT NULL, 
		tblob bytea, 
		tvarblob_20 bytea,
    PRIMARY KEY(idblobtypes)    
	);
		
INSERT INTO exodbc.blobtypes (idblobtypes, tblob, tvarblob_20) VALUES(1, '\x00000000000000000000000000000000', NULL);
INSERT INTO exodbc.blobtypes (idblobtypes, tblob, tvarblob_20) VALUES(2, '\xffffffffffffffffffffffffffffffff', NULL);
INSERT INTO exodbc.blobtypes (idblobtypes, tblob, tvarblob_20) VALUES(3, '\xabcdeff01234567890abcdef01234567', NULL);
INSERT INTO exodbc.blobtypes (idblobtypes, tblob, tvarblob_20) VALUES(4, NULL, '\xabcdeff01234567890abcdef01234567');
INSERT INTO exodbc.blobtypes (idblobtypes, tblob, tvarblob_20) VALUES(5, NULL, '\xabcdeff01234567890abcdef01234567ffffffff');
