DROP TABLE exodbc.multikey;

CREATE TABLE exodbc.multikey (
		id1 integer NOT NULL, 
		id2 integer NOT NULL, 
		value varchar(10), 
		id3 integer NOT NULL,
    PRIMARY KEY(id1, id2, id3)
	);
		 
