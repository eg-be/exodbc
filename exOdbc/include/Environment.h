/*!
 * \file Environment.h
 * \author Elias Gerber <eg@elisium.ch>
 * \date 25.07.2014
 * \brief Header file for the Environment class and its helpers.
 * \copyright GNU Lesser General Public License Version 3
*/ 

#pragma once
#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

// Same component headers
#include "exOdbc.h"
#include "Helpers.h"
#include "InfoObject.h"
#include "SqlHandle.h"

// Other headers
// System headers
#include <string>
#include <vector>


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
		: std::enable_shared_from_this<Environment>
	{
	// Test helpers:
#if EXODBC_TEST
		FRIEND_TEST(EnvironmentTest, FreeEnvironmentHandle);
		FRIEND_TEST(EnvironmentTest, AllocateEnvironmentHandle);
#endif

	public:
		/*!
		 * \brief	Default constructor.
		 * 			You must manually call Init() after creating the object.
		 * \see		Init()
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
		* \brief	Copy Constructor.
		* \details	If the passed Environment has a valid ODBC-Version set, the environment
		*			will be created with the same version.
		* \param	odbcVersion	The ODBC version.
		* \throw	Exception If AllocateEnvironmentHandle() or SetOdbcVersion() fails.
		*/
		Environment(const Environment& other);


		/*!
		 * \brief		Destructor. Tries to free the env-handle, if one is allocated.
		 * \details	If freeing the handle fails an error is logged, but no Exception
		 *				is being thrown. You will leak handles in this case.
		 */
		~Environment();


		/*!
		* \brief	Must be called if Environment has been created using Default Constructor.
		* \details	Can only be called once. Allocates the Environment handle and sets the
		*			passed OdbcVersion.
		* \param	odbcVersion The ODBC Version to set on this Environment.
		* \see		HasEnvironmentHandle()
		* \throw	Exception
		*/
		void Init(OdbcVersion odbcVersion);


		/*!
		 * \brief	Returns true is a Henv is allocated.
		 * \details	The environment handle is allocated if the object was created using one
		 *			of the non default constructors, or Init() has been called.
		 */
		bool			HasEnvironmentHandle() const { exASSERT(m_pHEnv); return m_pHEnv->IsAllocated(); };


		/*!
		* \brief	Returns the Environment handle.
		* \throw	Exception if no Henv is allocated.
		*/
		ConstSqlEnvHandlePtr	GetEnvironmentHandle() const { exASSERT(HasEnvironmentHandle()); return m_pHEnv; };

		
		/*!
		 * \brief	Sets ODBC version. Updates internally cached value by calling
		 *			ReadOdbcVersion() if setting seemed to be successfull.
		 *
		 * \param	version	The version.
		 * \throw	Exception if no Henv is allocated, or setting the version fails.
		 */
		void			SetOdbcVersion(OdbcVersion version);		


		/*!
		 * \brief	Reads the ODBC version from the environment and updates internally
		 *			cached value.
		 * 
		 * \return	The ODBC version or OV_UNKNOWN if read version is unknown.
		 * \throw	Exception If no Henv is allocated, or reading the version fails.
		 */
		OdbcVersion		ReadOdbcVersion() const;


		/*!
		* \brief	Gets the cached ODBC version if the cached version is not OV_UNKNWON.
		*			If cached version is OV_UNKOWN and the environment handle is allocated,
		*			it will try to read the odbc version using ReadOdbcVersion().
		*
		* \return	The ODBC version or OV_UNKNOWN if the handle is not allocated yet.
		* \throw	Exception If reading the version fails.
		*/
		OdbcVersion		GetOdbcVersion() const;


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
		* \brief	Tries to allocate a new environment handle to be used by this Environment.
		* 			Cannot be called if a Henv is allocated.
		*	\throw	Exception If Henv is already allocated or Allocating fails.
		*/
		//void			AllocateEnvironmentHandle();


		/*!
		* \brief	Tries to free an allocated Henv.
		* 			Can only be called if a Henv is allocated.
		* \throw	Exception if no Henv is allocated
		*/
		//void			FreeEnvironmentHandle();

		// Members
		// -------
		SqlEnvHandlePtr m_pHEnv;
		//SQLHENV m_henv;	///< Environment handle
		mutable OdbcVersion m_odbcVersion; ///< Cached ODBC version

	};  // class Environment

	typedef std::shared_ptr<Environment> EnvironmentPtr;
	typedef std::shared_ptr<const Environment> ConstEnvironmentPtr;
}


#endif // ENVIRONMENT_H
