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
#include "Helpers.h"
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
			schemas.push_back(it->GetCatalog());
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
			types.push_back(it->GetCatalog());
		}
		return types;
	}


	DatabaseCatalog::MetadataMode DatabaseCatalog::GetMetadataAttribute() const
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());

		// Read SQL_ATTR_METADATA_ID value, only modify if not already set to passed value
		SQLUINTEGER metadataAttr;
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


	SqlTypeInfosVector DatabaseCatalog::ReadSqlTypeInfo() const
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());

		SqlTypeInfosVector types;

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_pHStmt, true, true);

		SQLRETURN ret = SQLGetTypeInfo(m_pHStmt->GetHandle(), SQL_ALL_TYPES);
		THROW_IFN_SUCCEEDED(SQLGetTypeInfo, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		ret = SQLFetch(m_pHStmt->GetHandle());
		int count = 0;
		SQLAPICHARTYPE typeName[DB_MAX_TYPE_NAME_LEN + 1];
		SQLAPICHARTYPE literalPrefix[DB_MAX_LITERAL_PREFIX_LEN + 1];
		SQLAPICHARTYPE literalSuffix[DB_MAX_LITERAL_SUFFIX_LEN + 1];
		SQLAPICHARTYPE createParams[DB_MAX_CREATE_PARAMS_LIST_LEN + 1];
		SQLAPICHARTYPE localTypeName[DB_MAX_LOCAL_TYPE_NAME_LEN + 1];
		SQLLEN cb;
		while (ret == SQL_SUCCESS)
		{
			typeName[0] = 0;
			literalPrefix[0] = 0;
			literalSuffix[0] = 0;
			createParams[0] = 0;
			localTypeName[0] = 0;
			SSqlTypeInfo info;

			cb = 0;
			GetData(m_pHStmt, 1, SQLAPICHARTYPENAME, typeName, sizeof(typeName), &cb, NULL);
			info.m_typeName = SQLAPICHARPTR_TO_EXODBCSTR(typeName);

			GetData(m_pHStmt, 2, SQL_C_SSHORT, &info.m_sqlType, sizeof(info.m_sqlType), &cb, NULL);
			GetData(m_pHStmt, 3, SQL_C_SLONG, &info.m_columnSize, sizeof(info.m_columnSize), &cb, &info.m_columnSizeIsNull);
			GetData(m_pHStmt, 4, SQLAPICHARTYPENAME, literalPrefix, sizeof(literalPrefix), &cb, &info.m_literalPrefixIsNull);
			info.m_literalPrefix = SQLAPICHARPTR_TO_EXODBCSTR(literalPrefix);
			GetData(m_pHStmt, 5, SQLAPICHARTYPENAME, literalSuffix, sizeof(literalSuffix), &cb, &info.m_literalSuffixIsNull);
			info.m_literalSuffix = SQLAPICHARPTR_TO_EXODBCSTR(literalSuffix);
			GetData(m_pHStmt, 6, SQLAPICHARTYPENAME, createParams, sizeof(createParams), &cb, &info.m_createParamsIsNull);
			info.m_createParams = SQLAPICHARPTR_TO_EXODBCSTR(createParams);
			GetData(m_pHStmt, 7, SQL_C_SSHORT, &info.m_nullable, sizeof(info.m_nullable), &cb, NULL);
			GetData(m_pHStmt, 8, SQL_C_SSHORT, &info.m_caseSensitive, sizeof(info.m_caseSensitive), &cb, NULL);
			GetData(m_pHStmt, 9, SQL_C_SSHORT, &info.m_searchable, sizeof(info.m_searchable), &cb, NULL);
			GetData(m_pHStmt, 10, SQL_C_SSHORT, &info.m_unsigned, sizeof(info.m_unsigned), &cb, &info.m_unsignedIsNull);
			GetData(m_pHStmt, 11, SQL_C_SSHORT, &info.m_fixedPrecisionScale, sizeof(info.m_fixedPrecisionScale), &cb, NULL);
			GetData(m_pHStmt, 12, SQL_C_SSHORT, &info.m_autoUniqueValue, sizeof(info.m_autoUniqueValue), &cb, &info.m_autoUniqueValueIsNull);
			GetData(m_pHStmt, 13, SQLAPICHARTYPENAME, localTypeName, sizeof(localTypeName), &cb, &info.m_localTypeNameIsNull);
			info.m_localTypeName = SQLAPICHARPTR_TO_EXODBCSTR(localTypeName);
			GetData(m_pHStmt, 14, SQL_C_SSHORT, &info.m_minimumScale, sizeof(info.m_minimumScale), &cb, &info.m_minimumScaleIsNull);
			GetData(m_pHStmt, 15, SQL_C_SSHORT, &info.m_maximumScale, sizeof(info.m_maximumScale), &cb, &info.m_maximumScaleIsNull);
			GetData(m_pHStmt, 16, SQL_C_SSHORT, &info.m_sqlDataType, sizeof(info.m_sqlDataType), &cb, NULL);
			GetData(m_pHStmt, 17, SQL_C_SSHORT, &info.m_sqlDateTimeSub, sizeof(info.m_sqlDateTimeSub), &cb, &info.m_sqlDateTimeSubIsNull);
			GetData(m_pHStmt, 18, SQL_C_SSHORT, &info.m_numPrecRadix, sizeof(info.m_numPrecRadix), &cb, &info.m_numPrecRadixIsNull);
			GetData(m_pHStmt, 19, SQL_C_SSHORT, &info.m_intervalPrecision, sizeof(info.m_intervalPrecision), &cb, &info.m_intervalPrecisionIsNull);

			types.push_back(info);

			count++;
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

		// Iterate rows
		std::string catalogName, schemaName, tableName, columnName, typeName, remarks, defaultValue, isNullable;
		SQLSMALLINT sqlType, decimalDigits, numPrecRadix, nullable, sqlDataType, sqlDatetimeSub;
		SQLINTEGER columnSize, bufferSize, charOctetLength, ordinalPosition;
		bool isCatalogNull, isSchemaNull, isColumnSizeNull, isBufferSizeNull, isDecimalDigitsNull, isNumPrecRadixNull, isRemarksNull, isDefaultValueNull, isSqlDatetimeSubNull, isCharOctetLengthNull, isIsNullableNull;

		// Ensure ordinal-position is increasing constantly by one, starting at one
		SQLINTEGER m_lastIndex = 0;
		while ((ret = SQLFetch(m_pHStmt->GetHandle())) == SQL_SUCCESS)
		{
			// Fetch data from columns
			SQLLEN cb;
			GetData(m_pHStmt, 1, m_props.GetMaxCatalogNameLen(), catalogName, &isCatalogNull);
			GetData(m_pHStmt, 2, m_props.GetMaxSchemaNameLen(), schemaName, &isSchemaNull);
			GetData(m_pHStmt, 3, m_props.GetMaxTableNameLen(), tableName);
			GetData(m_pHStmt, 4, m_props.GetMaxColumnNameLen(), columnName);
			GetData(m_pHStmt, 5, SQL_C_SSHORT, &sqlType, sizeof(sqlType), &cb, NULL);
			GetData(m_pHStmt, 6, DB_MAX_TYPE_NAME_LEN, typeName);
			GetData(m_pHStmt, 7, SQL_C_SLONG, &columnSize, sizeof(columnSize), &cb, &isColumnSizeNull);
			GetData(m_pHStmt, 8, SQL_C_SLONG, &bufferSize, sizeof(bufferSize), &cb, &isBufferSizeNull);
			GetData(m_pHStmt, 9, SQL_C_SSHORT, &decimalDigits, sizeof(decimalDigits), &cb, &isDecimalDigitsNull);
			GetData(m_pHStmt, 10, SQL_C_SSHORT, &numPrecRadix, sizeof(numPrecRadix), &cb, &isNumPrecRadixNull);
			GetData(m_pHStmt, 11, SQL_C_SSHORT, &nullable, sizeof(nullable), &cb, NULL);
			GetData(m_pHStmt, 12, DB_MAX_COLUMN_REMARKS_LEN, remarks, &isRemarksNull);
			GetData(m_pHStmt, 13, DB_MAX_COLUMN_DEFAULT_LEN, defaultValue, &isDefaultValueNull);
			GetData(m_pHStmt, 14, SQL_C_SSHORT, &sqlDataType, sizeof(sqlDataType), &cb, NULL);
			GetData(m_pHStmt, 15, SQL_C_SSHORT, &sqlDatetimeSub, sizeof(sqlDatetimeSub), &cb, &isSqlDatetimeSubNull);
			GetData(m_pHStmt, 16, SQL_C_SLONG, &charOctetLength, sizeof(charOctetLength), &cb, &isCharOctetLengthNull);
			GetData(m_pHStmt, 17, SQL_C_SLONG, &ordinalPosition, sizeof(ordinalPosition), &cb, NULL);
			GetData(m_pHStmt, 18, DB_MAX_YES_NO_LEN, isNullable, &isIsNullableNull);

			if (++m_lastIndex != ordinalPosition)
			{
				Exception ex(u8"Columns are not ordered strictly by ordinal position");
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}

			ColumnInfo colInfo(catalogName, schemaName, tableName, columnName, sqlType, typeName, columnSize, bufferSize,
				decimalDigits, numPrecRadix, nullable, remarks, defaultValue, sqlDataType, sqlDatetimeSub, charOctetLength, ordinalPosition, isNullable,
				isCatalogNull, isSchemaNull, isColumnSizeNull, isBufferSizeNull, isDecimalDigitsNull, isNumPrecRadixNull, isRemarksNull, isDefaultValueNull,
				isSqlDatetimeSubNull, isIsNullableNull);
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
			SQLLEN cb;
			std::string catalogName, schemaName, tableName, columnName, keyName;
			bool isCatalogNull, isSchemaNull, isKeyNameNull;
			SQLSMALLINT keySequence;
			GetData(m_pHStmt, 1, m_props.GetMaxCatalogNameLen(), catalogName, &isCatalogNull);
			GetData(m_pHStmt, 2, m_props.GetMaxSchemaNameLen(), schemaName, &isSchemaNull);
			GetData(m_pHStmt, 3, m_props.GetMaxTableNameLen(), tableName);
			GetData(m_pHStmt, 4, m_props.GetMaxColumnNameLen(), columnName);
			GetData(m_pHStmt, 5, SQL_C_SHORT, &keySequence, sizeof(keySequence), &cb, NULL);
			GetData(m_pHStmt, 6, DB_MAX_PRIMARY_KEY_NAME_LEN, keyName, &isKeyNameNull);
			PrimaryKeyInfo pk(catalogName, schemaName, tableName, columnName, keySequence, keyName, isCatalogNull, isSchemaNull, isKeyNameNull);
			primaryKeys.push_back(pk);
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
			SQLLEN cb;

			SQLSMALLINT scopeVal;
			bool scopeIsNull;
			string columnName;
			SQLSMALLINT sqlType;
			string sqlTypeName;
			SQLINTEGER columnSize;
			SQLINTEGER bufferLength;
			SQLSMALLINT decimalDigits;
			SQLSMALLINT pseudoColVal;

			GetData(m_pHStmt, 1, SQL_C_SSHORT, &scopeVal, sizeof(scopeVal), &cb, &scopeIsNull);
			GetData(m_pHStmt, 2, m_props.GetMaxColumnNameLen(), columnName);
			GetData(m_pHStmt, 3, SQL_C_SSHORT, &sqlType, sizeof(sqlType), &cb, NULL);
			GetData(m_pHStmt, 4, DB_MAX_TYPE_NAME_LEN, sqlTypeName);
			GetData(m_pHStmt, 5, SQL_C_SLONG, &columnSize, sizeof(columnSize), &cb, NULL);
			GetData(m_pHStmt, 6, SQL_C_SLONG, &bufferLength, sizeof(bufferLength), &cb, NULL);
			GetData(m_pHStmt, 7, SQL_C_SSHORT, &decimalDigits, sizeof(decimalDigits), &cb, NULL);
			GetData(m_pHStmt, 8, SQL_C_SSHORT, &pseudoColVal, sizeof(pseudoColVal), &cb, NULL);

			SpecialColumnInfo::PseudoColumn pseudoCol = SpecialColumnInfo::PseudoColumn::UNKNOWN;
			if (pseudoColVal == (SQLSMALLINT)SpecialColumnInfo::PseudoColumn::NOT_PSEUDO)
				pseudoCol = SpecialColumnInfo::PseudoColumn::NOT_PSEUDO;
			else if (pseudoColVal == (SQLSMALLINT)SpecialColumnInfo::PseudoColumn::PSEUDO)
				pseudoCol = SpecialColumnInfo::PseudoColumn::PSEUDO;

			SpecialColumnInfo::RowIdScope scope;
			if (!scopeIsNull)
			{
				switch (scopeVal)
				{
				case (SQLSMALLINT)SpecialColumnInfo::RowIdScope::CURSOR:
					scope = SpecialColumnInfo::RowIdScope::CURSOR;
					break;
				case (SQLSMALLINT)SpecialColumnInfo::RowIdScope::SESSION:
					scope = SpecialColumnInfo::RowIdScope::SESSION;
					break;
				case (SQLSMALLINT)SpecialColumnInfo::RowIdScope::TRANSCATION:
					scope = SpecialColumnInfo::RowIdScope::TRANSCATION;
					break;
				default:
					exASSERT_MSG(false, boost::str(boost::format(u8"Unknown Row id scope value %d") % scopeVal));
				}
			}

			if (scopeIsNull)
			{
				SpecialColumnInfo specColInfo(columnName, sqlType, sqlTypeName, columnSize, bufferLength, decimalDigits, pseudoCol);
				columns.push_back(specColInfo);
			}
			else
			{
				SpecialColumnInfo specColInfo(columnName, scope, sqlType, sqlTypeName, columnSize, bufferLength, decimalDigits, pseudoCol);
				columns.push_back(specColInfo);
			}
		}
		THROW_IFN_NO_DATA(SQLFetch, ret);

		return columns;
	}
}
