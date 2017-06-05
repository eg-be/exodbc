/*!
* \file SqlInfoProperty.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 12.04.2017
* \brief Source file for SqlInfo.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "SqlInfoProperty.h"

// Same component headers
// Other headers
#include <sstream>

// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	// Construction
	// ------------
	SqlInfoProperty::SqlInfoProperty(SQLUSMALLINT infoId, const std::string& infoName, InfoType iType, ValueType vType)
		: m_infoId(infoId)
		, m_infoName(infoName)
		, m_infoType(iType)
		, m_valueType(vType)
		, m_valueRead(false)
	{
		switch (m_valueType)
		{
		case ValueType::USmallInt:
			m_value = (SQLUSMALLINT)0;
			break;
		case ValueType::UInt:
			m_value = (SQLUINTEGER)0;
			break;
		case ValueType::String_N_Y:
			m_value = u8"N";
			break;
		case ValueType::String_Any:
			m_value = u8"";
			break;
		default:
			exASSERT(false);
		}
	}


	// Destructor
	// -----------


	// Implementation
	// --------------
	void SqlInfoProperty::ReadProperty(ConstSqlDbcHandlePtr pHdbc)
	{
		SQLSMALLINT cb = 0;
		if (m_valueType == ValueType::USmallInt)
		{
			SQLUSMALLINT v;
			ReadInfoValue(pHdbc, GetInfoId(), (SQLPOINTER)&v, sizeof(v), &cb);
			m_value = v;
		}
		else if (m_valueType == ValueType::UInt)
		{
			SQLUINTEGER i;
			ReadInfoValue(pHdbc, GetInfoId(), (SQLPOINTER)&i, sizeof(i), &cb);
			m_value = i;
		}
		else
		{
			string s;
			ReadInfoValue(pHdbc, GetInfoId(), s);
			m_value = s;
		}
		m_valueRead = true;
	}


	void SqlInfoProperty::ReadInfoValue(ConstSqlDbcHandlePtr pHDbc, SQLUSMALLINT fInfoType, SQLPOINTER rgbInfoValue, SQLSMALLINT cbInfoValueMax, SQLSMALLINT* pcbInfoValue) const
	{
		exASSERT(pHDbc);
		exASSERT(pHDbc->IsAllocated());
		SQLRETURN ret = SQLGetInfo(pHDbc->GetHandle(), fInfoType, rgbInfoValue, cbInfoValueMax, pcbInfoValue);
		THROW_IFN_SUCCEEDED_MSG(SQLGetInfo, ret, SQL_HANDLE_DBC, pHDbc->GetHandle(), (boost::format(u8"Failed to read property %s (%d)") % GetName() % GetInfoId()).str());
	}


	void SqlInfoProperty::ReadInfoValue(ConstSqlDbcHandlePtr pHDbc, SQLUSMALLINT fInfoType, std::string& sValue) const
	{
		// Determine buffer length
		exASSERT(pHDbc);
		exASSERT(pHDbc->IsAllocated());
		SQLSMALLINT bufferSize = 0;
		SQLRETURN ret = SQLGetInfo(pHDbc->GetHandle(), fInfoType, NULL, 0, &bufferSize);
		{
			// \note: DB2 will here always return SQL_SUCCESS_WITH_INFO to report that data got truncated, although we didnt even feed in a buffer.
			// To avoid having tons of warning with the (wrong) info that data has been truncated, we just hide those messages here
			THROW_IFN_SUCCEEDED_SILENT_MSG(SQLGetInfo, ret, SQL_HANDLE_DBC, pHDbc->GetHandle(), (boost::format(u8"Failed to read property %s (%d)") % GetName() % GetInfoId()).str());
		}

		// According to the doc SQLGetInfo will always return byte-size. Therefore:
		exASSERT((bufferSize % sizeof(SQLAPICHARTYPE)) == 0);

		// Allocate buffer, add one for terminating 0 char.
		SQLSMALLINT charSize = (bufferSize / sizeof(SQLAPICHARTYPE)) + 1;
		bufferSize = charSize * sizeof(SQLAPICHARTYPE);
		std::unique_ptr<SQLAPICHARTYPE[]> buff(new SQLAPICHARTYPE[charSize]);
		buff[0] = 0;
		SQLSMALLINT cb;

		ReadInfoValue(pHDbc, fInfoType, (SQLPOINTER)buff.get(), bufferSize, &cb);

		sValue = SQLAPICHARPTR_TO_EXODBCSTR(buff.get());
	}


	std::string SqlInfoProperty::GetStringValue() const
	{
		return boost::apply_visitor(SqlInfoPropertyStringValueVisitor(), m_value);
	}


	std::string SqlInfoPropertyStringValueVisitor::operator ()(SQLUSMALLINT si) const
	{
		stringstream ss;
		ss << si << u8" (0x" << std::hex << si << u8")";
		return ss.str();
	}


	std::string SqlInfoPropertyStringValueVisitor::operator ()(SQLUINTEGER i) const
	{
		stringstream ss;
		ss << i << u8" (0x" << std::hex << i << u8")";
		return ss.str();
	}


	std::string SqlInfoPropertyStringValueVisitor::operator ()(const std::string& s) const
	{
		return s;
	}


	SqlInfoProperties::SqlInfoProperties(ConstSqlDbcHandlePtr pHdbc)
	{
		Init(pHdbc);
	}


	SqlInfoProperties::SqlInfoProperties(ConstSqlDbcHandlePtr pHdbc, OdbcVersion odbcVersion)
	{
		Init(pHdbc, odbcVersion);
	}


	void SqlInfoProperties::Init(ConstSqlDbcHandlePtr pHdbc, OdbcVersion odbcVersion)
	{
		exASSERT(pHdbc);
		exASSERT(pHdbc->IsAllocated());

		RegisterDbmsProperties(odbcVersion);
		RegisterDataSourceProperties(odbcVersion);
		RegisterDriverProperties(odbcVersion);
		RegisterSupportedSqlProperties(odbcVersion);
		RegisterSqlLimitsProperties(odbcVersion);
		RegisterScalerFunctionProperties(odbcVersion);
		RegisterConversionProperties(odbcVersion);
		
		ReadAllProperties(pHdbc);
	}


	void SqlInfoProperties::Init(ConstSqlDbcHandlePtr pHdbc)
	{
		exASSERT(pHdbc);
		exASSERT(pHdbc->IsAllocated());

		// Determine drivers odbc version:
		SqlInfoProperty prop(SQL_DRIVER_ODBC_VER, u8"SQL_DRIVER_ODBC_VER", SqlInfoProperty::InfoType::Driver, SqlInfoProperty::ValueType::String_Any);
        OdbcVersion ov = OdbcVersion::UNKNOWN;
        try
        {
            prop.ReadProperty(pHdbc);
            std::string driverOdbcVersion = prop.GetStringValue();
            ov = ParseOdbcVersion(driverOdbcVersion);            
        }
        catch(const SqlResultException& ex)
        {
            LOG_WARNING(boost::str(boost::format(u8"Failed to determine driver ODBC-Version, assuming %d") 
                % (unsigned long)OdbcVersion::V_2));
            ov = OdbcVersion::V_2;
        }
		exASSERT(ov != OdbcVersion::UNKNOWN);
		Init(pHdbc, ov);
	}


	void SqlInfoProperties::Reset() noexcept
	{
		m_props.clear();
	}


	void SqlInfoProperties::RegisterDriverProperties(OdbcVersion odbcVersion)
	{
		using it = SqlInfoProperty::InfoType;
		using vt = SqlInfoProperty::ValueType;
		SqlInfoProperty::InfoType iType = it::Driver;

		RegisterProperty(SQL_DATA_SOURCE_NAME, u8"SQL_DATA_SOURCE_NAME", iType, vt::String_Any);
		RegisterProperty(SQL_DRIVER_NAME, u8"SQL_DRIVER_NAME", iType, vt::String_Any);
		RegisterProperty(SQL_DRIVER_ODBC_VER, u8"SQL_DRIVER_ODBC_VER", iType, vt::String_Any);
		RegisterProperty(SQL_DRIVER_NAME, u8"SQL_DRIVER_NAME", iType, vt::String_Any);
		RegisterProperty(SQL_FILE_USAGE, u8"SQL_FILE_USAGE", iType, vt::UInt);
		RegisterProperty(SQL_GETDATA_EXTENSIONS, u8"SQL_GETDATA_EXTENSIONS", iType, vt::UInt);
		RegisterProperty(SQL_MAX_CONCURRENT_ACTIVITIES, u8"SQL_MAX_CONCURRENT_ACTIVITIES", iType, vt::USmallInt);
		RegisterProperty(SQL_MAX_DRIVER_CONNECTIONS, u8"SQL_MAX_DRIVER_CONNECTIONS", iType, vt::USmallInt);
		RegisterProperty(SQL_ODBC_INTERFACE_CONFORMANCE, u8"SQL_ODBC_INTERFACE_CONFORMANCE", iType, vt::UInt);
		//RegisterProperty(SQL_ODBC_STANDARD_CLI_CONFORMANCE, u8"SQL_ODBC_STANDARD_CLI_CONFORMANCE", iType, vt::UInt);
		RegisterProperty(SQL_ODBC_VER, u8"SQL_ODBC_VER", iType, vt::String_Any);
		RegisterProperty(SQL_ROW_UPDATES, u8"SQL_ROW_UPDATES", iType, vt::String_N_Y);
		RegisterProperty(SQL_SEARCH_PATTERN_ESCAPE, u8"SQL_SEARCH_PATTERN_ESCAPE", iType, vt::String_Any);
		RegisterProperty(SQL_SERVER_NAME, u8"SQL_SERVER_NAME", iType, vt::String_Any);

		if (odbcVersion >= OdbcVersion::V_3)
		{
			RegisterProperty(SQL_ACTIVE_ENVIRONMENTS, u8"SQL_ACTIVE_ENVIRONMENTS", iType, vt::USmallInt);
			RegisterProperty(SQL_ASYNC_MODE, u8"SQL_ASYNC_MODE", iType, vt::UInt);
			RegisterProperty(SQL_ASYNC_NOTIFICATION, u8"SQL_ASYNC_NOTIFICATION", iType, vt::UInt);
			RegisterProperty(SQL_BATCH_ROW_COUNT, u8"SQL_BATCH_ROW_COUNT", iType, vt::UInt);
			RegisterProperty(SQL_BATCH_SUPPORT, u8"SQL_BATCH_SUPPORT", iType, vt::UInt);
			RegisterProperty(SQL_DYNAMIC_CURSOR_ATTRIBUTES1, u8"SQL_DYNAMIC_CURSOR_ATTRIBUTES1", iType, vt::UInt);
			RegisterProperty(SQL_DYNAMIC_CURSOR_ATTRIBUTES2, u8"SQL_DYNAMIC_CURSOR_ATTRIBUTES2", iType, vt::UInt);
			RegisterProperty(SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1, u8"SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1", iType, vt::UInt);
			RegisterProperty(SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2, u8"SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2", iType, vt::UInt);
			RegisterProperty(SQL_INFO_SCHEMA_VIEWS, u8"SQL_INFO_SCHEMA_VIEWS", iType, vt::UInt);
			RegisterProperty(SQL_KEYSET_CURSOR_ATTRIBUTES1, u8"SQL_KEYSET_CURSOR_ATTRIBUTES1", iType, vt::UInt);
			RegisterProperty(SQL_KEYSET_CURSOR_ATTRIBUTES2, u8"SQL_KEYSET_CURSOR_ATTRIBUTES2", iType, vt::UInt);
			RegisterProperty(SQL_MAX_ASYNC_CONCURRENT_STATEMENTS, u8"SQL_MAX_ASYNC_CONCURRENT_STATEMENTS", iType, vt::UInt);
			RegisterProperty(SQL_PARAM_ARRAY_ROW_COUNTS, u8"SQL_PARAM_ARRAY_ROW_COUNTS", iType, vt::UInt);
			RegisterProperty(SQL_PARAM_ARRAY_SELECTS, u8"SQL_PARAM_ARRAY_SELECTS", iType, vt::UInt);
			RegisterProperty(SQL_STATIC_CURSOR_ATTRIBUTES1, u8"SQL_STATIC_CURSOR_ATTRIBUTES1", iType, vt::UInt);
			RegisterProperty(SQL_STATIC_CURSOR_ATTRIBUTES2, u8"SQL_STATIC_CURSOR_ATTRIBUTES2", iType, vt::UInt);
		}

		if (odbcVersion >= OdbcVersion::V_3_8)
		{
			RegisterProperty(SQL_ASYNC_DBC_FUNCTIONS, u8"SQL_ASYNC_DBC_FUNCTIONS", iType, vt::UInt);
			RegisterProperty(SQL_DRIVER_AWARE_POOLING_SUPPORTED, u8"SQL_DRIVER_AWARE_POOLING_SUPPORTED", iType, vt::UInt);
		}
	}


	void SqlInfoProperties::RegisterDbmsProperties(OdbcVersion odbcVersion)
	{
		using it = SqlInfoProperty::InfoType;
		using vt = SqlInfoProperty::ValueType;
		SqlInfoProperty::InfoType iType = it::DBMS;

		RegisterProperty(SQL_DATABASE_NAME, u8"SQL_DATABASE_NAME", iType, vt::String_Any);
		RegisterProperty(SQL_DBMS_NAME, u8"SQL_DBMS_NAME", iType, vt::String_Any);
		RegisterProperty(SQL_DBMS_VER, u8"SQL_DBMS_VER", iType, vt::String_Any);
	}


	void SqlInfoProperties::RegisterDataSourceProperties(OdbcVersion odbcVersion)
	{
		using it = SqlInfoProperty::InfoType;
		using vt = SqlInfoProperty::ValueType;
		SqlInfoProperty::InfoType iType = it::DataSource;

		RegisterProperty(SQL_ACCESSIBLE_PROCEDURES, u8"SQL_ACCESSIBLE_PROCEDURES", iType, vt::String_N_Y);
		RegisterProperty(SQL_ACCESSIBLE_TABLES, u8"SQL_ACCESSIBLE_TABLES", iType, vt::String_N_Y);
		RegisterProperty(SQL_BOOKMARK_PERSISTENCE, u8"SQL_BOOKMARK_PERSISTENCE", iType, vt::UInt);
		RegisterProperty(SQL_CATALOG_TERM, u8"SQL_CATALOG_TERM", iType, vt::String_Any);
		RegisterProperty(SQL_CONCAT_NULL_BEHAVIOR, u8"SQL_CONCAT_NULL_BEHAVIOR", iType, vt::UInt);
		RegisterProperty(SQL_CURSOR_COMMIT_BEHAVIOR, u8"SQL_CURSOR_COMMIT_BEHAVIOR", iType, vt::UInt);
		RegisterProperty(SQL_CURSOR_ROLLBACK_BEHAVIOR, u8"SQL_CURSOR_ROLLBACK_BEHAVIOR", iType, vt::UInt);
		RegisterProperty(SQL_DATA_SOURCE_READ_ONLY, u8"SQL_DATA_SOURCE_READ_ONLY", iType, vt::String_N_Y);
		RegisterProperty(SQL_DEFAULT_TXN_ISOLATION, u8"SQL_DEFAULT_TXN_ISOLATION", iType, vt::UInt);
		RegisterProperty(SQL_MULTIPLE_ACTIVE_TXN, u8"SQL_MULTIPLE_ACTIVE_TXN", iType, vt::String_N_Y);
		RegisterProperty(SQL_NEED_LONG_DATA_LEN, u8"SQL_NEED_LONG_DATA_LEN", iType, vt::String_N_Y);
		RegisterProperty(SQL_NULL_COLLATION, u8"SQL_NULL_COLLATION", iType, vt::USmallInt);
		RegisterProperty(SQL_PROCEDURE_TERM, u8"SQL_PROCEDURE_TERM", iType, vt::String_Any);
		RegisterProperty(SQL_SCHEMA_TERM, u8"SQL_SCHEMA_TERM", iType, vt::String_Any);
		RegisterProperty(SQL_SCROLL_OPTIONS, u8"SQL_SCROLL_OPTIONS", iType, vt::UInt);
		RegisterProperty(SQL_TABLE_TERM, u8"SQL_TABLE_TERM", iType, vt::String_Any);
		RegisterProperty(SQL_TXN_CAPABLE, u8"SQL_TXN_CAPABLE", iType, vt::USmallInt);
		RegisterProperty(SQL_TXN_ISOLATION_OPTION, u8"SQL_TXN_ISOLATION_OPTION", iType, vt::UInt);
		RegisterProperty(SQL_USER_NAME, u8"SQL_USER_NAME", iType, vt::String_Any);

		if (odbcVersion >= OdbcVersion::V_3)
		{
			RegisterProperty(SQL_COLLATION_SEQ, u8"SQL_COLLATION_SEQ", iType, vt::String_Any);
			RegisterProperty(SQL_CURSOR_SENSITIVITY, u8"SQL_CURSOR_SENSITIVITY", iType, vt::UInt);
			RegisterProperty(SQL_DESCRIBE_PARAMETER, u8"SQL_DESCRIBE_PARAMETER", iType, vt::String_N_Y);

		}

	}


	void SqlInfoProperties::RegisterSupportedSqlProperties(OdbcVersion odbcVersion)
	{
		using it = SqlInfoProperty::InfoType;
		using vt = SqlInfoProperty::ValueType;
		SqlInfoProperty::InfoType iType = it::SupportedSql;

		RegisterProperty(SQL_ALTER_TABLE, u8"SQL_ALTER_TABLE", iType, vt::UInt);
		RegisterProperty(SQL_CATALOG_LOCATION, u8"SQL_CATALOG_LOCATION", iType, vt::USmallInt);
		RegisterProperty(SQL_CATALOG_NAME_SEPARATOR, u8"SQL_CATALOG_NAME_SEPARATOR", iType, vt::String_Any);
		RegisterProperty(SQL_CATALOG_USAGE, u8"SQL_CATALOG_USAGE", iType, vt::UInt);
		RegisterProperty(SQL_COLUMN_ALIAS, u8"SQL_COLUMN_ALIAS", iType, vt::String_N_Y);
		RegisterProperty(SQL_CORRELATION_NAME, u8"SQL_CORRELATION_NAME", iType, vt::USmallInt);
		RegisterProperty(SQL_EXPRESSIONS_IN_ORDERBY, u8"SQL_EXPRESSIONS_IN_ORDERBY", iType, vt::String_N_Y);
		RegisterProperty(SQL_GROUP_BY, u8"SQL_GROUP_BY", iType, vt::USmallInt);
		RegisterProperty(SQL_IDENTIFIER_CASE, u8"SQL_IDENTIFIER_CASE", iType, vt::USmallInt);
		RegisterProperty(SQL_IDENTIFIER_QUOTE_CHAR, u8"SQL_IDENTIFIER_QUOTE_CHAR", iType, vt::String_Any);
		RegisterProperty(SQL_INDEX_KEYWORDS, u8"SQL_INDEX_KEYWORDS", iType, vt::UInt);
		RegisterProperty(SQL_INTEGRITY, u8"SQL_INTEGRITY", iType, vt::String_N_Y);
		RegisterProperty(SQL_KEYWORDS, u8"SQL_KEYWORDS", iType, vt::String_Any);
		RegisterProperty(SQL_LIKE_ESCAPE_CLAUSE, u8"SQL_LIKE_ESCAPE_CLAUSE", iType, vt::String_N_Y);
		RegisterProperty(SQL_NON_NULLABLE_COLUMNS, u8"SQL_NON_NULLABLE_COLUMNS", iType, vt::USmallInt);
		RegisterProperty(SQL_SQL_CONFORMANCE, u8"SQL_SQL_CONFORMANCE", iType, vt::UInt);
		RegisterProperty(SQL_OJ_CAPABILITIES, u8"SQL_OJ_CAPABILITIES", iType, vt::UInt);
		RegisterProperty(SQL_ORDER_BY_COLUMNS_IN_SELECT, u8"SQL_ORDER_BY_COLUMNS_IN_SELECT", iType, vt::String_N_Y);
		RegisterProperty(SQL_OUTER_JOINS, u8"SQL_OUTER_JOINS", iType, vt::String_N_Y);
		RegisterProperty(SQL_PROCEDURES, u8"SQL_PROCEDURES", iType, vt::String_N_Y);
		RegisterProperty(SQL_QUOTED_IDENTIFIER_CASE, u8"SQL_QUOTED_IDENTIFIER_CASE", iType, vt::USmallInt);
		RegisterProperty(SQL_SCHEMA_USAGE, u8"SQL_SCHEMA_USAGE", iType, vt::UInt);
		RegisterProperty(SQL_SPECIAL_CHARACTERS, u8"SQL_SPECIAL_CHARACTERS", iType, vt::String_Any);
		RegisterProperty(SQL_SUBQUERIES, u8"SQL_SUBQUERIES", iType, vt::UInt);
		RegisterProperty(SQL_UNION, u8"SQL_UNION", iType, vt::UInt);

		if (odbcVersion >= OdbcVersion::V_3)
		{
			RegisterProperty(SQL_AGGREGATE_FUNCTIONS, u8"SQL_AGGREGATE_FUNCTIONS", iType, vt::UInt);
			RegisterProperty(SQL_ALTER_DOMAIN, u8"SQL_ALTER_DOMAIN", iType, vt::UInt);
			//RegisterProperty(SQL_ALTER_SCHEMA, u8"SQL_ALTER_SCHEMA", iType, vt::UInt);
			//RegisterProperty(SQL_ANSI_SQL_DATETIME_LITERALS, u8"SQL_ANSI_SQL_DATETIME_LITERALS", iType, vt::UInt);
			RegisterProperty(SQL_CATALOG_NAME, u8"SQL_CATALOG_NAME", iType, vt::String_N_Y);
			RegisterProperty(SQL_CREATE_ASSERTION, u8"SQL_CREATE_ASSERTION", iType, vt::UInt);
			RegisterProperty(SQL_CREATE_CHARACTER_SET, u8"SQL_CREATE_CHARACTER_SET", iType, vt::UInt);
			RegisterProperty(SQL_CREATE_COLLATION, u8"SQL_CREATE_COLLATION", iType, vt::UInt);
			RegisterProperty(SQL_CREATE_DOMAIN, u8"SQL_CREATE_DOMAIN", iType, vt::UInt);
			RegisterProperty(SQL_CREATE_SCHEMA, u8"SQL_CREATE_SCHEMA", iType, vt::UInt);
			RegisterProperty(SQL_CREATE_TABLE, u8"SQL_CREATE_TABLE", iType, vt::UInt);
			RegisterProperty(SQL_CREATE_TRANSLATION, u8"SQL_CREATE_TRANSLATION", iType, vt::UInt);
			RegisterProperty(SQL_DDL_INDEX, u8"SQL_DDL_INDEX", iType, vt::UInt);
			RegisterProperty(SQL_DROP_ASSERTION, u8"SQL_DROP_ASSERTION", iType, vt::UInt);
			RegisterProperty(SQL_DROP_CHARACTER_SET, u8"SQL_DROP_CHARACTER_SET", iType, vt::UInt);
			RegisterProperty(SQL_DROP_COLLATION, u8"SQL_DROP_COLLATION", iType, vt::UInt);
			RegisterProperty(SQL_DROP_DOMAIN, u8"SQL_DROP_DOMAIN", iType, vt::UInt);
			RegisterProperty(SQL_DROP_SCHEMA, u8"SQL_DROP_SCHEMA", iType, vt::UInt);
			RegisterProperty(SQL_DROP_TABLE, u8"SQL_DROP_TABLE", iType, vt::UInt);
			RegisterProperty(SQL_DROP_TRANSLATION, u8"SQL_DROP_TRANSLATION", iType, vt::UInt);
			RegisterProperty(SQL_DROP_VIEW, u8"SQL_DROP_VIEW", iType, vt::UInt);
			RegisterProperty(SQL_INSERT_STATEMENT, u8"SQL_INSERT_STATEMENT", iType, vt::UInt);

		}
	}


	void SqlInfoProperties::RegisterSqlLimitsProperties(OdbcVersion odbcVersion)
	{
		using it = SqlInfoProperty::InfoType;
		using vt = SqlInfoProperty::ValueType;
		SqlInfoProperty::InfoType iType = it::SqlLimits;

		RegisterProperty(SQL_MAX_BINARY_LITERAL_LEN, u8"SQL_MAX_BINARY_LITERAL_LEN", iType, vt::UInt);
		RegisterProperty(SQL_MAX_CATALOG_NAME_LEN, u8"SQL_MAX_CATALOG_NAME_LEN", iType, vt::USmallInt);
		RegisterProperty(SQL_MAX_CHAR_LITERAL_LEN, u8"SQL_MAX_CHAR_LITERAL_LEN", iType, vt::UInt);
		RegisterProperty(SQL_MAX_COLUMN_NAME_LEN, u8"SQL_MAX_COLUMN_NAME_LEN", iType, vt::USmallInt);
		RegisterProperty(SQL_MAX_COLUMNS_IN_GROUP_BY, u8"SQL_MAX_COLUMNS_IN_GROUP_BY", iType, vt::USmallInt);
		RegisterProperty(SQL_MAX_COLUMNS_IN_INDEX, u8"SQL_MAX_COLUMNS_IN_INDEX", iType, vt::UInt);
		RegisterProperty(SQL_MAX_COLUMNS_IN_ORDER_BY, u8"SQL_MAX_COLUMNS_IN_ORDER_BY", iType, vt::USmallInt);
		RegisterProperty(SQL_MAX_COLUMNS_IN_SELECT, u8"SQL_MAX_COLUMNS_IN_SELECT", iType, vt::USmallInt);
		RegisterProperty(SQL_MAX_COLUMNS_IN_TABLE, u8"SQL_MAX_COLUMNS_IN_TABLE", iType, vt::USmallInt);
		RegisterProperty(SQL_MAX_CURSOR_NAME_LEN, u8"SQL_MAX_CURSOR_NAME_LEN", iType, vt::USmallInt);
		RegisterProperty(SQL_MAX_INDEX_SIZE, u8"SQL_MAX_INDEX_SIZE", iType, vt::UInt);
		RegisterProperty(SQL_MAX_PROCEDURE_NAME_LEN, u8"SQL_MAX_PROCEDURE_NAME_LEN", iType, vt::USmallInt);
		RegisterProperty(SQL_MAX_ROW_SIZE, u8"SQL_MAX_ROW_SIZE", iType, vt::UInt);
		RegisterProperty(SQL_MAX_ROW_SIZE_INCLUDES_LONG, u8"SQL_MAX_ROW_SIZE_INCLUDES_LONG", iType, vt::String_N_Y);
		RegisterProperty(SQL_MAX_SCHEMA_NAME_LEN, u8"SQL_MAX_SCHEMA_NAME_LEN", iType, vt::USmallInt);
		RegisterProperty(SQL_MAX_STATEMENT_LEN, u8"SQL_MAX_STATEMENT_LEN", iType, vt::UInt);
		RegisterProperty(SQL_MAX_TABLE_NAME_LEN, u8"SQL_MAX_TABLE_NAME_LEN", iType, vt::USmallInt);
		RegisterProperty(SQL_MAX_TABLES_IN_SELECT, u8"SQL_MAX_TABLES_IN_SELECT", iType, vt::USmallInt);
		RegisterProperty(SQL_MAX_USER_NAME_LEN, u8"SQL_MAX_USER_NAME_LEN", iType, vt::USmallInt);

		if (odbcVersion >= OdbcVersion::V_3)
		{
			RegisterProperty(SQL_MAX_IDENTIFIER_LEN, u8"SQL_MAX_IDENTIFIER_LEN", iType, vt::USmallInt);

		}
	}


	void SqlInfoProperties::RegisterScalerFunctionProperties(OdbcVersion odbcVersion)
	{
		using it = SqlInfoProperty::InfoType;
		using vt = SqlInfoProperty::ValueType;
		SqlInfoProperty::InfoType iType = it::ScalarFunction;

		RegisterProperty(SQL_CONVERT_FUNCTIONS, u8"SQL_CONVERT_FUNCTIONS", iType, vt::UInt);
		RegisterProperty(SQL_NUMERIC_FUNCTIONS, u8"SQL_NUMERIC_FUNCTIONS", iType, vt::UInt);
		RegisterProperty(SQL_STRING_FUNCTIONS, u8"SQL_STRING_FUNCTIONS", iType, vt::UInt);
		RegisterProperty(SQL_SYSTEM_FUNCTIONS, u8"SQL_SYSTEM_FUNCTIONS", iType, vt::UInt);
		RegisterProperty(SQL_TIMEDATE_ADD_INTERVALS, u8"SQL_TIMEDATE_ADD_INTERVALS", iType, vt::UInt);
		RegisterProperty(SQL_TIMEDATE_DIFF_INTERVALS, u8"SQL_TIMEDATE_DIFF_INTERVALS", iType, vt::UInt);
		RegisterProperty(SQL_TIMEDATE_FUNCTIONS, u8"SQL_TIMEDATE_FUNCTIONS", iType, vt::UInt);
	}


	void SqlInfoProperties::RegisterConversionProperties(OdbcVersion odbcVersion)
	{
		using it = SqlInfoProperty::InfoType;
		using vt = SqlInfoProperty::ValueType;
		SqlInfoProperty::InfoType iType = it::Conversion;

		RegisterProperty(SQL_CONVERT_BIGINT, u8"SQL_CONVERT_BIGINT", iType, vt::UInt);
		RegisterProperty(SQL_CONVERT_BINARY, u8"SQL_CONVERT_BINARY", iType, vt::UInt);
		RegisterProperty(SQL_CONVERT_BIT, u8"SQL_CONVERT_BIT", iType, vt::UInt);
		RegisterProperty(SQL_CONVERT_CHAR, u8"SQL_CONVERT_CHAR", iType, vt::UInt);
		RegisterProperty(SQL_CONVERT_DATE, u8"SQL_CONVERT_DATE", iType, vt::UInt);
		RegisterProperty(SQL_CONVERT_DECIMAL, u8"SQL_CONVERT_DECIMAL", iType, vt::UInt);
		RegisterProperty(SQL_CONVERT_DOUBLE, u8"SQL_CONVERT_DOUBLE", iType, vt::UInt);
		RegisterProperty(SQL_CONVERT_INTEGER, u8"SQL_CONVERT_INTEGER", iType, vt::UInt);
		RegisterProperty(SQL_CONVERT_LONGVARBINARY, u8"SQL_CONVERT_LONGVARBINARY", iType, vt::UInt);
		RegisterProperty(SQL_CONVERT_LONGVARCHAR, u8"SQL_CONVERT_LONGVARCHAR", iType, vt::UInt);
		RegisterProperty(SQL_CONVERT_NUMERIC, u8"SQL_CONVERT_NUMERIC", iType, vt::UInt);
		RegisterProperty(SQL_CONVERT_REAL, u8"SQL_CONVERT_REAL", iType, vt::UInt);
		RegisterProperty(SQL_CONVERT_SMALLINT, u8"SQL_CONVERT_SMALLINT", iType, vt::UInt);
		RegisterProperty(SQL_CONVERT_TIME, u8"SQL_CONVERT_TIME", iType, vt::UInt);
		RegisterProperty(SQL_CONVERT_TIMESTAMP, u8"SQL_CONVERT_TIMESTAMP", iType, vt::UInt);
		RegisterProperty(SQL_CONVERT_TINYINT, u8"SQL_CONVERT_TINYINT", iType, vt::UInt);
		RegisterProperty(SQL_CONVERT_VARBINARY, u8"SQL_CONVERT_VARBINARY", iType, vt::UInt);
		RegisterProperty(SQL_CONVERT_VARCHAR, u8"SQL_CONVERT_VARCHAR", iType, vt::UInt);

		if (odbcVersion >= OdbcVersion::V_3)
		{
			RegisterProperty(SQL_CONVERT_INTERVAL_YEAR_MONTH, u8"SQL_CONVERT_INTERVAL_YEAR_MONTH", iType, vt::UInt);
			RegisterProperty(SQL_CONVERT_INTERVAL_DAY_TIME, u8"SQL_CONVERT_INTERVAL_DAY_TIME", iType, vt::UInt);

		}
	}


	void SqlInfoProperties::RegisterProperty(SQLUSMALLINT id, const std::string& name, SqlInfoProperty::InfoType infoType, SqlInfoProperty::ValueType valueType)
	{
		SqlInfoProperty prop(id, name, infoType, valueType);
		m_props.insert( make_pair(id, prop));
	}


	SqlInfoProperties::PropertiesSet SqlInfoProperties::GetSubset(SqlInfoProperty::InfoType infoType) const noexcept
	{
		set<SqlInfoProperty, SLexicalCompare> props;
		for (PropsMap::const_iterator it = m_props.begin(); it != m_props.end(); ++it)
		{
			if (it->second.GetInfoType() == infoType)
				props.insert(it->second);
		}
		return props;
	}


	SqlInfoProperty SqlInfoProperties::GetProperty(SQLUSMALLINT infoId) const
	{
		PropsMap::const_iterator it = m_props.find(infoId);
		if (it == m_props.end())
		{
			NotFoundException nfe(boost::str(boost::format(u8"Property with id %d is not registered") % infoId));
			SET_EXCEPTION_SOURCE(nfe);
			throw nfe;
		}
		return it->second;
	}


	bool SqlInfoProperties::IsPropertyRegistered(SQLUSMALLINT infoId) const noexcept
	{
		PropsMap::const_iterator it = m_props.find(infoId);
		return it != m_props.end();
	}


	void SqlInfoProperties::EnsurePropertyRead(ConstSqlDbcHandlePtr pHdbc, SQLUSMALLINT infoId, bool forceUpdate)
	{
		PropsMap::iterator it = m_props.find(infoId);
		if (it == m_props.end())
		{
			NotFoundException nfe(boost::str(boost::format(u8"Property with id %d is not registered") % infoId));
			SET_EXCEPTION_SOURCE(nfe);
			throw nfe;
		}
		if(it->second.GetValueRead() || forceUpdate)
			it->second.ReadProperty(pHdbc);
	}


	void SqlInfoProperties::ReadAllProperties(ConstSqlDbcHandlePtr pHdbc)
	{
		for (PropsMap::iterator it = m_props.begin(); it != m_props.end(); ++it)
		{
			SqlInfoProperty& prop = it->second;
			try
			{
				prop.ReadProperty(pHdbc);
			}
			catch (const SqlResultException& ex)
			{
				LOG_WARNING(boost::str(boost::format(u8"Failed to read property %s (%d): %s") % prop.GetName() % prop.GetInfoId() % ex.ToString()));
			}
		}
	}


	std::string SqlInfoProperties::GetDbmsName() const
	{
		SqlInfoProperty prop = GetProperty(SQL_DBMS_NAME);
		return prop.GetStringValue();
	}


	std::string SqlInfoProperties::GetDriverName() const
	{
		SqlInfoProperty prop = GetProperty(SQL_DRIVER_NAME);
		return prop.GetStringValue();
	}


	OdbcVersion SqlInfoProperties::ParseOdbcVersion(const std::string& versionString)
	{
		OdbcVersion ov = OdbcVersion::UNKNOWN;
		std::vector<std::string> versions;
		boost::split(versions, versionString, boost::is_any_of(u8"."));
		if (versions.size() == 2)
		{
			try
			{
				short major = boost::lexical_cast<short>(versions[0]);
				short minor = boost::lexical_cast<short>(versions[1]);
				if (major >= 3 && minor >= 80)
				{
					ov = OdbcVersion::V_3_8;
				}
				else if (major >= 3)
				{
					ov = OdbcVersion::V_3;
				}
				else if (major >= 2)
				{
					ov = OdbcVersion::V_2;
				}
			}
			catch (boost::bad_lexical_cast& e)
			{
				LOG_WARNING(boost::str(boost::format(u8"Failed to determine odbc version from string '%s': %s") % versionString % e.what()));
			}
		}
		return ov;
	}


	OdbcVersion SqlInfoProperties::GetDriverOdbcVersion() const
	{
		SqlInfoProperty prop = GetProperty(SQL_DRIVER_ODBC_VER);
		std::string driverOdbcVersion = prop.GetStringValue();
		return ParseOdbcVersion(driverOdbcVersion);
	}


	DatabaseProduct SqlInfoProperties::DetectDbms() const
	{
		SqlInfoProperty prop = GetProperty(SQL_DBMS_NAME);
		std::string name = prop.GetStringValue();
		if (boost::algorithm::contains(name, u8"Microsoft SQL Server"))
		{
			return DatabaseProduct::MS_SQL_SERVER;
		}
		else if (boost::algorithm::contains(name, u8"MySQL"))
		{
			return DatabaseProduct::MY_SQL;
		}
		else if (boost::algorithm::contains(name, u8"DB2"))
		{
			return DatabaseProduct::DB2;
		}
		else if (boost::algorithm::contains(name, u8"EXCEL"))
		{
			return DatabaseProduct::EXCEL;
		}
		else if (boost::algorithm::contains(name, u8"ACCESS")
            || boost::algorithm::contains(name, u8"MDBTOOLS"))
		{
			return DatabaseProduct::ACCESS;
		}

		return DatabaseProduct::UNKNOWN;
	}


	bool SqlInfoProperties::GetSupportsTransactions() const
	{
		SqlInfoProperty prop = GetProperty(SQL_TXN_CAPABLE);
		SQLUSMALLINT v = boost::get<SQLUSMALLINT>(prop.GetValue());
		return v != SQL_TC_NONE;
	}


	bool SqlInfoProperties::GetSupportsCatalogs() const
	{
		if (!IsPropertyRegistered(SQL_CATALOG_NAME))
			return false;

		SqlInfoProperty prop = GetProperty(SQL_CATALOG_NAME);
		return prop.GetStringValue() == u8"Y";
	}


	string SqlInfoProperties::GetSchemaTerm() const
	{
		SqlInfoProperty prop = GetProperty(SQL_SCHEMA_TERM);
		return prop.GetStringValue();
	}


	string SqlInfoProperties::GetCatalogTerm() const
	{
		SqlInfoProperty prop = GetProperty(SQL_CATALOG_TERM);
		return prop.GetStringValue();
	}


	SQLUSMALLINT SqlInfoProperties::GetMaxCatalogNameLen() const
	{
		SqlInfoProperty prop = GetProperty(SQL_MAX_CATALOG_NAME_LEN);
		SQLUSMALLINT value = boost::get<SQLUSMALLINT>(prop.GetValue());
		if (value > 0)
		{
			return value;
		}

		return DB_MAX_CATALOG_NAME_LEN_DEFAULT;
	}


	SQLUSMALLINT SqlInfoProperties::GetMaxSchemaNameLen() const
	{
		SqlInfoProperty prop = GetProperty(SQL_MAX_SCHEMA_NAME_LEN);
		SQLUSMALLINT value = boost::get<SQLUSMALLINT>(prop.GetValue());
		if (value > 0)
		{
			return value;
		}
		return DB_MAX_SCHEMA_NAME_LEN_DEFAULT;
	}


	SQLUSMALLINT SqlInfoProperties::GetMaxTableNameLen() const
	{
		SqlInfoProperty prop = GetProperty(SQL_MAX_TABLE_NAME_LEN);
		SQLUSMALLINT value = boost::get<SQLUSMALLINT>(prop.GetValue());
		if (value > 0)
		{
			return value;
		}
		return DB_MAX_TABLE_NAME_LEN_DEFAULT;
	}


	SQLUSMALLINT SqlInfoProperties::GetMaxColumnNameLen() const
	{
		SqlInfoProperty prop = GetProperty(SQL_MAX_COLUMN_NAME_LEN);
		SQLUSMALLINT value = boost::get<SQLUSMALLINT>(prop.GetValue());
		if (value > 0)
		{
			return value;
		}
		return DB_MAX_COLUMN_NAME_LEN_DEFAULT;
	}


	bool SqlInfoProperties::GetForwardOnlyCursors() const
	{
		SqlInfoProperty prop = GetProperty(SQL_SCROLL_OPTIONS);
		SQLUINTEGER value = boost::get<SQLUINTEGER>(prop.GetValue());
		value &= ~SQL_SO_FORWARD_ONLY;
		return value == 0;
	}


	string SqlInfoProperties::GetSearchPatternEscape() const
	{
		SqlInfoProperty prop = GetProperty(SQL_SEARCH_PATTERN_ESCAPE);
		return prop.GetStringValue();
	}
}
