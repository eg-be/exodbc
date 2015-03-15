/*!
 * \file Environment.h
 * \author Elias Gerber <eg@zame.ch>
 * \date 25.07.2014
 * \brief Header file for the Environment class and its helpers.
 * \copyright wxWindows Library Licence, Version 3.1
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
		SQLWCHAR Dsn[SQL_MAX_DSN_LENGTH  + 1];

		std::wstring m_description;
	};


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
	*/
	class EXODBCAPI Environment
	{
	private:
		// Prevent copies. We would mess up the env-handle.
		Environment(const Environment& other) { exASSERT_MSG(false, L"Copy Constructor not Implemented"); };

	public:
		/*!
		 * \brief	Default constructor.
		 * 			You must manually call AllocHandle() and SetOdbcVersion() after 
		 * 			creating the object.
		 */
		Environment() throw();


		/*!
		 * \brief	Constructor. Tries to alloc the env-handle and to set the ODBC-version.
		 *
		 * \param	odbcVersion	The ODBC version.
		 * \throw	Exception
		 */
		Environment(OdbcVersion odbcVersion);


		/*!
		 * \brief		Destructor. Tries to free the env-handle, if one is allocated.
		 * \detailed	If freeing the handle fails an error is logged, but no Exception
		 *				is being thrown. You will leak handles in this case.
		 */
		~Environment();


		/*!
		 * \brief	Initializes this object.
		 * 			Set all members to 0.
		 * 			Cannot be called if a Henv is allocated.
		 *
		 * \throw	Exception If Henv is already allocated.
		 */
		void			Initialize();


		/*!
		 * \brief	Tries to allocate a new environment handle to be used by this Environment.
		 * 			Cannot be called if a Henv is allocated.
		 *	\throw	Exception If Henv is already allocated or Allocating fails.
		 */
		void			AllocateHenv();


		/*!
		 * \brief	Tries to free an allocated Henv.
		 * 			Can only be called if a Henv is allocated.
		 * \throw	Exception if no Henv is allocated
		 */
		void			FreeHenv();


		/*!
		 * \brief	Returns true is a Henv is allocated.
		 */
		bool			HasHenv() const	{ return m_henv != SQL_NULL_HENV; };


		/*!
		* \brief	Returns the Environment handle.
		* \throw	Exeption if no Henv is allocated.
		*/
		const SQLHENV&		GetHenv() const		{ exASSERT(HasHenv());  return m_henv; };

		
		/*!
		 * \brief	Sets ODBC version.
		 *
		 * \param	version	The version.
		 * \throw	Exception if no Henv is allocated, or setting the version fails.
		 */
		void			SetOdbcVersion(OdbcVersion version);		


		/*!
		 * \brief	Reads the ODBC version from the environment.
		 *
		 * \return	The ODBC version or OV_UNKNOWN if read version is unknown.
		 * \throw	Exception If no Henv is allocated, or reading the version fails.
		 */
		OdbcVersion		ReadOdbcVersion() const;


		enum ListMode { All, System, User };

		/*!
		 * \brief		List data sources.
		 * \detailed	List the Data Source Names (DSN) entries available.
		 *				Fails if no environment handle is allocated.
		 * \see			HasHenv()
		 * \param	mode	Decide to list all DSNs, or only user / system DSNs.
		 * \return	Found Data Source Names.
		 * \throw	Exception
		 */		
		std::vector<SDataSource> ListDataSources(ListMode mode) const;

	private:
		SQLHENV m_henv;
	};  // class Environment
}


#endif // Environment_H
