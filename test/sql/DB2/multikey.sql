DROP TABLE EXODBC.MULTIKEY;

CREATE TABLE EXODBC.MULTIKEY (
		ID1 INTEGER NOT NULL, 
		ID2 INTEGER NOT NULL, 
		VALUE VARCHAR(10), 
		ID3 INTEGER NOT NULL
	);
	
ALTER TABLE EXODBC.MULTIKEY ADD CONSTRAINT MULTIKEY_PK PRIMARY KEY
	(ID1, 
	 ID2, 
	 ID3);
	 
