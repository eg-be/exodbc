/*!
* \file Environment.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 25.07.2014
* \brief Source file for the Environment class and its helpers.
* \copyright GNU Lesser General Public License Version 3
*
*/ 
 
// Own header
#include "Environment.h"

// Same component headers
#include "Exception.h"

// Other headers
// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	// Construction
	// ------------
	
	Environment::Environment()
		: m_pHEnv(std::make_shared<SqlEnvHandle>())
		, m_odbcVersion(OdbcVersion::UNKNOWN)
	{
	}
	
	
	Environment::Environment(OdbcVersion odbcVersion)
		: m_pHEnv(std::make_shared<SqlEnvHandle>())
		, m_odbcVersion(OdbcVersion::UNKNOWN)
	{
		// Note: Init will call SetOdbcVersion, this will read
		// back the version reported by the driver - so it is
		// okay to set it to UNKNOWN in the constructor.
		Init(odbcVersion);
	}


	Environment::Environment(const Environment& other)
		: m_pHEnv(std::make_shared<SqlEnvHandle>())
		, m_odbcVersion(OdbcVersion::UNKNOWN)
	{
		OdbcVersion odbcVersion = other.GetOdbcVersion();
		if (odbcVersion != OdbcVersion::UNKNOWN)
		{
			Init(odbcVersion);
		}
	}


	// Destructor
	// -----------
	Environment::~Environment()
	{
		// Nothing special to free, our handle will free itself once it goes out of scope
	}


	// Implementation
	// --------------
	void Environment::EnableConnectionPooling(ConnectionPooling enablePooling)
	{
		SQLRETURN ret = SQLSetEnvAttr(SQL_NULL_HENV, SQL_ATTR_CONNECTION_POOLING, (SQLPOINTER)enablePooling, 0);
		if (!SQL_SUCCEEDED(ret))
		{
			std::string msg = boost::str(boost::format(u8"Failed to set Attribute SQL_ATTR_CONNECTION_POOLING to %d") % (int)enablePooling);
			SqlResultException ex(u8"SQLSetEnvAttr", ret, msg);
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}
	}


	std::shared_ptr<Environment> Environment::Create(OdbcVersion odbcVersion)
	{
		EnvironmentPtr pEnv = std::make_shared<Environment>(odbcVersion);
		return pEnv;
	}

	void Environment::Init(OdbcVersion odbcVersion)
	{
		exASSERT(odbcVersion != OdbcVersion::UNKNOWN);
		exASSERT(!m_pHEnv->IsAllocated());

		//AllocateEnvironmentHandle();
		m_pHEnv->Allocate();
		SetOdbcVersion(odbcVersion);
	}


	void Environment::SetOdbcVersion(OdbcVersion version)
	{
		exASSERT(IsEnvHandleAllocated());

		// Remember: because the SQLPOINTER is interpreted as an int value.. for int-attrs..
		SQLRETURN ret;
		switch(version)
		{
		case OdbcVersion::V_2:
			ret = SQLSetEnvAttr(m_pHEnv->GetHandle(), SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC2, 0);
			break;
		case OdbcVersion::V_3:
			ret = SQLSetEnvAttr(m_pHEnv->GetHandle(), SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, 0);
			break;
		case OdbcVersion::V_3_8:
			ret = SQLSetEnvAttr(m_pHEnv->GetHandle(), SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3_80, 0);
			break;
		default:
			THROW_WITH_SOURCE(IllegalArgumentException, (boost::format(u8"Unknown ODBC Version value: %d") % (int) version).str());
		}

		THROW_IFN_SUCCEEDED_MSG(SQLSetEnvAttr, ret, SQL_HANDLE_ENV, m_pHEnv->GetHandle(), (boost::format(u8"Failed to set SQL_ATTR_ODBC_VERSION to value %d") % (int) version).str());

		ReadOdbcVersion();
	}


	OdbcVersion Environment::ReadOdbcVersion() const
	{
		exASSERT(IsEnvHandleAllocated());

		unsigned long value = 0;
		SQLRETURN ret = SQLGetEnvAttr(m_pHEnv->GetHandle(), SQL_ATTR_ODBC_VERSION, &value, 0, NULL);

		THROW_IFN_SUCCEEDED_MSG(SQLGetEnvAttr, ret, SQL_HANDLE_ENV, m_pHEnv->GetHandle(), u8"Failed to read SQL_ATTR_ODBC_VERSION");

		switch(value)
		{
		case SQL_OV_ODBC2:
		{
			m_odbcVersion = OdbcVersion::V_2;
			break;
		}
		case SQL_OV_ODBC3:
		{
			m_odbcVersion = OdbcVersion::V_3;
			break;
		}
		case SQL_OV_ODBC3_80:
		{
			m_odbcVersion = OdbcVersion::V_3_8;
			break;
		}
		}

		return m_odbcVersion;
	}


	OdbcVersion Environment::GetOdbcVersion() const
	{
		if (m_odbcVersion == OdbcVersion::UNKNOWN && m_pHEnv && m_pHEnv->IsAllocated())
		{
			ReadOdbcVersion();
		}
		return m_odbcVersion;
	}


	void Environment::SetConnctionPoolingMatch(ConnectionPoolingMatch matchMode)
	{
		SQLRETURN ret = SQLSetEnvAttr(m_pHEnv->GetHandle(), SQL_ATTR_CP_MATCH, (SQLPOINTER)matchMode, 0);
		THROW_IFN_SUCCEEDED(SQLSetEnvAttr, ret, SQL_HANDLE_ENV, m_pHEnv->GetHandle());
	}


	DataSourcesVector Environment::ListDataSources(ListMode mode) const
	{
		exASSERT(IsEnvHandleAllocated());

		SQLSMALLINT nameBufferLength, descBufferLength = 0;
		SQLAPICHARTYPE nameBuffer[SQL_MAX_DSN_LENGTH + 1];

		// empty result-vector
		DataSourcesVector dataSources;

		SQLUSMALLINT direction = SQL_FETCH_FIRST;
		if (mode == ListMode::System)
			direction = SQL_FETCH_FIRST_SYSTEM;
		else if (mode == ListMode::User)
			direction = SQL_FETCH_FIRST_USER;

		// We need two passed, I dont know the max length of the description
		// I also dont know if the order how they are returned is the same
		// Or while doing the two iterations, sources might get added / removed
		// So we remember the max length of the buffer, and use that in the second pass
		// Like that we might miss some parts of the descriptions.. 
		SQLSMALLINT maxDescLength = 0;
		SQLRETURN ret = SQL_NO_DATA;
		ret = SQLDataSources(m_pHEnv->GetHandle(), direction, nameBuffer, SQL_MAX_DSN_LENGTH + 1, &nameBufferLength, NULL, 0, &descBufferLength);
		if(ret == SQL_NO_DATA)
		{
			// no data at all, but succeeded, else we would have an err-state
			return dataSources;
		}
		THROW_IFN_SUCCEEDED(SQLDataSources, ret, SQL_HANDLE_ENV, m_pHEnv->GetHandle());

		do
		{
			// Remember the max length 
			if (descBufferLength > maxDescLength)
			{
				maxDescLength = descBufferLength;
			}
			// Go on fetching lengths of descriptions
			ret = SQLDataSources(m_pHEnv->GetHandle(), SQL_FETCH_NEXT, nameBuffer, SQL_MAX_DSN_LENGTH + 1, &nameBufferLength, NULL, 0, &descBufferLength);
		}while(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
		THROW_IFN_NO_DATA(SQLDataSources, ret);

		// Now fetch with description
		std::unique_ptr<SQLAPICHARTYPE[]> descBuffer(new SQLAPICHARTYPE[maxDescLength + 1]);
		ret = SQLDataSources(m_pHEnv->GetHandle(), direction, nameBuffer, SQL_MAX_DSN_LENGTH + 1, &nameBufferLength, descBuffer.get(), maxDescLength + 1, &descBufferLength);
		if(ret == SQL_NO_DATA)
		{
			SqlResultException ex(u8"SQLDataSources", ret, u8"SQL_NO_DATA is not expected to happen here - we've found records in the previous round, they can't be gone now!");
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}
		do
		{
			// Store dataSource
			SDataSource ds;
			ds.m_dsn = SQLAPICHARPTR_TO_EXODBCSTR(nameBuffer);
			ds.m_description = SQLAPICHARPTR_TO_EXODBCSTR(descBuffer.get());
			dataSources.push_back(ds);
			ret = SQLDataSources(m_pHEnv->GetHandle(), SQL_FETCH_NEXT, nameBuffer, SQL_MAX_DSN_LENGTH + 1, &nameBufferLength, descBuffer.get(), maxDescLength + 1, &descBufferLength);
		}while(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);

		THROW_IFN_NO_DATA(SQLDataSources, ret);

		return dataSources;
	}

}
