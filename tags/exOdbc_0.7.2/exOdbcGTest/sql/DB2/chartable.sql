DROP TABLE EXODBC.CHARTABLE;

CREATE TABLE EXODBC.CHARTABLE (
		IDCHARTABLE INTEGER NOT NULL, 
		COL2 CHAR(128), 
		COL3 CHAR(128), 
		COL4 CHAR(128)
	);
		
ALTER TABLE EXODBC.CHARTABLE ADD CONSTRAINT CHARTABLE_PK PRIMARY KEY ( IDCHARTABLE );
		
INSERT INTO EXODBC.CHARTABLE VALUES (1,'r1_c2','r1_c3','r1_c4');