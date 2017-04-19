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
		, m_stmtMode(MetadataMode::PatternValue)
	{
	};


	DatabaseCatalog::DatabaseCatalog(ConstSqlDbcHandlePtr pHdbc, const SqlInfoProperties& props)
		:m_pHStmt(std::make_shared<SqlStmtHandle>())
		, m_stmtMode(MetadataMode::PatternValue)
	{
		Init(pHdbc, props);
	}


	// Destructor
	// -----------
	DatabaseCatalog::~DatabaseCatalog()
	{
		if (m_pHStmt->IsAllocated())
		{
			m_pHStmt->Free();
		}
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


	DatabaseCatalog::MetadataMode DatabaseCatalog::GetMetadataAttribute() const
	{
		exASSERT(m_pHStmt);
		exASSERT(m_pHStmt->IsAllocated());

		// Read SQL_ATTR_METADATA_ID value, only modify if not already set to passed value
		SQLUINTEGER metadataAttr;
		SQLRETURN ret = SQLGetStmtAttr(m_pHStmt->GetHandle(), SQL_ATTR_METADATA_ID, (SQLPOINTER)&metadataAttr, 0, nullptr);
		THROW_IFN_SUCCEEDED(SQLGetStmtAttr, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());
		exASSERT_MSG(metadataAttr == SQL_FALSE || metadataAttr == SQL_TRUE,
			boost::str(boost::format(u8"Unknown value %d for SQL_ATTR_METADATA_ID read") % metadataAttr));

		if (metadataAttr == SQL_FALSE)
			return MetadataMode::PatternValue;
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
			m_stmtMode = MetadataMode::PatternValue;
		else
			m_stmtMode = MetadataMode::Identifier;
	}


	TableInfosVector DatabaseCatalog::FindTables(const std::string* pTableName, const std::string* pSchemaName, 
		const std::string* pCatalogName, const std::string& tableType, MetadataMode mode) const
	{
		if (m_stmtMode != mode)
			SetMetadataAttribute(mode);

		if (mode == MetadataMode::Identifier)
		{
			exASSERT_MSG(pTableName != nullptr, u8"pTableName must not be a nullptr if MetadataMode::Identifier is set");
			exASSERT_MSG(pTableName != nullptr, u8"pSchemaName must not be a nullptr if MetadataMode::Identifier is set");
			exASSERT_MSG(pTableName != nullptr, u8"pCatalogName must not be a nullptr if MetadataMode::Identifier is set");
		}

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_pHStmt, true, true);

		TableInfosVector tables;

		std::unique_ptr<SQLAPICHARTYPE[]> buffCatalog(new SQLAPICHARTYPE[m_props.GetMaxCatalogNameLen() + 1]);
		std::unique_ptr<SQLAPICHARTYPE[]> buffSchema(new SQLAPICHARTYPE[m_props.GetMaxSchemaNameLen() + 1]);
		std::unique_ptr<SQLAPICHARTYPE[]> buffTableName(new SQLAPICHARTYPE[m_props.GetMaxTableNameLen() + 1]);
		std::unique_ptr<SQLAPICHARTYPE[]> buffTableType(new SQLAPICHARTYPE[DB_MAX_TABLE_TYPE_LEN + 1]);
		std::unique_ptr<SQLAPICHARTYPE[]> buffTableRemarks(new SQLAPICHARTYPE[DB_MAX_TABLE_REMARKS_LEN + 1]);
		bool isCatalogNull = false;
		bool isSchemaNull = false;

		// Query db
		SQLRETURN ret = SQLTables(m_pHStmt->GetHandle(),
			pCatalogName == nullptr ? NULL : EXODBCSTR_TO_SQLAPICHARPTR(*pCatalogName), SQL_NTS,   // catname                 
			pSchemaName == nullptr ? NULL : EXODBCSTR_TO_SQLAPICHARPTR(*pSchemaName), SQL_NTS,   // schema name
			pTableName == nullptr ? NULL : EXODBCSTR_TO_SQLAPICHARPTR(*pTableName), SQL_NTS,	// table name
			tableType.empty() ? NULL : EXODBCSTR_TO_SQLAPICHARPTR(tableType), SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLTables, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		while ((ret = SQLFetch(m_pHStmt->GetHandle())) == SQL_SUCCESS)
		{
			buffCatalog[0] = 0;
			buffSchema[0] = 0;
			buffTableName[0] = 0;
			buffTableType[0] = 0;
			buffTableRemarks[0] = 0;

			SQLLEN cb;
			GetData(m_pHStmt, 1, SQLAPICHARTYPENAME, buffCatalog.get(), m_props.GetMaxCatalogNameLen() * sizeof(SQLAPICHARTYPE), &cb, &isCatalogNull);
			GetData(m_pHStmt, 2, SQLAPICHARTYPENAME, buffSchema.get(), m_props.GetMaxSchemaNameLen() * sizeof(SQLAPICHARTYPE), &cb, &isSchemaNull);
			GetData(m_pHStmt, 3, SQLAPICHARTYPENAME, buffTableName.get(), m_props.GetMaxTableNameLen() * sizeof(SQLAPICHARTYPE), &cb, NULL);
			GetData(m_pHStmt, 4, SQLAPICHARTYPENAME, buffTableType.get(), DB_MAX_TABLE_TYPE_LEN * sizeof(SQLAPICHARTYPE), &cb, NULL);
			GetData(m_pHStmt, 5, SQLAPICHARTYPENAME, buffTableRemarks.get(), DB_MAX_TABLE_REMARKS_LEN * sizeof(SQLAPICHARTYPE), &cb, NULL);

			TableInfo table(SQLAPICHARPTR_TO_EXODBCSTR(buffTableName.get()), SQLAPICHARPTR_TO_EXODBCSTR(buffTableType.get()),
				SQLAPICHARPTR_TO_EXODBCSTR(buffTableRemarks.get()), SQLAPICHARPTR_TO_EXODBCSTR(buffCatalog.get()),
				SQLAPICHARPTR_TO_EXODBCSTR(buffSchema.get()), isCatalogNull, isSchemaNull, m_props.DetectDbms());
			tables.push_back(table);
		}
		THROW_IFN_NO_DATA(SQLFetch, ret);

		return tables;
	}
}
