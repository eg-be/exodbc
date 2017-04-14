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



	SqlInfoProperties::SqlInfoProperties(ConstSqlDbcHandlePtr pHdbc)
	{
		Init(pHdbc);
	}


	void SqlInfoProperties::Init(ConstSqlDbcHandlePtr pHdbc)
	{
		exASSERT(pHdbc);
		exASSERT(pHdbc->IsAllocated());

		RegisterDbmsProperties();
		RegisterDataSourceProperties();
		LoadProperties(pHdbc);
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


	void SqlInfoProperties::LoadProperties(ConstSqlDbcHandlePtr pHdbc)
	{
		exASSERT(pHdbc);
		exASSERT(pHdbc->IsAllocated());

		for (PropsMap::iterator it = m_props.begin(); it != m_props.end(); ++it)
		{
			SqlInfoProperty& prop = it->second;
			prop.ReadProperty(pHdbc);
		}
	}


}
