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
		Henv = 0;
		freeHenvOnDestroy = false;

		Initialize();
	}  // Constructor

	// Destructor
	// -----------
	DbEnvironment::~DbEnvironment()
	{
		if (freeHenvOnDestroy)
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
		Henv = 0;
		freeHenvOnDestroy = false;

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
		freeHenvOnDestroy = false;

		if (freeHenvOnDestroy && Henv)
			FreeHenv();

		Henv = 0;
		Dsn[0] = 0;
		Uid[0] = 0;
		AuthStr[0] = 0;
		ConnectionStr[0] = 0;
		Description.empty();
		FileType.empty();
		DefaultDir.empty();

		useConnectionStr = false;

		return true;
	}  // wxDbConnectInf::Initialize()


	/********** wxDbConnectInf::AllocHenv() **********/
	bool DbEnvironment::AllocHenv()
	{
		// This is here to help trap if you are getting a new henv
		// without releasing an existing henv
		exASSERT(!Henv);

		// Initialize the ODBC Environment for Database Operations

		// If we initialize using odbc3 we will fail:  See Ticket # 17
		//if (SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &Henv) != SQL_SUCCESS)

		if (SQLAllocEnv(&Henv) != SQL_SUCCESS)
		{
			BOOST_LOG_TRIVIAL(debug) << L"A problem occurred while trying to get a connection to the data source";
			return false;
		}

		freeHenvOnDestroy = true;

		return true;
	}  // wxDbConnectInf::AllocHenv()


	void DbEnvironment::FreeHenv()
	{
		exASSERT(Henv);

		if (Henv)
			SQLFreeEnv(Henv);

		Henv = 0;
		freeHenvOnDestroy = false;

	}  // wxDbConnectInf::FreeHenv()


	void DbEnvironment::SetDsn(const std::wstring &dsn)
	{
		exASSERT(dsn.length() < EXSIZEOF(Dsn));

		wcsncpy(Dsn, dsn.c_str(), EXSIZEOF(Dsn) - 1);
		Dsn[EXSIZEOF(Dsn)-1] = 0;  // Prevent buffer overrun
	}  // wxDbConnectInf::SetDsn()


	void DbEnvironment::SetUserID(const std::wstring &uid)
	{
		exASSERT(uid.length() < EXSIZEOF(Uid));
		wcsncpy(Uid, uid.c_str(), EXSIZEOF(Uid)-1);
		Uid[EXSIZEOF(Uid)-1] = 0;  // Prevent buffer overrun
	}  // wxDbConnectInf::SetUserID()


	void DbEnvironment::SetPassword(const std::wstring &password)
	{
		exASSERT(password.length() < EXSIZEOF(AuthStr));

		wcsncpy(AuthStr, password.c_str(), EXSIZEOF(AuthStr)-1);
		AuthStr[EXSIZEOF(AuthStr)-1] = 0;  // Prevent buffer overrun
	}  // wxDbConnectInf::SetPassword()

	void DbEnvironment::SetConnectionStr(const std::wstring &connectStr)
	{
		exASSERT(connectStr.length() < EXSIZEOF(ConnectionStr));

		useConnectionStr = connectStr.length() > 0;

		wcsncpy(ConnectionStr, connectStr.c_str(), EXSIZEOF(ConnectionStr)-1);
		ConnectionStr[EXSIZEOF(ConnectionStr)-1] = 0;  // Prevent buffer overrun
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
		SQLRETURN ret = SQLSetEnvAttr(Henv, SQL_ATTR_ODBC_VERSION, &v, NULL);
		if(ret == SQL_SUCCESS)
			return true;
		SQLWCHAR sqlState[5 + 1];
		SQLINTEGER nativeErr;
		SQLWCHAR msg[256 + 1];
		SQLSMALLINT msgLength;
		ret = SQLGetDiagRec(SQL_HANDLE_ENV, Henv, 1, sqlState, &nativeErr, msg, 256, &msgLength);
		return false;
	}


	int DbEnvironment::ReadSqlAttrOdbcVersion()
	{
		int value;
		SQLRETURN ret = SQLGetEnvAttr(Henv, SQL_ATTR_ODBC_VERSION, &value, NULL, NULL);
		if(ret != SQL_SUCCESS)
			return 0;

		return value;
	}
	// Interfaces
	// ----------

}
