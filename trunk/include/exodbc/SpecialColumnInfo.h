/*!
* \file SpecialColumnInfo.h
* \author Elias Gerber <eg@elisium.ch>
* \date 01.05.2017
* \brief Header file for SpecialColumnInfo.
* \copyright GNU Lesser General Public License Version 3
*
*/

#pragma once

// Same component headers
#include "exOdbc.h"
#include "AssertionException.h"

// Other headers
// System headers
#include <string>
#include <vector>

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
		/*!
		* \enum		IdentifierType
		* \brief	Attribute values to query special columns: Type of special columns to query.
		*/
		enum class IdentifierType
		{
			UNIQUE_ROW = SQL_BEST_ROWID,	///< Optimal set to identify a row uniquely. Can also be pseudo-columns like ROWID
			ROW_VERSION = SQL_ROWVER	///< Set of columns that are updated automatically if any value in the row changes.
		};


		/*!
		* \enum		RowIdScope
		* \brief	Attribute values to query special columns: Scope of row-id values.
		*/
		enum class RowIdScope
		{
			CURSOR = SQL_SCOPE_CURROW,	///< Row id values are valid only while cursor is positioned on that row. Might change on later select of the same row.
			TRANSCATION = SQL_SCOPE_TRANSACTION, ///< Row id values are valid during ongoing transaction.
			SESSION = SQL_SCOPE_SESSION	///< Row id values are valid beyond transaction boundaries.
		};


		/*!
		* \enum		PseudoColumn
		* \brief	Information if a column is a pseudo column or not.
		*/
		enum class PseudoColumn
		{
			UNKNOWN = SQL_PC_UNKNOWN,	///< Not known
			NOT_PSEUDO = SQL_PC_NOT_PSEUDO,	///< no pseudo column
			PSEUDO = SQL_PC_PSEUDO	///< pseudo column, like Oracle ROWID
		};



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
	* \typedef SpecialColumnInfoVector
	* \brief std::vector of SpecialColumnInfo objects.
	*/
	typedef std::vector<SpecialColumnInfo> SpecialColumnInfoVector;
}
