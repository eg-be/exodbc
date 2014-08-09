/*!
* \file DbEnvironment.h
* \author Elias Gerber <egerber@gmx.net>
* \date 25.07.2014
* 
* [Brief Header-file description]
*/ 

#pragma once
#ifndef DBENVIRONMENT_H
#define DBENVIRONMENT_H

// Same component headers
#include "exOdbc.h"
#include "Helpers.h"

// Other headers
// System headers
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>
#include <vector>

/* There are too many false positives for this one, particularly when using templates like wxVector<T> */
/* class 'foo' needs to have dll-interface to be used by clients of class 'bar'" */
#pragma warning(disable:4251)

#ifndef MAXNAME
#define MAXNAME         31
#endif

#ifndef SQL_MAX_AUTHSTR_LEN
// There does not seem to be a standard for this, so I am
// defaulting to the value that MS uses
#define SQL_MAX_AUTHSTR_LEN MAXNAME
#endif

#ifndef SQL_MAX_CONNECTSTR_LEN
// There does not seem to be a standard for this, so I am
// defaulting to the value that MS recommends
#define SQL_MAX_CONNECTSTR_LEN 1024
#endif

// Forward declarations
// --------------------

namespace exodbc
{
	// Structs
	// -------

	/*!
	* \brief Contains information about DataSource-Entries from the driver-manager
	* 
	* \see ListDataSources
	*/
	struct EXODBCAPI SDataSource
	{
		SDataSource() { Initialize(); };

		void Initialize() { Dsn[0] = 0; };
		wchar_t Dsn[SQL_MAX_DSN_LENGTH  + 1];

		std::wstring m_description;
	};

//	std::ostream& operator<< (std::ostream& stream, const SErrorInfo& ei) { stream << L"foo"; return stream; };

	// Classes
	// -------
	/*!
	* \brief Information about the data source we want to connect.
	* 
	* Stores information about the ODBC-Source we want to connect to, including
	* source-name, username and password, or alternatively, a connection-string.
	* 
	* This class will allocate the so called Environment-Handle that is required
	* for all later operations.
	*/
	class EXODBCAPI DbEnvironment
	{
	public:

		DbEnvironment();
		DbEnvironment(const std::wstring& dsn, const std::wstring& userID = std::wstring(), const std::wstring& password = std::wstring());
		DbEnvironment(const std::wstring& connectionString);

		~DbEnvironment();

		bool			Initialize();

		SErrorInfo		GetLastError();

		bool			AllocHenv();
		bool			FreeHenv();
		bool			HaveHenv()			{ return m_henv != NULL; };

		// Accessors
		const HENV&		GetHenv()          { return m_henv; }

		const wchar_t*	GetDsn()           { return m_dsn; }

		const wchar_t*	GetUid()           { return m_uid; }
		const wchar_t*	GetUserID()        { return m_uid; }

		const wchar_t*	GetAuthStr()       { return m_authStr; }
		const wchar_t*	GetPassword()      { return m_authStr; }

		const wchar_t*	GetConnectionStr() { return m_connectionStr; }
		bool			UseConnectionStr() { return m_useConnectionStr; }

		void			SetDsn(const std::wstring& dsn);
		void			SetUserID(const std::wstring& userID);
		void			SetPassword(const std::wstring &password);

		void			SetConnectionStr(const std::wstring &connectStr);

		bool			SetOdbcVersion(OdbcVersion version);		
		OdbcVersion		GetOdbcVersion();

		enum ListMode { All, System, User };
		std::vector<SDataSource> ListDataSources(ListMode mode = All);

	private:
		bool m_freeHenvOnDestroy;
		bool m_useConnectionStr;

		OdbcVersion m_requestedOdbcVersion;					// Sets to SQL_OV_ODBC2, SQL_OV_ODBC3 or SQL_OV_ODBC3_80, see AllocHenv

		HENV m_henv;
		wchar_t m_dsn[SQL_MAX_DSN_LENGTH+1];                  // Data Source Name
		wchar_t m_uid[SQL_MAX_USER_NAME_LEN+1];               // User ID
		wchar_t m_authStr[SQL_MAX_AUTHSTR_LEN+1];             // Authorization string (password)
		wchar_t m_connectionStr[SQL_MAX_CONNECTSTR_LEN+1];    // Connection string (password)

	};  // class DbEnvironment
}


#endif // DBENVIRONMENT_H
