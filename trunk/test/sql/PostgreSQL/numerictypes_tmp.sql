DROP TABLE exodbc.numerictypes_tmp;

CREATE TABLE exodbc.numerictypes_tmp (
		idnumerictypes integer NOT NULL, 
		tdecimal_18_0 numeric(18 , 0), 
		tdecimal_18_10 numeric(18 , 10), 
		tdecimal_5_3 numeric(5 , 3),
    PRIMARY KEY(idnumerictypes)
	);