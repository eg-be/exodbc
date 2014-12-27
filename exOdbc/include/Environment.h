/*!
 * \file Environment.h
 * \author Elias Gerber <eg@zame.ch>
 * \date 25.07.2014
 * \brief Header file for the Environment class and its helpers.
 * 
 * The Environment class corresponds to the wxDbConnectInf class
 * from wxWidgets 2.8.
*/ 

#pragma once
#ifndef Environment_H
#define Environment_H

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
	* \class SDataSource
	*
	* \brief Contains information about one DataSource-Entry from the driver-manager
	* 
	* \see Environment::ListDataSources
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
	* \class Environment
	*
	* \brief Represents the ODBC-Environment. Every Database needs an environment, 
	* but one environment can be used to open multiple databases.
	* 
	* This class will allocate the Environment-Handle that is required
	* for all later operations and sets the ODBC Version to 3.x or higher.
	* The handle is freed on destruction.
	*
	* The information needed to connect to a database (dsn, username, pw or 
	* connection-string) can be stored inside this object, but it is not required.
	* 
	*/
	class EXODBCAPI Environment
	{
	private:
		// Prevent copies. We would mess up the env-handle.
		Environment(const Environment& other) { exNOT_IMPL; };

	public:
		/*!
		 * \fn	Environment::Environment();
		 *
		 * \brief	Default constructor.
		 * 			You must manually call AllocHandle() and SetOdbcVersion() after 
		 * 			creating the object.
		 */
		Environment();

		/*!
		 * \fn	Environment::Environment(OdbcVersion odbcVersion);
		 *
		 * \brief	Constructor. Tries to alloc the env-handle and to set the ODBC-version.
		 *
		 * \param	odbcVersion	The ODBC version.
		 */
		Environment(OdbcVersion odbcVersion);

		/*!
		 * \fn	Environment::Environment(const std::wstring& dsn, const std::wstring& userID = std::wstring(), const std::wstring& password = std::wstring(), OdbcVersion odbcVersion = OV_3);
		 *
		 * \brief	Constructor. Tries to alloc the env-handle and to set the ODBC-version.
		 * 			The connection-information is set to not use a connection-string when connecting to the database later.
		 *
		 * \param	dsn		   	The dsn.
		 * \param	userID	   	(Optional) identifier for the user.
		 * \param	password   	(Optional) the password.
		 * \param	odbcVersion	(Optional) the ODBC version.
		 */
		Environment(const std::wstring& dsn, const std::wstring& userID = std::wstring(), const std::wstring& password = std::wstring(), OdbcVersion odbcVersion = OV_3);

		/*!
		 * \fn	Environment::Environment(const std::wstring& connectionString, OdbcVersion odbcVersion = OV_3);
		 *
		 * \brief	Constructor. Tries to alloc the env-handle and to set the ODBC-version.
		 * 			Connection-information is set to use a connection-string when connecting to the database laster.
		 *
		 * \param	connectionString	The connection string.
		 * \param	odbcVersion			(Optional) the ODBC version.
		 */
		Environment(const std::wstring& connectionString, OdbcVersion odbcVersion = OV_3);

		/*!
		 * \fn	Environment::~Environment();
		 *
		 * \brief	Destructor. Tries to free the env-handle, if one is allocated.
		 */
		~Environment();

		/*!
		 * \fn	bool Environment::Initialize();
		 *
		 * \brief	Initializes this object.
		 * 			Set all members to 0.
		 * 			Cannot be called if a Henv is allocated.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool			Initialize();

		/*!
		 * \brief	Tries to allocate a new Henv.
		 * 			Cannot be called if a Henv is allocated.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool			AllocHenv();

		/*!
		 * \brief	Tries to free an allocated Henv.
		 * 			Can only be called if a Henv is allocated.
		 * \return	true if it succeeds, false if it fails.
		 */
		bool			FreeHenv();

		/*!
		 * \brief	Returns true is a Henv is allocated.
		 * \return	Returns true is a Henv is allocated.
		 */
		bool			HasHenv() const	{ return m_henv != NULL; };

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
		 * \fn	void Environment::SetDsn(const std::wstring& dsn);
		 *
		 * \brief	Sets a dsn.
		 *
		 * \param	dsn	The dsn. Cannot be longer than SQL_MAX_DSN_LENGTH
		 */
		void			SetDsn(const std::wstring& dsn);

		/*!
		 * \fn	void Environment::SetUserID(const std::wstring& userID);
		 *
		 * \brief	Sets user identifier.
		 *
		 * \param	userID	Identifier for the user. Cannot be longer than SQL_MAX_USER_NAME_LEN
		 */
		void			SetUserID(const std::wstring& userID);

		/*!
		 * \fn	void Environment::SetPassword(const std::wstring &password);
		 *
		 * \brief	Sets a password.
		 *
		 * \param	password	The password. Cannot be longer than SQL_MAX_AUTHSTR_LEN
		 */
		void			SetPassword(const std::wstring &password);

		/*!
		 * \fn	void Environment::SetConnectionStr(const std::wstring &connectStr);
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
		 * \fn	bool Environment::SetOdbcVersion(OdbcVersion version);
		 *
		 * \brief	Sets ODBC version.
		 *
		 * \param	version	The version.
		 *
		 * \return	true if it succeeds, false if it fails.
		 */
		bool			SetOdbcVersion(OdbcVersion version);		

		/*!
		 * \fn	OdbcVersion Environment::ReadOdbcVersion();
		 *
		 * \brief	Gets ODBC version.
		 *
		 * \return	The ODBC version or OV_UNKNOWN if reading the version fails.
		 */
		OdbcVersion		ReadOdbcVersion() const;

		enum ListMode { All, System, User };

		/*!
		 * \fn	bool Environment::ListDataSources(ListMode mode = All, std::vector<SDataSource>& dataSources) const;
		 *
		 * \brief	List data sources.
		 *
		 * \param	mode			   	Decide to list all DSNs, or only user / system DSNs.
		 * \param [in,out]	dataSources	The data sources.
		 *
		 * \return	true if succeeds, false if fails
		 */		
		bool ListDataSources(ListMode mode, std::vector<SDataSource>& dataSources) const;

	private:
		bool m_useConnectionStr;

		HENV m_henv;
		wchar_t m_dsn[SQL_MAX_DSN_LENGTH+1];                  // Data Source Name
		wchar_t m_uid[SQL_MAX_USER_NAME_LEN+1];               // User ID
		wchar_t m_authStr[SQL_MAX_AUTHSTR_LEN+1];             // Authorization string (password)
		wchar_t m_connectionStr[SQL_MAX_CONNECTSTR_LEN+1];    // Connection string (password)

	};  // class Environment
}


#endif // Environment_H
