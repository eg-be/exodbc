DROP TABLE EXODBC.CHARTYPES_TMP;

CREATE TABLE EXODBC.CHARTYPES_TMP (
		IDCHARTYPES INTEGER NOT NULL, 
		TVARCHAR VARCHAR(128), 
		TCHAR CHAR(128), 
		TVARCHAR_10 VARCHAR(10), 
		TCHAR_10 CHAR(10)
	);
	
INSERT INTO EXODBC.CHARTYPES_TMP (IDCHARTYPES, TVARCHAR, TCHAR, TVARCHAR_10, TCHAR_10) VALUES (1,' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~',NULL,NULL,NULL);
INSERT INTO EXODBC.CHARTYPES_TMP (IDCHARTYPES, TVARCHAR, TCHAR, TVARCHAR_10, TCHAR_10) VALUES (2,NULL,' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~',NULL,NULL);
INSERT INTO EXODBC.CHARTYPES_TMP (IDCHARTYPES, TVARCHAR, TCHAR, TVARCHAR_10, TCHAR_10) VALUES (3,'äöüàéè',NULL,NULL,NULL);
INSERT INTO EXODBC.CHARTYPES_TMP (IDCHARTYPES, TVARCHAR, TCHAR, TVARCHAR_10, TCHAR_10) VALUES (4,NULL,'äöüàéè',NULL,NULL);
INSERT INTO EXODBC.CHARTYPES_TMP (IDCHARTYPES, TVARCHAR, TCHAR, TVARCHAR_10, TCHAR_10) VALUES (5,NULL,NULL,'abc','abc');
INSERT INTO EXODBC.CHARTYPES_TMP (IDCHARTYPES, TVARCHAR, TCHAR, TVARCHAR_10, TCHAR_10) VALUES (6,NULL,NULL,'abcde12345','abcde12345');
