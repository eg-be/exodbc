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
	// -------------
	/********** wxDbConnectInf Constructor - form 1 **********/
	DbEnvironment::DbEnvironment()
	{
		m_henv = 0;
		m_freeHenvOnDestroy = false;

		Initialize();
	}  // Constructor

	// Destructor
	// -----------
	DbEnvironment::~DbEnvironment()
	{
		if (m_freeHenvOnDestroy)
		{
			FreeHenv();
		}
	}  // wxDbConnectInf Destructor

	// Implementation
	// --------------



	/********** wxDbConnectInf Constructor - form 2 **********/
	DbEnvironment::DbEnvironment(HENV henv, const std::wstring &dsn, const std::wstring &userID,
		const std::wstring &password, const std::wstring &defaultDir,
		const std::wstring &fileType, const std::wstring &description)
	{
		m_henv = 0;
		m_freeHenvOnDestroy = false;

		Initialize();

		if (henv)
			SetHenv(henv);
		else
			AllocHenv();

		SetDsn(dsn);
		SetUserID(userID);
		SetPassword(password);
		SetDescription(description);
		SetFileType(fileType);
		SetDefaultDir(defaultDir);
	}  // wxDbConnectInf Constructor



	/********** wxDbConnectInf::Initialize() **********/
	bool DbEnvironment::Initialize()
	{
		m_freeHenvOnDestroy = false;

		if (m_freeHenvOnDestroy && m_henv)
			FreeHenv();

		m_requestedOdbcVersion = OV_2;
		m_henv = 0;
		m_dsn[0] = 0;
		m_uid[0] = 0;
		m_authStr[0] = 0;
		m_connectionStr[0] = 0;
		m_description.empty();
		m_fileType.empty();
		m_defaultDir.empty();

		m_useConnectionStr = false;

		return true;
	}  // wxDbConnectInf::Initialize()


	/********** wxDbConnectInf::AllocHenv() **********/
	bool DbEnvironment::AllocHenv()
	{
		// This is here to help trap if you are getting a new henv
		// without releasing an existing henv
		exASSERT(!m_henv);

		// Initialize the ODBC Environment for Database Operations
		if(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_henv) != SQL_SUCCESS)
		{
			BOOST_LOG_TRIVIAL(debug) << L"Failed to allocate an odbc environment handle using SqlAllocHandle (Odbc v3.x): " << GetLastError();
			return false;
		}
		// I dont know why we cannot use the value stored in m_requestedOdbcVersion. It just works with the constants
		SQLRETURN ret;
		switch(m_requestedOdbcVersion)
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
			BOOST_LOG_TRIVIAL(debug) << L"Unknown ODBC Version value: " << m_requestedOdbcVersion;
			return false;
		}

		if(ret != SQL_SUCCESS)
		{
			BOOST_LOG_TRIVIAL(debug) << L"Failed to set ODBC Version of SQL Environment to " << m_requestedOdbcVersion << L": " << GetLastError();
			return false;
		}

		m_freeHenvOnDestroy = true;

		return true;
	}  // wxDbConnectInf::AllocHenv()


	bool DbEnvironment::FreeHenv()
	{
		exASSERT(m_henv);

		SQLRETURN ret = SQL_SUCCESS;

		if (m_henv)
		{
			ret = SQLFreeHandle(SQL_HANDLE_ENV, m_henv);
		}
		if(ret != SQL_SUCCESS)
		{
			BOOST_LOG_TRIVIAL(debug) << L"Failed to free env-handle for odbc-version " << m_requestedOdbcVersion << L": " << GetLastError();
		}

		m_henv = 0;
		m_freeHenvOnDestroy = false;

		return ret == SQL_SUCCESS;

	}  // wxDbConnectInf::FreeHenv()


	void DbEnvironment::SetDsn(const std::wstring &dsn)
	{
		exASSERT(dsn.length() < EXSIZEOF(m_dsn));

		wcsncpy(m_dsn, dsn.c_str(), EXSIZEOF(m_dsn) - 1);
		m_dsn[EXSIZEOF(m_dsn)-1] = 0;  // Prevent buffer overrun
	}  // wxDbConnectInf::SetDsn()


	void DbEnvironment::SetUserID(const std::wstring &uid)
	{
		exASSERT(uid.length() < EXSIZEOF(m_uid));
		wcsncpy(m_uid, uid.c_str(), EXSIZEOF(m_uid)-1);
		m_uid[EXSIZEOF(m_uid)-1] = 0;  // Prevent buffer overrun
	}  // wxDbConnectInf::SetUserID()


	void DbEnvironment::SetPassword(const std::wstring &password)
	{
		exASSERT(password.length() < EXSIZEOF(m_authStr));

		wcsncpy(m_authStr, password.c_str(), EXSIZEOF(m_authStr)-1);
		m_authStr[EXSIZEOF(m_authStr)-1] = 0;  // Prevent buffer overrun
	}  // wxDbConnectInf::SetPassword()

	void DbEnvironment::SetConnectionStr(const std::wstring &connectStr)
	{
		exASSERT(connectStr.length() < EXSIZEOF(m_connectionStr));

		m_useConnectionStr = connectStr.length() > 0;

		wcsncpy(m_connectionStr, connectStr.c_str(), EXSIZEOF(m_connectionStr)-1);
		m_connectionStr[EXSIZEOF(m_connectionStr)-1] = 0;  // Prevent buffer overrun
	}  // wxDbConnectInf::SetConnectionStr()


	bool DbEnvironment::SetOdbcVersion(OdbcVersion version)
	{
		// Must be set before we've allocated a handle
		exASSERT(m_henv == NULL);

		m_requestedOdbcVersion = version;
		return true;
	}


	SErrorInfo DbEnvironment::GetLastError()
	{
		SErrorInfo ei;
		ei.NativeError = 0;
		ei.SqlState[0] = 0;

		// Determine msg-length
		SQLSMALLINT msgLength;
		SQLRETURN ret = SQLGetDiagRec(SQL_HANDLE_ENV, m_henv, 1, ei.SqlState, &ei.NativeError, NULL, NULL, &msgLength);
		if(ret != SQL_SUCCESS)
		{
			BOOST_LOG_TRIVIAL(debug) << L"SQLGetDiagRec failed with return code " << ret;
			return ei;
		}
		SQLWCHAR* msg = new SQLWCHAR[msgLength + 1];
		ret = SQLGetDiagRec(SQL_HANDLE_ENV, m_henv, 1, ei.SqlState, &ei.NativeError, msg, msgLength + 1, &msgLength);
		if(ret != SQL_SUCCESS)
		{
			BOOST_LOG_TRIVIAL(debug) << L"SQLGetDiagRec failed with return code " << ret;
			return ei;
		}

		ei.Msg = msg;
		delete[] msg;
		return ei;
	}


	OdbcVersion DbEnvironment::GetOdbcVersion()
	{
		unsigned long value = 0;
		SQLRETURN ret = SQLGetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION, &value, NULL, NULL);
		if(ret != SQL_SUCCESS)
		{
			BOOST_LOG_TRIVIAL(debug) << L"Failed to read SQL_ATTR_ODBC_VERSION: " << GetLastError();
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

	vector<SDataSource> DbEnvironment::ListDataSources(ListMode mode /* = All */)
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
		if(res != SQL_NO_DATA)
		{
			BOOST_LOG_TRIVIAL(warning) << L"ListDataSources did not end with SQL_NO_DATA when determining max desc buffer length";
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
			BOOST_LOG_TRIVIAL(warning) << L"ListDataSources did not end with SQL_NO_DATA when listening DataSources";
		}

		delete[] descBuffer;
		return dataSources;
	}

	// Interfaces
	// ----------

}
