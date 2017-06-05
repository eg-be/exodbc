DROP TABLE exodbc.floattypes;

CREATE TABLE exodbc.floattypes (
		idfloattypes integer NOT NULL, 
		tdouble float8, 
		tfloat float4,
    PRIMARY KEY(idfloattypes) 
	);
		
INSERT INTO exodbc.floattypes (idfloattypes, tdouble, tfloat) VALUES (1,NULL,0);
INSERT INTO exodbc.floattypes (idfloattypes, tdouble, tfloat) VALUES (2,NULL,2.684);
INSERT INTO exodbc.floattypes (idfloattypes, tdouble, tfloat) VALUES (3,NULL,-2.684);
INSERT INTO exodbc.floattypes (idfloattypes, tdouble, tfloat) VALUES (4,0,NULL);
INSERT INTO exodbc.floattypes (idfloattypes, tdouble, tfloat) VALUES (5,3.141592,NULL);
INSERT INTO exodbc.floattypes (idfloattypes, tdouble, tfloat) VALUES (6,-3.141592,NULL);
