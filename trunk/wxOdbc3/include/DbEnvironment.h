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
	private:
		// Prevent copies. We would mess up the env-handle.
		DbEnvironment(const DbEnvironment& other) {};

	public:
		/*!
		 * \fn	DbEnvironment::DbEnvironment();
		 *
		 * \brief	Default constructor.
		 * 			You must manually call AllocHandle() and SetOdbcVersion() after 
		 * 			creating the object.
		 */
		DbEnvironment();

		/*!
		 * \fn	DbEnvironment::DbEnvironment(OdbcVersion odbcVersion);
		 *
		 * \brief	Constructor. Initializes the env-handle and sets the odbdc-version.
		 *
		 * \param	odbcVersion	The ODBC version.
		 */
		DbEnvironment(OdbcVersion odbcVersion);

		/*!
		 * \fn	DbEnvironment::DbEnvironment(const std::wstring& dsn, const std::wstring& userID = std::wstring(), const std::wstring& password = std::wstring(), OdbcVersion odbcVersion = OV_3);
		 *
		 * \brief	Constructor. Initializes the env-handle and sets the odbdc-version.
		 * 			The connection-information is set to not use a connection-string when connecting to the database later.
		 *
		 * \param	dsn		   	The dsn.
		 * \param	userID	   	(Optional) identifier for the user.
		 * \param	password   	(Optional) the password.
		 * \param	odbcVersion	(Optional) the ODBC version.
		 */
		DbEnvironment(const std::wstring& dsn, const std::wstring& userID = std::wstring(), const std::wstring& password = std::wstring(), OdbcVersion odbcVersion = OV_3);

		/*!
		 * \fn	DbEnvironment::DbEnvironment(const std::wstring& connectionString, OdbcVersion odbcVersion = OV_3);
		 *
		 * \brief	Constructor. Initializes the env-handle and sets the odbdc-version.
		 * 			Connectoin-information is set to use a connection-string when connecting to the database laster.
		 *
		 * \param	connectionString	The connection string.
		 * \param	odbcVersion			(Optional) the ODBC version.
		 */
		DbEnvironment(const std::wstring& connectionString, OdbcVersion odbcVersion = OV_3);

		/*!
		 * \fn	DbEnvironment::~DbEnvironment();
		 *
		 * \brief	Destructor. Tries to free the env-handle, if one is allocated.
		 */
		~DbEnvironment();

		/*!
		 * \fn	bool DbEnvironment::Initialize();
		 *
		 * \brief	Initializes this object.
		 * 			Set all members to 0.
		 * 			Cannot be called if a Henv is allocated.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool			Initialize();

		/*!
		 * \fn	bool DbEnvironment::AllocHenv();
		 *
		 * \brief	Tries to allocate a new Henv and set the ODBC-Version.
		 * 			Cannot be called if a Henv is allocated.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool			AllocHenv();

		/*!
		 * \fn	bool DbEnvironment::FreeHenv();
		 *
		 * \brief	Tries to free an allocated Henv.
		 * 			Can only be called if a Henv is allocated.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool			FreeHenv();

		/*!
		 * \fn	bool DbEnvironment::HaveHenv()
		 *
		 * \brief	Returns true is a Henv is allocated.
		 *
		 * \return	Returns true is a Henv is allocated.
		 */
		bool			HaveHenv() const	{ return m_henv != NULL; };

		// Accessors
		const HENV&		GetHenv() const		{ return m_henv; };

		const wchar_t*	GetDsn() const		{ return m_dsn; };

		const wchar_t*	GetUid() const		{ return m_uid; };
		const wchar_t*	GetUserID() const	{ return m_uid; }

		const wchar_t*	GetAuthStr() const	{ return m_authStr; }
		const wchar_t*	GetPassword() const	{ return m_authStr; }

		const wchar_t*	GetConnectionStr() const { return m_connectionStr; }
		bool			UseConnectionStr() const { return m_useConnectionStr; }

		/*!
		 * \fn	void DbEnvironment::SetDsn(const std::wstring& dsn);
		 *
		 * \brief	Sets a dsn.
		 *
		 * \param	dsn	The dsn. Cannot be longer than SQL_MAX_DSN_LENGTH
		 */
		void			SetDsn(const std::wstring& dsn);

		/*!
		 * \fn	void DbEnvironment::SetUserID(const std::wstring& userID);
		 *
		 * \brief	Sets user identifier.
		 *
		 * \param	userID	Identifier for the user. Cannot be longer than SQL_MAX_USER_NAME_LEN
		 */
		void			SetUserID(const std::wstring& userID);

		/*!
		 * \fn	void DbEnvironment::SetPassword(const std::wstring &password);
		 *
		 * \brief	Sets a password.
		 *
		 * \param	password	The password. Cannot be longer than SQL_MAX_AUTHSTR_LEN
		 */
		void			SetPassword(const std::wstring &password);

		/*!
		 * \fn	void DbEnvironment::SetConnectionStr(const std::wstring &connectStr);
		 *
		 * \brief	Sets connection string.
		 *
		 * \param	connectStr	The connect string. Cannot be longer than SQL_MAX_CONNECTSTR_LEN
		 * 						Notes the connection-information to use a connection-string when
		 * 						connecting to the database if called with a non-empty connection
		 * 						string. Removed if called with an empty connection string.
		 */
		void			SetConnectionStr(const std::wstring &connectStr);

		/*!
		 * \fn	bool DbEnvironment::SetOdbcVersion(OdbcVersion version);
		 *
		 * \brief	Sets ODBC version.
		 *
		 * \param	version	The version.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool			SetOdbcVersion(OdbcVersion version);		

		/*!
		 * \fn	OdbcVersion DbEnvironment::GetOdbcVersion();
		 *
		 * \brief	Gets ODBC version.
		 *
		 * \return	The ODBC version or OV_UNKNOWN if reading the version fails.
		 */
		OdbcVersion		GetOdbcVersion() const;

		enum ListMode { All, System, User };

		/*!
		 * \fn	std::vector<SDataSource> DbEnvironment::ListDataSources(ListMode mode = All);
		 *
		 * \brief	List data sources.
		 *
		 * \param	mode	(Optional) Decide to list all DSNs, or only user / system DSNs.
		 *
		 * \return	A std::vector&lt;SDataSource&gt;
		 */
		std::vector<SDataSource> ListDataSources(ListMode mode = All) const;

	private:
		bool m_freeHenvOnDestroy;
		bool m_useConnectionStr;

		HENV m_henv;
		wchar_t m_dsn[SQL_MAX_DSN_LENGTH+1];                  // Data Source Name
		wchar_t m_uid[SQL_MAX_USER_NAME_LEN+1];               // User ID
		wchar_t m_authStr[SQL_MAX_AUTHSTR_LEN+1];             // Authorization string (password)
		wchar_t m_connectionStr[SQL_MAX_CONNECTSTR_LEN+1];    // Connection string (password)

	};  // class DbEnvironment
}


#endif // DBENVIRONMENT_H
