/*!
* \file SqlInfo.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 12.04.2017
* \brief Source file for SqlInfo.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "SqlInfo.h"

// Same component headers
// Other headers
// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	// Construction
	// ------------


	// Destructor
	// -----------


	// Implementation
	// --------------
	SqlInfo::ValueType SqlInfo::GetValueType() const noexcept
	{
		switch (m_value.which())
		{
		case 0:
			return ValueType::USmallInt;
		case 1:
			return ValueType::UInt;
		default:
			return ValueType::String;
		}
	}
}
