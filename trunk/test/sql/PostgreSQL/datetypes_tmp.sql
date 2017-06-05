DROP TABLE exodbc.datetypes_tmp;

CREATE TABLE exodbc.datetypes_tmp (
		iddatetypes integer NOT NULL, 
		tdate date, 
		ttime time, 
		ttimestamp timestamp,
    PRIMARY KEY(iddatetypes) 
	);