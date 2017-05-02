﻿/*!
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
		, m_isCatalogNull(false)
		, m_isSchemaNull(false)
		, m_isColumnSizeNull(false)
		, m_isBufferSizeNull(false)
		, m_isDecimalDigitsNull(false)
		, m_isNumPrecRadixNull(false)
		, m_isRemarksNull(false)
		, m_isDefaultValueNull(false)
		, m_isSqlDatetimeSubNull(false)
		, m_isCharOctetLengthNull(false)
		, m_isIsNullableNull(false)
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
	{
		exASSERT(!m_columnName.empty());
	}

	std::string ColumnInfo::GetQueryName() const noexcept
	{
		// When querying, we use only the Column-name
		return GetPureName();
	}


	std::string ColumnInfo::GetPureName() const noexcept
	{
		return m_columnName;
	}
}