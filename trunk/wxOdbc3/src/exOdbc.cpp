/*!
* \file wxOdbc3.cpp
* \author Elias Gerber <egerber@gmx.net>
* \date 23.07.2014
* 
* [Brief CPP-file description]
*/ 

#include "stdafx.h"

// Own header
#include "exOdbc.h"

// Same component headers
#include "Helpers.h"

// Other headers

namespace exodbc {

	// Static consts
	// -------------
	const wchar_t* emptyString				= L"";
	const wchar_t* SQL_LOG_FILENAME         = L"sqllog.txt";
	const wchar_t* SQL_CATALOG_FILENAME     = L"catalog.txt";

	// Implementation
	// --------------
	SDbInfo::SDbInfo()
	{
		dbmsName[0] = 0;
		dbmsVer[0] = 0;
		driverName[0] = 0;
		odbcVer[0] = 0;
		drvMgrOdbcVer[0] = 0;
		driverVer[0] = 0;
		serverName[0] = 0;
		databaseName[0] = 0;
		outerJoins[0] = 0;
		procedureSupport[0] = 0;
		accessibleTables[0] = 0;
		maxConnections = 0;
		maxStmts = 0;
		cliConfLvl = 0;
		cursorCommitBehavior = 0;
		cursorRollbackBehavior = 0;
		supportNotNullClause = 0;
		supportIEF[0] = 0;
		txnIsolation = 0;
		txnIsolationOptions = 0;
		posOperations = 0;
		posStmts = 0;
		scrollOptions = 0;
		txnCapable = 0;
		maxCatalogNameLen = 0;
		maxSchemaNameLen = 0;
		maxTableNameLen = 0;
	}

	std::wstring SDbInfo::ToStr() const
	{
		std::wstringstream ws;
		ws << L"***** DATA SOURCE INFORMATION *****" << std::endl;
		ws << L" SERVER Name: " << serverName << std::endl;
		ws << L"  DBMS Name: " << dbmsName << L"; DBMS Version: " << dbmsVer << std::endl;
		ws << L"  ODBC Version: " << odbcVer << L"; Driver Version: " << driverVer << std::endl;

		ws << L"  SAG CLI Conf. Level: ";
		switch(cliConfLvl)
		{
		case SQL_OSCC_NOT_COMPLIANT:    ws << L"Not Compliant";    break;
		case SQL_OSCC_COMPLIANT:        ws << L"Compliant";        break;
		}
		ws << std::endl;

		ws << L"  Max. Connections: "       << maxConnections   << std::endl;
		ws << L"  Outer Joins: "            << outerJoins       << std::endl;
		ws << L"  Support for Procedures: " << procedureSupport << std::endl;
		ws << L"  All tables accessible : " << accessibleTables << std::endl;
		ws << L"  Cursor COMMIT Behavior: ";
		switch(cursorCommitBehavior)
		{
		case SQL_CB_DELETE:        ws << L"Delete cursors";      break;
		case SQL_CB_CLOSE:         ws << L"Close cursors";       break;
		case SQL_CB_PRESERVE:      ws << L"Preserve cursors";    break;
		}
		ws << std::endl;

		ws << L"  Cursor ROLLBACK Behavior: ";
		switch(cursorRollbackBehavior)
		{
		case SQL_CB_DELETE:      ws << L"Delete cursors";      break;
		case SQL_CB_CLOSE:       ws << L"Close cursors";       break;
		case SQL_CB_PRESERVE:    ws << L"Preserve cursors";    break;
		}
		ws << std::endl;

		ws << L"  Support NOT NULL clause: ";
		switch(supportNotNullClause)
		{
		case SQL_NNC_NULL:        ws << L"No";        break;
		case SQL_NNC_NON_NULL:    ws << L"Yes";       break;
		}
		ws << std::endl;

		ws << L"  Support IEF (Ref. Integrity): " << supportIEF   << std::endl;

		ws << L"  Default Transaction Isolation: ";
		switch(txnIsolation)
		{
		case SQL_TXN_READ_UNCOMMITTED:  ws << L"Read Uncommitted";    break;
		case SQL_TXN_READ_COMMITTED:    ws << L"Read Committed";      break;
		case SQL_TXN_REPEATABLE_READ:   ws << L"Repeatable Read";     break;
		case SQL_TXN_SERIALIZABLE:      ws << L"Serializable";        break;
		}
		ws << std::endl;

		ws << L"  Transaction Isolation Options: ";
		if (txnIsolationOptions & SQL_TXN_READ_UNCOMMITTED)
			ws << L"Read Uncommitted, ";
		if (txnIsolationOptions & SQL_TXN_READ_COMMITTED)
			ws << L"Read Committed, ";
		if (txnIsolationOptions & SQL_TXN_REPEATABLE_READ)
			ws << L"Repeatable Read, ";
		if (txnIsolationOptions & SQL_TXN_SERIALIZABLE)
			ws << L"Serializable, ";
		ws << std::endl;

		ws << L"  Position Operations Supported (SQLSetPos): ";
		if (posOperations & SQL_POS_POSITION)
			ws << L"Position, ";
		if (posOperations & SQL_POS_REFRESH)
			ws << L"Refresh, ";
		if (posOperations & SQL_POS_UPDATE)
			ws << L"Upd, ";
		if (posOperations & SQL_POS_DELETE)
			ws << L"Del, ";
		if (posOperations & SQL_POS_ADD)
			ws << L"Add";
		ws << std::endl;

		ws << L"  Positioned Statements Supported: ";
		if (posStmts & SQL_PS_POSITIONED_DELETE)
			ws << L"Pos delete, ";
		if (posStmts & SQL_PS_POSITIONED_UPDATE)
			ws << L"Pos update, ";
		if (posStmts & SQL_PS_SELECT_FOR_UPDATE)
			ws << L"Select for update";
		ws << std::endl;

		ws << L"  Scroll Options: ";
		if (scrollOptions & SQL_SO_FORWARD_ONLY)
			ws << L"Fwd Only, ";
		if (scrollOptions & SQL_SO_STATIC)
			ws << L"Static, ";
		if (scrollOptions & SQL_SO_KEYSET_DRIVEN)
			ws << L"Keyset Driven, ";
		if (scrollOptions & SQL_SO_DYNAMIC)
			ws << L"Dynamic, ";
		if (scrollOptions & SQL_SO_MIXED)
			ws << L"Mixed";
		ws << std::endl;

		ws << L"  Transaction Capable?: ";
		switch(txnCapable)
		{
		case SQL_TC_NONE:          ws << L"No";            break;
		case SQL_TC_DML:           ws << L"DML Only";      break;
		case SQL_TC_DDL_COMMIT:    ws << L"DDL Commit";    break;
		case SQL_TC_DDL_IGNORE:    ws << L"DDL Ignore";    break;
		case SQL_TC_ALL:           ws << L"DDL & DML";     break;
		}
		ws << std::endl;

		return ws.str();
	}

