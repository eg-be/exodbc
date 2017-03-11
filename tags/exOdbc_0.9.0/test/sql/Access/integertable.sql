DROP TABLE integertable;

CREATE TABLE integertable (
		idintegertable Long PRIMARY KEY,
		tint1 Long,
		tint2 Long,
		tint3 Long
	);

INSERT INTO integertable (idintegertable, tint1, tint2, tint3) VALUES (1,2,3,4);
INSERT INTO integertable (idintegertable, tint1, tint2, tint3) VALUES (2,20,NULL,40);
