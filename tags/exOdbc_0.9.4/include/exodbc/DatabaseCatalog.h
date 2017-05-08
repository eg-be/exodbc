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
#include "TableInfo.h"
#include "ColumnInfo.h"
#include "PrimaryKeyInfo.h"
#include "SqlTypeInfo.h"
#include "SpecialColumnInfo.h"

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
			PatternOrOrdinary,	///< Set if SQL_ATTR_METADATA_ID is SQL_FALSE.
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
		* \brief Calls Reset()
		*/
		~DatabaseCatalog();


		/*!
		* \brief Must be called manually if default constructor was used. 
		*		Internally stores a copy of the passed shared_ptr and allocated
		*		a statement from the passed connection handle.
		*		Allocated statement is freed on destruction
		* \param pHdbc Connection handle to be used to create statements and read properties.
		* \param props SqlInfoProperties Property SQL_SEARCH_PATTERN_ESCAPE must be
		*				registered, if it is not read already, it is read using the
		*				passed connection handle.
		*/
		void Init(ConstSqlDbcHandlePtr pHdbc, const SqlInfoProperties& props);


		/*!
		* \brief Frees all handles allocated by this DatabaseCatalog. Init() can
		*	be called again after this function succeeds.
		*/
		void Reset();


		/*!
		* \brief List all catalogs available in the Database.
		*/
		std::vector<std::string> ListCatalogs() const;


		/*!
		* \brief List all schemas available in the Database.
		*/
		std::vector<std::string> ListSchemas() const;


		/*!
		* \brief List all table types availabl ein the Database.
		*/
		std::vector<std::string> ListTableTypes() const;


		/*!
		* \brief Searches for tables using pattern value (PV) arguments for
		*			table, schema and catalog name.
		* \details tableName, schemaName and catalogName are treated as pattern value 
		*			arguments. Use '_' to match any single character or '%' to 
		*			match any sequence of zero or more characters. Passing
		*			an empty string ("") matches only the empty string.\n
		*			tableType can be a comma separated list of values. If an empty string
		*			is passed, the argument is ignored (equal to SQL_ALL_TABLE_TYPES).\n
		*			The case of table, schema and catalog name is significant.\n
		*			Note that if the Environment ODBC Version is less than 3.x, 
		*			catalogName does not accept search patterns.\n
		*			If GetSupportsSchemas() returns false and schemaName is set to
		*			an empty string, schemaName is ignored.\n
		*			If GetSupportsCatalogs() returns false and catalogName is set to
		*			an empty string, schemaName is ignored.\n
		*/
		TableInfoVector SearchTables(const std::string& tableName, const std::string& schemaName,
			const std::string& catalogName, const std::string& tableType) const;


		enum class SchemaOrCatalogType
		{
			Schema,
			Catalog
		};
		/*!
		* \brief Searches for tables using pattern value (PV) arguments for table
		*			name and schema or catalog name. If schemaOrCatalogType is set 
		*			to SchemaOrCatalogType::Schema,
		*			the passed argument schemaOrCatalogName is treated as a
		*			schema name and catalog name is ignored. If SchemaOrCatalogType is 
		*			SchemaOrCatalogType::Catalog, the argument schemaOrCatalogName is treated as catalog
		*			name and schema name is ignored.
		* \details tableName and schemaNameOrCatalogName are treated as pattern value
		*			arguments. Use '_' to match any single character or '%' to
		*			match any sequence of zero or more characters. Passing
		*			an empty string ("") matches only the empty string.\n
		*			The case of tableName and schema or catalog name is significant.\n
		*			tableType can be a comma separated list of values. If an empty string
		*			is passed, the argument is ignored (equal to SQL_ALL_TABLE_TYPES).\n
		*			Note that if the Environment ODBC Version is less than 3.x,
		*			catalogName does not accept search patterns.\n
		*
		*/
		TableInfoVector SearchTables(const std::string& tableName, const std::string& schemaOrCatalogName, 
			SchemaOrCatalogType schemaOrCatalogType, const std::string& tableType) const;


		/*!
		* \brief Searches for tables using pattern value (PV) arguments for table
		*			name and schema or catalog name. The SqlInfoProperties are
		*			examined to decide if the Database supports catalogs or schemas.
		*			If the Database does not support either catalogs or schemas (or
		*			both), the method will fail and throw an Exception.
		* \details tableName and schemaNameOrCatalogName are treated as pattern value
		*			arguments. Use '_' to match any single character or '%' to
		*			match any sequence of zero or more characters. Passing
		*			an empty string ("") matches only the empty string.\n
		*			The case of tableName and schema or catalog name is significant.\n
		*			To decide whether to treat schemaOrCatalogName as catalog or schema
		*			name, the GetSupportsSchemas() and GetSupportsCatalogs() method are
		*			examined. Exactly one of them must return true.\n
		*			tableType can be a comma separated list of values. If an empty string
		*			is passed, the argument is ignored (equal to SQL_ALL_TABLE_TYPES).\n
		*			Note that if the Environment ODBC Version is less than 3.x,
		*			catalogName does not accept search patterns.\n
		* \see		GetSupportsSchemas()
		* \see		GetSupportsCatalogs()
		*
		*/
		TableInfoVector SearchTables(const std::string& tableName, const std::string& schemaOrCatalogName, 
			const std::string& tableType) const;


		/*!
		* \brief Searches for tables using a pattern value (PV) argument for the table name.
		* \details Schema name, catalog name are not set when querying the database.
		*/
		TableInfoVector SearchTables(const std::string& tableName, const std::string& tableType = u8"") const;


		/*!
		* \brief Search for exactly one table matching the passed arguments.
		* \details	If any of the passed arguments except tableName is empty, the argument is ignored.
		*			tableName is not allowed to be empty.
		* \throw	NotFoundException If not exactly one table is found.
		*/
		TableInfo FindOneTable(const std::string& tableName, const std::string& schemaName = u8"",
			const std::string& catalogName = u8"", const std::string& tableType = u8"") const;


		/*!
		* \brief Get value of SqlGetInfo property SQL_SEARCH_PATTERN_ESCAPE. Escape search strings
		*		with returned value.
		*/
		std::string GetSearchPatternEscape() const { return m_props.GetSearchPatternEscape(); };


		/*!
		* \brief Escapes the pattern value arguments '%' and '_' with the escape sequence returned
		*		by GetSearchPatternEscape()
		*/
		std::string EscapePatternValueArguments(const std::string& input) const;


		/*!
		* \brief	Returns false If SqlInfoProperties::GetSchemaTerm() returns an empty string.
		*/
		bool GetSupportsSchemas() const;


		/*!
		* \brief	Returns false if SqlInfoProperties::GetSupportsCatalogs() returns false or
		*			SqlInfoProperties::GetCatalogTerm() returns an empty string,
		*/
		bool GetSupportsCatalogs() const;


		/*!
		* \brief	Read the columns of a table. Fetch the required TableInfo using one
		*			of the FindTables() functions.
		*/
		ColumnInfoVector ReadColumnInfo(const TableInfo& tableInfo) const;


		/*!
		* \brief	Read the primary keys of a table. Fetch the required TableInfo using one
		*			of the FindTables() functions.
		*/
		PrimaryKeyInfoVector ReadPrimaryKeyInfo(const TableInfo& tableInfo) const;


		/*!
		* \brief Read information about all SQL types supported by the database, using SQLGetTypeInfo.
		*/
		SqlTypeInfoVector ReadSqlTypeInfo() const;


		/*!
		* \brief	Read special column information for a table.
		* \details	This is a wrapper around SQLSpecialColumns.
		* \param	tableInfo	Identify the table to query database about.
		* \param	idType		The identifier type to query
		* \param	scope		The row id scope to query, if identifier is set to IdentifierType::IDENTIFY_ROW_UNIQUELY
		* \param	includeNullableColumns Include columns that can have NULL values in the special columns or not.
		*/
		SpecialColumnInfoVector ReadSpecialColumnInfo(const TableInfo& tableInfo, SpecialColumnInfo::IdentifierType idType, 
			SpecialColumnInfo::RowIdScope scope, bool includeNullableColumns = true) const;


	private:
		/*!
		* \brief Searches for tables using the passed search-arguments.
		* \details If mode is to MetadataMode::PatternOrOrdinary, pTableName, pSchemaName
		*			and pCatalogName are treated as pattern value arguments. Use '_' to
		*			match any single character or '%' to match any sequence of zero or
		*			more characters. Passing a null pointer to search argument is
		*			equivalent to passing '%' as search argument value, but passing
		*			an empty string ("") matches only the empty string.\n
		*			The case is significant if MetadataMode::PatternOrOrdinary is set.\n
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
		TableInfoVector SearchTables(SQLAPICHARTYPE* pTableName, SQLAPICHARTYPE* pSchemaName,
			SQLAPICHARTYPE* pCatalogName, const std::string& tableType, MetadataMode mode) const;
		void SetMetadataAttribute(MetadataMode mode) const;
		MetadataMode GetMetadataAttribute() const;


		/*!
		* \brief Reads column information from SQLColumns for columns matching passed arguments.
		* \details	If mode is set to MetadataMode::PatternOrOrdinary, pColumnName, pTableName and
		*			pSchemaName are treated as pattern value arguments.\n
		*			catalogName is an ordinary argument (OA), it is treated as a literal string.\n
		*			If a nullptr is passed an arguments, the argument is ignored,
		*			that means set to a nullptr when querying the database using SQLColumns.\n
		*			If mode is set to MetadataMode::Identifier, all arguments (including catalogName)
		*			are treated as identifier values (ID).
		*/
		ColumnInfoVector ReadColumnInfo(SQLAPICHARTYPE* pColumnName, SQLAPICHARTYPE* pTableName,
			SQLAPICHARTYPE* pSchemaName, SQLAPICHARTYPE* pCatalogName, MetadataMode mode) const;



		/*!
		* \brief Read Primary Key information from SQLPrimaryKeys for tables matching the passed arguments.
		* \details	If mode is set MetadataMode::PatternOrOrdinary, pTableName, pSchemaName and
		*			pCatalogName are treated as ordinary value (OV) arguments. Strings are treated
		*			literally and the case is significant.
		*			If mode is set to MetadataMode::Identifier, all arguments are treated as
		*			identifier values (ID).\n
		*			pTableName is not allowed to be a null pointer.
		*/
		PrimaryKeyInfoVector ReadPrimaryKeyInfo(SQLAPICHARTYPE* pTableName, SQLAPICHARTYPE* pSchemaName,
			SQLAPICHARTYPE* pCatalogName, MetadataMode mode) const;


		/*!
		* \brief	Read special column information for a table using SQLSpecialColumns.
		If mode is set MetadataMode::PatternOrOrdinary, pTableName, pSchemaName and
		*			pCatalogName are treated as ordinary value (OV) arguments. Strings are treated
		*			literally and the case is significant.
		*			If mode is set to MetadataMode::Identifier, all arguments are treated as
		*			identifier values (ID).\n
		*			pTableName is not allowed to be a null pointer.
		*/
		SpecialColumnInfoVector ReadSpecialColumnInfo(SpecialColumnInfo::IdentifierType idType,
			SQLAPICHARTYPE* pTableName, SQLAPICHARTYPE* pSchemaName, SQLAPICHARTYPE* pCatalogName, 
			SpecialColumnInfo::RowIdScope scope, bool includeNullableColumns, MetadataMode mode) const;


		ConstSqlDbcHandlePtr m_pHdbc;
		SqlStmtHandlePtr m_pHStmt;
		SqlInfoProperties m_props;
		// cache the currently active value:
		mutable MetadataMode m_stmtMode;
	};

	typedef std::shared_ptr<DatabaseCatalog> DatabaseCatalogPtr;

}	// namespace exodbc


