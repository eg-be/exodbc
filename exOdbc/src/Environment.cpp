/*!
* \file Environment.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 25.07.2014
* \brief Source file for the Environment class and its helpers.
* \copyright GNU Lesser General Public License Version 3
*
*/ 
 
#include "stdafx.h"

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
		: m_pHenv(std::make_shared<SqlEnvHandle>())
		, m_odbcVersion(OdbcVersion::UNKNOWN)
	{
	}
	
	
	Environment::Environment(OdbcVersion odbcVersion)
		: m_pHenv(std::make_shared<SqlEnvHandle>())
		, m_odbcVersion(OdbcVersion::UNKNOWN)
	{
		// Note: Init will call SetOdbcVersion, this will read
		// back the version reported by the driver - so it is
		// okay to set it to UNKNOWN in the constructor.
		Init(odbcVersion);
	}


	Environment::Environment(const Environment& other)
		: m_pHenv(std::make_shared<SqlEnvHandle>())
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
	void Environment::Init(OdbcVersion odbcVersion)
	{
		exASSERT(odbcVersion != OdbcVersion::UNKNOWN);
		exASSERT(!m_pHenv->IsAllocated());

		//AllocateEnvironmentHandle();
		m_pHenv->Allocate(NULL);
		SetOdbcVersion(odbcVersion);
	}


	//void Environment::AllocateEnvironmentHandle()
	//{
	//	// This is here to help trap if you are getting a new henv
	//	// without releasing an existing henv
	//	exASSERT(!HasEnvironmentHandle());

	//	// Initialize the ODBC Environment for Database Operations
	//	SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_henv);
	//	if (!SQL_SUCCEEDED(ret))
	//	{
	//		SqlResultException ex(L"SQLAllocHandle", ret, L"Failed to allocated ODBC-Env Handle");
	//		SET_EXCEPTION_SOURCE(ex);
	//		throw ex;
	//	}
	//}


	//void Environment::FreeEnvironmentHandle()
	//{
	//	exASSERT(m_henv);

	//	SQLRETURN ret = SQL_SUCCESS;

	//	if (SQL_NULL_HENV != m_henv)
	//	{
	//		// Returns only SQL_SUCCESS, SQL_ERROR, or SQL_INVALID_HANDLE.
	//		ret = SQLFreeHandle(SQL_HANDLE_ENV, m_henv);
	//		if (ret == SQL_ERROR)
	//		{
	//			// if SQL_ERROR is returned, the handle is still valid, error information can be fetched
	//			SqlResultException ex(L"SQLFreeHandle", ret, SQL_HANDLE_ENV, m_henv, L"Freeing ODBC-Environment Handle failed with SQL_ERROR, handle is still valid. Are all Connection-handles freed?");
	//			SET_EXCEPTION_SOURCE(ex);
	//			throw ex;
	//		}
	//		else if (ret == SQL_INVALID_HANDLE)
	//		{
	//			// If we've received INVALID_HANDLE our handle has probably already be deleted - anyway, its invalid, reset it.
	//			m_henv = SQL_NULL_HENV;
	//			SqlResultException ex(L"SQLFreeHandle", ret, L"Freeing ODBC-Env Handle failed with SQL_INVALID_HANDLE.");
	//			SET_EXCEPTION_SOURCE(ex);
	//			throw ex;
	//		}
	//		// We have SUCCESS
	//		m_henv = SQL_NULL_HENV;
	//	}
	//}


	void Environment::SetOdbcVersion(OdbcVersion version)
	{
		exASSERT(HasEnvironmentHandle());

		// Remember: because the SQLPOINTER is interpreted as an int value.. for int-attrs..
		SQLRETURN ret;
		switch(version)
		{
		case OdbcVersion::V_2:
			ret = SQLSetEnvAttr(m_pHenv->GetHandle(), SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC2, NULL);
			break;
		case OdbcVersion::V_3:
			ret = SQLSetEnvAttr(m_pHenv->GetHandle(), SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, NULL);
			break;
		case OdbcVersion::V_3_8:
			ret = SQLSetEnvAttr(m_pHenv->GetHandle(), SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3_80, NULL);
			break;
		default:
			THROW_WITH_SOURCE(IllegalArgumentException, (boost::wformat(L"Unknown ODBC Version value: %d") % (int) version).str());
		}

		THROW_IFN_SUCCEEDED_MSG(SQLSetEnvAttr, ret, SQL_HANDLE_ENV, m_pHenv->GetHandle(), (boost::wformat(L"Failed to set SQL_ATTR_ODBC_VERSION to value %d") % (int) version).str());

		ReadOdbcVersion();
	}


	OdbcVersion Environment::ReadOdbcVersion() const
	{
		exASSERT(HasEnvironmentHandle());

		unsigned long value = 0;
		SQLRETURN ret = SQLGetEnvAttr(m_pHenv->GetHandle(), SQL_ATTR_ODBC_VERSION, &value, NULL, NULL);

		THROW_IFN_SUCCEEDED_MSG(SQLGetEnvAttr, ret, SQL_HANDLE_ENV, m_pHenv->GetHandle(), L"Failed to read SQL_ATTR_ODBC_VERSION");

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
		if (m_odbcVersion == OdbcVersion::UNKNOWN && m_pHenv && m_pHenv->IsAllocated())
		{
			ReadOdbcVersion();
		}
		return m_odbcVersion;
	}


	DataSourcesVector Environment::ListDataSources(ListMode mode) const
	{
		exASSERT(m_pHenv);
		exASSERT(m_pHenv->IsAllocated());

		SQLSMALLINT nameBufferLength, descBufferLength = 0;
		SQLWCHAR nameBuffer[SQL_MAX_DSN_LENGTH + 1];

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
		ret = SQLDataSources(m_pHenv->GetHandle(), direction, nameBuffer, SQL_MAX_DSN_LENGTH + 1, &nameBufferLength, NULL, NULL, &descBufferLength);
		if(ret == SQL_NO_DATA)
		{
			// no data at all, but succeeded, else we would have an err-state
			return dataSources;
		}
		THROW_IFN_SUCCEEDED(SQLDataSources, ret, SQL_HANDLE_ENV, m_pHenv->GetHandle());

		do
		{
			// Remember the max length 
			if (descBufferLength > maxDescLength)
			{
				maxDescLength = descBufferLength;
			}
			// Go on fetching lengths of descriptions
			ret = SQLDataSources(m_pHenv->GetHandle(), SQL_FETCH_NEXT, nameBuffer, SQL_MAX_DSN_LENGTH + 1, &nameBufferLength, NULL, NULL, &descBufferLength);
		}while(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
		THROW_IFN_NO_DATA(SQLDataSources, ret);

		// Now fetch with description
		std::unique_ptr<SQLWCHAR[]> descBuffer(new SQLWCHAR[maxDescLength + 1]);
		ret = SQLDataSources(m_pHenv->GetHandle(), direction, nameBuffer, SQL_MAX_DSN_LENGTH + 1, &nameBufferLength, descBuffer.get(), maxDescLength + 1, &descBufferLength);
		if(ret == SQL_NO_DATA)
		{
			SqlResultException ex(L"SQLDataSources", ret, L"SQL_NO_DATA is not expected to happen here - we've found records in the previous round, they can't be gone now!");
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}
		do
		{
			// Store dataSource
			SDataSource ds;
			ds.m_dsn = nameBuffer;
			ds.m_description = descBuffer.get();
			dataSources.push_back(ds);
			ret = SQLDataSources(m_pHenv->GetHandle(), SQL_FETCH_NEXT, nameBuffer, SQL_MAX_DSN_LENGTH + 1, &nameBufferLength, descBuffer.get(), maxDescLength + 1, &descBufferLength);
		}while(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);

		THROW_IFN_NO_DATA(SQLDataSources, ret);

		return dataSources;
	}

	// Interfaces
	// ----------

}
