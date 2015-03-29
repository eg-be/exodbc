/*!
* \file exOdbc.cpp
* \author Elias Gerber <eg@zame.ch>
* \date 23.07.2014
* \brief Source file for the stuff from exOdbc.h
* \copyright wxWindows Library Licence, Version 3.1
*/ 

#include "stdafx.h"

// Own header
#include "exOdbc.h"

// Same component headers
#include "Helpers.h"

// Other headers

// Debug
#include "DebugNew.h"

namespace exodbc {

	// Static consts
	// -------------
	const SQLWCHAR* emptyString				= L"";
	const SQLWCHAR* SQL_LOG_FILENAME = L"sqllog.txt";
	const SQLWCHAR* SQL_CATALOG_FILENAME = L"catalog.txt";

	// Implementation
	// --------------
	SDbInfo::SDbInfo()
	{
		m_maxConnections = 0;
		m_maxStmts = 0;
		m_cliConfLvl = 0;
		m_cursorCommitBehavior = 0;
		m_cursorRollbackBehavior = 0;
		m_supportNotNullClause = 0;
		m_txnIsolation = 0;
		m_txnIsolationOptions = 0;
		m_posOperations = 0;
		m_posStmts = 0;
		m_scrollOptions = 0;
		m_txnCapable = 0;
		m_maxCatalogNameLen = 0;
		m_maxSchemaNameLen = 0;
		m_maxTableNameLen = 0;
	}

	std::wstring SDbInfo::ToStr() const
	{
		std::wstringstream ws;
		ws << L"***** DATA SOURCE INFORMATION *****" << std::endl;
		ws << L" SERVER Name: " << m_serverName << std::endl;
		ws << L"  DBMS Name: " << m_dbmsName << L"; DBMS Version: " << m_dbmsVer << std::endl;
		ws << L"  ODBC Version: " << m_odbcVer << L"; Driver Version: " << m_driverVer << std::endl;

		ws << L"  SAG CLI Conf. Level: ";
		switch(m_cliConfLvl)
		{
		case SQL_OSCC_NOT_COMPLIANT:    ws << L"Not Compliant";    break;
		case SQL_OSCC_COMPLIANT:        ws << L"Compliant";        break;
		}
		ws << std::endl;

		ws << L"  Max. Connections: "       << m_maxConnections   << std::endl;
		ws << L"  Outer Joins: "            << m_outerJoins       << std::endl;
		ws << L"  Support for Procedures: " << m_procedureSupport << std::endl;
		ws << L"  All tables accessible : " << m_accessibleTables << std::endl;
		ws << L"  Cursor COMMIT Behavior: ";
		switch(m_cursorCommitBehavior)
		{
		case SQL_CB_DELETE:        ws << L"Delete cursors";      break;
		case SQL_CB_CLOSE:         ws << L"Close cursors";       break;
		case SQL_CB_PRESERVE:      ws << L"Preserve cursors";    break;
		}
		ws << std::endl;

		ws << L"  Cursor ROLLBACK Behavior: ";
		switch(m_cursorRollbackBehavior)
		{
		case SQL_CB_DELETE:      ws << L"Delete cursors";      break;
		case SQL_CB_CLOSE:       ws << L"Close cursors";       break;
		case SQL_CB_PRESERVE:    ws << L"Preserve cursors";    break;
		}
		ws << std::endl;

		ws << L"  Support NOT NULL clause: ";
		switch(m_supportNotNullClause)
		{
		case SQL_NNC_NULL:        ws << L"No";        break;
		case SQL_NNC_NON_NULL:    ws << L"Yes";       break;
		}
		ws << std::endl;

		ws << L"  Support IEF (Ref. Integrity): " << m_supportIEF   << std::endl;

		ws << L"  Default Transaction Isolation: ";
		switch(m_txnIsolation)
		{
		case SQL_TXN_READ_UNCOMMITTED:  ws << L"Read Uncommitted";    break;
		case SQL_TXN_READ_COMMITTED:    ws << L"Read Committed";      break;
		case SQL_TXN_REPEATABLE_READ:   ws << L"Repeatable Read";     break;
		case SQL_TXN_SERIALIZABLE:      ws << L"Serializable";        break;
		}
		ws << std::endl;

		ws << L"  Transaction Isolation Options: ";
		if (m_txnIsolationOptions & SQL_TXN_READ_UNCOMMITTED)
			ws << L"Read Uncommitted, ";
		if (m_txnIsolationOptions & SQL_TXN_READ_COMMITTED)
			ws << L"Read Committed, ";
		if (m_txnIsolationOptions & SQL_TXN_REPEATABLE_READ)
			ws << L"Repeatable Read, ";
		if (m_txnIsolationOptions & SQL_TXN_SERIALIZABLE)
			ws << L"Serializable, ";
		ws << std::endl;

		ws << L"  Position Operations Supported (SQLSetPos): ";
		if (m_posOperations & SQL_POS_POSITION)
			ws << L"Position, ";
		if (m_posOperations & SQL_POS_REFRESH)
			ws << L"Refresh, ";
		if (m_posOperations & SQL_POS_UPDATE)
			ws << L"Upd, ";
		if (m_posOperations & SQL_POS_DELETE)
			ws << L"Del, ";
		if (m_posOperations & SQL_POS_ADD)
			ws << L"Add";
		ws << std::endl;

		ws << L"  Positioned Statements Supported: ";
		if (m_posStmts & SQL_PS_POSITIONED_DELETE)
			ws << L"Pos delete, ";
		if (m_posStmts & SQL_PS_POSITIONED_UPDATE)
			ws << L"Pos update, ";
		if (m_posStmts & SQL_PS_SELECT_FOR_UPDATE)
			ws << L"Select for update";
		ws << std::endl;

		ws << L"  Scroll Options: ";
		if (m_scrollOptions & SQL_SO_FORWARD_ONLY)
			ws << L"Fwd Only, ";
		if (m_scrollOptions & SQL_SO_STATIC)
			ws << L"Static, ";
		if (m_scrollOptions & SQL_SO_KEYSET_DRIVEN)
			ws << L"Keyset Driven, ";
		if (m_scrollOptions & SQL_SO_DYNAMIC)
			ws << L"Dynamic, ";
		if (m_scrollOptions & SQL_SO_MIXED)
			ws << L"Mixed";
		ws << std::endl;

		ws << L"  Transaction Capable?: ";
		switch(m_txnCapable)
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
		return m_maxCatalogNameLen != 0 ? m_maxCatalogNameLen : DB_MAX_CATALOG_NAME_LEN_DEFAULT;
	}

