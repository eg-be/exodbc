/*!
* \file DbEnvironment.cpp
* \author Elias Gerber <egerber@gmx.net>
* \date 25.07.2014
* 
* [Brief CPP-file description]
*/ 

#include "stdafx.h"

// Own header
#include "DbEnvironment.h"

// Same component headers
// Other headers

// Static consts
// -------------

using namespace std;

namespace exodbc
{


	// Construction
	// ------------
	
	DbEnvironment::DbEnvironment()
		: m_henv(NULL)
		, m_freeHenvOnDestroy(false)
	{
		Initialize();
	}
	
	
	DbEnvironment::DbEnvironment(OdbcVersion odbcVersion)
		: m_henv(NULL)
		, m_freeHenvOnDestroy(false)
	{
		Initialize();
		AllocHenv();
		SetOdbcVersion(odbcVersion);
	} 

	DbEnvironment::DbEnvironment(const std::wstring& dsn, const std::wstring& userID, const std::wstring& password, OdbcVersion odbcVersion /* = OV_3 */ )
		: m_henv(NULL)
		, m_freeHenvOnDestroy(false)
	{
		Initialize();
		AllocHenv(); // note: might fail
		SetOdbcVersion(odbcVersion);

		SetDsn(dsn);
		SetUserID(userID);
		SetPassword(password);
	}

	DbEnvironment::DbEnvironment(const std::wstring& connectionString, OdbcVersion odbcVersion /* = OV_3 */ )
		: m_henv(NULL)
		, m_freeHenvOnDestroy(false)
	{
		m_henv = 0;
		m_freeHenvOnDestroy = false;

		Initialize();
		AllocHenv(); // note: might fail
		SetOdbcVersion(odbcVersion);

		SetConnectionStr(connectionString);
	}


	// Destructor
	// -----------
	DbEnvironment::~DbEnvironment()
	{
		if (m_freeHenvOnDestroy)
		{
			FreeHenv(); // note: might fail
		}
	}

	// Implementation
	// --------------
	bool DbEnvironment::Initialize()
	{
		exASSERT(m_henv == NULL);

		m_henv = NULL;
		m_freeHenvOnDestroy = false;

		m_henv = 0;
		m_dsn[0] = 0;
		m_uid[0] = 0;
		m_authStr[0] = 0;
		m_connectionStr[0] = 0;

		m_useConnectionStr = false;

		return true;
	}


	bool DbEnvironment::AllocHenv()
	{
		// This is here to help trap if you are getting a new henv
		// without releasing an existing henv
		exASSERT(!m_henv);

		// Initialize the ODBC Environment for Database Operations
		SQLRETURN ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_henv);
		if(ret != SQL_SUCCESS)
		{
			LOG_ERROR_ENV_MSG(m_henv, ret, SQLAllocHandle, L"Failed to allocated ODBC-Env Handle");
			return false;
		}

		m_freeHenvOnDestroy = true;

