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
	private:
		bool freeHenvOnDestroy;
		bool useConnectionStr;

	public:
		HENV Henv;
		wchar_t Dsn[SQL_MAX_DSN_LENGTH+1];                  // Data Source Name
		wchar_t Uid[SQL_MAX_USER_NAME_LEN+1];               // User ID
		wchar_t AuthStr[SQL_MAX_AUTHSTR_LEN+1];             // Authorization string (password)
		wchar_t ConnectionStr[SQL_MAX_CONNECTSTR_LEN+1];    // Connection string (password)

		std::wstring Description;                              // Not sure what the max length is
		std::wstring FileType;                                 // Not sure what the max length is

		// Optionals needed for some databases like dBase
		std::wstring DefaultDir;                               // Directory that db file resides in

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
		const HENV       &GetHenv()          { return Henv; }

		const wchar_t    *GetDsn()           { return Dsn; }

		const wchar_t    *GetUid()           { return Uid; }
		const wchar_t    *GetUserID()        { return Uid; }

		const wchar_t    *GetAuthStr()       { return AuthStr; }
		const wchar_t    *GetPassword()      { return AuthStr; }

		const wchar_t    *GetConnectionStr() { return ConnectionStr; }
		bool             UseConnectionStr() { return useConnectionStr; }

		const wchar_t    *GetDescription()   { return Description.c_str(); }
		const wchar_t    *GetFileType()      { return FileType.c_str(); }
		const wchar_t    *GetDefaultDir()    { return DefaultDir.c_str(); }

		void             SetHenv(const HENV henv)               { Henv = henv; }

		void             SetDsn(const std::wstring &dsn);

		void             SetUserID(const std::wstring &userID);
		void             SetUid(const std::wstring &uid)            { SetUserID(uid); }

		void             SetPassword(const std::wstring &password);
		void             SetAuthStr(const std::wstring &authstr)    { SetPassword(authstr); }

		void             SetConnectionStr(const std::wstring &connectStr);

		void             SetDescription(const std::wstring &desc)   { Description   = desc;     }
		void             SetFileType(const std::wstring &fileType)  { FileType      = fileType; }
		void             SetDefaultDir(const std::wstring &defDir)  { DefaultDir    = defDir;   }

		bool			SetSqlAttrOdbcVersion(int version);		
		int				ReadSqlAttrOdbcVersion();
	};  // class wxDbConnectInf
}


#endif // DBENVIRONMENT_H