	SQLUSMALLINT SDbInfo::GetMaxSchemaNameLen() const
	{
		return m_maxSchemaNameLen != 0 ? m_maxSchemaNameLen : DB_MAX_SCHEMA_NAME_LEN_DEFAULT;
	}

	SQLUSMALLINT SDbInfo::GetMaxTableNameLen() const
	{
		return m_maxTableNameLen != 0 ? m_maxTableNameLen : DB_MAX_TABLE_NAME_LEN_DEFAULT;
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

	SColumnInfo::SColumnInfo()
		: m_sqlType(SQL_UNKNOWN_TYPE)
		, m_columnSize(0)
		, m_bufferSize(0)
		, m_decimalDigits(0)
		, m_numPrecRadix(0)
		, m_nullable(SQL_NULLABLE_UNKNOWN)
		, m_sqlDataType(SQL_UNKNOWN_TYPE)
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
		, m_hasSpecialSqlQueryName(false)
	{

	}


	std::wstring STableInfo::GetSqlName(int flags /* = QNF_CATALOG | QNF_SCHEMA | QNF_TABLE */) const
	{
		if (m_hasSpecialSqlQueryName)
		{
			return m_specialSqlQueryName;
		}

		exASSERT(!m_tableName.empty());

		std::wstringstream ws;
		if (flags & QNF_CATALOG && HasCatalog())
		{
			ws << m_catalogName << L"."; 
		}
		if (flags & QNF_SCHEMA && HasSchema())
		{
			ws << m_schemaName << L".";
		}
		if (flags & QNF_TABLE)
		{
			ws << m_tableName << L".";
		}

		std::wstring str = ws.str();
		boost::erase_last(str, L".");
		return str;
	}


	std::wstring SColumnInfo::GetSqlName(QueryNameFlags flags /* = QNF_TABLE | QNF_COLUMN */) const
	{
		exASSERT(!m_tableName.empty());
		exASSERT(!m_columnName.empty());

		std::wstringstream ws;
		if (flags & QNF_CATALOG && HasCatalog())
		{
			ws << m_catalogName << L".";
		}
		if (flags & QNF_SCHEMA && HasSchema())
		{
			ws << m_schemaName << L".";
		}
		if (flags & QNF_TABLE)
		{
			ws << m_tableName << L".";
		}
		if (flags & QNF_COLUMN)
		{
			ws << m_columnName << L".";
		}

		std::wstring str = ws.str();
		boost::erase_last(str, L".");
		return str;
	}



	std::wstring STablePrimaryKeyInfo::GetSqlName(QueryNameFlags flags /* = QNF_TABLE | QNF_COLUMN */) const
	{
		exASSERT(!m_tableName.empty());

		std::wstringstream ws;
		if (flags & QNF_CATALOG && !m_isCatalogNull)
		{
			ws << m_catalogName << L".";
		}
		if (flags & QNF_SCHEMA && !m_isSchemaNull)
		{
			ws << m_schemaName << L".";
		}
		if (flags & QNF_TABLE)
		{
			ws << m_tableName << L".";
		}
		if (flags & QNF_COLUMN)
		{
			ws << m_columnName << L".";
		}

		std::wstring str = ws.str();
		boost::erase_last(str, L".");
		return str;
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