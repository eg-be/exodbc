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
		case SQL_C_SSHORT:
			return SqlSShortBuffer();
		case SQL_C_ULONG:
			return SqlULongBuffer();
		case SQL_C_SLONG:
			return SqlSLongBuffer();
		case SQL_C_UBIGINT:
			return SqlUBigIntBuffer();
		case SQL_C_SBIGINT:
			return SqlSBigIntBuffer();
		case SQL_C_TYPE_TIME:
			return SqlTypeTimeStructBuffer();
		case SQL_C_TIME:
			return SqlTimeStructBuffer();
		case SQL_C_TYPE_DATE:
			return SqlTypeDateStructBuffer();
		case SQL_C_DATE:
			return SqlDateStructBuffer();
		case SQL_C_TYPE_TIMESTAMP:
			return SqlTypeTimestampStructBuffer();
		case SQL_C_TIMESTAMP:
			return SqlTimeStructBuffer();
		case SQL_C_NUMERIC:
			return SqlNumericStructBuffer();
		case SQL_C_FLOAT:
			return SqlFloatBuffer();
		case SQL_C_DOUBLE:
			return SqlDoubleBuffer();
		default:
			NotSupportedException nse(NotSupportedException::Type::SQL_C_TYPE, sqlCType);
			SET_EXCEPTION_SOURCE(nse);
			throw nse;
		}
	}

	// Interfaces
	// ----------
}
