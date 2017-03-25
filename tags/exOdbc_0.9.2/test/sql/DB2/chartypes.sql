DROP TABLE EXODBC.CHARTYPES;

CREATE TABLE EXODBC.CHARTYPES (
		IDCHARTYPES INTEGER NOT NULL, 
		TVARCHAR VARCHAR(128), 
		TCHAR CHAR(128)
	);
	
ALTER TABLE EXODBC.CHARTYPES ADD CONSTRAINT CHARTYPES_PK PRIMARY KEY ( IDCHARTYPES );		
	
INSERT INTO EXODBC.CHARTYPES (IDCHARTYPES, TVARCHAR, TCHAR) VALUES (1,' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~',NULL);
INSERT INTO EXODBC.CHARTYPES (IDCHARTYPES, TVARCHAR, TCHAR) VALUES (2,NULL,' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~');
INSERT INTO EXODBC.CHARTYPES (IDCHARTYPES, TVARCHAR, TCHAR) VALUES (3,'abcdef',NULL);
INSERT INTO EXODBC.CHARTYPES (IDCHARTYPES, TVARCHAR, TCHAR) VALUES (4,NULL,'abcdef');
