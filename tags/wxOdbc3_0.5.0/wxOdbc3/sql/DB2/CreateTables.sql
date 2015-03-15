--<ScriptOptions statementTerminator=";"/>

CREATE LARGE TABLESPACE "USERSPACE1"
	IN DATABASE PARTITION GROUP "IBMDEFAULTGROUP"
	PAGESIZE 4 K
	MANAGED BY AUTOMATIC STORAGE
	AUTORESIZE YES
	EXTENTSIZE 32
	BUFFERPOOL "IBMDEFAULTBP"
	OVERHEAD INHERIT
	TRANSFERRATE INHERIT
	DATA TAG INHERIT
	USING STOGROUP "IBMSTOGROUP"
	NO FILE SYSTEM CACHING;

CREATE TABLE "WXODBC3"."BLOBTYPES" (
		"IDBLOBTYPES" INTEGER NOT NULL, 
		"BLOB" BLOB(16) INLINE LENGTH 68
	)
	ORGANIZE BY ROW
	DATA CAPTURE NONE 
	IN "USERSPACE1"
	COMPRESS NO;

CREATE TABLE "WXODBC3"."CHARTYPES" (
		"IDCHARTYPES" INTEGER NOT NULL, 
		"VARCHAR" VARCHAR(128), 
		"CHAR" CHAR(128)
	)
	ORGANIZE BY ROW
	DATA CAPTURE NONE 
	IN "USERSPACE1"
	COMPRESS NO;

CREATE TABLE "WXODBC3"."DATETYPES" (
		"IDDATETYPES" INTEGER NOT NULL, 
		"DATE" DATE, 
		"TIME" TIME, 
		"TIMETAMP" TIMESTAMP
	)
	ORGANIZE BY ROW
	DATA CAPTURE NONE 
	IN "USERSPACE1"
	COMPRESS NO;

CREATE TABLE "WXODBC3"."FLOATTYPES" (
		"IDFLOATTYPES" INTEGER NOT NULL, 
		"DOUBLE" DOUBLE, 
		"FLOAT" DOUBLE
	)
	ORGANIZE BY ROW
	DATA CAPTURE NONE 
	IN "USERSPACE1"
	COMPRESS NO;

CREATE TABLE "WXODBC3"."INTEGERTYPES" (
		"IDINTEGERTYPES" INTEGER NOT NULL, 
		"SMALLINT" SMALLINT, 
		"INT" INTEGER, 
		"BIGINT" BIGINT
	)
	ORGANIZE BY ROW
	DATA CAPTURE NONE 
	IN "USERSPACE1"
	COMPRESS NO;

CREATE TABLE "WXODBC3"."NUMERICTYPES" (
		"IDNUMERICTYPES" INTEGER NOT NULL, 
		"DECIMAL_18_0" DECIMAL(18 , 0), 
		"DECIMAL_18_10" DECIMAL(18 , 10)
	)
	ORGANIZE BY ROW
	DATA CAPTURE NONE 
	IN "USERSPACE1"
	COMPRESS NO;

ALTER TABLE "WXODBC3"."BLOBTYPES" ADD CONSTRAINT "SQL140713172005040" PRIMARY KEY
	("IDBLOBTYPES");

ALTER TABLE "WXODBC3"."CHARTYPES" ADD CONSTRAINT "SQL140223170703300" PRIMARY KEY
	("IDCHARTYPES");

ALTER TABLE "WXODBC3"."DATETYPES" ADD CONSTRAINT "SQL140223221611740" PRIMARY KEY
	("IDDATETYPES");

ALTER TABLE "WXODBC3"."FLOATTYPES" ADD CONSTRAINT "SQL140223172006570" PRIMARY KEY
	("IDFLOATTYPES");

ALTER TABLE "WXODBC3"."INTEGERTYPES" ADD CONSTRAINT "SQL140223170050240" PRIMARY KEY
	("IDINTEGERTYPES");

ALTER TABLE "WXODBC3"."NUMERICTYPES" ADD CONSTRAINT "SQL140713150131550" PRIMARY KEY
	("IDNUMERICTYPES");

GRANT USE OF TABLESPACE "USERSPACE1" TO  PUBLIC;
