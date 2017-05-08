/*!
* \file DatabaseCatalog.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 19.04.2017
* \brief Source file for the DatabaseCatalog class and its helpers.
* \copyright GNU Lesser General Public License Version 3
*
*/

// Own header
#include "DatabaseCatalog.h"

// Same component headers
#include "SqlStatementCloser.h"
#include "SpecializedExceptions.h"

// Other headers
// Debug
#include "DebugNew.h"

using namespace std;

namespace exodbc
{
	// Construction
	// ------------
	DatabaseCatalog::DatabaseCatalog()
		:m_pHStmt(std::make_shared<SqlStmtHandle>())
		, m_stmtMode(MetadataMode::PatternOrOrdinary)
	{
	};


	DatabaseCatalog::DatabaseCatalog(ConstSqlDbcHandlePtr pHdbc, const SqlInfoProperties& props)
		:m_pHStmt(std::make_shared<SqlStmtHandle>())
		, m_stmtMode(MetadataMode::PatternOrOrdinary)
	{
		Init(pHdbc, props);
	}


	// Destructor
	// -----------
	DatabaseCatalog::~DatabaseCatalog()
	{
		Reset();
	}


	// Implementation
	// --------------
	void DatabaseCatalog::Init(ConstSqlDbcHandlePtr pHdbc, const SqlInfoProperties& props)
	{
		exASSERT(m_pHdbc == nullptr);
		exASSERT(props.IsPropertyRegistered(SQL_SEARCH_PATTERN_ESCAPE));
		m_pHdbc = pHdbc;
		m_props = props;
		m_props.EnsurePropertyRead(pHdbc, SQL_SEARCH_PATTERN_ESCAPE, false);
		m_pHStmt->AllocateWithParent(pHdbc);
		m_stmtMode = GetMetadataAttribute();
	}


	void DatabaseCatalog::Reset()
	{
		if (m_pHStmt->IsAllocated())
		{
			m_pHStmt->Free();
		}
		m_pHdbc.reset();
	}


	vector<string> DatabaseCatalog::ListCatalogs() const
	{
		string catalog = SQL_ALL_CATALOGS;
		string empty = u8"";
		TableInfoVector tableInfos = SearchTables(EXODBCSTR_TO_SQLAPICHARPTR(empty), EXODBCSTR_TO_SQLAPICHARPTR(empty), 
				EXODBCSTR_TO_SQLAPICHARPTR(catalog), empty, MetadataMode::PatternOrOrdinary);
		vector<string> catalogs;
		for (TableInfoVector::const_iterator it = tableInfos.begin(); it != tableInfos.end(); ++it)
		{
			catalogs.push_back(it->GetCatalog());
		}
		return catalogs;
	}


	vector<string> DatabaseCatalog::ListSchemas() const
	{
		string schema = SQL_ALL_SCHEMAS;
		string empty = u8"";
		TableInfoVector tableInfos = SearchTables(EXODBCSTR_TO_SQLAPICHARPTR(empty), EXODBCSTR_TO_SQLAPICHARPTR(schema),
			EXODBCSTR_TO_SQLAPICHARPTR(empty), empty, MetadataMode::PatternOrOrdinary);
		vector<string> schemas;
		for (TableInfoVector::const_iterator it = tableInfos.begin(); it != tableInfos.end(); ++it)
		{
			schemas.push_back(it->GetSchema());
		}
		return schemas;
	}


	vector<string> DatabaseCatalog::ListTableTypes() const
	{
		string type = SQL_ALL_TABLE_TYPES;
		string empty = u8"";
		TableInfoVector tableInfos = SearchTables(EXODBCSTR_TO_SQLAPICHARPTR(empty), EXODBCSTR_TO_SQLAPICHARPTR(empty),
			EXODBCSTR_TO_SQLAPICHARPTR(empty), type, MetadataMode::PatternOrOrdinary);
		vector<string> types;
		for (TableInfoVector::const_iterator it = tableInfos.begin(); it != tableInfos.end(); ++it)
		{
			types.push_back(it->GetType());
		}
		return types;
	}


