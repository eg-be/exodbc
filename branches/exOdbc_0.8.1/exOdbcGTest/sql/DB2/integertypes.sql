DROP TABLE EXODBC.INTEGERTYPES;

CREATE TABLE EXODBC.INTEGERTYPES (
		IDINTEGERTYPES INTEGER NOT NULL, 
		TSMALLINT SMALLINT, 
		TINT INTEGER, 
		TBIGINT BIGINT
	);

ALTER TABLE EXODBC.INTEGERTYPES ADD CONSTRAINT INTEGERTYPES_PK PRIMARY KEY ( IDINTEGERTYPES );		
	
INSERT INTO EXODBC.INTEGERTYPES (IDINTEGERTYPES, TSMALLINT, TINT, TBIGINT) VALUES (1,-32768,NULL,NULL);
INSERT INTO EXODBC.INTEGERTYPES (IDINTEGERTYPES, TSMALLINT, TINT, TBIGINT) VALUES (2,32767,NULL,NULL);
INSERT INTO EXODBC.INTEGERTYPES (IDINTEGERTYPES, TSMALLINT, TINT, TBIGINT) VALUES (3,NULL,-2147483648,NULL);
INSERT INTO EXODBC.INTEGERTYPES (IDINTEGERTYPES, TSMALLINT, TINT, TBIGINT) VALUES (4,NULL,2147483647,NULL);
INSERT INTO EXODBC.INTEGERTYPES (IDINTEGERTYPES, TSMALLINT, TINT, TBIGINT) VALUES (5,NULL,NULL,-9223372036854775808);
INSERT INTO EXODBC.INTEGERTYPES (IDINTEGERTYPES, TSMALLINT, TINT, TBIGINT) VALUES (6,NULL,NULL,9223372036854775807);
INSERT INTO EXODBC.INTEGERTYPES (IDINTEGERTYPES, TSMALLINT, TINT, TBIGINT) VALUES (7,-13,26,10502);