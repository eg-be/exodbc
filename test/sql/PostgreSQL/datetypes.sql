DROP TABLE exodbc.datetypes;

CREATE TABLE exodbc.datetypes (
		iddatetypes integer NOT NULL, 
		tdate date, 
		ttime time, 
		ttimestamp timestamp,
    PRIMARY KEY(iddatetypes) 
	);
		
INSERT INTO exodbc.datetypes (iddatetypes, tdate, ttime, ttimestamp ) VALUES (1,'1983-01-26','13:55:56','1983-01-26 13:55:56');
INSERT INTO exodbc.datetypes (iddatetypes, tdate, ttime, ttimestamp ) VALUES (2,NULL,NULL,'1983-01-26 13:55:56.123456');
INSERT INTO exodbc.datetypes (iddatetypes, tdate, ttime, ttimestamp ) VALUES (3,NULL,NULL,NULL);
INSERT INTO exodbc.datetypes (iddatetypes, tdate, ttime, ttimestamp ) VALUES (4,NULL,NULL,'1983-01-26 13:55:56.01');
