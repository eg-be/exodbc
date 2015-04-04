DROP TABLE integertypes;

CREATE TABLE integertypes (
		idintegertypes Long PRIMARY KEY,
		tsmallint Integer,
		tint Long,
		tbigint Long
	);

INSERT INTO integertypes (idintegertypes, tsmallint, tint) VALUES (1,-32768,NULL, NULL);
INSERT INTO integertypes (idintegertypes, tsmallint, tint) VALUES (2,32767,NULL, NULL);
INSERT INTO integertypes (idintegertypes, tsmallint, tint) VALUES (3,NULL,-2147483648, NULL);
INSERT INTO integertypes (idintegertypes, tsmallint, tint) VALUES (4,NULL,2147483647, NULL);
INSERT INTO integertypes (idintegertypes, tsmallint, tint) VALUES (5,NULL,NULL, NULL);
INSERT INTO integertypes (idintegertypes, tsmallint, tint) VALUES (6,NULL,NULL, NULL);
INSERT INTO integertypes (idintegertypes, tsmallint, tint) VALUES (7,-13,26, 10502);
