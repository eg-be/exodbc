DROP TABLE EXODBC.FLOATTYPES_TMP;

CREATE TABLE EXODBC.FLOATTYPES_TMP (
		IDFLOATTYPES INTEGER NOT NULL, 
		TDOUBLE DOUBLE, 
		TFLOAT REAL
	);

ALTER TABLE EXODBC.FLOATTYPES_TMP ADD CONSTRAINT FLOATTYPES_TMP_PK PRIMARY KEY ( IDFLOATTYPES );
