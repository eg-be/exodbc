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

	SqlCBufferVariant CreateBuffer(SQLSMALLINT sqlCType, const ColumnBindInfo& bindInfo)
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
		case SQL_C_DOUBLE:
			return SqlDoubleBuffer();
		case SQL_C_FLOAT:
			return SqlRealBuffer();
		default:
			NotSupportedException nse(NotSupportedException::Type::SQL_C_TYPE, sqlCType);
			SET_EXCEPTION_SOURCE(nse);
			throw nse;
		}
	}


	SqlCBufferVariant CreateArrayBuffer(SQLSMALLINT sqlCType, const ColumnInfo& columnInfo)
	{
		exASSERT(IsArrayType(sqlCType));

		switch (sqlCType)
		{
		case SQL_C_CHAR:
		case SQL_C_WCHAR:
		{
			SQLINTEGER columnSize = columnInfo.IsColumnSizeNull() ? 0 : columnInfo.GetColumnSize();
			SQLSMALLINT numPrecRadix = columnInfo.IsNumPrecRadixNull() ? 0 : columnInfo.GetNumPrecRadix();
			SQLSMALLINT decimalDigits = columnInfo.IsDecimalDigitsNull() ? 0 : columnInfo.GetDecimalDigits();
			SQLLEN arraySize = CalculateDisplaySize(sqlCType, columnSize, numPrecRadix, decimalDigits);
			if (sqlCType == SQL_C_CHAR)
			{
				return SqlCharArray(arraySize);
			}
			else
			{
				return SqlWCharArray(arraySize);
			}
			break;
		}
		case SQL_C_BINARY:
			exASSERT(!columnInfo.IsColumnSizeNull());
			return SqlBinaryArray(columnInfo.GetColumnSize());
		default:
			NotSupportedException nse(NotSupportedException::Type::SQL_C_TYPE, sqlCType);
			SET_EXCEPTION_SOURCE(nse);
			throw nse;
		}
	}


	bool IsArrayType(SQLSMALLINT sqlCType)
	{
		return sqlCType == SQL_C_WCHAR || sqlCType == SQL_C_CHAR || sqlCType == SQL_C_BINARY;
	}


	SQLLEN CalculateDisplaySize(SQLSMALLINT sqlType, SQLINTEGER columnSize, SQLSMALLINT numPrecRadix, SQLSMALLINT decimalDigits)
	{
		// we calculate this from DisplaySize
		// see here: https://msdn.microsoft.com/en-us/library/ms713974%28v=vs.85%29.aspx
		// for an explanation

		switch (sqlType)
		{
		case SQL_CHAR:
		case SQL_WCHAR:
		case SQL_VARCHAR:
		case SQL_WVARCHAR:
			// add one for trailing '0'
			exASSERT(columnSize != SQL_NO_TOTAL);
			exASSERT(columnSize > 0);
			return (columnSize + 1);
		case SQL_NUMERIC:
		case SQL_DECIMAL:
			exASSERT(numPrecRadix == 10);
			exASSERT(columnSize > 0);
			return columnSize + 2 + 1;
		case SQL_BIT:
			return 1 + 1;
		case SQL_TINYINT:
			return 4 + 1;
		case SQL_SMALLINT:
			return 6 + 1;
		case SQL_INTEGER:
			return 11 + 1;
		case SQL_BIGINT:
			return 20 + 1;
		case SQL_REAL:
			return 14 + 1;
		case SQL_FLOAT:
		case SQL_DOUBLE:
			return 24 + 1;
		case SQL_BINARY:
			exASSERT(columnSize > 0);
			return columnSize * 2 + 1;
		case SQL_TYPE_DATE:
		case SQL_DATE:
			return 11 + 1;
		case SQL_TYPE_TIME:
		case SQL_TIME:
			exASSERT(decimalDigits >= 0);
			if (decimalDigits == 0)
			{
				return 8 + 1;
			}
			else
			{
				return 9 + decimalDigits + 1;
			}
		case SQL_TYPE_TIMESTAMP:
		case SQL_TIMESTAMP:
			exASSERT(decimalDigits >= 0);
			if (decimalDigits == 0)
			{
				return 19 + 1;
			}
			else
			{
				return 20 + decimalDigits + 1;
			}
		case SQL_GUID:
			return 36 + 1;

		default:
			NotSupportedException nse(NotSupportedException::Type::SQL_TYPE, sqlType);
			SET_EXCEPTION_SOURCE(nse);
			throw nse;
		}
	}

	// Interfaces
	// ----------
}
