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
#include "SqlHandle.h"
#include "SqlInfoProperty.h"

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
	* \details	A SpecialColumnInfo is either part of an optimal set of columns to identify
	*			a row, or a set of columns that are updated whenever any value in the row changes.\n
	* \see	SpecialColumnInfo::IdentifierType
	* \see https://msdn.microsoft.com/en-us/library/ms714602%28v=vs.85%29.aspx
	*
	*/
	class EXODBCAPI SpecialColumnInfo
	{
	public:
		/*!
		* \enum		IdentifierType
		* \brief	Attribute values to query special columns: Type of special columns to query: Either columns
		*			to match a single row, or columns that are updated whenever any value in the row changes.
		*/
		enum class IdentifierType
		{
			UNIQUE_ROW = SQL_BEST_ROWID,	///< Optimal set to identify a row uniquely. Can also be pseudo-columns like ROWID
			ROW_VERSION = SQL_ROWVER	///< Set of columns that are updated automatically if any value in the row changes.
		};


		/*!
		* \enum		RowIdScope
		* \brief	Attribute values to query special columns: Scope of row-id values. Can only be applied if 
		*			SpecialColumnInfo::IdentifierType was set to UNIQUE_ROW while reading the special columns.
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


		/*!
		* \brief Default constructor, all members are set to empty  or 0 values, null flags are set to true.
		*/
		SpecialColumnInfo()
			: m_isScopeNull(true)
			, m_pseudoColumn(PseudoColumn::UNKNOWN)
			, m_sqlType(0)
			, m_columnSize(0)
			, m_bufferLength(0)
			, m_decimalDigits(0)
		{};


		/*!
		* \brief Constructor to use when special columns have been queried using IdentifierType::UNIQUE_ROW.
		*	Sets null flag for Scope to false and sets internal IdentifierType to UNIQUE_ROW.
		*/
		SpecialColumnInfo(RowIdScope scope, const std::string& columnName, SQLSMALLINT sqlType, const std::string& sqlTypeName,
			SQLINTEGER columnSize, SQLINTEGER bufferLength, SQLSMALLINT decimalDigits, PseudoColumn pseudoColumn)
			: m_isScopeNull(true)
			, m_identType(IdentifierType::UNIQUE_ROW)
			, m_columnName(columnName)
			, m_scope(scope)
			, m_sqlType(sqlType)
			, m_sqlTypeName(sqlTypeName)
			, m_columnSize(columnSize)
			, m_bufferLength(bufferLength)
			, m_decimalDigits(decimalDigits)
			, m_pseudoColumn(pseudoColumn)
		{};


		/*!
		* \brief Constructor to use when special columns have been queried using IdentifierType::ROW_VERSION.
		*		Sets null flag for Scope to true and sets internal IdentifierType to ROW_VERSION.
		*/
		SpecialColumnInfo(const std::string& columnName, SQLSMALLINT sqlType, const std::string& sqlTypeName,
			SQLINTEGER columnSize, SQLINTEGER bufferLength, SQLSMALLINT decimalDigits, PseudoColumn pseudoColumn)
			: m_isScopeNull(true)
			, m_identType(IdentifierType::ROW_VERSION)
			, m_columnName(columnName)
			, m_sqlType(sqlType)
			, m_sqlTypeName(sqlTypeName)
			, m_columnSize(columnSize)
			, m_bufferLength(bufferLength)
			, m_decimalDigits(decimalDigits)
			, m_pseudoColumn(pseudoColumn)
		{};


		/*!
		* \brief Create from a statement that is assumed to hold the results of SQLSpecialColumns. The cursor must
		*		be positioned at the row and is not modified, but column values are read.
		* \throw Exception If reading any value fails, or if props does not hold all required properties.
		*/
		SpecialColumnInfo(ConstSqlStmtHandlePtr pStmt, const SqlInfoProperties& props, IdentifierType identType);


		/*!
		* \return Column name. Empty value might be returned.
		*/
		std::string GetColumnName() const noexcept { return m_columnName; };


		/*!
		* \return Scope.
		* \throw Exception if null flag for Scope is true.
		*/
		RowIdScope GetScope() const { exASSERT(!m_isScopeNull); return m_scope; };


		/*!
		* \return SQL Type.
		*/
		SQLSMALLINT GetSqlType() const noexcept { return m_sqlType; };


		/*!
		* \return SQL Type name. Empty value might be returned.
		*/
		std::string GetSqlTypeName() const noexcept { return m_sqlTypeName; };


		/*!
		* \return Column size.
		*/
		SQLINTEGER GetColumnSize() const noexcept { return m_columnSize; };


		/*!
		* \return Buffer length.
		*/
		SQLINTEGER GetBufferLength() const noexcept { return m_bufferLength; };


		/*!
		* \return Decimal Digits.
		*/
		SQLSMALLINT GetDecimalDigits() const noexcept { return m_decimalDigits; };


		/*!
		* \return Pseudo Column.
		*/
		PseudoColumn GetPseudoColumn() const noexcept { return m_pseudoColumn; };

	private:
		IdentifierType m_identType;
		RowIdScope	m_scope;
		bool		m_isScopeNull;

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
