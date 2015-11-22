#pragma once
/*!
* \file exOdbcGTestHelpers.h
* \author Elias Gerber <eg@elisium.ch>
* \date 22.11.2015
* \copyright GNU Lesser General Public License Version 3
*
* A file for helpers common to all tests. But not as complicated
* as the last stuff, that was pretty useless.
*/

#pragma once

// Same component headers
#include "exOdbcGTest.h"

// Other headers
#include "Environment.h"
#include "Database.h"

// System headers

// Forward declarations
// --------------------

namespace exodbctest
{
	extern exodbc::EnvironmentPtr CreateEnv(exodbc::OdbcVersion odbcVersion = exodbc::OdbcVersion::V_3);
	extern exodbc::DatabasePtr OpenTestDb(exodbc::ConstEnvironmentPtr pEnv);
	extern exodbc::DatabasePtr OpenTestDb(exodbc::OdbcVersion odbcVersion = exodbc::OdbcVersion::V_3);

	// Structs
	// -------

	// Classes
	// -------

} // namespace exodbc

