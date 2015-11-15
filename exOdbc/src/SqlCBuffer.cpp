/*!
* \file SqlCBuffer.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 15.11.2015
* \brief Source file for SqlCBuffer.
* \copyright GNU Lesser General Public License Version 3
*
*/

#include "stdafx.h"

// Own header
#include "SqlCBuffer.h"

// Same component headers
// Other headers

// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{

	SqlCBufferVariant CreateBuffer(SQLSMALLINT sqlCType)
	{
		switch (sqlCType)
		{
		case SQL_C_USHORT:
			return SqlUShortBuffer();
		case SQL_C_SHORT:
			return SqlShortBuffer();
		case SQL_C_ULONG:
			return SqlULongBuffer();
		case SQL_C_SLONG:
			return SqlLongBuffer();
		case SQL_C_UBIGINT:
			return SqlUBigIntBuffer();
		case SQL_C_SBIGINT:
			return SqlBigIntBuffer();
		default:
			NotSupportedException nse(NotSupportedException::Type::SQL_C_TYPE, sqlCType);
			SET_EXCEPTION_SOURCE(nse);
			throw nse;
		}
	}

	// Interfaces
	// ----------
}
