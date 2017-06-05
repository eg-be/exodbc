DROP TABLE exodbc.chartypes_tmp;

CREATE TABLE exodbc.chartypes_tmp (
		idchartypes integer NOT NULL, 
		tvarchar varchar(128), 
		tchar char(128),
    PRIMARY KEY(idchartypes) 
	);
	
