DROP TABLE EXODBC.CHARTYPES_TMP;

CREATE TABLE EXODBC.CHARTYPES_TMP (
		IDCHARTYPES INTEGER NOT NULL, 
		TVARCHAR VARCHAR(128), 
		TCHAR CHAR(128)
	);

ALTER TABLE EXODBC.CHARTYPES_TMP ADD CONSTRAINT CHARTYPES_TMP_PK PRIMARY KEY ( IDCHARTYPES );
	