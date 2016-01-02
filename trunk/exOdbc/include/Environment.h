/*!
 * \file Environment.h
 * \author Elias Gerber <eg@elisium.ch>
 * \date 25.07.2014
 * \brief Header file for the Environment class and its helpers.
 * \copyright GNU Lesser General Public License Version 3
*/ 

#pragma once

// Same component headers
#include "exOdbc.h"
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
	* This class will manage the ODBC Environment Handle that is required
	* for all later operations in a SqlEnvHandlePtr. The shared_ptr will be valid
	* during object lifetime as the SqlEnvHandle is allocated during construction.
	* The Connection handle itself will be created during Init().
	* 
	*/
	class EXODBCAPI Environment
	{
	public:
		/*!
		 * \brief	Default constructor.
		 * 			You must manually call Init() after creating the object.
		 *			Sets ODBC Version to OdbcVersion::UNKNOWN.
		 * \see		Init()
		 * \throw	std::bad_alloc If creation of SqlEnvHandlePtr fails.
		 */
		Environment() noexcept;


		/*!
		 * \brief	Constructor. Calls Init() with passed odbcVersion.
		 *			After Construction the ODBC Environment Handle is allocated
		 *			and OdbcVersion is set to passed odbcVersion.
		 *
		 * \param	odbcVersion	The ODBC version to set.
		 * \throw	std::bad_alloc If creation of SqlEnvHandlePtr fails.
		 * \throw	Exception If Init() fails.
		 */
		Environment(OdbcVersion odbcVersion);


		/*!
		* \brief	Copy Constructor.
		* \details	If the passed Environment has a valid ODBC Version set, the environment
		*			will be created with the same version by calling Init().
		* \param	other Environment to copy.
		* \throw	Exception If Init() fails.
		*/
		Environment(const Environment& other);


		/*!
		 * \brief		Destructor. 
		 */
		~Environment();


		/*!
		* \brief	Must be called if Environment has been created using Default Constructor.
		* \details	Can only be called once. Allocates the ODBC Environment handle and sets the
		*			passed OdbcVersion.
		* \param	odbcVersion The ODBC Version to set on this Environment.
		* \see		IsEnvHandleAllocated()
		* \throw	Exception
		*/
		void Init(OdbcVersion odbcVersion);


		/*!
		 * \brief	Returns true if the SqlEnvHandle has an allocated ODBC Environment handle.
		 */
		bool			IsEnvHandleAllocated() const { exASSERT(m_pHEnv); return m_pHEnv->IsAllocated(); };


		/*!
		* \brief	Returns the shared_ptr to the SqlEnvHandle holding the ODBC Environment handle.
		*			Note that this does not guarantee that the SqlEnvHandle returned actually has 
		*			Environment Handle allocated or that the pointer is not NULL.
		*/
		ConstSqlEnvHandlePtr	GetSqlEnvHandle() const noexcept { return m_pHEnv; };

		
		/*!
		 * \brief	Sets ODBC version. Updates internally cached value by calling
		 *			ReadOdbcVersion() if setting seemed to be successfull.
		 *
		 * \param	version	The version.
		 * \throw	Exception if no ODBC Environment Handle is allocated, or setting the version fails.
		 */
		void			SetOdbcVersion(OdbcVersion version);		


		/*!
		 * \brief	Reads the ODBC version from the environment and updates internally
		 *			cached value.
		 * 
		 * \return	The ODBC version or OV_UNKNOWN if read version is unknown.
		 * \throw	Exception If no ODBC Environment Handle is allocated, or reading the version fails.
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
		 *				Fails if no ODBC Environment Handle is allocated.
		 * \param	mode	Decide to list all DSNs, or only user / system DSNs.
		 * \return	Found Data Source Names.
		 * \throw	Exception
		 */		
		DataSourcesVector ListDataSources(ListMode mode) const;


	private:		
		// Members
		// -------
		SqlEnvHandlePtr m_pHEnv;	///< Environment handle
		mutable OdbcVersion m_odbcVersion; ///< Cached ODBC version

	};  // class Environment

	typedef std::shared_ptr<Environment> EnvironmentPtr;
	typedef std::shared_ptr<const Environment> ConstEnvironmentPtr;
}

