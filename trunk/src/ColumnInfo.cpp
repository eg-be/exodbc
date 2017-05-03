/*!
* \file ColumnInfo.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 01.05.2017
* \brief Source file for ColumnInfo.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "ColumnInfo.h"

// Same component headers
#include "AssertionException.h"
#include "Helpers.h"

// Other headers
// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace std;

namespace exodbc
{
	// Class ColumnInfo
	// ===============
	ColumnInfo::ColumnInfo()
		: m_sqlType(SQL_UNKNOWN_TYPE)
		, m_columnSize(0)
		, m_bufferSize(0)
		, m_decimalDigits(0)
		, m_numPrecRadix(0)
		, m_nullable(SQL_NULLABLE_UNKNOWN)
		, m_sqlDataType(SQL_UNKNOWN_TYPE)
		, m_sqlDatetimeSub(0)
		, m_charOctetLength(0)
		, m_ordinalPosition(0)
		, m_isCatalogNull(true)
		, m_isSchemaNull(true)
		, m_isColumnSizeNull(true)
		, m_isBufferSizeNull(true)
		, m_isDecimalDigitsNull(true)
		, m_isNumPrecRadixNull(true)
		, m_isRemarksNull(true)
		, m_isDefaultValueNull(true)
		, m_isSqlDatetimeSubNull(true)
		, m_isCharOctetLengthNull(true)
		, m_isIsNullableNull(true)
	{ }


	ColumnInfo::ColumnInfo(const std::string& catalogName, const std::string& schemaName, const std::string& tableName, const std::string& columnName, SQLSMALLINT sqlType,
		const std::string& typeName, SQLINTEGER columnSize, SQLINTEGER bufferSize, SQLSMALLINT decimalDigits, SQLSMALLINT numPrecRadix, SQLSMALLINT nullable,
		const std::string& remarks, const std::string& defaultValue, SQLSMALLINT sqlDataType, SQLSMALLINT sqlDatetimeSub, SQLINTEGER charOctetLength, SQLINTEGER ordinalPosition,
		const std::string& isNullable, bool isCatalogNull, bool isSchemaNull, bool isColumnSizeNull, bool isBufferSizeNull, bool isDecimalDigitsNull, bool isNumPrecRadixNull,
		bool isRemarksNull, bool isDefaultValueNull, bool isSqlDatetimeSubNull, bool isIsNullableNull)
		: m_catalogName(catalogName)
		, m_schemaName(schemaName)
		, m_tableName(tableName)
		, m_columnName(columnName)
		, m_sqlType(sqlType)
		, m_typeName(typeName)
		, m_columnSize(columnSize)
		, m_bufferSize(bufferSize)
		, m_decimalDigits(decimalDigits)
		, m_numPrecRadix(numPrecRadix)
		, m_nullable(nullable)
		, m_remarks(remarks)
		, m_defaultValue(defaultValue)
		, m_sqlDataType(sqlDataType)
		, m_sqlDatetimeSub(sqlDatetimeSub)
		, m_charOctetLength(charOctetLength)
		, m_ordinalPosition(ordinalPosition)
		, m_isNullable(isNullable)
		, m_isCatalogNull(isCatalogNull)
		, m_isSchemaNull(isSchemaNull)
		, m_isColumnSizeNull(isColumnSizeNull)
		, m_isBufferSizeNull(isBufferSizeNull)
		, m_isDecimalDigitsNull(isDecimalDigitsNull)
		, m_isNumPrecRadixNull(isNumPrecRadixNull)
		, m_isRemarksNull(isRemarksNull)
		, m_isDefaultValueNull(isDefaultValueNull)
		, m_isSqlDatetimeSubNull(isSqlDatetimeSubNull)
		, m_isIsNullableNull(isIsNullableNull)
	{ }


	ColumnInfo::ColumnInfo(ConstSqlStmtHandlePtr pStmt, const SqlInfoProperties& props)
	{
		exASSERT(pStmt);
		exASSERT(pStmt->IsAllocated());

		SQLLEN cb = 0;
		GetData(pStmt, 1, props.GetMaxCatalogNameLen(), m_catalogName, &m_isCatalogNull);
		GetData(pStmt, 2, props.GetMaxSchemaNameLen(), m_schemaName, &m_isSchemaNull);
		GetData(pStmt, 3, props.GetMaxTableNameLen(), m_tableName);
		GetData(pStmt, 4, props.GetMaxColumnNameLen(), m_columnName);
		GetData(pStmt, 5, SQL_C_SSHORT, &m_sqlType, sizeof(m_sqlType), &cb, nullptr);
		GetData(pStmt, 6, DB_MAX_TYPE_NAME_LEN, m_typeName);
		GetData(pStmt, 7, SQL_C_SLONG, &m_columnSize, sizeof(m_columnSize), &cb, &m_isColumnSizeNull);
		GetData(pStmt, 8, SQL_C_SLONG, &m_bufferSize, sizeof(m_bufferSize), &cb, &m_isBufferSizeNull);
		GetData(pStmt, 9, SQL_C_SSHORT, &m_decimalDigits, sizeof(m_decimalDigits), &cb, &m_isDecimalDigitsNull);
		GetData(pStmt, 10, SQL_C_SSHORT, &m_numPrecRadix, sizeof(m_numPrecRadix), &cb, &m_isNumPrecRadixNull);
		GetData(pStmt, 11, SQL_C_SSHORT, &m_nullable, sizeof(m_nullable), &cb, nullptr);
		GetData(pStmt, 12, DB_MAX_COLUMN_REMARKS_LEN, m_remarks, &m_isRemarksNull);
		GetData(pStmt, 13, DB_MAX_COLUMN_DEFAULT_LEN, m_defaultValue, &m_isDefaultValueNull);
		GetData(pStmt, 14, SQL_C_SSHORT, &m_sqlDataType, sizeof(m_sqlDataType), &cb, nullptr);
		GetData(pStmt, 15, SQL_C_SSHORT, &m_sqlDatetimeSub, sizeof(m_sqlDatetimeSub), &cb, &m_isSqlDatetimeSubNull);
		GetData(pStmt, 16, SQL_C_SLONG, &m_charOctetLength, sizeof(m_charOctetLength), &cb, &m_isCharOctetLengthNull);
		GetData(pStmt, 17, SQL_C_SLONG, &m_ordinalPosition, sizeof(m_ordinalPosition), &cb, nullptr);
		GetData(pStmt, 18, DB_MAX_YES_NO_LEN, m_isNullable, &m_isIsNullableNull);
	}


	std::string ColumnInfo::GetQueryName() const
	{
		// When querying, we use only the Column-name
		exASSERT(!m_columnName.empty());
		return m_columnName;
	}
}
