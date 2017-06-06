DROP TABLE numerictypes;

CREATE TABLE numerictypes (
		idnumerictypes Long PRIMARY KEY, 
		tdecimal_18_0 DECIMAL(18 , 0), 
		tdecimal_18_10 DECIMAL(18 , 10), 
		tdecimal_5_3 DECIMAL(5 , 3)
	);
	
INSERT INTO numerictypes (idnumerictypes, tdecimal_18_0, tdecimal_18_10, tdecimal_5_3) VALUES (1,0,NULL,NULL);
INSERT INTO numerictypes (idnumerictypes, tdecimal_18_0, tdecimal_18_10, tdecimal_5_3) VALUES (2,123456789012345678,NULL,12.345);
INSERT INTO numerictypes (idnumerictypes, tdecimal_18_0, tdecimal_18_10, tdecimal_5_3) VALUES (3,-123456789012345678,NULL,NULL);
INSERT INTO numerictypes (idnumerictypes, tdecimal_18_0, tdecimal_18_10, tdecimal_5_3) VALUES (4,NULL,0.0000000000,NULL);
INSERT INTO numerictypes (idnumerictypes, tdecimal_18_0, tdecimal_18_10, tdecimal_5_3) VALUES (5,NULL,12345678.9012345678,NULL);
INSERT INTO numerictypes (idnumerictypes, tdecimal_18_0, tdecimal_18_10, tdecimal_5_3) VALUES (6,NULL,-12345678.9012345678,NULL);
INSERT INTO numerictypes (idnumerictypes, tdecimal_18_0, tdecimal_18_10, tdecimal_5_3) VALUES (7,NULL,123.0567,NULL);

