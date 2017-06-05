DROP TABLE exodbc.floattypes_tmp;

CREATE TABLE exodbc.floattypes_tmp (
		idfloattypes integer NOT NULL, 
		tdouble float8, 
		tfloat float4,
    PRIMARY KEY(idfloattypes) 
	);