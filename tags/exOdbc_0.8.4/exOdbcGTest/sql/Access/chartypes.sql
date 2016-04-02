DROP TABLE chartypes;

CREATE TABLE chartypes (
		idchartypes Long PRIMARY KEY,
		tvarchar VARCHAR(128),
		tchar VARCHAR(128),
		tvarchar_10 VARCHAR(10),
		tchar_10 VARCHAR(10)
	);

INSERT INTO chartypes (idchartypes, tvarchar, tchar, tvarchar_10, tchar_10) VALUES (1, ' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~', NULL, NULL, NULL);
INSERT INTO chartypes (idchartypes, tvarchar, tchar, tvarchar_10, tchar_10) VALUES (2, NULL, ' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~', NULL, NULL);
INSERT INTO chartypes (idchartypes, tvarchar, tchar, tvarchar_10, tchar_10) VALUES (3, 'äöüàéè', NULL, NULL, NULL);
INSERT INTO chartypes (idchartypes, tvarchar, tchar, tvarchar_10, tchar_10) VALUES (4, NULL, 'äöüàéè', NULL, NULL);
INSERT INTO chartypes (idchartypes, tvarchar, tchar, tvarchar_10, tchar_10) VALUES (5, NULL, NULL, 'abc', 'abc');
INSERT INTO chartypes (idchartypes, tvarchar, tchar, tvarchar_10, tchar_10) VALUES (6, NULL, NULL, 'abcde12345', 'abcde12345');
