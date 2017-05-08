DROP TABLE chartypes;

CREATE TABLE chartypes (
		idchartypes Long PRIMARY KEY,
		tvarchar VARCHAR(128),
		tchar VARCHAR(128)
	);

INSERT INTO chartypes (idchartypes, tvarchar, tchar) VALUES (1, ' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~', NULL);
INSERT INTO chartypes (idchartypes, tvarchar, tchar) VALUES (2, NULL, ' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~');
INSERT INTO chartypes (idchartypes, tvarchar, tchar) VALUES (3, 'abcdef', NULL);
INSERT INTO chartypes (idchartypes, tvarchar, tchar) VALUES (4, NULL, 'abcdef');
