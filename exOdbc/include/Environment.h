/*!
 * \file Environment.h
 * \author Elias Gerber <eg@elisium.ch>
 * \date 25.07.2014
 * \brief Header file for the Environment class and its helpers.
 * \copyright wxWindows Library Licence, Version 3.1
 * 
 * The Environment class corresponds to the wxDbConnectInf class
 * from wxWidgets 2.8.
*/ 

#pragma once
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

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

// \todo: remove once ticket #87 is done.
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
	// Test helpers:
#if EXODBC_TEST
		friend class EnvironmentTest;
		FRIEND_TEST(EnvironmentTest, FreeEnvironmentHandle);
#endif

	private:
		// Prevent copies. We would mess up the env-handle.
		Environment(const Environment& other) { exASSERT_MSG(false, L"Copy Constructor not Implemented"); };

	public:
		/*!
		 * \brief	Default constructor.
		 * 			You must manually call AllocateHandle() and SetOdbcVersion() after 
		 * 			creating the object.
		 * \see		AllocateEnvironmentHandle()
		 * \see		SetOdbcVersion()
		 */
		Environment() throw();


		/*!
		 * \brief	Constructor. Tries to alloc the environment handle and to set the ODBC-version.
		 *
		 * \param	odbcVersion	The ODBC version.
		 * \throw	Exception If AllocateEnvironmentHandle() or SetOdbcVersion() fails.
		 */
		Environment(OdbcVersion odbcVersion);


		/*!
		 * \brief		Destructor. Tries to free the env-handle, if one is allocated.
		 * \details	If freeing the handle fails an error is logged, but no Exception
		 *				is being thrown. You will leak handles in this case.
		 */
		~Environment();


		/*!
		 * \brief	Tries to allocate a new environment handle to be used by this Environment.
		 * 			Cannot be called if a Henv is allocated.
		 *	\throw	Exception If Henv is already allocated or Allocating fails.
		 */
		void			AllocateEnvironmentHandle();


		/*!
		 * \brief	Returns true is a Henv is allocated.
		 */
		bool			HasEnvironmentHandle() const	{ return m_henv != SQL_NULL_HENV; };


		/*!
		* \brief	Returns the Environment handle.
		* \throw	Exeption if no Henv is allocated.
		*/
		SQLHENV			GetEnvironmentHandle() const		{ exASSERT(HasEnvironmentHandle());  return m_henv; };

		
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


		enum class ListMode { All, System, User };

		/*!
		 * \brief		List data sources.
		 * \details	List the Data Source Names (DSN) entries available.
		 *				Fails if no environment handle is allocated.
		 * \see			HasHEnv()
		 * \param	mode	Decide to list all DSNs, or only user / system DSNs.
		 * \return	Found Data Source Names.
		 * \throw	Exception
		 */		
		DataSourcesVector ListDataSources(ListMode mode) const;


	private:
		/*!
		* \brief	Initializes this object.
		* 			Set all members to 0.
		* 			Cannot be called if a Henv is allocated.
		*
		* \throw	Exception If Henv is already allocated.
		*/
		void			Initialize();


		/*!
		* \brief	Tries to free an allocated Henv.
		* 			Can only be called if a Henv is allocated.
		* \throw	Exception if no Henv is allocated
		*/
		void			FreeEnvironmentHandle();

		SQLHENV m_henv;
	};  // class Environment
}


#endif // ENVIRONMENT_H
