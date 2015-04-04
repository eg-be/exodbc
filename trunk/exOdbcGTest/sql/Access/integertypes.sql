DROP TABLE integertypes;

CREATE TABLE integertypes (
		idintegertypes Long PRIMARY KEY,
		tsmallint Integer,
		tint Long
	);

INSERT INTO integertypes (idintegertypes, tsmallint, tint) VALUES (1,-32768,NULL);
INSERT INTO integertypes (idintegertypes, tsmallint, tint) VALUES (2,32767,NULL);
INSERT INTO integertypes (idintegertypes, tsmallint, tint) VALUES (3,NULL,-2147483648);
INSERT INTO integertypes (idintegertypes, tsmallint, tint) VALUES (4,NULL,2147483647);
INSERT INTO integertypes (idintegertypes, tsmallint, tint) VALUES (5,NULL,NULL);
INSERT INTO integertypes (idintegertypes, tsmallint, tint) VALUES (6,NULL,NULL);
INSERT INTO integertypes (idintegertypes, tsmallint, tint) VALUES (7,-13,26);