	SQLUSMALLINT SDbInfo::GetMaxCatalogNameLen() const
	{
		return maxCatalogNameLen != 0 ? maxCatalogNameLen : DB_MAX_CATALOG_NAME_LEN_DEFAULT;
	}

	SQLUSMALLINT SDbInfo::GetMaxSchemaNameLen() const
	{
		return maxSchemaNameLen != 0 ? maxSchemaNameLen : DB_MAX_SCHEMA_NAME_LEN_DEFAULT;
	}

	SQLUSMALLINT SDbInfo::GetMaxTableNameLen() const
	{
		return maxTableNameLen != 0 ? maxTableNameLen : DB_MAX_TABLE_NAME_LEN_DEFAULT;
	}

	SSqlTypeInfo::SSqlTypeInfo()
	{
		FsqlType = 0;
		Precision = 0;
		PrecisionIsNull = false;
		LiteralPrefixIsNull = false;
		LiteralSuffixIsNull = false;
		CreateParamsIsNull = false;
		Nullable = SQL_NULLABLE_UNKNOWN;
		CaseSensitive = SQL_FALSE;
		Searchable = SQL_PRED_NONE;
		Unsigned = SQL_FALSE;
		UnsignedIsNull = false;
		FixedPrecisionScale = SQL_FALSE;
		AutoUniqueValue = SQL_FALSE;
		AutoUniqueValueIsNull = false;
		LocalTypeNameIsNull = false;
		MinimumScale = 0;
		MinimumScaleIsNull = false;
		MaximumScale = 0;
		MaximumScaleIsNull = false;
		SqlDataType = 0;
		SqlDateTimeSub = 0;
		SqlDateTimeSubIsNull = false;
		NumPrecRadix = 0;
		NumPrecRadixIsNull = false;
		IntervalPrecision = 0;
		IntervalPrecisionIsNull = false;
	}

