DROP TABLE floattypes;

CREATE TABLE floattypes (
		idfloattypes Long PRIMARY KEY,
		tdouble DOUBLE,
		tfloat FLOAT
	);

INSERT INTO floattypes (idfloattypes, tdouble, tfloat) VALUES (1,NULL,0);
INSERT INTO floattypes (idfloattypes, tdouble, tfloat) VALUES (2,NULL,3.141);
INSERT INTO floattypes (idfloattypes, tdouble, tfloat) VALUES (3,NULL,-3.141);
INSERT INTO floattypes (idfloattypes, tdouble, tfloat) VALUES (4,0,NULL);
INSERT INTO floattypes (idfloattypes, tdouble, tfloat) VALUES (5,3.141592,NULL);
INSERT INTO floattypes (idfloattypes, tdouble, tfloat) VALUES (6,-3.141592,NULL);
