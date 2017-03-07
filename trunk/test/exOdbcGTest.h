/*!
* \file exOdbcGTest.h
* \author Elias Gerber <eg@elisium.ch>
* \date 22.07.2014
* \copyright GNU Lesser General Public License Version 3
* 
* [Brief Header-file description]
*/ 

#pragma once

// Some compiler tuning for different platforms
// --------------------------------------------
#ifdef _WIN32
	#include <SDKDDKVer.h>
#endif

// Same component headers
#include "TestParams.h"

// Other headers
// System headers
#include <vector>

// Forward declarations
// --------------------

// Globals
// -------
namespace exodbctest
{
	extern TestParams g_odbcInfo;

	// Structs
	// -------

	// Classes
	// -------

} // namespace exodbctest
