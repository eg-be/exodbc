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
		THROW_IFN_SUCCEEDED_MSG(SQLGetInfo, ret, SQL_HANDLE_DBC, pHDbc->GetHandle(), (boost::format(u8"GetInfo for fInfoType %d failed") % fInfoType).str());
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
			THROW_IFN_SUCCEEDED_SILENT_MSG(SQLGetInfo, ret, SQL_HANDLE_DBC, pHDbc->GetHandle(), (boost::format(u8"GetInfo for fInfoType %d failed") % fInfoType).str());
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
		return boost::apply_visitor(SqlInfoPropertyNameVisitor(), m_value);
	}


	std::string SqlInfoPropertyNameVisitor::operator ()(SQLUSMALLINT si) const
	{
		stringstream ss;
		ss << si << u8" (0x" << std::hex << si << u8")";
		return ss.str();
	}


	std::string SqlInfoPropertyNameVisitor::operator ()(SQLUINTEGER i) const
	{
		stringstream ss;
		ss << i << u8" (0x" << std::hex << i << u8")";
		return ss.str();
	}


	std::string SqlInfoPropertyNameVisitor::operator ()(const std::string& s) const
	{
		return s;
	}



	SqlInfoProperties::SqlInfoProperties(ConstSqlDbcHandlePtr pHdbc, bool readAllProperties)
	{
		Init(pHdbc, readAllProperties);
	}


	void SqlInfoProperties::Init(ConstSqlDbcHandlePtr pHdbc, bool readAllProperties)
	{
		exASSERT(pHdbc);
		exASSERT(pHdbc->IsAllocated());
		m_pHdbc = pHdbc;

		RegisterDbmsProperties();
		RegisterDataSourceProperties();
		RegisterDriverInformation();

		if (readAllProperties)
		{
			ReadAllProperties();
		}
	}


	void SqlInfoProperties::RegisterDriverInformation()
	{
		using it = SqlInfoProperty::InfoType;
		using vt = SqlInfoProperty::ValueType;
		SqlInfoProperty::InfoType iType = it::Driver;

		RegisterProperty(SQL_ACTIVE_ENVIRONMENTS, u8"SQL_ACTIVE_ENVIRONMENTS", iType, vt::USmallInt);
		RegisterProperty(SQL_ASYNC_DBC_FUNCTIONS, u8"SQL_ASYNC_DBC_FUNCTIONS", iType, vt::UInt);
		RegisterProperty(SQL_ASYNC_MODE, u8"SQL_ASYNC_MODE", iType, vt::UInt);
		RegisterProperty(SQL_ASYNC_NOTIFICATION, u8"SQL_ASYNC_NOTIFICATION", iType, vt::UInt);
		RegisterProperty(SQL_BATCH_ROW_COUNT, u8"SQL_BATCH_ROW_COUNT", iType, vt::UInt);
		RegisterProperty(SQL_BATCH_SUPPORT, u8"SQL_BATCH_SUPPORT", iType, vt::UInt);
		RegisterProperty(SQL_DATA_SOURCE_NAME, u8"SQL_DATA_SOURCE_NAME", iType, vt::String_Any);
		RegisterProperty(SQL_DRIVER_AWARE_POOLING_SUPPORTED, u8"SQL_DRIVER_AWARE_POOLING_SUPPORTED", iType, vt::UInt);
		RegisterProperty(SQL_DRIVER_NAME, u8"SQL_DRIVER_NAME", iType, vt::String_Any);
		RegisterProperty(SQL_DRIVER_ODBC_VER, u8"SQL_DRIVER_ODBC_VER", iType, vt::String_Any);
		RegisterProperty(SQL_DRIVER_NAME, u8"SQL_DRIVER_NAME", iType, vt::String_Any);
		RegisterProperty(SQL_DYNAMIC_CURSOR_ATTRIBUTES1, u8"SQL_DYNAMIC_CURSOR_ATTRIBUTES1", iType, vt::UInt);
		RegisterProperty(SQL_DYNAMIC_CURSOR_ATTRIBUTES2, u8"SQL_DYNAMIC_CURSOR_ATTRIBUTES2", iType, vt::UInt);
		RegisterProperty(SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1, u8"SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES1", iType, vt::UInt);
		RegisterProperty(SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2, u8"SQL_FORWARD_ONLY_CURSOR_ATTRIBUTES2", iType, vt::UInt);
		RegisterProperty(SQL_FILE_USAGE, u8"SQL_FILE_USAGE", iType, vt::UInt);
		RegisterProperty(SQL_GETDATA_EXTENSIONS, u8"SQL_GETDATA_EXTENSIONS", iType, vt::UInt);
		RegisterProperty(SQL_INFO_SCHEMA_VIEWS, u8"SQL_INFO_SCHEMA_VIEWS", iType, vt::UInt);
		RegisterProperty(SQL_KEYSET_CURSOR_ATTRIBUTES1, u8"SQL_KEYSET_CURSOR_ATTRIBUTES1", iType, vt::UInt);
		RegisterProperty(SQL_KEYSET_CURSOR_ATTRIBUTES2, u8"SQL_KEYSET_CURSOR_ATTRIBUTES2", iType, vt::UInt);
		RegisterProperty(SQL_MAX_ASYNC_CONCURRENT_STATEMENTS, u8"SQL_MAX_ASYNC_CONCURRENT_STATEMENTS", iType, vt::UInt);
		RegisterProperty(SQL_MAX_CONCURRENT_ACTIVITIES, u8"SQL_MAX_CONCURRENT_ACTIVITIES", iType, vt::USmallInt);
		RegisterProperty(SQL_MAX_DRIVER_CONNECTIONS, u8"SQL_MAX_DRIVER_CONNECTIONS", iType, vt::USmallInt);
		RegisterProperty(SQL_ODBC_INTERFACE_CONFORMANCE, u8"SQL_ODBC_INTERFACE_CONFORMANCE", iType, vt::UInt);
		//RegisterProperty(SQL_ODBC_STANDARD_CLI_CONFORMANCE, u8"SQL_ODBC_STANDARD_CLI_CONFORMANCE", iType, vt::UInt);
		RegisterProperty(SQL_ODBC_VER, u8"SQL_ODBC_VER", iType, vt::String_Any);
		RegisterProperty(SQL_PARAM_ARRAY_ROW_COUNTS, u8"SQL_PARAM_ARRAY_ROW_COUNTS", iType, vt::UInt);
		RegisterProperty(SQL_PARAM_ARRAY_SELECTS, u8"SQL_PARAM_ARRAY_SELECTS", iType, vt::UInt);
		RegisterProperty(SQL_ROW_UPDATES, u8"SQL_ROW_UPDATES", iType, vt::String_N_Y);
		RegisterProperty(SQL_SEARCH_PATTERN_ESCAPE, u8"SQL_SEARCH_PATTERN_ESCAPE", iType, vt::String_Any);
		RegisterProperty(SQL_SERVER_NAME, u8"SQL_SERVER_NAME", iType, vt::String_Any);
		RegisterProperty(SQL_STATIC_CURSOR_ATTRIBUTES1, u8"SQL_STATIC_CURSOR_ATTRIBUTES1", iType, vt::UInt);
		RegisterProperty(SQL_STATIC_CURSOR_ATTRIBUTES2, u8"SQL_STATIC_CURSOR_ATTRIBUTES2", iType, vt::UInt);

	}


	void SqlInfoProperties::RegisterDbmsProperties()
	{
		using it = SqlInfoProperty::InfoType;
		using vt = SqlInfoProperty::ValueType;
		SqlInfoProperty::InfoType iType = it::DBMS;

		RegisterProperty(SQL_DATABASE_NAME, u8"SQL_DATABASE_NAME", iType, vt::String_Any);
		RegisterProperty(SQL_DBMS_NAME, u8"SQL_DBMS_NAME", iType, vt::String_Any);
		RegisterProperty(SQL_DBMS_VER, u8"SQL_DBMS_VER", iType, vt::String_Any);
	}


	void SqlInfoProperties::RegisterDataSourceProperties()
	{
		using it = SqlInfoProperty::InfoType;
		using vt = SqlInfoProperty::ValueType;
		SqlInfoProperty::InfoType iType = it::DataSource;

		RegisterProperty(SQL_ACCESSIBLE_PROCEDURES, u8"SQL_ACCESSIBLE_PROCEDURES", iType, vt::String_N_Y);
		RegisterProperty(SQL_ACCESSIBLE_TABLES, u8"SQL_ACCESSIBLE_TABLES", iType, vt::String_N_Y);
		RegisterProperty(SQL_BOOKMARK_PERSISTENCE, u8"SQL_BOOKMARK_PERSISTENCE", iType, vt::UInt);
		RegisterProperty(SQL_CATALOG_TERM, u8"SQL_CATALOG_TERM", iType, vt::String_Any);
		RegisterProperty(SQL_COLLATION_SEQ, u8"SQL_COLLATION_SEQ", iType, vt::String_Any);
		RegisterProperty(SQL_CONCAT_NULL_BEHAVIOR, u8"SQL_CONCAT_NULL_BEHAVIOR", iType, vt::UInt);
		RegisterProperty(SQL_CURSOR_COMMIT_BEHAVIOR, u8"SQL_CURSOR_COMMIT_BEHAVIOR", iType, vt::UInt);
		RegisterProperty(SQL_CURSOR_ROLLBACK_BEHAVIOR, u8"SQL_CURSOR_ROLLBACK_BEHAVIOR", iType, vt::UInt);
		RegisterProperty(SQL_CURSOR_SENSITIVITY, u8"SQL_CURSOR_SENSITIVITY", iType, vt::UInt);
		RegisterProperty(SQL_DATA_SOURCE_READ_ONLY, u8"SQL_DATA_SOURCE_READ_ONLY", iType, vt::String_N_Y);
		RegisterProperty(SQL_DEFAULT_TXN_ISOLATION, u8"SQL_DEFAULT_TXN_ISOLATION", iType, vt::UInt);
		RegisterProperty(SQL_DESCRIBE_PARAMETER, u8"SQL_DESCRIBE_PARAMETER", iType, vt::String_N_Y);
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

	}


	void SqlInfoProperties::RegisterProperty(SQLUSMALLINT id, const std::string& name, SqlInfoProperty::InfoType infoType, SqlInfoProperty::ValueType valueType)
	{
		SqlInfoProperty prop(id, name, infoType, valueType);
		m_props.insert( make_pair(id, prop));
	}


	set<SqlInfoProperty, SqlInfoProperties::SLexicalCompare> SqlInfoProperties::GetProperties(SqlInfoProperty::InfoType infoType) const noexcept
	{
		set<SqlInfoProperty, SLexicalCompare> props;
		for (PropsMap::const_iterator it = m_props.begin(); it != m_props.end(); ++it)
		{
			if (it->second.GetInfoType() == infoType)
				props.insert(it->second);
		}
		return props;
	}


	SqlInfoProperty SqlInfoProperties::GetProperty(SQLUSMALLINT infoId)
	{
		PropsMap::iterator it = m_props.find(infoId);
		if (it == m_props.end())
		{
			NotFoundException nfe(boost::str(boost::format(u8"Property with id %d is not registered") % infoId));
			SET_EXCEPTION_SOURCE(nfe);
			throw nfe;
		}
		SqlInfoProperty& prop = it->second;
		if (!prop.GetValueRead())
		{
			prop.ReadProperty(m_pHdbc);
		}
		return prop;
	}


	void SqlInfoProperties::ReadAllProperties()
	{
		for (PropsMap::iterator it = m_props.begin(); it != m_props.end(); ++it)
		{
			SqlInfoProperty& prop = it->second;
			prop.ReadProperty(m_pHdbc);
		}
	}


}
