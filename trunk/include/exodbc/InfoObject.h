/*!
* \file InfoObject.h
* \author Elias Gerber <eg@elisium.ch>
* \date 14.06.2014
* \brief Header file for info objects.
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "SqlHandle.h"

// Other headers

// System headers
#include <string>
#include <vector>
#include <map>
#include <set>

// Forward declarations
// --------------------

namespace exodbc
{
	/*!
	* \class	SpecialColumnInfo
	* \brief	Information about a special column fetched using the catalog function SQLSpecialColumns.
	* \see: https://msdn.microsoft.com/en-us/library/ms714602%28v=vs.85%29.aspx
	*
	*/
	class EXODBCAPI SpecialColumnInfo
	{
	public:
		SpecialColumnInfo()
			: m_hasScope(false)
			, m_pseudoColumn(PseudoColumn::UNKNOWN)
		{};

		SpecialColumnInfo(const std::string& columnName, RowIdScope scope, SQLSMALLINT sqlType, const std::string& sqlTypeName,
			SQLINTEGER columnSize, SQLINTEGER bufferLength, SQLSMALLINT decimalDigits, PseudoColumn pseudoColumn)
			: m_columnName(columnName)
			, m_scope(scope)
			, m_hasScope(true)
			, m_sqlType(sqlType)
			, m_sqlTypeName(sqlTypeName)
			, m_columnSize(columnSize)
			, m_bufferLength(bufferLength)
			, m_decimalDigits(decimalDigits)
			, m_pseudoColumn(pseudoColumn)
		{};

		SpecialColumnInfo(const std::string& columnName, SQLSMALLINT sqlType, const std::string& sqlTypeName,
			SQLINTEGER columnSize, SQLINTEGER bufferLength, SQLSMALLINT decimalDigits, PseudoColumn pseudoColumn)
			: m_columnName(columnName)
			, m_hasScope(false)
			, m_sqlType(sqlType)
			, m_sqlTypeName(sqlTypeName)
			, m_columnSize(columnSize)
			, m_bufferLength(bufferLength)
			, m_decimalDigits(decimalDigits)
			, m_pseudoColumn(pseudoColumn)
		{};


		std::string GetColumnName() const noexcept { return m_columnName; };
		RowIdScope GetScope() const { exASSERT(m_hasScope); return m_scope; };
		SQLSMALLINT GetSqlType() const noexcept { return m_sqlType; };
		std::string GetSqlTypeName() const noexcept { return m_sqlTypeName; };
		SQLINTEGER GetColumnSize() const noexcept { return m_columnSize; };
		SQLINTEGER GetBufferLength() const noexcept { return m_bufferLength; };
		SQLSMALLINT GetDecimalDigits() const noexcept { return m_decimalDigits; };
		PseudoColumn GetPseudoColumn() const noexcept { return m_pseudoColumn; };

	private:
		RowIdScope	m_scope;
		bool		m_hasScope;

		std::string m_columnName;
		SQLSMALLINT m_sqlType;
		std::string m_sqlTypeName;
		SQLINTEGER m_columnSize;
		SQLINTEGER m_bufferLength;
		SQLSMALLINT m_decimalDigits;
		PseudoColumn m_pseudoColumn;
	};

	/*!
	* \typedef SpecialColumnInfosVector
	* \brief std::vector of SpecialColumnInfo objects.
	*/
	typedef std::vector<SpecialColumnInfo> SpecialColumnInfosVector;


	/*!
	* \struct SDataSource
	* \brief Contains information about a DataSource-Entry from the driver-manager
	* \see Environment::ListDataSources
	*/
	struct EXODBCAPI SDataSource
	{
		std::string m_dsn;			///< DSN name.
		std::string m_description;	///< Description.
	};

	/*!
	* \typedef DataSourcesVector
	* \brief std::vector of SDataSource objects.
	*/
	typedef std::vector<SDataSource> DataSourcesVector;


	/*!
	* \struct	STablePrivilegesInfo
	* \brief	TablePrivileges fetched using the catalog function SQLTablePrivilege
	*/
	struct EXODBCAPI STablePrivilegesInfo
	{
		std::string	m_catalogName;
		std::string	m_schemaName;
		std::string	m_tableName;
		std::string	m_grantor;
		std::string	m_grantee;
		std::string	m_privilege;
		std::string	m_grantable;

		bool			m_isCatalogNull;
		bool			m_isSchemaNull;
		bool			m_isGrantorNull;
		bool			m_isGrantableNull;
	};

	/*!
	* \typedef TablePrivilegesVector
	* \brief std::vector of STablePrivilegesInfo objects.
	*/
	typedef std::vector<STablePrivilegesInfo> TablePrivilegesVector;


	/*!
	* \struct SColumnDescription
	* \brief	Result of SQLDescribeCol operation.
	*/
	struct SColumnDescription
	{
		SColumnDescription()
			: m_sqlType(SQL_UNKNOWN_TYPE)
			, m_charSize(0)
			, m_decimalDigits(0)
			, m_nullable(SQL_NULLABLE_UNKNOWN)
		{};
		SColumnDescription(SQLSMALLINT paramSqlType, SQLULEN paramCharSize, SQLSMALLINT paramDecimalDigits, SQLSMALLINT paramNullable)
			: m_sqlType(paramSqlType)
			, m_charSize(paramCharSize)
			, m_decimalDigits(paramDecimalDigits)
			, m_nullable(paramNullable)
		{};
		SColumnDescription(SQLSMALLINT paramSqlType, SQLULEN paramCharSize, SQLSMALLINT paramDecimalDigits)
			: m_sqlType(paramSqlType)
			, m_charSize(paramCharSize)
			, m_decimalDigits(paramDecimalDigits)
			, m_nullable(SQL_NULLABLE_UNKNOWN)
		{};

		std::string m_name;
		SQLSMALLINT m_sqlType;
		SQLULEN m_charSize;
		SQLSMALLINT m_decimalDigits;
		SQLSMALLINT m_nullable;
	};


	/*!
	* \struct SParameterDescription
	* \brief	Result of SQLDescribeParam operation.
	*/
	struct SParameterDescription
	{
		SParameterDescription()
			: m_sqlType(SQL_UNKNOWN_TYPE)
			, m_charSize(0)
			, m_decimalDigits(0)
			, m_nullable(SQL_NULLABLE_UNKNOWN)
		{};
		SParameterDescription(SQLSMALLINT paramSqlType, SQLULEN paramCharSize, SQLSMALLINT paramDecimalDigits, SQLSMALLINT paramNullable)
			: m_sqlType(paramSqlType)
			, m_charSize(paramCharSize)
			, m_decimalDigits(paramDecimalDigits)
			, m_nullable(paramNullable)
		{};
		SParameterDescription(SQLSMALLINT paramSqlType, SQLULEN paramCharSize, SQLSMALLINT paramDecimalDigits)
			: m_sqlType(paramSqlType)
			, m_charSize(paramCharSize)
			, m_decimalDigits(paramDecimalDigits)
			, m_nullable(SQL_NULLABLE_UNKNOWN)
		{};

		SQLSMALLINT m_sqlType;
		SQLULEN m_charSize;
		SQLSMALLINT m_decimalDigits;
		SQLSMALLINT m_nullable;
	};
}
