DROP TABLE EXODBC.FLOATTYPES;

CREATE TABLE EXODBC.FLOATTYPES (
		IDFLOATTYPES INTEGER NOT NULL, 
		TDOUBLE DOUBLE, 
		TFLOAT DOUBLE
	);
	
ALTER TABLE EXODBC.FLOATTYPES ADD CONSTRAINT FLOATTYPES_PK PRIMARY KEY ( IDFLOATTYPES );
	
INSERT INTO EXODBC.FLOATTYPES (IDFLOATTYPES, TDOUBLE, TFLOAT) VALUES (1,NULL,0);
INSERT INTO EXODBC.FLOATTYPES (IDFLOATTYPES, TDOUBLE, TFLOAT) VALUES (2,NULL,3.141);
INSERT INTO EXODBC.FLOATTYPES (IDFLOATTYPES, TDOUBLE, TFLOAT) VALUES (3,NULL,-3.141);
INSERT INTO EXODBC.FLOATTYPES (IDFLOATTYPES, TDOUBLE, TFLOAT) VALUES (4,0,NULL);
INSERT INTO EXODBC.FLOATTYPES (IDFLOATTYPES, TDOUBLE, TFLOAT) VALUES (5,3.141592,NULL);
INSERT INTO EXODBC.FLOATTYPES (IDFLOATTYPES, TDOUBLE, TFLOAT) VALUES (6,-3.141592,NULL);