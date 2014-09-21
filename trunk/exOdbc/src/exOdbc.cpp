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
		m_dbmsName[0] = 0;
		m_dbmsVer[0] = 0;
		m_driverName[0] = 0;
		m_odbcVer[0] = 0;
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
		searchPatternEscape[0] = 0;
	}

	std::wstring SDbInfo::ToStr() const
	{
		std::wstringstream ws;
		ws << L"***** DATA SOURCE INFORMATION *****" << std::endl;
		ws << L" SERVER Name: " << serverName << std::endl;
		ws << L"  DBMS Name: " << m_dbmsName << L"; DBMS Version: " << m_dbmsVer << std::endl;
		ws << L"  ODBC Version: " << m_odbcVer << L"; Driver Version: " << driverVer << std::endl;

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

	SQLUSMALLINT SDbInfo::GetMaxColumnNameLen() const
	{
		return m_maxColumnNameLen != 0 ? m_maxColumnNameLen : DB_MAX_COLUMN_NAME_LEN_DEFAULT;
	}

	SSqlTypeInfo::SSqlTypeInfo()
	{
		m_sqlType = 0;
		m_columnSize = 0;
		m_columnSizeIsNull = false;
		m_literalPrefixIsNull = false;
		m_literalSuffixIsNull = false;
		m_createParamsIsNull = false;
		m_nullable = SQL_NULLABLE_UNKNOWN;
		m_caseSensitive = SQL_FALSE;
		m_searchable = SQL_PRED_NONE;
		m_unsigned = SQL_FALSE;
		m_unsignedIsNull = false;
		m_fixedPrecisionScale = SQL_FALSE;
		m_autoUniqueValue = SQL_FALSE;
		m_autoUniqueValueIsNull = false;
		m_localTypeNameIsNull = false;
		m_minimumScale = 0;
		m_minimumScaleIsNull = false;
		m_maximumScale = 0;
		m_maximumScaleIsNull = false;
		m_sqlDataType = 0;
		m_sqlDateTimeSub = 0;
		m_sqlDateTimeSubIsNull = false;
		m_numPrecRadix = 0;
		m_numPrecRadixIsNull = false;
		m_intervalPrecision = 0;
		m_intervalPrecisionIsNull = false;
	}

	STableColumnInfo::STableColumnInfo()
		: m_sqlType(0)
		, m_columnSize(0)
		, m_bufferSize(0)
		, m_decimalDigits(0)
		, m_numPrecRadix(0)
		, m_nullable(SQL_NULLABLE_UNKNOWN)
		, m_sqlDataType(0)
		, m_sqlDatetimeSub(0)
		, m_charOctetLength(0)
		, m_ordinalPosition(0)
		, m_isCatalogNull(false)
		, m_isSchemaNull(false)
		, m_isColumnSizeNull(false)
		, m_isBufferSizeNull(false)
		, m_isDecimalDigitsNull(false)
		, m_isNumPrecRadixNull(false)
		, m_isRemarksNull(false)
		, m_isDefaultValueNull(false)
		, m_isDatetimeSubNull(false)
		, m_isCharOctetLengthNull(false)
		, m_isIsNullableNull(false)
	{ }

	STableInfo::STableInfo()
		: m_isCatalogNull(false)
		, m_isSchemaNull(false)
	{

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

		std::wstring sFSqlType = (boost::wformat(L"%18s (%4d)") %SqlType2s(m_sqlType) %m_sqlType).str();
		std::wstring sSqlDataType = (boost::wformat(L"%18s (%4d)") %SqlType2s(m_sqlDataType) %m_sqlDataType).str();
		std::wstring sPrecision = (m_columnSizeIsNull ? L"NULL" : (boost::wformat(L"%d") %m_columnSize).str());
		std::wstring sLiteralPrefix = m_literalPrefixIsNull ? L"NULL" : m_literalPrefix;
		std::wstring sLiteralSuffix = m_literalSuffixIsNull ? L"NULL" : m_literalSuffix;
		std::wstring sCreateParams = m_createParamsIsNull ? L"NULL" : m_createParams;
		std::wstring sNullable = L"???";
		std::wstring sCaseSensitive = SqlTrueFalse2s(m_caseSensitive);
		std::wstring sSearchable = L"???";		
		std::wstring sUnsigned = m_unsignedIsNull ? L"NULL" : SqlTrueFalse2s(m_unsigned);
		std::wstring sFixedPrecisionScale = SqlTrueFalse2s(m_fixedPrecisionScale);
		std::wstring sAutoUniqueValue = m_autoUniqueValueIsNull ? L"NULL" : SqlTrueFalse2s(m_autoUniqueValue);
		std::wstring sLocalTypeName = m_localTypeNameIsNull ? L"NULL" : m_localTypeName;
		std::wstring sMinimumScale = m_minimumScaleIsNull ? L"NULL" : (boost::wformat(L"%d") %m_minimumScale).str();
		std::wstring sMaximumScale = m_maximumScaleIsNull ? L"NULL" : (boost::wformat(L"%d") %m_maximumScale).str();
		std::wstring sSqlDateTimeSub = m_sqlDateTimeSubIsNull ? L"NULL" : (boost::wformat(L"%d") %m_sqlDateTimeSub).str();
		std::wstring sNumPrecRadix = m_numPrecRadixIsNull ? L"NULL" : (boost::wformat(L"%d") %m_numPrecRadix).str();
		std::wstring sIntervalPrecision = m_intervalPrecisionIsNull ? L"NULL" : (boost::wformat(L"%d") %m_intervalPrecision).str();

		switch(m_nullable)
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
		switch(m_searchable)
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

		std::wstring s =       (boost::wformat(L"| %25s | %2s | %34s | %45s | %5s | %10s | %8s | %6s | %6s | %10s | %10s | %10s | %5s | %5s | %5s | %5s | %5s | %5s | %34s |") %sFSqlType %sSqlDataType %m_typeName %sLocalTypeName %sUnsigned %sPrecision %sNullable %sAutoUniqueValue %sCaseSensitive %sSearchable %sLiteralPrefix %sLiteralSuffix %sFixedPrecisionScale %sMinimumScale %sMinimumScale %sSqlDateTimeSub %sNumPrecRadix %sIntervalPrecision %sCreateParams).str();

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
		ws << L" Name:                   " << m_typeName << std::endl;
		ws << L"  SQL Type:              " << m_sqlType << std::endl;
		ws << L"  Precision:             " << (m_columnSizeIsNull ? L"NULL" : (boost::wformat(L"%d") %m_columnSize).str()) << std::endl;
		ws << L"  Literal Prefix:        " << (m_literalPrefixIsNull ? L"NULL" : m_literalPrefix) << std::endl;
		ws << L"  Literal Suffix:        " << (m_literalSuffixIsNull ? L"NULL" : m_literalSuffix) << std::endl;
		ws << L"  Create Params:         " << (m_createParamsIsNull ? L"NULL" : m_createParams) << std::endl;
		
		ws << L"  Nullable:              ";
		switch(m_nullable)
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
		
		ws << L"  Case Sensitive:        " << (m_caseSensitive == SQL_TRUE ? L"SQL_TRUE" : L"SQL_FALSE") << std::endl;
		ws << L"  Searchable:            ";
		switch(m_searchable)
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

		ws << L"  Unsigned:              " << (m_unsignedIsNull ? L"NULL" : (m_unsigned == SQL_TRUE ? L"SQL_TRUE" : L"SQL_FALSE")) << std::endl;
		ws << L"  Fixed Precision Scale: " << (m_fixedPrecisionScale == SQL_TRUE ? L"SQL_TRUE" : L"SQL_FALSE") << std::endl;
		ws << L"  Auto Unique Value:     " << (m_autoUniqueValueIsNull ? L"NULL" : (m_autoUniqueValue == SQL_TRUE ? L"SQL_TRUE" : L"SQL_FALSE")) << std::endl;
		ws << L"  Local Type Name:       " << (m_localTypeNameIsNull ? L"NULL" : m_localTypeName) << std::endl;
		ws << L"  Minimum Scale:         " << (m_minimumScaleIsNull ? L"NULL" : (boost::wformat(L"%d") %m_minimumScale).str()) << std::endl;
		ws << L"  Maximum Scale:         " << (m_maximumScaleIsNull ? L"NULL" : (boost::wformat(L"%d") %m_maximumScale).str()) << std::endl;
		ws << L"  SQL Data type:         " << m_sqlDataType << std::endl; 
		ws << L"  SQL Date Time Sub:     " << (m_sqlDateTimeSubIsNull ? L"NULL" : (boost::wformat(L"%d") %m_sqlDateTimeSub).str()) << std::endl;
		ws << L"  Num Prec Radix:        " << (m_numPrecRadixIsNull ? L"NULL" : (boost::wformat(L"%d") %m_numPrecRadix).str()) << std::endl;
		ws << L"  Interval Precision:    " << (m_intervalPrecisionIsNull ? L"NULL" : (boost::wformat(L"%d") %m_intervalPrecision).str()) << std::endl;
		return ws.str();
	}

}