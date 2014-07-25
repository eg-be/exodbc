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
#include "wxOdbc3.h"
#include "Helpers.h"

// Other headers
// System headers
#include <windows.h>
#include <sql.h>
#include <sqlext.h>
#include <string>

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
	class WXDLLIMPEXP_ODBC DbEnvironment
	{
	public:

		DbEnvironment();
		DbEnvironment(HENV henv, const std::wstring &dsn, const std::wstring &userID = std::wstring(),
			const std::wstring &password = std::wstring(), const std::wstring &defaultDir = std::wstring(),
			const std::wstring &description = std::wstring(), const std::wstring &fileType = std::wstring());

		~DbEnvironment();

		bool             Initialize();

		bool             AllocHenv();
		void             FreeHenv();

		// Accessors
		const HENV       &GetHenv()          { return m_henv; }

		const wchar_t    *GetDsn()           { return m_dsn; }

		const wchar_t    *GetUid()           { return m_uid; }
		const wchar_t    *GetUserID()        { return m_uid; }

		const wchar_t    *GetAuthStr()       { return m_authStr; }
		const wchar_t    *GetPassword()      { return m_authStr; }

		const wchar_t    *GetConnectionStr() { return m_connectionStr; }
		bool             UseConnectionStr() { return m_useConnectionStr; }

		const wchar_t    *GetDescription()   { return m_description.c_str(); }
		const wchar_t    *GetFileType()      { return m_fileType.c_str(); }
		const wchar_t    *GetDefaultDir()    { return m_defaultDir.c_str(); }

		void             SetHenv(const HENV henv)               { m_henv = henv; }

		void             SetDsn(const std::wstring &dsn);

		void             SetUserID(const std::wstring &userID);
		void             SetUid(const std::wstring &uid)            { SetUserID(uid); }

		void             SetPassword(const std::wstring &password);
		void             SetAuthStr(const std::wstring &authstr)    { SetPassword(authstr); }

		void             SetConnectionStr(const std::wstring &connectStr);

		void             SetDescription(const std::wstring &desc)   { m_description   = desc;     }
		void             SetFileType(const std::wstring &fileType)  { m_fileType      = fileType; }
		void             SetDefaultDir(const std::wstring &defDir)  { m_defaultDir    = defDir;   }

		bool			SetSqlAttrOdbcVersion(int version);		
		int				ReadSqlAttrOdbcVersion();

	private:
		bool m_freeHenvOnDestroy;
		bool m_useConnectionStr;

		HENV m_henv;
		wchar_t m_dsn[SQL_MAX_DSN_LENGTH+1];                  // Data Source Name
		wchar_t m_uid[SQL_MAX_USER_NAME_LEN+1];               // User ID
		wchar_t m_authStr[SQL_MAX_AUTHSTR_LEN+1];             // Authorization string (password)
		wchar_t m_connectionStr[SQL_MAX_CONNECTSTR_LEN+1];    // Connection string (password)

		std::wstring m_description;                              // Not sure what the max length is
		std::wstring m_fileType;                                 // Not sure what the max length is

		// Optionals needed for some databases like dBase
		std::wstring m_defaultDir;                               // Directory that db file resides in


	};  // class wxDbConnectInf
}


#endif // DBENVIRONMENT_H
