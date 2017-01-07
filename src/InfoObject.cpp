/*!
* \file InfoObject.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 14.06.2015
* \brief Source file for info objects.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "InfoObject.h"

// Same component headers
#include "Exception.h"
#include "Helpers.h"

// Other headers
// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	// Class TableInfo
	// ===============

	TableInfo::TableInfo()
		: m_isCatalogNull(false)
		, m_isSchemaNull(false)
		, m_dbms(DatabaseProduct::UNKNOWN)
	{ }


	TableInfo::TableInfo(const std::string& tableName, const std::string& tableType, const std::string& tableRemarks, const std::string& catalogName, const std::string schemaName, DatabaseProduct dbms /* = DatabaseProduct::UNKNOWN */)
		: m_tableName(tableName)
		, m_tableType(tableType)
		, m_tableRemarks(tableRemarks)
		, m_catalogName(catalogName)
		, m_schemaName(schemaName)
		, m_dbms(dbms)
		, m_isCatalogNull(catalogName.empty())
		, m_isSchemaNull(schemaName.empty())
	{}


	TableInfo::TableInfo(const std::string& tableName, const std::string& tableType, const std::string& tableRemarks, const std::string& catalogName, const std::string schemaName, bool isCatalogNull, bool isSchemaNull, DatabaseProduct dbms /* = DatabaseProduct::UNKNOWN */)
		: m_tableName(tableName)
		, m_tableType(tableType)
		, m_tableRemarks(tableRemarks)
		, m_catalogName(catalogName)
		, m_schemaName(schemaName)
		, m_dbms(dbms)
		, m_isCatalogNull(isCatalogNull)
		, m_isSchemaNull(isSchemaNull)
	{}


	std::string TableInfo::GetQueryName() const
	{
		exASSERT( ! m_tableName.empty());

		switch (m_dbms)
		{
		case DatabaseProduct::ACCESS:
			// For access, return only the pure table name.
			return m_tableName;
			break;
		case DatabaseProduct::EXCEL:
			// For excel, add '[' and ']' around the pure table name.
			return u8"[" + m_tableName + u8"]";
			break;
		}

		// As default, include everything we have
		std::stringstream ss;
		if (HasCatalog())
		{
			ss << m_catalogName << u8".";
		}
		if (HasSchema())
		{
			ss << m_schemaName << u8".";
		}
		ss << m_tableName;

		std::string str = ss.str();
		return str;
	}


	std::string TableInfo::GetPureName() const
	{
		exASSERT(! m_tableName.empty());

		return m_tableName;
	}


	bool TableInfo::operator==(const TableInfo& other) const noexcept
	{
		return m_tableName == other.m_tableName
			&& m_schemaName == other.m_schemaName
			&& m_catalogName == other.m_catalogName
			&& m_tableType == other.m_tableType
			&& m_tableRemarks == other.m_tableRemarks;
	}


	bool TableInfo::operator!=(const TableInfo& other) const noexcept
	{
		return !(*this == other);
	}


	// Class ColumnInfo
	// ================

	ColumnInfo::ColumnInfo()
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
		, m_isSqlDatetimeSubNull(false)
		, m_isCharOctetLengthNull(false)
		, m_isIsNullableNull(false)
	{ }


	ColumnInfo::ColumnInfo(const std::string& catalogName, const std::string& schemaName, const std::string& tableName, const std::string& columnName, SQLSMALLINT sqlType, 
		const std::string& typeName, SQLINTEGER columnSize, SQLINTEGER bufferSize, SQLSMALLINT decimalDigits, SQLSMALLINT numPrecRadix, SQLSMALLINT nullable, 
		const std::string& remarks, const std::string& defaultValue, SQLSMALLINT sqlDataType, SQLSMALLINT sqlDatetimeSub, SQLINTEGER charOctetLength, SQLINTEGER ordinalPosition, 
		const std::string& isNullable, bool isCatalogNull, bool isSchemaNull, bool isColumnSizeNull, bool isBufferSizeNull, bool isDecimalDigitsNull, bool isNumPrecRadixNull, 
		bool isRemarksNull, bool isDefaultValueNull, bool isSqlDatetimeSubNull, bool isIsNullableNull)
		: m_catalogName(catalogName)
		, m_schemaName(schemaName)
		, m_tableName(tableName)
		, m_columnName(columnName)
		, m_sqlType(sqlType)
		, m_typeName(typeName)
		, m_columnSize(columnSize)
		, m_bufferSize(bufferSize)
		, m_decimalDigits(decimalDigits)
		, m_numPrecRadix(numPrecRadix)
		, m_nullable(nullable)
		, m_remarks(remarks)
		, m_defaultValue(defaultValue)
		, m_sqlDataType(sqlDataType)
		, m_sqlDatetimeSub(sqlDatetimeSub)
		, m_charOctetLength(charOctetLength)
		, m_ordinalPosition(ordinalPosition)
		, m_isNullable(isNullable)
		, m_isCatalogNull(isCatalogNull)
		, m_isSchemaNull(isSchemaNull)
		, m_isColumnSizeNull(isColumnSizeNull)
		, m_isBufferSizeNull(isBufferSizeNull)
		, m_isDecimalDigitsNull(isDecimalDigitsNull)
		, m_isNumPrecRadixNull(isNumPrecRadixNull)
		, m_isRemarksNull(isRemarksNull)
		, m_isDefaultValueNull(isDefaultValueNull)
		, m_isSqlDatetimeSubNull(isSqlDatetimeSubNull)
		, m_isIsNullableNull(isIsNullableNull)
	{
		exASSERT(!m_columnName.empty());
	}

	std::string ColumnInfo::GetQueryName() const noexcept
	{
		// When querying, we use only the Column-name
		return GetPureName();
	}


	std::string ColumnInfo::GetPureName() const noexcept
	{
		return m_columnName;
	}


	// Class TablePrimaryKeyInfo
	// =========================
	TablePrimaryKeyInfo::TablePrimaryKeyInfo()
		: m_keySequence(0)
		, m_isPrimaryKeyNameNull(true)
		, m_isCatalogNull(true)
		, m_isSchemaNull(true)
	{}


	TablePrimaryKeyInfo::TablePrimaryKeyInfo(const std::string& tableName, const std::string& columnName, SQLSMALLINT keySequence)
		: m_tableName(tableName)
		, m_columnName(columnName)
		, m_keySequence(keySequence)
		, m_isCatalogNull(true)
		, m_isSchemaNull(true)
		, m_isPrimaryKeyNameNull(true)
	{}


	TablePrimaryKeyInfo::TablePrimaryKeyInfo(const std::string& catalogName, const std::string& schemaName, const std::string& tableName, const std::string& columnName, 
		SQLSMALLINT keySequence, const std::string& keyName, bool isCatalogNull, bool isSchemaNull, bool isPrimaryKeyNameNull)
		: m_catalogName(catalogName)
		, m_schemaName(schemaName)
		, m_tableName(tableName)
		, m_columnName(columnName)
		, m_keySequence(keySequence)
		, m_primaryKeyName(keyName)
		, m_isCatalogNull(isCatalogNull)
		, m_isSchemaNull(isSchemaNull)
		, m_isPrimaryKeyNameNull(isPrimaryKeyNameNull)
	{}


	std::string TablePrimaryKeyInfo::GetQueryName() const
	{
		return GetPureName();
	}


	std::string TablePrimaryKeyInfo::GetPureName() const
	{
		exASSERT(!m_columnName.empty());
		return m_columnName;
	}


	DatabaseInfo::DatabaseInfo()
	{
	}


	void DatabaseInfo::SetProperty(StringProperty prop, const std::string& value)
	{
		m_stringMap[prop] = value;
	}


	void DatabaseInfo::SetProperty(USmallIntProperty prop, SQLUSMALLINT value)
	{
		m_uSmallIntMap[prop] = value;
	}


	void DatabaseInfo::SetProperty(UIntProperty prop, SQLUINTEGER value)
	{
		m_uIntMap[prop] = value;
	}


	void DatabaseInfo::SetProperty(IntProperty prop, SQLINTEGER value)
	{
		m_intMap[prop] = value;
	}


	void DatabaseInfo::ReadAndStoryProperty(SqlDbcHandlePtr pHDbc, StringProperty prop)
	{
		std::string value;
		GetInfo(pHDbc, (SQLUSMALLINT)prop, value);
		SetProperty(prop, value);
	}


	void DatabaseInfo::ReadAndStoryProperty(SqlDbcHandlePtr pHDbc, USmallIntProperty prop)
	{
		SQLUSMALLINT value;
		SQLSMALLINT cb;
		GetInfo(pHDbc, (SQLUSMALLINT)prop, &value, sizeof(value), &cb);
		SetProperty(prop, value);
	}


	void DatabaseInfo::ReadAndStoryProperty(SqlDbcHandlePtr pHDbc, UIntProperty prop)
	{
		SQLUINTEGER value;
		SQLSMALLINT cb;
		GetInfo(pHDbc, (SQLUSMALLINT)prop, &value, sizeof(value), &cb);
		SetProperty(prop, value);
	}


	void DatabaseInfo::ReadAndStoryProperty(SqlDbcHandlePtr pHDbc, IntProperty prop)
	{
		SQLINTEGER value;
		SQLSMALLINT cb;
		GetInfo(pHDbc, (SQLUSMALLINT)prop, &value, sizeof(value), &cb);
		SetProperty(prop, value);
	}


	std::string DatabaseInfo::GetPropertyName(StringProperty prop) const
	{
		switch (prop)
		{
		case StringProperty::ServerName:
			return u8"SQL_SERVER_NAME";
		case StringProperty::DatabaseName:
			return u8"SQL_DATABASE_NAME";
		case StringProperty::DbmsName:
			return u8"SQL_DBMS_NAME";
		case StringProperty::DbmsVersion:
			return u8"SQL_DBMS_VER";
		case StringProperty::DriverName:
			return u8"SQL_DRIVER_NAME";
		case StringProperty::DriverOdbcVersion:
			return u8"SQL_DRIVER_ODBC_VER";
		case StringProperty::DriverVersion:
			return u8"SQL_DRIVER_VER";
		case StringProperty::OdbcSupportIEF:
			return u8"SQL_ODBC_SQL_OPT_IEF";
		case StringProperty::OdbcVersion:
			return u8"SQL_ODBC_VER";
		case StringProperty::OuterJoins:
			return u8"SQL_OUTER_JOINS";
		case StringProperty::ProcedureSupport:
			return u8"SQL_PROCEDURES";
		case StringProperty::AccessibleTables:
			return u8"SQL_ACCESSIBLE_TABLES";
		case StringProperty::SearchPatternEscape:
			return u8"SQL_SEARCH_PATTERN_ESCAPE";
		default:
			return u8"???";
		}
	}


	std::string DatabaseInfo::GetPropertyName(USmallIntProperty prop) const
	{
		switch (prop)
		{
		case USmallIntProperty::MaxConnections:
			return u8"SQL_MAX_DRIVER_CONNECTIONS";
		case USmallIntProperty::MaxConcurrentActivs:
			return u8"SQL_MAX_CONCURRENT_ACTIVITIES";
		case USmallIntProperty::OdbcSagCliConformance:
			return u8"SQL_ODBC_SAG_CLI_CONFORMANCE";
		case USmallIntProperty::CursorCommitBehavior:
			return u8"SQL_CURSOR_COMMIT_BEHAVIOR";
		case USmallIntProperty::CursorRollbackBehavior:
			return u8"SQL_CURSOR_ROLLBACK_BEHAVIOR";
		case USmallIntProperty::NonNullableColumns:
			return u8"SQL_NON_NULLABLE_COLUMNS";
		case USmallIntProperty::TxnCapable:
			return u8"SQL_TXN_CAPABLE";
		case USmallIntProperty::MaxCatalogNameLen:
			return u8"SQL_MAX_CATALOG_NAME_LEN";
		case USmallIntProperty::MaxSchemaNameLen:
			return u8"SQL_MAX_SCHEMA_NAME_LEN";
		case USmallIntProperty::MaxTableNameLen:
			return u8"SQL_MAX_TABLE_NAME_LEN";
		case USmallIntProperty::MaxColumnNameLen:
			return u8"SQL_MAX_COLUMN_NAME_LEN";
		default:
			return u8"???";
		}
	}


	std::string DatabaseInfo::GetPropertyName(UIntProperty prop) const
	{
		switch (prop)
		{
		case UIntProperty::DefaultTxnIsolation:
			return u8"SQL_DEFAULT_TXN_ISOLATION";
		case UIntProperty::TxnIsolationOption:
			return u8"SQL_TXN_ISOLATION_OPTION";
		case UIntProperty::ScrollOptions:
			return u8"SQL_SCROLL_OPTIONS";
		case UIntProperty::CursorSensitity:
			return u8"SQL_CURSOR_SENSITIVITY";
		case UIntProperty::DynamicCursorAttributes1:
			return u8"SQL_DYNAMIC_CURSOR_ATTRIBUTES1";
		case UIntProperty::ForwardOnlyCursorAttributes1:
			return u8"SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1";
		case UIntProperty::KeysetCursorAttributes1:
			return u8"SQL_KEYSET_CURSOR_ATTRIBUTES1";
		case UIntProperty::StaticCursorAttributes1:
			return u8"SQL_STATIC_CURSOR_ATTRIBUTES1";
		case UIntProperty::KeysetCursorAttributes2:
			return u8"SQL_KEYSET_CURSOR_ATTRIBUTES2";
		case UIntProperty::StaticCursorAttributes2:
			return u8"SQL_STATIC_CURSOR_ATTRIBUTES2";

		default:
			return u8"???";
		}
	}


	std::string DatabaseInfo::GetPropertyName(IntProperty prop) const
	{
		switch (prop)
		{
		case IntProperty::PosOperations:
			return u8"SQL_POS_OPERATIONS";
		case IntProperty::PositionedStatements:
			return u8"SQL_POSITIONED_STATEMENTS";
		default:
			return u8"???";
		}
	}


	std::string DatabaseInfo::GetStringProperty(StringProperty prop) const
	{
		StringMap::const_iterator it = m_stringMap.find(prop);
		if (it == m_stringMap.end())
		{
			NotFoundException nfe(boost::str(boost::format(u8"WString Property '%s' (%d) not found") % GetPropertyName(prop) % (SQLUSMALLINT)prop));
			SET_EXCEPTION_SOURCE(nfe);
			throw nfe;
		}
		return it->second;
	}


	SQLUSMALLINT DatabaseInfo::GetUSmallIntProperty(USmallIntProperty prop) const
	{
		USmallIntMap::const_iterator it = m_uSmallIntMap.find(prop);
		if (it == m_uSmallIntMap.end())
		{
			NotFoundException nfe(boost::str(boost::format(u8"USmallInt Property '%s' (%d) not found") % GetPropertyName(prop) % (SQLUSMALLINT)prop));
			SET_EXCEPTION_SOURCE(nfe);
			throw nfe;
		}
		return it->second;
	}


	SQLUINTEGER DatabaseInfo::GetUIntProperty(UIntProperty prop) const
	{
		UIntMap::const_iterator it = m_uIntMap.find(prop);
		if (it == m_uIntMap.end())
		{
			NotFoundException nfe(boost::str(boost::format(u8"UInt Property '%s' (%d) not found") % GetPropertyName(prop) % (SQLUSMALLINT)prop));
			SET_EXCEPTION_SOURCE(nfe);
			throw nfe;
		}
		return it->second;
	}


	SQLINTEGER DatabaseInfo::GetIntProperty(IntProperty prop) const
	{
		IntMap::const_iterator it = m_intMap.find(prop);
		if (it == m_intMap.end())
		{
			NotFoundException nfe(boost::str(boost::format(u8"Int Property '%s' (%d) not found") % GetPropertyName(prop) % (SQLUSMALLINT)prop));
			SET_EXCEPTION_SOURCE(nfe);
			throw nfe;
		}
		return it->second;
	}


	bool DatabaseInfo::GetSupportsTransactions() const
	{
		SQLUSMALLINT value = GetUSmallIntProperty(USmallIntProperty::TxnCapable);
		return value != SQL_TC_NONE;
	}


	bool DatabaseInfo::GetForwardOnlyCursors() const
	{
		SQLUINTEGER value = GetUIntProperty(UIntProperty::ScrollOptions);
		value &= ~SQL_SO_FORWARD_ONLY;
		return value == 0;
	}


	std::string DatabaseInfo::GetDriverName() const
	{
		return GetStringProperty(StringProperty::DriverName);
	}


	std::string DatabaseInfo::GetDriverOdbcVersion() const
	{
		return GetStringProperty(StringProperty::DriverOdbcVersion);
	}


	std::string DatabaseInfo::GetDriverVersion() const
	{
		return GetStringProperty(StringProperty::DriverVersion);
	}


	std::string DatabaseInfo::GetDbmsName() const
	{
		return GetStringProperty(StringProperty::DbmsName);
	}


	std::string DatabaseInfo::ToString() const
	{
		std::stringstream ss;
		ss << std::endl;
		ss << u8"***** DATA SOURCE INFORMATION *****" << std::endl;
		for (StringMap::const_iterator it = m_stringMap.begin(); it != m_stringMap.end(); ++it)
		{
			string tmp = boost::str(boost::format(u8"%-38s : '%s'") % GetPropertyName(it->first) % it->second);
			ss << tmp << std::endl;
		}
		ss << std::endl;
		for (USmallIntMap::const_iterator it = m_uSmallIntMap.begin(); it != m_uSmallIntMap.end(); ++it)
		{
			string tmp = boost::str(boost::format(u8"%-38s : %#8x (%8d)") % GetPropertyName(it->first) % it->second % it->second);
			ss << tmp << std::endl;
		}
		ss << std::endl;
		for (UIntMap::const_iterator it = m_uIntMap.begin(); it != m_uIntMap.end(); ++it)
		{
			string tmp = boost::str(boost::format(u8"%-38s : %#8x (%8d)") % GetPropertyName(it->first) % it->second % it->second);
			ss << tmp << std::endl;
		}
		ss << std::endl;
		for (IntMap::const_iterator it = m_intMap.begin(); it != m_intMap.end(); ++it)
		{
			string tmp = boost::str(boost::format(u8"%-38s : %8d") % GetPropertyName(it->first) % it->second);
			ss << tmp << std::endl;
		}
		ss << std::endl;
		
		ss << u8"  SAG CLI Conf. Level: ";
		switch (GetUSmallIntProperty(USmallIntProperty::OdbcSagCliConformance))
		{
		case SQL_OSCC_NOT_COMPLIANT:    ss << u8"Not Compliant";    break;
		case SQL_OSCC_COMPLIANT:        ss << u8"Compliant";        break;
		}
		ss << std::endl;

		ss << u8"  Cursor COMMIT Behavior: ";
		switch (GetUSmallIntProperty(USmallIntProperty::CursorCommitBehavior))
		{
		case SQL_CB_DELETE:        ss << u8"Delete cursors";      break;
		case SQL_CB_CLOSE:         ss << u8"Close cursors";       break;
		case SQL_CB_PRESERVE:      ss << u8"Preserve cursors";    break;
		}
		ss << std::endl;

		ss << u8"  Cursor ROLLBACK Behavior: ";
		switch (GetUSmallIntProperty(USmallIntProperty::CursorRollbackBehavior))
		{
		case SQL_CB_DELETE:      ss << u8"Delete cursors";      break;
		case SQL_CB_CLOSE:       ss << u8"Close cursors";       break;
		case SQL_CB_PRESERVE:    ss << u8"Preserve cursors";    break;
		}
		ss << std::endl;

		ss << u8"  Support NOT NULL clause: ";
		switch (GetUSmallIntProperty(USmallIntProperty::NonNullableColumns))
		{
		case SQL_NNC_NULL:        ss << u8"No";        break;
		case SQL_NNC_NON_NULL:    ss << u8"Yes";       break;
		}
		ss << std::endl;

		ss << u8"  Default Transaction Isolation: ";
		switch (GetUIntProperty(UIntProperty::DefaultTxnIsolation))
		{
		case SQL_TXN_READ_UNCOMMITTED:  ss << u8"Read Uncommitted";    break;
		case SQL_TXN_READ_COMMITTED:    ss << u8"Read Committed";      break;
		case SQL_TXN_REPEATABLE_READ:   ss << u8"Repeatable Read";     break;
		case SQL_TXN_SERIALIZABLE:      ss << u8"Serializable";        break;
		}
		ss << std::endl;

		SQLUINTEGER txnIsolationOptions = GetUIntProperty(UIntProperty::TxnIsolationOption);
		ss << u8"  Transaction Isolation Options: ";
		if (txnIsolationOptions & SQL_TXN_READ_UNCOMMITTED)
			ss << u8"Read Uncommitted, ";
		if (txnIsolationOptions & SQL_TXN_READ_COMMITTED)
			ss << u8"Read Committed, ";
		if (txnIsolationOptions & SQL_TXN_REPEATABLE_READ)
			ss << u8"Repeatable Read, ";
		if (txnIsolationOptions & SQL_TXN_SERIALIZABLE)
			ss << u8"Serializable, ";
		ss << std::endl;

		ss << u8"  Transaction Capable?: ";
		switch (GetUSmallIntProperty(USmallIntProperty::TxnCapable))
		{
		case SQL_TC_NONE:          ss << u8"No";            break;
		case SQL_TC_DML:           ss << u8"DML Only";      break;
		case SQL_TC_DDL_COMMIT:    ss << u8"DDL Commit";    break;
		case SQL_TC_DDL_IGNORE:    ss << u8"DDL Ignore";    break;
		case SQL_TC_ALL:           ss << u8"DDL & DMu8";     break;
		}
		ss << std::endl;

		SQLINTEGER posOperations = GetIntProperty(IntProperty::PosOperations);
		ss << u8"  Position Operations Supported (SQLSetPos): ";
		if (posOperations & SQL_POS_POSITION)
			ss << u8"Position, ";
		if (posOperations & SQL_POS_REFRESH)
			ss << u8"Refresh, ";
		if (posOperations & SQL_POS_UPDATE)
			ss << u8"Upd, ";
		if (posOperations & SQL_POS_DELETE)
			ss << u8"Del, ";
		if (posOperations & SQL_POS_ADD)
			ss << u8"Add";
		ss << std::endl;

		SQLINTEGER posStmts = GetIntProperty(IntProperty::PositionedStatements);
		ss << u8"  Positioned Statements Supported: ";
		if (posStmts & SQL_PS_POSITIONED_DELETE)
			ss << u8"Pos delete, ";
		if (posStmts & SQL_PS_POSITIONED_UPDATE)
			ss << u8"Pos update, ";
		if (posStmts & SQL_PS_SELECT_FOR_UPDATE)
			ss << u8"Select for update";
		ss << std::endl;

		SQLUINTEGER scrollOptions = GetUIntProperty(UIntProperty::ScrollOptions);
		ss << u8"  Scroll Options: ";
		if (scrollOptions & SQL_SO_FORWARD_ONLY)
			ss << u8"Fwd Only, ";
		if (scrollOptions & SQL_SO_STATIC)
			ss << u8"Static, ";
		if (scrollOptions & SQL_SO_KEYSET_DRIVEN)
			ss << u8"Keyset Driven, ";
		if (scrollOptions & SQL_SO_DYNAMIC)
			ss << u8"Dynamic, ";
		if (scrollOptions & SQL_SO_MIXED)
			ss << u8"Mixed";
		ss << std::endl;

		return ss.str();
	}


	SQLUSMALLINT DatabaseInfo::GetMaxCatalogNameLen() const
	{
		SQLUSMALLINT value = GetUSmallIntProperty(USmallIntProperty::MaxCatalogNameLen);
		if (value > 0)
		{
			return value;
		}

		return DB_MAX_CATALOG_NAME_LEN_DEFAULT;
	}


	SQLUSMALLINT DatabaseInfo::GetMaxSchemaNameLen() const
	{
		SQLUSMALLINT value = GetUSmallIntProperty(USmallIntProperty::MaxSchemaNameLen);
		if (value > 0)
		{
			return value;
		}
		return DB_MAX_SCHEMA_NAME_LEN_DEFAULT;
	}


	SQLUSMALLINT DatabaseInfo::GetMaxTableNameLen() const
	{
		SQLUSMALLINT value = GetUSmallIntProperty(USmallIntProperty::MaxTableNameLen);
		if (value > 0)
		{
			return value;
		}
		return DB_MAX_TABLE_NAME_LEN_DEFAULT;
	}


	SQLUSMALLINT DatabaseInfo::GetMaxColumnNameLen() const
	{
		SQLUSMALLINT value = GetUSmallIntProperty(USmallIntProperty::MaxColumnNameLen);
		if (value > 0)
		{
			return value;
		}
		return DB_MAX_COLUMN_NAME_LEN_DEFAULT;
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


	//STableInfo::STableInfo()
	//	: m_isCatalogNull(false)
	//	, m_isSchemaNull(false)
	//	, m_queryNameHint(TableQueryNameHint::ALL)
	//{

	//}


	//std::string STableInfo::GetPureTableName() const
	//{
	//	exASSERT(!m_tableName.empty());
	//	return m_tableName;
	//}


	//std::string STableInfo::GetSqlName() const
	//{
	//	exASSERT(!m_tableName.empty());

	//	bool includeCat = true;
	//	bool includeSchem = true;
	//	bool includeName = true;
	//	switch (m_queryNameHint)
	//	{
	//	case TableQueryNameHint::ALL:
	//		includeCat = true;
	//		includeSchem = true;
	//		includeName = true;
	//		break;
	//	case TableQueryNameHint::CATALOG_TABLE:
	//		includeCat = true;
	//		includeSchem = false;
	//		includeName = true;
	//		break;
	//	case TableQueryNameHint::SCHEMA_TABLE:
	//		includeCat = false;
	//		includeSchem = true;
	//		includeName = true;
	//		break;
	//	case TableQueryNameHint::TABLE_ONLY:
	//		includeCat = false;
	//		includeSchem = false;
	//		includeName = true;
	//		break;
	//	case TableQueryNameHint::EXCEL:
	//		return u8"[" + m_tableName + u8"]";
	//	}

	//	std::stringstream ss;
	//	if (includeCat && HasCatalog())
	//	{
	//		ss << m_catalogName << u8".";
	//	}
	//	if (includeSchem && HasSchema())
	//	{
	//		ss << m_schemaName << u8".";
	//	}
	//	if (includeName)
	//	{
	//		ss << m_tableName << u8".";
	//	}

	//	std::string str = ss.str();
	//	boost::erase_last(str, u8".");
	//	return str;
	//}


	//std::string ColumnInfo::GetSqlName() const
	//{
	//	exASSERT(!m_columnName.empty());

	//	//bool includeTableName = false;
	//	//bool includeColumnName = true;

	//	////switch (m_queryNameHint)
	//	////{
	//	////case ColumnQueryNameHint::COLUMN:
	//	//includeTableName = false;
	//	//includeColumnName = true;
	//	//	break;
	//	//case ColumnQueryNameHint::TABLE_COLUMN:
	//	//	includeTableName = true;
	//	//	includeColumnName = true;
	//	//	break;
	//	//}
	//	//std::stringstream ss;

	//	//if (includeTableName)
	//	//{
	//	//	exASSERT(!m_tableName.empty());
	//	//	ss << m_tableName << u8".";
	//	//}
	//	//if (includeColumnName)
	//	//{
	//	//ss << m_columnName << u8".";
	//	//}

	//	//std::string str = ss.str();
	//	//boost::erase_last(str, u8".");
	//	return m_columnName;
	//}



	//std::string TablePrimaryKeyInfo::GetSqlName(QueryNameFlags flags /* = QNF_TABLE | QNF_COLUMN */) const
	//{
	//	exASSERT(!m_tableName.empty());

	//	std::stringstream ss;
	//	if (flags & QNF_CATALOG && !m_isCatalogNull)
	//	{
	//		ss << m_catalogName << u8".";
	//	}
	//	if (flags & QNF_SCHEMA && !m_isSchemaNull)
	//	{
	//		ss << m_schemaName << u8".";
	//	}
	//	if (flags & QNF_TABLE)
	//	{
	//		ss << m_tableName << u8".";
	//	}
	//	if (flags & QNF_COLUMN)
	//	{
	//		ss << m_columnName << u8".";
	//	}

	//	std::string str = ss.str();
	//	boost::erase_last(str, u8".");
	//	return str;
	//}


std::string SSqlTypeInfo::ToOneLineStrForTrac(bool withHeaderLine /* = false */) const
{
	std::stringstream ss;
	if (withHeaderLine)
	{
		ss << (boost::format(u8"||= %25s=||= %25s=||= %34s =||= %45s =||= %5s =||= %10s =||= %8s =||= %6s =||= %6s =||= %10s =||= %10s =||= %10s =||= %5s =||= %5s =||= %5s =||= %5s =||= %5s =||= %5s =||= %34s =||") % u8"SQLType" %u8"SQL Data Type (3)" %u8"!TypeName" %u8"Local !TypeName" %u8"Unsigned" %u8"Precision" %u8"Nullable" %u8"Auto Inc." %u8"Case Sens." %u8"Searchable" %u8"Prefix" %u8"Suffix" %u8"Fixed Prec. Scale" %u8"Min. Scale" %u8"Max. Scale" %u8"Sql DateTimeSub" %u8"Num. Prec. Radix" %u8"Interval Precision" %u8"Create Params").str() << std::endl;
	}

	std::string sFSqlType = (boost::format(u8"%18s (%4d)") % SqlType2s(m_sqlType) % m_sqlType).str();
	std::string sSqlDataType = (boost::format(u8"%18s (%4d)") % SqlType2s(m_sqlDataType) % m_sqlDataType).str();
	std::string sPrecision = (m_columnSizeIsNull ? u8"NULL" : (boost::format(u8"%d") % m_columnSize).str());
	std::string sLiteralPrefix = m_literalPrefixIsNull ? u8"NULL" : m_literalPrefix;
	std::string sLiteralSuffix = m_literalSuffixIsNull ? u8"NULL" : m_literalSuffix;
	std::string sCreateParams = m_createParamsIsNull ? u8"NULL" : m_createParams;
	std::string sNullable = u8"???";
	std::string sCaseSensitive = SqlTrueFalse2s(m_caseSensitive);
	std::string sSearchable = u8"???";
	std::string sUnsigned = m_unsignedIsNull ? u8"NULL" : SqlTrueFalse2s(m_unsigned);
	std::string sFixedPrecisionScale = SqlTrueFalse2s(m_fixedPrecisionScale);
	std::string sAutoUniqueValue = m_autoUniqueValueIsNull ? u8"NULL" : SqlTrueFalse2s(m_autoUniqueValue);
	std::string sLocalTypeName = m_localTypeNameIsNull ? u8"NULL" : m_localTypeName;
	std::string sMinimumScale = m_minimumScaleIsNull ? u8"NULL" : (boost::format(u8"%d") % m_minimumScale).str();
	std::string sMaximumScale = m_maximumScaleIsNull ? u8"NULL" : (boost::format(u8"%d") % m_maximumScale).str();
	std::string sSqlDateTimeSub = m_sqlDateTimeSubIsNull ? u8"NULL" : (boost::format(u8"%d") % m_sqlDateTimeSub).str();
	std::string sNumPrecRadix = m_numPrecRadixIsNull ? u8"NULL" : (boost::format(u8"%d") % m_numPrecRadix).str();
	std::string sIntervalPrecision = m_intervalPrecisionIsNull ? u8"NULL" : (boost::format(u8"%d") % m_intervalPrecision).str();

	switch (m_nullable)
	{
	case SQL_NO_NULLS:
		sNullable = u8"NO_NULLS";
		break;
	case SQL_NULLABLE:
		sNullable = u8"NULLABLE";
		break;
	default:
		sNullable = u8"UNKNOWN";
	}
	switch (m_searchable)
	{
	case SQL_PRED_NONE:
		sSearchable = u8"PRED_NONE";
		break;
	case SQL_PRED_CHAR:
		sSearchable = u8"PRED_CHAR";
		break;
	case SQL_PRED_BASIC:
		sSearchable = u8"PRED_BASIC";
		break;
	case SQL_SEARCHABLE:
		sSearchable = u8"SEARCHABLE";
		break;
	}

	std::string s = (boost::format(u8"|| %25s|| %2s|| %34s || %45s || %5s || %10s || %8s || %6s || %6s || %10s || %10s || %10s || %5s || %5s || %5s || %5s || %5s || %5s || %34s ||") % sFSqlType %sSqlDataType %m_typeName %sLocalTypeName %sUnsigned %sPrecision %sNullable %sAutoUniqueValue %sCaseSensitive %sSearchable %sLiteralPrefix %sLiteralSuffix %sFixedPrecisionScale %sMinimumScale %sMaximumScale %sSqlDateTimeSub %sNumPrecRadix %sIntervalPrecision %sCreateParams).str();

	ss << s;

	return ss.str();
}


	std::string SSqlTypeInfo::ToOneLineStr(bool withHeaderLines /* = false */, bool withEndLine /* = false */) const
	{
		std::stringstream ss;
		if (withHeaderLines)
		{
			ss << (boost::format(u8"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------")).str() << std::endl;
			ss << (boost::format(u8"| %25s | %25s | %34s | %45s | %5s | %10s | %8s | %6s | %6s | %10s | %10s | %10s | %5s | %5s | %5s | %5s | %5s | %5s | %34s |") % u8"SQLType" %u8"SQL Data Type (3)" %u8"TypeName" %u8"Local TypeName" %u8"Unsig" %u8"Precision" %u8"Nullable" %u8"AutoI" %u8"CaseS." %u8"Searchable" %u8"Prefix" %u8"Suffix" %u8"FixPS" %u8"MinSc" %u8"MaxSc" %u8"DTS3" %u8"NuPR" %u8"IntPr" %u8"Create Params").str() << std::endl;
			ss << (boost::format(u8"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------")).str() << std::endl;
		}

		std::string sFSqlType = (boost::format(u8"%18s (%4d)") % SqlType2s(m_sqlType) % m_sqlType).str();
		std::string sSqlDataType = (boost::format(u8"%18s (%4d)") % SqlType2s(m_sqlDataType) % m_sqlDataType).str();
		std::string sPrecision = (m_columnSizeIsNull ? u8"NULL" : (boost::format(u8"%d") % m_columnSize).str());
		std::string sLiteralPrefix = m_literalPrefixIsNull ? u8"NULL" : m_literalPrefix;
		std::string sLiteralSuffix = m_literalSuffixIsNull ? u8"NULL" : m_literalSuffix;
		std::string sCreateParams = m_createParamsIsNull ? u8"NULL" : m_createParams;
		std::string sNullable = u8"???";
		std::string sCaseSensitive = SqlTrueFalse2s(m_caseSensitive);
		std::string sSearchable = u8"???";
		std::string sUnsigned = m_unsignedIsNull ? u8"NULL" : SqlTrueFalse2s(m_unsigned);
		std::string sFixedPrecisionScale = SqlTrueFalse2s(m_fixedPrecisionScale);
		std::string sAutoUniqueValue = m_autoUniqueValueIsNull ? u8"NULL" : SqlTrueFalse2s(m_autoUniqueValue);
		std::string sLocalTypeName = m_localTypeNameIsNull ? u8"NULL" : m_localTypeName;
		std::string sMinimumScale = m_minimumScaleIsNull ? u8"NULL" : (boost::format(u8"%d") % m_minimumScale).str();
		std::string sMaximumScale = m_maximumScaleIsNull ? u8"NULL" : (boost::format(u8"%d") % m_maximumScale).str();
		std::string sSqlDateTimeSub = m_sqlDateTimeSubIsNull ? u8"NULL" : (boost::format(u8"%d") % m_sqlDateTimeSub).str();
		std::string sNumPrecRadix = m_numPrecRadixIsNull ? u8"NULL" : (boost::format(u8"%d") % m_numPrecRadix).str();
		std::string sIntervalPrecision = m_intervalPrecisionIsNull ? u8"NULL" : (boost::format(u8"%d") % m_intervalPrecision).str();

		switch (m_nullable)
		{
		case SQL_NO_NULLS:
			sNullable = u8"NO_NULLS";
			break;
		case SQL_NULLABLE:
			sNullable = u8"NULLABLE";
			break;
		default:
			sNullable = u8"UNKNOWN";
		}
		switch (m_searchable)
		{
		case SQL_PRED_NONE:
			sSearchable = u8"PRED_NONE";
			break;
		case SQL_PRED_CHAR:
			sSearchable = u8"PRED_CHAR";
			break;
		case SQL_PRED_BASIC:
			sSearchable = u8"PRED_BASIC";
			break;
		case SQL_SEARCHABLE:
			sSearchable = u8"SEARCHABLE";
			break;
		}

		std::string s = (boost::format(u8"| %25s | %2s | %34s | %45s | %5s | %10s | %8s | %6s | %6s | %10s | %10s | %10s | %5s | %5s | %5s | %5s | %5s | %5s | %34s |") % sFSqlType %sSqlDataType %m_typeName %sLocalTypeName %sUnsigned %sPrecision %sNullable %sAutoUniqueValue %sCaseSensitive %sSearchable %sLiteralPrefix %sLiteralSuffix %sFixedPrecisionScale %sMinimumScale %sMinimumScale %sSqlDateTimeSub %sNumPrecRadix %sIntervalPrecision %sCreateParams).str();

		ss << s;

		if (withEndLine)
		{
			ss << std::endl;
			ss << (boost::format(u8"----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------")).str() << std::endl;
		}

		return ss.str();
	}

	std::string SSqlTypeInfo::ToStr() const
	{
		std::stringstream ss;
		std::string sNull = u8"NULL";

		ss << u8"***** DATA TYPE INFORMATION *****" << std::endl;
		ss << u8" Name:                   " << m_typeName << std::endl;
		ss << u8"  SQL Type:              " << m_sqlType << std::endl;
		ss << u8"  Precision:             " << (m_columnSizeIsNull ? u8"NULL" : (boost::format(u8"%d") % m_columnSize).str()) << std::endl;
		ss << u8"  Literal Prefix:        " << (m_literalPrefixIsNull ? u8"NULL" : m_literalPrefix) << std::endl;
		ss << u8"  Literal Suffix:        " << (m_literalSuffixIsNull ? u8"NULL" : m_literalSuffix) << std::endl;
		ss << u8"  Create Params:         " << (m_createParamsIsNull ? u8"NULL" : m_createParams) << std::endl;

		ss << u8"  Nullable:              ";
		switch (m_nullable)
		{
		case SQL_NO_NULLS:
			ss << u8"SQL_NO_NULL";
			break;
		case SQL_NULLABLE:
			ss << u8"SQL_NULLABLE";
			break;
		case SQL_NULLABLE_UNKNOWN:
			ss << u8"SQL_NULLABLE_UNKNOWN";
			break;
		default:
			ss << u8"???";
			break;
		}
		ss << std::endl;

		ss << u8"  Case Sensitive:        " << (m_caseSensitive == SQL_TRUE ? u8"SQL_TRUE" : u8"SQL_FALSE") << std::endl;
		ss << u8"  Searchable:            ";
		switch (m_searchable)
		{
		case SQL_PRED_NONE:
			ss << u8"SQL_PRED_NONE";
			break;
		case SQL_PRED_CHAR:
			ss << u8"SQL_PRED_CHAR";
			break;
		case SQL_PRED_BASIC:
			ss << u8"SQL_PRED_BASIC";
			break;
		case SQL_SEARCHABLE:
			ss << u8"SQL_SEARCHABLE";
			break;
		default:
			ss << u8"???";
			break;
		}
		ss << std::endl;

		ss << u8"  Unsigned:              " << (m_unsignedIsNull ? u8"NULL" : (m_unsigned == SQL_TRUE ? u8"SQL_TRUE" : u8"SQL_FALSE")) << std::endl;
		ss << u8"  Fixed Precision Scale: " << (m_fixedPrecisionScale == SQL_TRUE ? u8"SQL_TRUE" : u8"SQL_FALSE") << std::endl;
		ss << u8"  Auto Unique Value:     " << (m_autoUniqueValueIsNull ? u8"NULL" : (m_autoUniqueValue == SQL_TRUE ? u8"SQL_TRUE" : u8"SQL_FALSE")) << std::endl;
		ss << u8"  Local Type Name:       " << (m_localTypeNameIsNull ? u8"NULL" : m_localTypeName) << std::endl;
		ss << u8"  Minimum Scale:         " << (m_minimumScaleIsNull ? u8"NULL" : (boost::format(u8"%d") % m_minimumScale).str()) << std::endl;
		ss << u8"  Maximum Scale:         " << (m_maximumScaleIsNull ? u8"NULL" : (boost::format(u8"%d") % m_maximumScale).str()) << std::endl;
		ss << u8"  SQL Data type:         " << m_sqlDataType << std::endl;
		ss << u8"  SQL Date Time Sub:     " << (m_sqlDateTimeSubIsNull ? u8"NULL" : (boost::format(u8"%d") % m_sqlDateTimeSub).str()) << std::endl;
		ss << u8"  Num Prec Radix:        " << (m_numPrecRadixIsNull ? u8"NULL" : (boost::format(u8"%d") % m_numPrecRadix).str()) << std::endl;
		ss << u8"  Interval Precision:    " << (m_intervalPrecisionIsNull ? u8"NULL" : (boost::format(u8"%d") % m_intervalPrecision).str()) << std::endl;
		return ss.str();
	}


	bool SSqlTypeInfo::operator<(const SSqlTypeInfo& other) const
	{
		return m_sqlDataType < other.m_sqlDataType;
	}
}
