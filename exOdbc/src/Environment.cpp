/*!
* \file Environment.cpp
* \author Elias Gerber <eg@zame.ch>
* \date 25.07.2014
* \brief Source file for the Environment class and its helpers.
* \copyright wxWindows Library Licence, Version 3.1
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
		: m_henv(SQL_NULL_HENV)
	{
		// Note: Init will set members to NULL, but asserts if m_henv is set
		Initialize();
	}
	
	
	Environment::Environment(OdbcVersion odbcVersion)
		: m_henv(SQL_NULL_HENV)
	{
		// Note: Init will set members to NULL, but asserts if m_henv is set
		Initialize();
		AllocateEnvironmentHandle();
		SetOdbcVersion(odbcVersion);
	} 


	// Destructor
	// -----------
	Environment::~Environment()
	{
		// Do not throw from here
		try
		{
			if (HasEnvironmentHandle())
			{
				FreeEnvironmentHandle();
			}
		}
		catch (Exception ex)
		{
			LOG_ERROR(ex.ToString());
		}
	}


	// Implementation
	// --------------
	void Environment::Initialize()
	{
		exASSERT(!HasEnvironmentHandle());
		
		m_henv = SQL_NULL_HENV;
	}


	void Environment::AllocateEnvironmentHandle()
	{
		// This is here to help trap if you are getting a new henv
		// without releasing an existing henv
		exASSERT(!HasEnvironmentHandle());

		// Initialize the ODBC Environment for Database Operations
		SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_henv);
		if (!SQL_SUCCEEDED(ret))
		{
			SqlResultException ex(L"SQLAllocHandle", ret, L"Failed to allocated ODBC-Env Handle");
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}
	}


	void Environment::FreeEnvironmentHandle()
	{
		exASSERT(m_henv);

		SQLRETURN ret = SQL_SUCCESS;

		if (SQL_NULL_HENV != m_henv)
		{
			// Returns only SQL_SUCCESS, SQL_ERROR, or SQL_INVALID_HANDLE.
			ret = SQLFreeHandle(SQL_HANDLE_ENV, m_henv);
			if (ret == SQL_ERROR)
			{
				// if SQL_ERROR is returned, the handle is still valid, error information can be fetched
				SqlResultException ex(L"SQLFreeHandle", ret, SQL_HANDLE_ENV, m_henv, L"Freeing ODBC-Environment Handle failed with SQL_ERROR, handle is still valid. Are all Connection-handles freed?");
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
			else if (ret == SQL_INVALID_HANDLE)
			{
				// If we've received INVALID_HANDLE our handle has probably already be deleted - anyway, its invalid, reset it.
				m_henv = SQL_NULL_HENV;
				SqlResultException ex(L"SQLFreeHandle", ret, L"Freeing ODBC-Env Handle failed with SQL_INVALID_HANDLE.");
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
			// We have SUCCESS
			m_henv = SQL_NULL_HENV;
		}
	}


	void Environment::SetOdbcVersion(OdbcVersion version)
	{
		exASSERT(HasEnvironmentHandle());

		// Remember: because the SQLPOINTER is interpreted as an int value.. for int-attrs..
		SQLRETURN ret;
		switch(version)
		{
		case OdbcVersion::V_2:
			ret = SQLSetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC2, NULL);
			break;
		case OdbcVersion::V_3:
			ret = SQLSetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, NULL);
			break;
		case OdbcVersion::V_3_8:
			ret = SQLSetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3_80, NULL);
			break;
		default:
			THROW_WITH_SOURCE(IllegalArgumentException, (boost::wformat(L"Unknown ODBC Version value: %d") % (int) version).str());
		}

		THROW_IFN_SUCCEEDED_MSG(SQLSetEnvAttr, ret, SQL_HANDLE_ENV, m_henv, (boost::wformat(L"Failed to set SQL_ATTR_ODBC_VERSION to value %d") % (int) version).str());
	}


	OdbcVersion Environment::ReadOdbcVersion() const
	{
		unsigned long value = 0;
		SQLRETURN ret = SQLGetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION, &value, NULL, NULL);

		THROW_IFN_SUCCEEDED_MSG(SQLGetEnvAttr, ret, SQL_HANDLE_ENV, m_henv, L"Failed to read SQL_ATTR_ODBC_VERSION");

		switch(value)
		{
		case SQL_OV_ODBC2:
			return OdbcVersion::V_2;
		case SQL_OV_ODBC3:
			return OdbcVersion::V_3;
		case SQL_OV_ODBC3_80:
			return OdbcVersion::V_3_8;
		}

		return OdbcVersion::UNKNOWN;
	}


	std::vector<SDataSource> Environment::ListDataSources(ListMode mode) const
	{
		SQLSMALLINT nameBufferLength, descBufferLength = 0;
		SQLWCHAR nameBuffer[SQL_MAX_DSN_LENGTH + 1];

		// empty result-vector
		std::vector<SDataSource> dataSources;

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
		ret = SQLDataSources(m_henv, direction, nameBuffer, SQL_MAX_DSN_LENGTH + 1, &nameBufferLength, NULL, NULL, &descBufferLength);
		if(ret == SQL_NO_DATA)
		{
			// no data at all, but succeeded, else we would have an err-state
			return dataSources;
		}
		THROW_IFN_SUCCEEDED(SQLDataSources, ret, SQL_HANDLE_ENV, m_henv);

		do
		{
			// Remember the max length 
			if (descBufferLength > maxDescLength)
			{
				maxDescLength = descBufferLength;
			}
			// Go on fetching lengths of descriptions
			ret = SQLDataSources(m_henv, SQL_FETCH_NEXT, nameBuffer, SQL_MAX_DSN_LENGTH + 1, &nameBufferLength, NULL, NULL, &descBufferLength);
		}while(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);
		THROW_IFN_NO_DATA(SQLDataSources, ret);

		// Now fetch with description
		std::unique_ptr<SQLWCHAR[]> descBuffer(new SQLWCHAR[maxDescLength + 1]);
		ret = SQLDataSources(m_henv, direction, nameBuffer, SQL_MAX_DSN_LENGTH + 1, &nameBufferLength, descBuffer.get(), maxDescLength + 1, &descBufferLength);
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
			ret = SQLDataSources(m_henv, SQL_FETCH_NEXT, nameBuffer, SQL_MAX_DSN_LENGTH + 1, &nameBufferLength, descBuffer.get(), maxDescLength + 1, &descBufferLength);
		}while(ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO);

		THROW_IFN_NO_DATA(SQLDataSources, ret);

		return dataSources;
	}

	// Interfaces
	// ----------

}
