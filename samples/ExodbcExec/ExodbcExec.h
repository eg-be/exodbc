/*!
* \file OdbcExec.h
* \author Elias Gerber <eg@elisium.ch>
* \date 31.03.2017
* \brief Header file for odbcexec
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
// Other headers
#include "exodbc/Database.h"

// System headers
#include <string>
#include <vector>

// Forward declarations
// --------------------

namespace exodbcexec
{
	// Typedefs
	// --------

	// Structs
	// -------

	// Classes
	// -------

	/*!
	* \class ExodbcExec
	* \brief An e
	*/
	class ExodbcExec
	{
	public:
		ExodbcExec(exodbc::DatabasePtr pDb);

		void Run(const CommandGenerator& comGen);

	private:
		exodbc::DatabasePtr m_pDb;
	};
}
