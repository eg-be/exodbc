/*!
* \file DbEnvironment.cpp
* \author Elias Gerber <egerber@gmx.net>
* \date 25.07.2014
* 
* [Brief CPP-file description]
*/ 

//#include "stdafx.h"

// Own header
#include "DbEnvironment.h"
// Same component headers
// Other headers

// Static consts
// -------------

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

		// If we initialize using odbc3 we will fail:  See Ticket # 17
		//if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &Henv) != SQL_SUCCESS)

		if (SQLAllocEnv(&m_henv) != SQL_SUCCESS)
		{
			BOOST_LOG_TRIVIAL(debug) << L"A problem occurred while trying to get a connection to the data source";
			return false;
		}

		m_freeHenvOnDestroy = true;

		return true;
	}  // wxDbConnectInf::AllocHenv()


	void DbEnvironment::FreeHenv()
	{
		exASSERT(m_henv);

		if (m_henv)
			SQLFreeEnv(m_henv);

		m_henv = 0;
		m_freeHenvOnDestroy = false;

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


	bool DbEnvironment::SetSqlAttrOdbcVersion(int version)
	{
		// TODO: This never worked. Its odbc 3. See Ticket # 17
		exASSERT(false);

		if( ! (version == SQL_OV_ODBC2 || version == SQL_OV_ODBC3 || version == SQL_OV_ODBC3_80))
		{
			return false;
		}
		SQLINTEGER v = version;
		SQLRETURN ret = SQLSetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION, &v, NULL);
		if(ret == SQL_SUCCESS)
			return true;
		SQLWCHAR sqlState[5 + 1];
		SQLINTEGER nativeErr;
		SQLWCHAR msg[256 + 1];
		SQLSMALLINT msgLength;
		ret = SQLGetDiagRec(SQL_HANDLE_ENV, m_henv, 1, sqlState, &nativeErr, msg, 256, &msgLength);
		return false;
	}


	int DbEnvironment::ReadSqlAttrOdbcVersion()
	{
		int value;
		SQLRETURN ret = SQLGetEnvAttr(m_henv, SQL_ATTR_ODBC_VERSION, &value, NULL, NULL);
		if(ret != SQL_SUCCESS)
			return 0;

		return value;
	}
	// Interfaces
	// ----------

}