	std::wstring SSqlTypeInfo::ToOneLineStr(bool withHeaderLines /* = false */, bool withEndLine /* = false */) const
	{
		std::wstringstream ws;
		if(withHeaderLines)
		{
			ws << (boost::wformat(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------")).str() << std::endl;
			ws << (boost::wformat(L"| %25s | %25s | %34s | %45s | %5s | %10s | %8s | %6s | %6s | %10s | %10s | %10s | %5s | %5s | %5s | %5s | %5s | %5s | %34s |") %L"SQLType" %L"SQL Data Type (3)" %L"TypeName" %L"Local TypeName" %L"Unsig" %L"Precision" %L"Nullable" %L"AutoI" %L"CaseS." %L"Searchable" %L"Prefix" %L"Suffix" %L"FixPS" %L"MinSc" %L"MaxSc" %L"DTS3" %L"NuPR" %L"IntPr" %L"Create Params").str() << std::endl;
			ws << (boost::wformat(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------")).str() << std::endl;
		}

		std::wstring sFSqlType = (boost::wformat(L"%18s (%4d)") %SqlType2s(FsqlType) %FsqlType).str();
		std::wstring sSqlDataType = (boost::wformat(L"%18s (%4d)") %SqlType2s(SqlDataType) %SqlDataType).str();
		std::wstring sPrecision = (PrecisionIsNull ? L"NULL" : (boost::wformat(L"%d") %Precision).str());
		std::wstring sLiteralPrefix = LiteralPrefixIsNull ? L"NULL" : LiteralPrefix;
		std::wstring sLiteralSuffix = LiteralSuffixIsNull ? L"NULL" : LiteralSuffix;
		std::wstring sCreateParams = CreateParamsIsNull ? L"NULL" : CreateParams;
		std::wstring sNullable = L"???";
		std::wstring sCaseSensitive = SqlTrueFalse2s(CaseSensitive);
		std::wstring sSearchable = L"???";		
		std::wstring sUnsigned = UnsignedIsNull ? L"NULL" : SqlTrueFalse2s(Unsigned);
		std::wstring sFixedPrecisionScale = SqlTrueFalse2s(FixedPrecisionScale);
		std::wstring sAutoUniqueValue = AutoUniqueValueIsNull ? L"NULL" : SqlTrueFalse2s(AutoUniqueValue);
		std::wstring sLocalTypeName = LocalTypeNameIsNull ? L"NULL" : LocalTypeName;
		std::wstring sMinimumScale = MinimumScaleIsNull ? L"NULL" : (boost::wformat(L"%d") %MinimumScale).str();
		std::wstring sMaximumScale = MaximumScaleIsNull ? L"NULL" : (boost::wformat(L"%d") %MaximumScale).str();
		std::wstring sSqlDateTimeSub = SqlDateTimeSubIsNull ? L"NULL" : (boost::wformat(L"%d") %SqlDateTimeSub).str();
		std::wstring sNumPrecRadix = NumPrecRadixIsNull ? L"NULL" : (boost::wformat(L"%d") %NumPrecRadix).str();
		std::wstring sIntervalPrecision = IntervalPrecisionIsNull ? L"NULL" : (boost::wformat(L"%d") %IntervalPrecision).str();

		switch(Nullable)
		{
		case SQL_NO_NULLS:
				sNullable = L"NO_NULLS";
				break;
			case SQL_NULLABLE:
				sNullable = L"NULLABLE";
				break;
			default:
				sNullable = L"UNKNOWN";
		}
		switch(Searchable)
		{
		case SQL_PRED_NONE:
			sSearchable = L"PRED_NONE";
			break;
		case SQL_PRED_CHAR:
			sSearchable = L"PRED_CHAR";
			break;
		case SQL_PRED_BASIC:
			sSearchable = L"PRED_BASIC";
			break;
		case SQL_SEARCHABLE:
			sSearchable = L"SEARCHABLE";
			break;
		}

		std::wstring s =       (boost::wformat(L"| %25s | %2s | %34s | %45s | %5s | %10s | %8s | %6s | %6s | %10s | %10s | %10s | %5s | %5s | %5s | %5s | %5s | %5s | %34s |") %sFSqlType %sSqlDataType %TypeName %sLocalTypeName %sUnsigned %sPrecision %sNullable %sAutoUniqueValue %sCaseSensitive %sSearchable %sLiteralPrefix %sLiteralSuffix %sFixedPrecisionScale %sMinimumScale %sMinimumScale %sSqlDateTimeSub %sNumPrecRadix %sIntervalPrecision %sCreateParams).str();

		ws << s;

		if(withEndLine)
		{
			ws << std::endl;
			ws << (boost::wformat(L"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------")).str() << std::endl;
		}

		return ws.str();
	}

	std::wstring SSqlTypeInfo::ToStr() const
	{
		std::wstringstream ws;
		std::wstring sNull = L"NULL";

		ws << L"***** DATA TYPE INFORMATION *****" << std::endl;
		ws << L" Name:                   " << TypeName << std::endl;
		ws << L"  SQL Type:              " << FsqlType << std::endl;
		ws << L"  Precision:             " << (PrecisionIsNull ? L"NULL" : (boost::wformat(L"%d") %Precision).str()) << std::endl;
		ws << L"  Literal Prefix:        " << (LiteralPrefixIsNull ? L"NULL" : LiteralPrefix) << std::endl;
		ws << L"  Literal Suffix:        " << (LiteralSuffixIsNull ? L"NULL" : LiteralSuffix) << std::endl;
		ws << L"  Create Params:         " << (CreateParamsIsNull ? L"NULL" : CreateParams) << std::endl;
		
		ws << L"  Nullable:              ";
		switch(Nullable)
		{
		case SQL_NO_NULLS:
			ws << L"SQL_NO_NULL";
			break;
		case SQL_NULLABLE:
			ws << L"SQL_NULLABLE";
			break;
		case SQL_NULLABLE_UNKNOWN:
			ws << L"SQL_NULLABLE_UNKNOWN";
			break;
		default:
			ws << L"???";
			break;
		}
		ws << std::endl;
		
		ws << L"  Case Sensitive:        " << (CaseSensitive == SQL_TRUE ? L"SQL_TRUE" : L"SQL_FALSE") << std::endl;
		ws << L"  Searchable:            ";
		switch(Searchable)
		{
		case SQL_PRED_NONE:
			ws << L"SQL_PRED_NONE";
			break;
		case SQL_PRED_CHAR:
			ws << L"SQL_PRED_CHAR";
			break;
		case SQL_PRED_BASIC:
			ws << L"SQL_PRED_BASIC";
			break;
		case SQL_SEARCHABLE:
			ws << L"SQL_SEARCHABLE";
			break;
		default:
			ws << L"???";
			break;
		}
		ws << std::endl;

		ws << L"  Unsigned:              " << (UnsignedIsNull ? L"NULL" : (Unsigned == SQL_TRUE ? L"SQL_TRUE" : L"SQL_FALSE")) << std::endl;
		ws << L"  Fixed Precision Scale: " << (FixedPrecisionScale == SQL_TRUE ? L"SQL_TRUE" : L"SQL_FALSE") << std::endl;
		ws << L"  Auto Unique Value:     " << (AutoUniqueValueIsNull ? L"NULL" : (AutoUniqueValue == SQL_TRUE ? L"SQL_TRUE" : L"SQL_FALSE")) << std::endl;
		ws << L"  Local Type Name:       " << (LocalTypeNameIsNull ? L"NULL" : LocalTypeName) << std::endl;
		ws << L"  Minimum Scale:         " << (MinimumScaleIsNull ? L"NULL" : (boost::wformat(L"%d") %MinimumScale).str()) << std::endl;
		ws << L"  Maximum Scale:         " << (MaximumScaleIsNull ? L"NULL" : (boost::wformat(L"%d") %MaximumScale).str()) << std::endl;
		ws << L"  SQL Data type:         " << SqlDataType << std::endl; 
		ws << L"  SQL Date Time Sub:     " << (SqlDateTimeSubIsNull ? L"NULL" : (boost::wformat(L"%d") %SqlDateTimeSub).str()) << std::endl;
		ws << L"  Num Prec Radix:        " << (NumPrecRadixIsNull ? L"NULL" : (boost::wformat(L"%d") %NumPrecRadix).str()) << std::endl;
		ws << L"  Interval Precision:    " << (IntervalPrecisionIsNull ? L"NULL" : (boost::wformat(L"%d") %IntervalPrecision).str()) << std::endl;
		return ws.str();
	}

}