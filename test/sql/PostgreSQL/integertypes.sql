DROP TABLE exodbc.integertypes;

CREATE TABLE exodbc.integertypes (
		idintegertypes integer NOT NULL, 
		tsmallint smallint, 
		tint integer, 
		tbigint bigint,
    PRIMARY KEY(idintegertypes)
	);


INSERT INTO exodbc.integertypes (idintegertypes, tsmallint, tint, tbigint) VALUES (1,-32768,NULL,NULL);
INSERT INTO exodbc.integertypes (idintegertypes, tsmallint, tint, tbigint) VALUES (2,32767,NULL,NULL);
INSERT INTO exodbc.integertypes (idintegertypes, tsmallint, tint, tbigint) VALUES (3,NULL,-2147483648,NULL);
INSERT INTO exodbc.integertypes (idintegertypes, tsmallint, tint, tbigint) VALUES (4,NULL,2147483647,NULL);
INSERT INTO exodbc.integertypes (idintegertypes, tsmallint, tint, tbigint) VALUES (5,NULL,NULL,-9223372036854775808);
INSERT INTO exodbc.integertypes (idintegertypes, tsmallint, tint, tbigint) VALUES (6,NULL,NULL,9223372036854775807);
INSERT INTO exodbc.integertypes (idintegertypes, tsmallint, tint, tbigint) VALUES (7,-13,26,10502);
