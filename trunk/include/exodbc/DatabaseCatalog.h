/*!
 * \file DatabaseCatalog.h
 * \author Elias Gerber <eg@elisium.ch>
 * \date 19.04.2017
 * \brief Header file for the DatabaseCatalog class and its helpers.
 * \copyright GNU Lesser General Public License Version 3
 *
 */

#pragma once

// Same component headers
#include "exOdbc.h"
#include "SqlHandle.h"
#include "SqlInfoProperty.h"
#include "InfoObject.h"
// Other headers
// System headers


// Forward declarations
// --------------------
namespace exodbc
{
	/*!
	* \class DatabaseCatalog
	* \brief Provides access to the various catalog functions.
	* \details Internally, the class holds a statement handle to query the
	*			Database about tables, columns, etc. The class supports reading
	*			and changing the value of SQL_ATTR_METADATA_ID to work with
	*			search patterns or identifier strings. The default is to set
	*			SQL_ATTR_METADATA_ID to SQL_FALSE if it is not already SQL_FALSE,
	*			allowing to use pattern value arguments.
	*/
	class DatabaseCatalog
	{
	public:
		/*!
		* \enum MetadataMode
		* \brief Holds the value of SQL_ATTR_METADATA_ID for the statement used
		*		within the DatabaseCatalog class.
		*/
		enum class MetadataMode
		{
			PatternValue,	///< Set if SQL_ATTR_METADATA_ID is SQL_FALSE.
			Identifier		///< Set if SQL_ATTR_METADATA_ID is SQL_TRUE.
		};


		/*!
		* \brief Default constructor. Must call Init() later manually.
		*/
		DatabaseCatalog();


		/*!
		* \brief Construct DatabaseCatalog by calling Init()
		* \see Init()
		*/
		DatabaseCatalog(ConstSqlDbcHandlePtr pHdbc, const SqlInfoProperties& props);
		

		/*!
		* \brief Frees the allocated statement handle.
		*/
		~DatabaseCatalog();


		/*!
		* \brief Must be called manually if default constructor was used. 
		*		Internally stores a copy of the passed shared_ptr and allocated
		*		a statement from the passed connection handle.
		*		Allocated statement is freed on destruction
		* \param searchPatternEscape Value of SqlGetInfo attribute SQL_SEARCH_PATTERN_ESCAPE.
		*/
		void Init(ConstSqlDbcHandlePtr pHdbc, const SqlInfoProperties& props);


		/*!
		* \brief Searches for tables using the passed search-arguments.
		* \details If mode is to MetadataMode::PatternValue, tableName, schemaName
		*			and catalogName are treated as pattern value arguments. Use '_' to
		*			match any single character or '%' to match any sequence of zero or 
		*			more characters. Passing a null pointer to search argument is 
		*			equivalent to passing '%' as search argument value, but passing
		*			an empty string ("") matches only the empty string.\n
		*			If mode is set to MetadataMode::Identifier, tableName, schemaName
		*			and catalogName are treated as identifier arguments. If a search
		*			argument is a quoted string, the string within quotation  marks is
		*			treated literally. If a search argument is not quoted, the driver
		*			folds it to uppercase. Passing a null pointer as search argument will
		*			result in a failure.\n
		*			tableType can be a comma separated list of values. If an empty string
		*			is passed, the argument is ignored (equal to SQL_ALL_TABLE_TYPES).
		*			
		*/
		TableInfosVector FindTables(const char* pTableName, const char* pSchemaName, 
			const char* pCatalogName, const std::string& tableType, MetadataMode mode) const;


		/*!
		* \brief Searches for tables using the passed search-arguments.
		* \see FindTables(const std::string* pTableName, const std::string* pSchemaName, 
		*		const std::string* pCatalogName, const std::string& tableType, MetadataMode mode)
		*/
		TableInfosVector FindTable(const std::string& tableName, const std::string& schemaName,
			const std::string& catalogName, const std::string& tableType, MetadataMode mode) const;


		/*!
		* \brief Get value of SqlGetInfo property SQL_SEARCH_PATTERN_ESCAPE. Escape search strings
		*		with returned value.
		*/
		std::string GetSearchPatternEscape() const { return m_props.GetSearchPatternEscape(); };

	private:
		void SetMetadataAttribute(MetadataMode mode) const;
		MetadataMode GetMetadataAttribute() const;

		ConstSqlDbcHandlePtr m_pHdbc;
		SqlStmtHandlePtr m_pHStmt;
		SqlInfoProperties m_props;
		// cache the currently active value:
		mutable MetadataMode m_stmtMode;
	};


}	// namespace exodbc


