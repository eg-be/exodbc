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
		* \struct SDataSource
		* \brief Contains information about a DataSource-Entry from the driver-manager
		* \see Environment::ListDataSources
		*/
		struct SDataSource
		{
			std::string m_dsn;			///< DSN name.
			std::string m_description;	///< Description.
		};


		/*!
		* \typedef DataSourceVector
		* \brief std::vector of SDataSource objects.
		*/
		typedef std::vector<SDataSource> DataSourceVector;


		/*!
		* \brief	Enable connection pooling on the driver manager.
		* \details	This option must be set before the Environment is allocated
		*			to enable it on further Environments.
		* \see		SqlConnect https://msdn.microsoft.com/en-us/library/ms711810%28v=vs.85%29.aspx
		* \throw	SqlResultException if SetSetEnvAttr does not succeed.
		*/
		static void EnableConnectionPooling(ConnectionPooling enablePooling);


		/*!
		* \brief	Set Attribute SQL_ATTR_TRACEFILE to passed path.
		* \throw	Exception
		*/
		static void SetTracefile(const std::string& path);
		

		/*!
		* \brief	Get Attribute SQL_ATTR_TRACEFILE.
		* \throw	Exception
		*/
		static std::string GetTracefile();


		/*!
		* \brief	Set Attribute SQL_ATTR_TRACE to SQL_OPT_TRACE_ON or 
		*			SQL_OPT_TRACE_OFF, depending on enable.
		* \details	Note that the trace option does not depend on any handle,
		*			it will be activated globally for the running application.
		* \throw	Exception
		*/
		static void SetTrace(bool enable);


		/*!
		* \brief	Get Attribute SQL_ATTR_TRACE.
		* \throw	Exception
		*/
		static bool GetTrace();


		/*!
		 * \brief	Default constructor.
		 * 			You must manually call Init() after creating the object.
		 *			Sets ODBC Version to OdbcVersion::UNKNOWN.
		 * \see		Init()
		 * \throw	std::bad_alloc If creation of SqlEnvHandlePtr fails.
		 */
		Environment();


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
		* \brief	Create new Environment using passed ODBC version. Created Environment
		*			is wrapped into a shared_ptr.
		*/
		static std::shared_ptr<Environment> Create(OdbcVersion odbcVersion);


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

		/*!
		* \enum		ListMode
		* \brief	What data sources to list in ListDataSources()
		*/
		enum class ListMode 
		{ 
			All,	///< System and User entries.
			System, //< Only system entries.
			User	//< Only user entries.
		};

		/*!
		 * \brief		List data sources.
		 * \details	List the Data Source Names (DSN) entries available.
		 *				Fails if no ODBC Environment Handle is allocated.
		 * \param	mode	Decide to list all DSNs, or only user / system DSNs.
		 * \return	Found Data Source Names.
		 * \throw	Exception
		 */		
		DataSourceVector ListDataSources(ListMode mode) const;


		/*!
		* \brief	Set if connection opened from this environment must
		*			match strict or not if connection pooling is enabled.
		* \see		EnableConnectionPooling.
		* \throw	Exception If no handle is allocated, or setting the attribute fails.
		*/
		void SetConnctionPoolingMatch(ConnectionPoolingMatch matchMode);

	private:		
		// Members
		// -------
		SqlEnvHandlePtr m_pHEnv;	///< Environment handle
		mutable OdbcVersion m_odbcVersion; ///< Cached ODBC version

	};  // class Environment

	typedef std::shared_ptr<Environment> EnvironmentPtr;
	typedef std::shared_ptr<const Environment> ConstEnvironmentPtr;
}