	DatabaseCatalog::MetadataMode DatabaseCatalog::GetMetadataAttribute() const
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());

		// Read SQL_ATTR_METADATA_ID value: We need to init that value, it seems like MySql connector does not
		// overwrite it completely on 64bit builds..
		SQLULEN metadataAttr = SQL_FALSE;
		SQLRETURN ret = SQLGetStmtAttr(m_pHStmt->GetHandle(), SQL_ATTR_METADATA_ID, (SQLPOINTER)&metadataAttr, 0, nullptr);
		if (!SQL_SUCCEEDED(ret))
		{
			SqlResultException sre(u8"SQLGetStmtAttr", ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());
			if (sre.HasErrorInfo(ErrorHelper::SQLSTATE_OPTIONAL_FEATURE_NOT_IMPLEMENTED))
			{
				// Not implemented by the driver, assume its default: 
				LOG_INFO(boost::str(boost::format(u8"Driver %s does not support SQL_ATTR_METADATA_ID (SQLGetStmtAttr returned SQLSTATE %s), asssuming default value of FALSE")
					% m_props.GetDriverName() % ErrorHelper::SQLSTATE_OPTIONAL_FEATURE_NOT_IMPLEMENTED));
				return MetadataMode::PatternOrOrdinary;
			}
			else
			{
				SET_EXCEPTION_SOURCE(sre);
				throw sre;
			}
		}
		exASSERT_MSG(metadataAttr == SQL_FALSE || metadataAttr == SQL_TRUE,
			boost::str(boost::format(u8"Unknown value %d for SQL_ATTR_METADATA_ID read") % metadataAttr));

		if (metadataAttr == SQL_FALSE)
			return MetadataMode::PatternOrOrdinary;
		else
			return MetadataMode::Identifier;
	}


	void DatabaseCatalog::SetMetadataAttribute(MetadataMode mode) const
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());

		SQLUINTEGER newValue;
		if (mode == MetadataMode::Identifier)
			newValue = SQL_TRUE;
		else
			newValue = SQL_FALSE;

		SQLRETURN ret = SQLSetStmtAttr(m_pHStmt->GetHandle(), SQL_ATTR_METADATA_ID, (SQLPOINTER)&newValue, 0);
		THROW_IFN_SUCCEEDED(SQLSetStmtAttr, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());
		
		if (newValue == SQL_FALSE)
			m_stmtMode = MetadataMode::PatternOrOrdinary;
		else
			m_stmtMode = MetadataMode::Identifier;
	}


	bool DatabaseCatalog::GetSupportsCatalogs() const
	{
		return m_props.GetSupportsCatalogs() && !m_props.GetCatalogTerm().empty();
	}


	bool DatabaseCatalog::GetSupportsSchemas() const
	{
		return !m_props.GetSchemaTerm().empty();
	}


	std::string DatabaseCatalog::EscapePatternValueArguments(const std::string& input) const
	{
		string esc = GetSearchPatternEscape();
		string escaped = input;
		boost::algorithm::replace_all(escaped, u8"%", esc + u8"%");
		boost::algorithm::replace_all(escaped, u8"_", esc + u8"_");
		return escaped;
	}


	TableInfoVector DatabaseCatalog::SearchTables(const std::string& tableName, 
		const std::string& schemaName, 
		const std::string& catalogName, const std::string& tableType /* = u8"" */) const
	{
		return SearchTables(EXODBCSTR_TO_SQLAPICHARPTR(tableName),
			schemaName.empty() && !GetSupportsSchemas() ? nullptr : EXODBCSTR_TO_SQLAPICHARPTR(schemaName),
			catalogName.empty() && !GetSupportsCatalogs() ? nullptr : EXODBCSTR_TO_SQLAPICHARPTR(catalogName),
			tableType, MetadataMode::PatternOrOrdinary);
	}


	TableInfoVector DatabaseCatalog::SearchTables(const std::string& tableName, 
		const std::string& schemaOrCatalogName, SchemaOrCatalogType schemaOrCatalogType,
		const std::string& tableType /* = u8"" */) const
	{
		if (schemaOrCatalogType == SchemaOrCatalogType::Schema)
		{
			return SearchTables(EXODBCSTR_TO_SQLAPICHARPTR(tableName),
				EXODBCSTR_TO_SQLAPICHARPTR(schemaOrCatalogName),
				nullptr,
				tableType, MetadataMode::PatternOrOrdinary);
		}
		else
		{
			return SearchTables(EXODBCSTR_TO_SQLAPICHARPTR(tableName),
				nullptr,
				EXODBCSTR_TO_SQLAPICHARPTR(schemaOrCatalogName),
				tableType, MetadataMode::PatternOrOrdinary);
		}
	}


	TableInfoVector DatabaseCatalog::SearchTables(const std::string& tableName, const std::string& schemaOrCatalogName, const std::string& tableType) const
	{
		bool supportsCatalogs = GetSupportsCatalogs();
		bool supportsSchemas = GetSupportsSchemas();
		if (supportsCatalogs && supportsSchemas)
		{
			NotAllowedException nae(u8"Database supports catalogs and schemas, unable to determine type of schemaOrCatalogName argument");
			SET_EXCEPTION_SOURCE(nae);
			throw nae;
		}
		if (!(supportsCatalogs || supportsSchemas))
		{
			NotAllowedException nae(u8"Database does not support catalogs or schemas, cannot use schemaOrCatalogName argument");
			SET_EXCEPTION_SOURCE(nae);
			throw nae;
		}
		return SearchTables(EXODBCSTR_TO_SQLAPICHARPTR(tableName),
			supportsSchemas ? EXODBCSTR_TO_SQLAPICHARPTR(schemaOrCatalogName) : nullptr,
			supportsCatalogs ? EXODBCSTR_TO_SQLAPICHARPTR(schemaOrCatalogName) : nullptr,
			tableType, MetadataMode::PatternOrOrdinary);
	}


	TableInfoVector DatabaseCatalog::SearchTables(const std::string& tableName, const std::string& tableType /* = u8"%" */) const
	{
		return SearchTables(EXODBCSTR_TO_SQLAPICHARPTR(tableName), nullptr, nullptr, tableType, MetadataMode::PatternOrOrdinary);
	}


	TableInfo DatabaseCatalog::FindOneTable(const std::string& tableName, const std::string& schemaName /* = u8"" */, 
		const std::string& catalogName /* = u8"" */, const std::string& tableType /* = u8"" */) const
	{
		exASSERT(!tableName.empty());
		bool haveSchema = !schemaName.empty();
		bool haveCatalog = !catalogName.empty();
		TableInfoVector matchingTables;
		if (haveSchema && haveCatalog)
			matchingTables = SearchTables(tableName, schemaName, catalogName, tableType);
		else if (haveSchema)
			matchingTables = SearchTables(tableName, schemaName, SchemaOrCatalogType::Schema, tableType);
		else if (haveCatalog)
			matchingTables = SearchTables(tableName, catalogName, SchemaOrCatalogType::Catalog, tableType);
		else
			matchingTables = SearchTables(tableName, tableType);
		if (matchingTables.size() < 1)
		{
			NotFoundException nfe(boost::str(boost::format(u8"No Table was found matching tableName '%s', catalogName '%s', schemaName '%s', tableType '%s'.")
				% tableName % catalogName % schemaName % tableType));
			SET_EXCEPTION_SOURCE(nfe);
			throw nfe;
		}
		else if (matchingTables.size() > 1)
		{
			stringstream ss;
			ss << u8"Multiple Tables were found matching tableName '" << tableName << "', catalogName '" << catalogName
				<< u8"' schemaName '" << schemaName << u8"' tableType '" << tableType << u8"': " << std::endl;
			int count = 0;
			for (TableInfoVector::const_iterator it = matchingTables.begin(); it != matchingTables.end(); ++it)
			{
				ss << u8"TableInfo[" << count << u8"]: " << it->ToString() << std::endl;
			}
			NotFoundException nfe(ss.str());
			SET_EXCEPTION_SOURCE(nfe);
			throw nfe;
		}
		return matchingTables.front();
	}


	TableInfoVector DatabaseCatalog::SearchTables(SQLAPICHARTYPE* pTableName, SQLAPICHARTYPE* pSchemaName,
		SQLAPICHARTYPE* pCatalogName, const std::string& tableType, MetadataMode mode) const
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());

		if (m_stmtMode != mode)
			SetMetadataAttribute(mode);

		if (mode == MetadataMode::Identifier)
		{
			exASSERT_MSG(pTableName != nullptr, u8"pTableName must not be a nullptr if MetadataMode::Identifier is set");
			exASSERT_MSG(pSchemaName != nullptr, u8"pSchemaName must not be a nullptr if MetadataMode::Identifier is set");
			exASSERT_MSG(pCatalogName != nullptr, u8"pCatalogName must not be a nullptr if MetadataMode::Identifier is set");
		}

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_pHStmt, true, true);

		TableInfoVector tables;

		// Query db
		SQLRETURN ret = SQLTables(m_pHStmt->GetHandle(),
			pCatalogName == nullptr ? NULL : pCatalogName, SQL_NTS,   // catname                 
			pSchemaName == nullptr ? NULL : pSchemaName, SQL_NTS,   // schema name
			pTableName == nullptr ? NULL : pTableName, SQL_NTS,	// table name
			tableType.empty() ? NULL : EXODBCSTR_TO_SQLAPICHARPTR(tableType), SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLTables, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		while ((ret = SQLFetch(m_pHStmt->GetHandle())) == SQL_SUCCESS)
		{
			TableInfo ti(m_pHStmt, m_props);
			tables.push_back(ti);
		}
		THROW_IFN_NO_DATA(SQLFetch, ret);

		return tables;
	}


	ColumnInfoVector DatabaseCatalog::ReadColumnInfo(const TableInfo& tableInfo) const
	{
		return ReadColumnInfo(nullptr,
			EXODBCSTR_TO_SQLAPICHARPTR(tableInfo.GetName()),
			tableInfo.HasSchema() ? EXODBCSTR_TO_SQLAPICHARPTR(tableInfo.GetSchema()) : nullptr,
			tableInfo.HasCatalog() ? EXODBCSTR_TO_SQLAPICHARPTR(tableInfo.GetCatalog()) : nullptr,
			MetadataMode::PatternOrOrdinary);
	}


	PrimaryKeyInfoVector DatabaseCatalog::ReadPrimaryKeyInfo(const TableInfo& tableInfo) const
	{
		return ReadPrimaryKeyInfo(EXODBCSTR_TO_SQLAPICHARPTR(tableInfo.GetName()),
			tableInfo.HasSchema() ? EXODBCSTR_TO_SQLAPICHARPTR(tableInfo.GetSchema()) : nullptr,
			tableInfo.HasCatalog() ? EXODBCSTR_TO_SQLAPICHARPTR(tableInfo.GetCatalog()) : nullptr,
			MetadataMode::PatternOrOrdinary);
	}


	SqlTypeInfoVector DatabaseCatalog::ReadSqlTypeInfo() const
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());

		SqlTypeInfoVector types;

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_pHStmt, true, true);

		SQLRETURN ret = SQLGetTypeInfo(m_pHStmt->GetHandle(), SQL_ALL_TYPES);
		THROW_IFN_SUCCEEDED(SQLGetTypeInfo, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		ret = SQLFetch(m_pHStmt->GetHandle());

		while (ret == SQL_SUCCESS)
		{
			SqlTypeInfo typeInfo(m_pHStmt, m_props);
			types.push_back(typeInfo);

			ret = SQLFetch(m_pHStmt->GetHandle());
		}
		THROW_IFN_NO_DATA(SQLFetch, ret);

		return types;
	}


	ColumnInfoVector DatabaseCatalog::ReadColumnInfo(SQLAPICHARTYPE* pColumnName, SQLAPICHARTYPE* pTableName, 
		SQLAPICHARTYPE* pSchemaName, SQLAPICHARTYPE* pCatalogName, MetadataMode mode) const
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());

		if (m_stmtMode != mode)
			SetMetadataAttribute(mode);

		if (mode == MetadataMode::Identifier)
		{
			exASSERT_MSG(pColumnName != nullptr, u8"pColumnName must not be a nullptr if MetadataMode::Identifier is set");
			exASSERT_MSG(pTableName != nullptr, u8"pTableName must not be a nullptr if MetadataMode::Identifier is set");
			exASSERT_MSG(pSchemaName != nullptr, u8"pSchemaName must not be a nullptr if MetadataMode::Identifier is set");
			exASSERT_MSG(pCatalogName != nullptr, u8"pCatalogName must not be a nullptr if MetadataMode::Identifier is set");
		}

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_pHStmt, true, true);

		// Query columns
		int colCount = 0;
		SQLRETURN ret = SQLColumns(m_pHStmt->GetHandle(),
			pCatalogName == nullptr ? NULL : pCatalogName, SQL_NTS,	// catalog
			pSchemaName == nullptr ? NULL : pSchemaName, SQL_NTS,	// schema
			pTableName == nullptr ? NULL : pTableName, SQL_NTS, // tablename
			pColumnName == nullptr ? NULL : pColumnName, SQL_NTS);	// All column

		THROW_IFN_SUCCEEDED(SQLColumns, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		ColumnInfoVector columns;

		while ((ret = SQLFetch(m_pHStmt->GetHandle())) == SQL_SUCCESS)
		{
			// Fetch data from columns
			ColumnInfo colInfo(m_pHStmt, m_props);
			columns.push_back(colInfo);
		}
		THROW_IFN_NO_DATA(SQLFetch, ret);

		return columns;
	}


	PrimaryKeyInfoVector DatabaseCatalog::ReadPrimaryKeyInfo(SQLAPICHARTYPE* pTableName, SQLAPICHARTYPE* pSchemaName, 
		SQLAPICHARTYPE* pCatalogName, MetadataMode mode) const
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());
		exASSERT(pTableName != nullptr);

		if (m_stmtMode != mode)
			SetMetadataAttribute(mode);

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_pHStmt, true, true);

		PrimaryKeyInfoVector primaryKeys;

		SQLRETURN ret = SQLPrimaryKeys(m_pHStmt->GetHandle(),
			pCatalogName, SQL_NTS,
			pSchemaName, SQL_NTS,
			pTableName, SQL_NTS);

		THROW_IFN_SUCCEEDED(SQLPrimaryKeys, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		while ((ret = SQLFetch(m_pHStmt->GetHandle())) == SQL_SUCCESS)
		{
			PrimaryKeyInfo pki(m_pHStmt, m_props);
			primaryKeys.push_back(pki);
		}
		THROW_IFN_NO_DATA(SQLFetch, ret);

		return primaryKeys;
	}


	SpecialColumnInfoVector DatabaseCatalog::ReadSpecialColumnInfo(const TableInfo& tableInfo, SpecialColumnInfo::IdentifierType idType, 
		SpecialColumnInfo::RowIdScope scope, bool includeNullableColumns /* = true */) const
	{
		return ReadSpecialColumnInfo(idType, EXODBCSTR_TO_SQLAPICHARPTR(tableInfo.GetName()),
			tableInfo.HasSchema() ? EXODBCSTR_TO_SQLAPICHARPTR(tableInfo.GetSchema()) : nullptr,
			tableInfo.HasCatalog() ? EXODBCSTR_TO_SQLAPICHARPTR(tableInfo.GetCatalog()) : nullptr,
			scope, includeNullableColumns, MetadataMode::PatternOrOrdinary);
	}


	SpecialColumnInfoVector DatabaseCatalog::ReadSpecialColumnInfo(SpecialColumnInfo::IdentifierType idType,
		SQLAPICHARTYPE* pTableName, SQLAPICHARTYPE* pSchemaName, SQLAPICHARTYPE* pCatalogName,
		SpecialColumnInfo::RowIdScope scope, bool includeNullableColumns, MetadataMode mode) const
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());
		exASSERT(pTableName != nullptr);

		if (m_stmtMode != mode)
			SetMetadataAttribute(mode);

		SpecialColumnInfoVector columns;

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_pHStmt, true, true);

		SQLSMALLINT nullable = SQL_NO_NULLS;
		if (includeNullableColumns)
		{
			nullable = SQL_NULLABLE;
		}

		SQLRETURN ret = SQLSpecialColumns(m_pHStmt->GetHandle(), (SQLSMALLINT)idType,
			pCatalogName, SQL_NTS,
			pSchemaName, SQL_NTS,
			pTableName, SQL_NTS,
			(SQLSMALLINT)scope, nullable);
		THROW_IFN_SUCCEEDED(SQLSpecialColumns, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		while ((ret = SQLFetch(m_pHStmt->GetHandle())) == SQL_SUCCESS)
		{
			SpecialColumnInfo specColInfo(m_pHStmt, m_props, idType);
			columns.push_back(specColInfo);
		}
		THROW_IFN_NO_DATA(SQLFetch, ret);

		return columns;
	}
}
