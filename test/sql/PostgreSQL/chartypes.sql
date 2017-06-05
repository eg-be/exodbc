DROP TABLE exodbc.chartypes;

CREATE TABLE exodbc.chartypes (
		idchartypes integer NOT NULL, 
		tvarchar varchar(128), 
		tchar char(128),
    PRIMARY KEY(idchartypes) 
	);
		
INSERT INTO exodbc.chartypes (idchartypes, tvarchar, tchar) VALUES (1,' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~',NULL);
INSERT INTO exodbc.chartypes (idchartypes, tvarchar, tchar) VALUES (2,NULL,' !"#$%&''()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~');
INSERT INTO exodbc.chartypes (idchartypes, tvarchar, tchar) VALUES (3,'abcdef',NULL);
INSERT INTO exodbc.chartypes (idchartypes, tvarchar, tchar) VALUES (4,NULL,'abcdef');