		return true;
	}


	bool DbEnvironment::FreeHenv()
	{
		exASSERT(m_henv);

		SQLRETURN ret = SQL_SUCCESS;

		if (m_henv)
		{
			ret = SQLFreeHandle(SQL_HANDLE_ENV, m_henv);
			if(ret != SQL_SUCCESS)
			{
				// if SQL_ERROR is returned, the handle is still valid, error information can be fetched
				if(ret == SQL_ERROR)
					LOG_ERROR_ENV_MSG(m_henv, ret, SQLFreeHandle, L"Freeing ODBC-Env Handle failed with SQL_ERROR, handle is still valid");
				else
					LOG_ERROR_ENV_MSG(m_henv, ret, SQLFreeHandle, L"Freeing ODBC-Env Handle failed, handle is invalid");
			}
			if(ret != SQL_ERROR)
			{
				m_freeHenvOnDestroy = false;
				m_henv = NULL;
			}
		}

		return ret == SQL_SUCCESS;
	}


	void DbEnvironment::SetDsn(const std::wstring &dsn)
	{
		exASSERT(dsn.length() < EXSIZEOF(m_dsn));

		wcsncpy(m_dsn, dsn.c_str(), EXSIZEOF(m_dsn) - 1);
		m_dsn[EXSIZEOF(m_dsn)-1] = 0;  // Prevent buffer overrun
	} 


	void DbEnvironment::SetUserID(const std::wstring &uid)
	{
		exASSERT(uid.length() < EXSIZEOF(m_uid));
		wcsncpy(m_uid, uid.c_str(), EXSIZEOF(m_uid)-1);
		m_uid[EXSIZEOF(m_uid)-1] = 0;  // Prevent buffer overrun
	}


	void DbEnvironment::SetPassword(const std::wstring &password)
	{
		exASSERT(password.length() < EXSIZEOF(m_authStr));

		wcsncpy(m_authStr, password.c_str(), EXSIZEOF(m_authStr)-1);
		m_authStr[EXSIZEOF(m_authStr)-1] = 0;  // Prevent buffer overrun
	}

	void DbEnvironment::SetConnectionStr(const std::wstring &connectStr)
	{
		exASSERT(connectStr.length() < EXSIZEOF(m_connectionStr));

		m_useConnectionStr = connectStr.length() > 0;

		wcsncpy(m_connectionStr, connectStr.c_str(), EXSIZEOF(m_connectionStr)-1);
		m_connectionStr[EXSIZEOF(m_connectionStr)-1] = 0;  // Prevent buffer overrun
	}


	bool DbEnvironment::SetOdbcVersion(OdbcVersion version)
	{
		exASSERT(m_henv);

		// I dont know why we cannot use the value stored in m_requestedOdbcVersion. It just works with the constants
		// because the SQLPOINTER is interpreted as an int value.. for int-attrs..
		SQLRETURN ret;
		switch(version)
		{
		case OV_2:
			ret = SQLSetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC2, NULL);
			break;
		case OV_3:
			ret = SQLSetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, NULL);
			break;
		case OV_3_8:
			ret = SQLSetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3_80, NULL);
			break;
		default:
			LOG_ERROR((boost::wformat(L"Unknown ODBC Version value: %d") %version).str());
			return false;
		}

		if(ret != SQL_SUCCESS)
		{
			LOG_ERROR_ENV_MSG(m_henv, ret, SQLSetEnvAttr, (boost::wformat(L"Failed to set SQL_ATTR_ODBC_VERSION to value %d") %version).str());
			return false;
		}


		return true;
	}


	OdbcVersion DbEnvironment::GetOdbcVersion() const
	{
		unsigned long value = 0;
		SQLRETURN ret = SQLGetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION, &value, NULL, NULL);
		if(ret != SQL_SUCCESS)
		{
			BOOST_LOG_TRIVIAL(debug) << L"Failed to read SQL_ATTR_ODBC_VERSION: " << GetLastEnvError(m_henv);
			return OV_UNKNOWN;
		}

		switch(value)
		{
		case SQL_OV_ODBC2:
			return OV_2;
		case SQL_OV_ODBC3:
			return OV_3;
		case SQL_OV_ODBC3_80:
			return OV_3_8;
		}

		return OV_UNKNOWN;
	}

	vector<SDataSource> DbEnvironment::ListDataSources(ListMode mode /* = All */) const
	{
		SQLSMALLINT nameBufferLength, descBufferLength = 0;
		wchar_t nameBuffer[SQL_MAX_DSN_LENGTH + 1];

		vector<SDataSource> dataSources;

		SQLUSMALLINT direction = SQL_FETCH_FIRST;
		if(mode == System)
			direction = SQL_FETCH_FIRST_SYSTEM;
		else if(mode == User)
			direction = SQL_FETCH_FIRST_USER;

		// We need two passed, I dont know the max length of the description
		// I also dont know if the order how they are returned is the same
		// Or while doing the two iterations, sources might get added / removed
		// So we remember the max length of the buffer, and use that in the second pass
		// Like that we might miss some parts of the descriptions.. 
		SQLSMALLINT maxDescLength = 0;
		SQLRETURN res = SQL_NO_DATA;
		res = SQLDataSources(m_henv, direction, nameBuffer, SQL_MAX_DSN_LENGTH + 1, &nameBufferLength, NULL, NULL, &descBufferLength);
		if(res == SQL_NO_DATA)
		{
			return dataSources;
		}
		do
		{
			// Remember the max length 
			if(descBufferLength > maxDescLength)
				maxDescLength = descBufferLength;
			// Go on fetching lengths of descriptions
			res = SQLDataSources(m_henv, SQL_FETCH_NEXT, nameBuffer, SQL_MAX_DSN_LENGTH + 1, &nameBufferLength, NULL, NULL, &descBufferLength);
		}while(res == SQL_SUCCESS || res == SQL_SUCCESS_WITH_INFO);
		if(res != SQL_NO_ACTION)
		{
			LOG_ERROR_EXPECTED_SQL_NO_DATA(res, SQLDataSources);
		}

		// Now fetch with description
		wchar_t* descBuffer = new wchar_t[maxDescLength + 1];
		res = SQLDataSources(m_henv, direction, nameBuffer, SQL_MAX_DSN_LENGTH + 1, &nameBufferLength, descBuffer, maxDescLength + 1, &descBufferLength);
		if(res == SQL_NO_DATA)
		{
			delete[] descBuffer;
			return dataSources;
		}
		do
		{
			// Store dataSource
			SDataSource ds;
			wcscpy(ds.Dsn, nameBuffer);
			ds.m_description = descBuffer;
			dataSources.push_back(ds);
			res = SQLDataSources(m_henv, SQL_FETCH_NEXT, nameBuffer, SQL_MAX_DSN_LENGTH + 1, &nameBufferLength, descBuffer, maxDescLength + 1, &descBufferLength);
		}while(res == SQL_SUCCESS || res == SQL_SUCCESS_WITH_INFO);
		if(res != SQL_NO_DATA)
		{
			LOG_ERROR_EXPECTED_SQL_NO_DATA(res, SQLDataSources);
		}

		delete[] descBuffer;
		return dataSources;
	}

	// Interfaces
	// ----------

}
