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
		* \brief Searches for tables using pattern value (PV) arguments.
		* \details tableName, schemaName and catalogName are treated as pattern value 
		*			arguments. Use '_' to match any single character or '%' to 
		*			match any sequence of zero or more characters. Passing
		*			an empty string ("") matches only the empty string.\n
		*			tableType can be a comma separated list of values. If an empty string
		*			is passed, the argument is ignored (equal to SQL_ALL_TABLE_TYPES).\n
		*			Note that if the Environment ODBC Version is less than 3.x, 
		*			catalogName does not accept search patterns.\n
		*			If SqlInfoProperties::GetSupportsCatalogs() returns false, any passed
		*			catalogName value is ignored (and a nullptr is passed to SQLTables).
		*
		*/
		TableInfosVector SearchTables(const std::string& tableName = u8"%", const std::string& schemaName = u8"%",
			const std::string& catalogName = u8"%", const std::string& tableType = u8"") const;


		/*!
		* \brief Searches for tables using a pattern value (PV) argument for the table name.
		* \details Schema name, catalog name are not set when querying the database, table type
		*		   will be set to an empty string. This should return the same values as if SearchTables
		*			is called with '%' set schema and catalog name.
		* \see	SearchTables(const std::string& tableName = u8"%", const std::string& schemaName = u8"%",
			const std::string& catalogName = u8"%", const std::string& tableType = u8"")
		*/
		TableInfosVector SearchTablesByName(const std::string& tableName = u8"%") const;


		/*!
		* \brief Get value of SqlGetInfo property SQL_SEARCH_PATTERN_ESCAPE. Escape search strings
		*		with returned value.
		*/
		std::string GetSearchPatternEscape() const { return m_props.GetSearchPatternEscape(); };

	private:
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
		TableInfosVector SearchTables(SQLAPICHARTYPE* pTableName, SQLAPICHARTYPE* pSchemaName,
			SQLAPICHARTYPE* pCatalogName, const std::string& tableType, MetadataMode mode) const;
		void SetMetadataAttribute(MetadataMode mode) const;
		MetadataMode GetMetadataAttribute() const;

		ConstSqlDbcHandlePtr m_pHdbc;
		SqlStmtHandlePtr m_pHStmt;
		SqlInfoProperties m_props;
		// cache the currently active value:
		mutable MetadataMode m_stmtMode;
	};


}	// namespace exodbc


