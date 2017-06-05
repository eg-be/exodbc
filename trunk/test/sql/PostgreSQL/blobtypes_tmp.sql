DROP TABLE exodbc.blobtypes_tmp

CREATE TABLE exodbc.blobtypes_tmp (
		idblobtypes integer NOT NULL, 
		tblob bytea, 
		tvarblob_20 bytea,
    PRIMARY KEY(idblobtypes)    
	);
