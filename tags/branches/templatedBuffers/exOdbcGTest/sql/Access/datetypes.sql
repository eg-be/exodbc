DROP TABLE datetypes;

CREATE TABLE datetypes (
		iddatetypes Long PRIMARY KEY,
		tdate DATETIME,
		ttime DATETIME,
		ttimestamp DATETIME
	);

INSERT INTO datetypes (iddatetypes, tdate, ttime, ttimestamp) VALUES (1, '1983-01-26', '13:55:56', '1983-01-26 13:55:56');
INSERT INTO datetypes (iddatetypes, tdate, ttime, ttimestamp) VALUES (2, NULL, NULL, '1983-01-26 13:55:56');
INSERT INTO datetypes (iddatetypes, tdate, ttime, ttimestamp) VALUES (3, NULL, NULL, NULL);
INSERT INTO datetypes (iddatetypes, tdate, ttime, ttimestamp) VALUES (4, NULL, NULL, '1983-01-26 13:55:56');

