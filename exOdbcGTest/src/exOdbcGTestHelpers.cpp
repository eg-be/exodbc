/*!
* \file exOdbcGTestHelpers.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 22.11.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "exOdbcGTestHelpers.h"

// Same component headers
// Other headers


// Debug
#include "DebugNew.h"

namespace exodbctest
{
	exodbc::EnvironmentPtr CreateEnv(exodbc::OdbcVersion odbcVersion /* = exodbc::OdbcVersion::V_3 */)
	{
		exodbc::EnvironmentPtr pEnv = std::make_shared<exodbc::Environment>(odbcVersion);
		return pEnv;
	}


	exodbc::DatabasePtr OpenTestDb(exodbc::ConstEnvironmentPtr pEnv)
	{
		exodbc::DatabasePtr pDb = std::make_shared<exodbc::Database>(pEnv);
		
		if (exodbc::g_odbcInfo.HasConnectionString())
		{
			pDb->Open(exodbc::g_odbcInfo.m_connectionString);
		}
		else
		{
			pDb->Open(exodbc::g_odbcInfo.m_dsn, exodbc::g_odbcInfo.m_username, exodbc::g_odbcInfo.m_password);
		}
		return pDb;
	}


	exodbc::DatabasePtr OpenTestDb(exodbc::OdbcVersion odbcVersion /* = exodbc::OdbcVersion::V_3 */)
	{
		exodbc::EnvironmentPtr pEnv = CreateEnv(odbcVersion);
		return OpenTestDb(pEnv);
	}

	// Static consts
	// -------------

	// Construction
	// -------------

	// Destructor
	// -----------

	// Implementation
	// --------------



	// Interfaces
	// ----------

} // namespace exodbc