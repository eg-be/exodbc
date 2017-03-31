DROP TABLE numerictypes_tmp;

CREATE TABLE numerictypes_tmp (
		idnumerictypes Long PRIMARY KEY, 
		tdecimal_18_0 DECIMAL(18 , 0), 
		tdecimal_18_10 DECIMAL(18 , 10), 
		tdecimal_5_3 DECIMAL(5 , 3)
	);
