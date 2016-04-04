/*!
* \file Table.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 25.07.2014
* \brief Source file for the Table class and its helpers.
* \copyright GNU Lesser General Public License Version 3
*
* Header file for the Table class and its helpers.
*/

#include "stdafx.h"

// Own header
#include "Table.h"

// Same component headers
#include "Helpers.h"
#include "Database.h"
#include "Environment.h"
#include "Exception.h"
#include "Sql2BufferTypeMap.h"
#include "SqlStatementCloser.h"
#include "SqlCBufferVisitors.h"
#include "ExecutableStatement.h"

// Other headers

// Debug
#include "DebugNew.h"

using namespace std;

namespace exodbc
{
	// Construction
	// ------------
	Table::Table()
		: m_autoCreatedColumns(false)
		, m_haveTableInfo(false)
		, m_pDb(NULL)
		, m_pSql2BufferTypeMap(NULL)
		, m_isOpen(false)
		, m_tableAccessFlags(TableAccessFlag::AF_NONE)
		, m_openFlags(TableOpenFlag::TOF_NONE)
	{ }


	Table::Table(ConstDatabasePtr pDb, TableAccessFlags afs, const std::wstring& tableName, const std::wstring& schemaName /* = L"" */, const std::wstring& catalogName /* = L"" */, const std::wstring& tableType /* = L"" */)
		: m_autoCreatedColumns(false)
		, m_haveTableInfo(false)
		, m_pDb(NULL)
		, m_pSql2BufferTypeMap(NULL)
		, m_isOpen(false)
		, m_tableAccessFlags(TableAccessFlag::AF_NONE)
		, m_openFlags(TableOpenFlag::TOF_NONE)
	{
		Init(pDb, afs, tableName, schemaName, catalogName, tableType);
	}


	Table::Table(ConstDatabasePtr pDb, TableAccessFlags afs, const TableInfo& tableInfo)
		: m_autoCreatedColumns(false)
		, m_haveTableInfo(false)
		, m_pDb(NULL)
		, m_pSql2BufferTypeMap(NULL)
		, m_isOpen(false)
		, m_tableAccessFlags(TableAccessFlag::AF_NONE)
		, m_openFlags(TableOpenFlag::TOF_NONE)
	{
		Init(pDb, afs, tableInfo);
	}


	Table::Table(const Table& other)
		: m_autoCreatedColumns(false)
		, m_haveTableInfo(false)
		, m_pDb(NULL)
		, m_pSql2BufferTypeMap(NULL)
		, m_isOpen(false)
		, m_tableAccessFlags(TableAccessFlag::AF_NONE)
		, m_openFlags(TableOpenFlag::TOF_NONE)
	{
		// note: This constructor will always copy the search-names. Maybe they were set on other,
		// and then the TableInfo was searched. Do not loose the information about the search-names.
		if (other.HasTableInfo())
		{
			Init(other.m_pDb, other.GetAccessFlags(), other.m_tableInfo);
			m_initialTableName = other.m_initialTableName;
			m_initialSchemaName = other.m_initialSchemaName;
			m_initialCatalogName = other.m_initialCatalogName;
			m_initialTypeName = other.m_initialTypeName;
		}
		else
		{
			Init(other.m_pDb, other.GetAccessFlags(), other.m_initialTableName, other.m_initialSchemaName, other.m_initialCatalogName, other.m_initialTypeName);
		}
	}


	// Destructor
	// -----------
	Table::~Table()
	{
		try
		{
			if (IsOpen())
			{
				Close();
			}

			// If the table was created manually, we need to delete the column buffers
			// that were allocated during SetColumn.
			// If columns were created automatically, they were deleted from Open() in case of failure.
			// \note: We must free the buffers before deleting our statements - the buffers may still
			// be bound to our statements.
			//if (m_autoCreatedColumns)
			//{
			//	for (ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin(); it != m_columnBuffers.end(); ++it)
			//	{
			//		ColumnBuffer* pBuffer = it->second;
			//		delete pBuffer;
			//	}
			//	m_columnBuffers.clear();
			//}

			// FreeStatements();
			
			// Notify the columnBuffers that we have just freed all statements


			//exASSERT(m_columnBuffers.size() == 0);
		}
		catch (Exception& ex)
		{
			LOG_ERROR(ex.ToString());
		}
	}


	// Implementation
	// --------------
	void Table::Init(ConstDatabasePtr pDb, TableAccessFlags afs, const TableInfo& tableInfo)
	{
		exASSERT(pDb);
		exASSERT(pDb->IsOpen());
		
		// Remember properties passed
		exASSERT(m_pDb == NULL);
		m_pDb = pDb;

		// tableinfo passed
		m_haveTableInfo = true;
		m_tableInfo = tableInfo;

		// remember buffertype map, and allocate statements (by setting access flags)
		m_pSql2BufferTypeMap = m_pDb->GetSql2BufferTypeMap();
		SetAccessFlags(afs);

		// statements should now be allocated
		exASSERT(HasAllStatements());
	}


	void Table::Init(ConstDatabasePtr pDb, TableAccessFlags afs, const std::wstring& tableName, const std::wstring& schemaName /* = L"" */, const std::wstring& catalogName /* = L"" */, const std::wstring& tableType /* = L"" */)
	{
		exASSERT(pDb);
		exASSERT(pDb->IsOpen());

		// Remember properties passed
		exASSERT(m_pDb == NULL);
		m_pDb = pDb;

		// no tableinfo passed
		m_haveTableInfo = false;

		// but table search params
		m_initialTableName = tableName;
		m_initialSchemaName = schemaName;
		m_initialCatalogName = catalogName;
		m_initialTypeName = tableType;

		// remember buffertype map, and allocate statements (by setting access flags)
		m_pSql2BufferTypeMap = m_pDb->GetSql2BufferTypeMap();
		SetAccessFlags(afs);

		// statements should now be allocated
		exASSERT(HasAllStatements());
	}


	void Table::AllocateStatements(bool forwardOnlyCursors)
	{
		exASSERT(!IsOpen());
		exASSERT(m_pDb->IsOpen());

		//exASSERT(m_stmtCount->)
		//exASSERT(!m_pHStmtSelect->IsAllocated());
		//exASSERT(!m_pHStmtCount->IsAllocated());
		//exASSERT(SQL_NULL_HSTMT == m_hStmtCount);
		//exASSERT(SQL_NULL_HSTMT == m_hStmtSelect);
		//exASSERT(SQL_NULL_HSTMT == m_hStmtInsert);
		//exASSERT(SQL_NULL_HSTMT == m_hStmtUpdatePk);
		//exASSERT(SQL_NULL_HSTMT == m_hStmtDeleteWhere);
		//exASSERT(SQL_NULL_HSTMT == m_hStmtUpdateWhere);

		//SQLHDBC hdbc = m_pDb->GetConnectionHandle();
		ConstSqlDbcHandlePtr pHDbc = m_pDb->GetSqlDbcHandle();

		// Allocate handles needed, on failure try to de-allocate everything
		try
		{
			if (TestAccessFlag(TableAccessFlag::AF_SELECT))
			{
				// note: The count statements never needs scrollable cursors. If we enable them and then execute a
				// SELECT COUNT, ms sql server will report a warning saying 'Cursor type changed'.
				// and the inserts and updates do not need to be scrollable.
				m_execStmtCount.Init(m_pDb, true);
				m_execStmtSelect.Init(m_pDb, forwardOnlyCursors);

				// Create the buffer required for counts
				m_pSelectCountResultBuffer = UBigIntColumnBuffer::Create(L"", ColumnFlag::CF_SELECT);
			}
			if (TestAccessFlag(TableAccessFlag::AF_DELETE_PK))
			{
				m_execStmtDeletePk.Init(m_pDb, true);
			}
			if (TestAccessFlag(TableAccessFlag::AF_UPDATE_PK))
			{
				m_execStmtUpdatePk.Init(m_pDb, true);
			}
			if (TestAccessFlag(TableAccessFlag::AF_INSERT))
			{
				m_execStmtInsert.Init(m_pDb, true);
			}
		}
		catch (const Exception& ex)
		{
			HIDE_UNUSED(ex);
			m_pSelectCountResultBuffer.reset();
			m_execStmtCount.Reset();
			m_execStmtSelect.Reset();
			m_execStmtInsert.Reset();
			m_execStmtUpdatePk.Reset();
			m_execStmtDeletePk.Reset();

			// rethrow
			throw;
		}
	}


	bool Table::HasAllStatements() const throw()
	{
		bool haveAll = true;
		//if (haveAll && TestAccessFlag(TableAccessFlag::AF_SELECT))
		//{
		//	haveAll = m_pHStmtSelect->IsAllocated() && m_pHStmtCount->IsAllocated();
		//}
		//if (haveAll && TestAccessFlag(AF_UPDATE_PK))
		//{
		//	haveAll = (SQL_NULL_HSTMT != m_hStmtUpdatePk);
		//}
		//if (haveAll && TestAccessFlag(AF_UPDATE_WHERE))
		//{
		//	haveAll = (SQL_NULL_HSTMT != m_hStmtUpdateWhere);
		//}
		//if (haveAll && TestAccessFlag(AF_INSERT))
		//{
		//	haveAll = (SQL_NULL_HSTMT != m_hStmtInsert);
		//}
		//if (haveAll && TestAccessFlag(AF_DELETE_PK))
		//{
		//	haveAll = (SQL_NULL_HSTMT != m_hStmtDeletePk);
		//}
		//if (haveAll && TestAccessFlag(AF_DELETE_WHERE))
		//{
		//	haveAll = (SQL_NULL_HSTMT != m_hStmtDeleteWhere);
		//}
		return haveAll;
	}


	std::vector<ColumnBufferPtrVariant> Table::CreateAutoColumnBufferPtrs(bool skipUnsupportedColumns, bool setAsTableColumns)
	{
		exASSERT(m_pDb->IsOpen());

		const TableInfo& tableInfo = GetTableInfo();

		// Query Columns and create SqlCBuffers
		ColumnInfosVector columnInfos = m_pDb->ReadTableColumnInfo(tableInfo);
		exASSERT(columnInfos.size() <= SHRT_MAX);
		SQLSMALLINT numCols = (SQLSMALLINT)columnInfos.size();
		if (numCols == 0)
		{
			Exception ex((boost::wformat(L"No columns found for table '%s'") % tableInfo.GetQueryName()).str());
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}
		// Derive the ColumnFlags from the TableAccessFlags
		ColumnFlags flags;
		if (TestAccessFlag(TableAccessFlag::AF_SELECT))
		{
			flags.Set(ColumnFlag::CF_SELECT);
		}
		if (TestAccessFlag(TableAccessFlag::AF_UPDATE_WHERE) || TestAccessFlag(TableAccessFlag::AF_UPDATE_PK))
		{
			flags.Set(ColumnFlag::CF_UPDATE);
		}
		if (TestAccessFlag(TableAccessFlag::AF_INSERT))
		{
			flags.Set(ColumnFlag::CF_INSERT);
		}

		std::vector<ColumnBufferPtrVariant> columns;
		for (int columnIndex = 0; columnIndex < (SQLUSMALLINT)columnInfos.size(); columnIndex++)
		{
			const ColumnInfo& colInfo = columnInfos[columnIndex];
			try
			{
				SQLSMALLINT sqlCType = m_pSql2BufferTypeMap->GetBufferType(colInfo.GetSqlType());
				ColumnBufferPtrVariant columnPtrVariant;
				if (IsArrayType(sqlCType))
				{
					columnPtrVariant = CreateColumnArrayBufferPtr(sqlCType, colInfo.GetQueryName(), colInfo);
				}
				else
				{
					columnPtrVariant = CreateColumnBufferPtr(sqlCType, colInfo.GetQueryName());
				}
				std::shared_ptr<ColumnFlags> pColumnFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), columnPtrVariant);
				pColumnFlags->Set(flags);
				std::shared_ptr<ExtendedColumnPropertiesHolder> pExtendedProps = boost::apply_visitor(ExtendedColumnPropertiesHolderPtrVisitor(), columnPtrVariant);
				pExtendedProps->SetSqlType(colInfo.GetSqlType());
				if(!colInfo.IsColumnSizeNull())
				{
					pExtendedProps->SetColumnSize(colInfo.GetColumnSize());
				}
				if (!colInfo.IsDecimalDigitsNull())
				{
					pExtendedProps->SetDecimalDigits(colInfo.GetDecimalDigits());
				}
				columns.push_back(columnPtrVariant);
			}
			catch (const boost::bad_polymorphic_get& ex)
			{
				WrapperException we(ex);
				SET_EXCEPTION_SOURCE(we);
				throw we;
			}
			catch (const NotSupportedException& nse)
			{
				if (skipUnsupportedColumns)
				{
					// Ignore unsupported column. (note: If it has thrown from the constructor, memory is already deleted)
					LOG_WARNING(boost::str(boost::wformat(L"Failed to create ColumnBuffer for column '%s': %s") % colInfo.GetQueryName() % nse.ToString()));
					continue;
				}
				else
				{
					// rethrow
					throw;
				}
			}
		}

		if (setAsTableColumns)
		{
			for (size_t i = 0; i < columns.size(); ++i)
			{
				m_columns[(SQLUSMALLINT)i] = columns[i];
			}
			m_autoCreatedColumns = true;
		}

		return columns;
	}


	void Table::SetColumnFlag(SQLUSMALLINT columnIndex, ColumnFlag flag) const
	{
		exASSERT(!IsOpen());

		ColumnBufferPtrVariant var = GetColumnBufferPtrVariant(columnIndex);
		ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), var);
		pFlags->Set(flag);
	}


	void Table::ClearColumnFlag(SQLUSMALLINT columnIndex, ColumnFlag flag) const
	{
		exASSERT(!IsOpen());

		ColumnBufferPtrVariant var = GetColumnBufferPtrVariant(columnIndex);
		ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), var);
		pFlags->Clear(flag);
	}


	ColumnBufferPtrVariantMap Table::GetPrimaryKeyColumnBuffers() const
	{
		ColumnBufferPtrVariantMap primaryKeys;
		for (auto it = m_columns.begin(); it != m_columns.end(); ++it)
		{
			ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), it->second);
			if (pFlags->Test(ColumnFlag::CF_PRIMARY_KEY))
			{
				primaryKeys[it->first] = it->second;
			}
		}
		return primaryKeys;
	}


	const TableInfo& Table::GetTableInfo()
	{
		if (!m_haveTableInfo)
		{
			m_tableInfo = m_pDb->FindOneTable(m_initialTableName, m_initialSchemaName, m_initialCatalogName, m_initialTypeName);
			m_haveTableInfo = true;
		}

		return m_tableInfo;
	}


	TableInfo Table::GetTableInfo() const
	{
		if (!m_haveTableInfo)
		{
			return m_pDb->FindOneTable(m_initialTableName, m_initialSchemaName, m_initialCatalogName, m_initialTypeName);
		}
		return m_tableInfo;
	}


	void Table::CheckPrivileges() const
	{
		exASSERT(m_pDb);

		const TableInfo& tableInfo = GetTableInfo();

		TablePrivileges tablePrivs;
		tablePrivs.Init(m_pDb, tableInfo);

		if (TestAccessFlag(TableAccessFlag::AF_SELECT) && !tablePrivs.Test(TablePrivilege::SELECT))
		{
			MissingTablePrivilegeException e(TablePrivilege::SELECT, tableInfo);
			SET_EXCEPTION_SOURCE(e);
			throw e;
		}
		if (TestAccessFlag(TableAccessFlag::AF_INSERT) && !tablePrivs.Test(TablePrivilege::INSERT))
		{
			MissingTablePrivilegeException e(TablePrivilege::INSERT, tableInfo);
			SET_EXCEPTION_SOURCE(e);
			throw e;
		}
		if ((TestAccessFlag(TableAccessFlag::AF_DELETE_PK) || TestAccessFlag(TableAccessFlag::AF_DELETE_WHERE)) && !tablePrivs.Test(TablePrivilege::DEL))
		{
			MissingTablePrivilegeException e(TablePrivilege::DEL, tableInfo);
			SET_EXCEPTION_SOURCE(e);
			throw e;
		}
		if ((TestAccessFlag(TableAccessFlag::AF_UPDATE_PK) || TestAccessFlag(TableAccessFlag::AF_UPDATE_WHERE)) && !tablePrivs.Test(TablePrivilege::UPDATE))
		{
			MissingTablePrivilegeException e(TablePrivilege::UPDATE, tableInfo);
			SET_EXCEPTION_SOURCE(e);
			throw e;
		}
	}


	void Table::FreeStatements()
	{
		// Do NOT check for IsOpen() here. If Open() fails it will call FreeStatements to do its cleanup
		// exASSERT(IsOpen());

		m_pSelectCountResultBuffer.reset();

		m_execStmtCount.Reset();
		m_execStmtSelect.Reset();
		m_execStmtInsert.Reset();
		m_execStmtUpdatePk.Reset();
		m_execStmtDeletePk.Reset();
	}


	std::wstring Table::BuildSelectFieldsStatement() const
	{
		exASSERT(m_columns.size() > 0);

		std::wstring fields = L"";
		ColumnBufferPtrVariantMap::const_iterator it = m_columns.begin();
		while (it != m_columns.end())
		{
			ColumnBufferPtrVariant var = it->second;
			ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), var);
			if (pFlags->Test(ColumnFlag::CF_SELECT))
			{
				std::wstring queryName = boost::apply_visitor(QueryNameVisitor(), var);
				fields += queryName;
				fields += L", ";
			}
			++it;
		}
		boost::algorithm::erase_last(fields, L", ");

		return fields;
	}


	void Table::BindDeleteParameters()
	{
		exASSERT(!m_columns.empty());
		exASSERT(TestAccessFlag(TableAccessFlag::AF_DELETE_PK));
		exASSERT(m_execStmtDeletePk.IsInitialized());

		// Build a statement with parameter-markers
		vector<ColumnBufferPtrVariant> whereParamsToBind;
		wstring whereMarkers;
		auto it = m_columns.begin();
		while (it != m_columns.end())
		{
			ColumnBufferPtrVariant pVar = it->second;
			ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), pVar);
			if (pFlags->Test(ColumnFlag::CF_PRIMARY_KEY))
			{
				whereMarkers += boost::apply_visitor(QueryNameVisitor(), pVar);
				whereMarkers += L" = ?, ";
				whereParamsToBind.push_back(pVar);
			}
			++it;
		}
		boost::erase_last(whereMarkers, L", ");
		wstring stmt = boost::str(boost::wformat(L"DELETE FROM %s WHERE %s") % m_tableInfo.GetQueryName() % whereMarkers);

		// check that we actually have some columns to update and some for the where part
		exASSERT_MSG(!whereParamsToBind.empty(), L"No Columns usable to construct WHERE statement");

		// .. and prepare stmt
		m_execStmtDeletePk.Prepare(stmt);

		// and binding of columns... we must do that after calling Prepare - or
		// not use SqlDescribeParam during Bind
		// first come the set params, then the where markers
		SQLSMALLINT paramNr = 1;
		for (auto it = whereParamsToBind.begin(); it != whereParamsToBind.end(); ++it)
		{
			m_execStmtDeletePk.BindParameter(*it, paramNr);
			++paramNr;
		}
	}


	void Table::BindUpdatePkParameters()
	{
		exASSERT(!m_columns.empty());
		exASSERT(TestAccessFlag(TableAccessFlag::AF_UPDATE_PK));
		exASSERT(m_execStmtUpdatePk.IsInitialized());

		// Build a statement with parameter-markers
		vector<ColumnBufferPtrVariant> setParamsToBind;
		vector<ColumnBufferPtrVariant> whereParamsToBind;
		wstring setMarkers;
		wstring whereMarkers;
		auto it = m_columns.begin();
		while (it != m_columns.end())
		{
			ColumnBufferPtrVariant pVar = it->second;
			ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), pVar);
			if (pFlags->Test(ColumnFlag::CF_PRIMARY_KEY))
			{
				whereMarkers += boost::apply_visitor(QueryNameVisitor(), pVar);
				whereMarkers += L" = ?, ";
				whereParamsToBind.push_back(pVar);
			}
			else if (pFlags->Test(ColumnFlag::CF_UPDATE))
			{				
				setMarkers += boost::apply_visitor(QueryNameVisitor(), pVar);
				setMarkers += L" = ?, ";
				setParamsToBind.push_back(pVar);
			}
			++it;
		}
		boost::erase_last(setMarkers, L", ");
		boost::erase_last(whereMarkers, L", ");
		wstring stmt = boost::str(boost::wformat(L"UPDATE %s SET %s WHERE %s") % m_tableInfo.GetQueryName() % setMarkers % whereMarkers);

		// check that we actually have some columns to update and some for the where part
		exASSERT_MSG(!setParamsToBind.empty(), L"No Columns flaged for UPDATEing");
		exASSERT_MSG(!whereParamsToBind.empty(), L"No Columns usable to construct WHERE statement");

		// .. and prepare stmt
		m_execStmtUpdatePk.Prepare(stmt);

		// and binding of columns... we must do that after calling Prepare - or
		// not use SqlDescribeParam during Bind
		// first come the set params, then the where markers
		SQLSMALLINT paramNr = 1;
		for (auto it = setParamsToBind.begin(); it != setParamsToBind.end(); ++it)
		{
			m_execStmtUpdatePk.BindParameter(*it, paramNr);
			++paramNr;
		}
		for (auto it = whereParamsToBind.begin(); it != whereParamsToBind.end(); ++it)
		{
			m_execStmtUpdatePk.BindParameter(*it, paramNr);
			++paramNr;
		}
	}


	void Table::BindInsertParameters()
	{
		exASSERT( ! m_columns.empty());
		exASSERT(TestAccessFlag(TableAccessFlag::AF_INSERT));
		exASSERT(m_execStmtInsert.IsInitialized());

		// Build a statement with parameter-markers
		vector<ColumnBufferPtrVariant> colsToBind;
		wstring markers;
		wstring fields;
		auto it = m_columns.begin();
		while (it != m_columns.end())
		{
			ColumnBufferPtrVariant pVar = it->second;
			ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), pVar);
			if (pFlags->Test(ColumnFlag::CF_INSERT))
			{
				fields+= boost::apply_visitor(QueryNameVisitor(), pVar);
				fields += L", ";
				markers+= L"?, ";
				colsToBind.push_back(pVar);
			}
			++it;
		}
		boost::erase_last(fields, L", ");
		boost::erase_last(markers, L", ");
		wstring stmt = boost::str(boost::wformat(L"INSERT INTO %s (%s) VALUES(%s)") % m_tableInfo.GetQueryName() % fields % markers);

		// check that we actually have some column to insert
		exASSERT_MSG( ! colsToBind.empty(), L"No Columns flaged for INSERTing");

		// .. and prepare stmt
		m_execStmtInsert.Prepare(stmt);

		// and binding of columns... we must do that after calling Prepare - or
		// not use SqlDescribeParam during Bind
		for (size_t i = 0; i < colsToBind.size(); ++i)
		{
			m_execStmtInsert.BindParameter(colsToBind[i], (SQLSMALLINT)(i + 1));
		}
	}


	bool Table::ColumnBufferExists(SQLSMALLINT columnIndex) const throw()
	{
		ColumnBufferPtrVariantMap::const_iterator it = m_columns.find(columnIndex);
		return it != m_columns.end();
	}


	const ColumnBufferPtrVariant& Table::GetColumnBufferPtrVariant(SQLSMALLINT columnIndex) const
	{
		ColumnBufferPtrVariantMap::const_iterator it = m_columns.find(columnIndex);
		if (it == m_columns.end())
		{
			IllegalArgumentException ex(boost::str(boost::wformat(L"ColumnIndex %d is not a zero-based bound or manually defined ColumnBuffer") % columnIndex));
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}

		return it->second;
	}


	const ColumnBufferPtrVariant& Table::GetNonNullColumnBufferPtrVariant(SQLSMALLINT columnIndex) const
	{
		const ColumnBufferPtrVariant& columnPtrVariant = GetColumnBufferPtrVariant(columnIndex);
		if (boost::apply_visitor(IsNullVisitor(), columnPtrVariant))
		{
			std::wstring queryName = boost::apply_visitor(QueryNameVisitor(), columnPtrVariant);
			NullValueException ex(queryName);
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}
		return columnPtrVariant;
	}


	void Table::QueryPrimaryKeysAndUpdateColumns() const
	{
		exASSERT(m_haveTableInfo);
		exASSERT(m_pDb);
		
		TablePrimaryKeysVector keys = m_pDb->ReadTablePrimaryKeys(m_tableInfo);
		for (auto it = keys.begin(); it != keys.end(); ++it)
		{
			const TablePrimaryKeyInfo& keyInfo = *it;
			SQLUSMALLINT colIndex = GetColumnBufferIndex(keyInfo.GetQueryName());
			ColumnBufferPtrVariant column = GetColumnBufferPtrVariant(colIndex);
			ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), column);
			pFlags->Set(ColumnFlag::CF_PRIMARY_KEY);
		}
	}


	void Table::CheckColumnFlags() const
	{
		for (auto it = m_columns.begin(); it != m_columns.end(); ++it)
		{
			ColumnBufferPtrVariant columnBuffer = it->second;
			const std::wstring& queryName = boost::apply_visitor(QueryNameVisitor(), columnBuffer);
			ColumnFlagsPtr pColumnFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), columnBuffer);
			if (pColumnFlags->Test(ColumnFlag::CF_SELECT) && !TestAccessFlag(TableAccessFlag::AF_SELECT))
			{
				Exception ex(boost::str(boost::wformat(L"Defined Column %s (%d) has ColumnFlag CF_SELECT set, but the AccessFlag AF_SELECT is not set on the table %s") % queryName % it->first %m_tableInfo.GetQueryName()));
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
			if (pColumnFlags->Test(ColumnFlag::CF_INSERT) && !TestAccessFlag(TableAccessFlag::AF_INSERT))
			{
				Exception ex(boost::str(boost::wformat(L"Defined Column %s (%d) has ColumnFlag CF_INSERT set, but the AccessFlag AF_INSERT is not set on the table %s") % queryName % it->first %m_tableInfo.GetQueryName()));
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
			if (pColumnFlags->Test(ColumnFlag::CF_UPDATE) && !(TestAccessFlag(TableAccessFlag::AF_UPDATE_PK) || TestAccessFlag(TableAccessFlag::AF_UPDATE_WHERE)))
			{
				Exception ex(boost::str(boost::wformat(L"Defined Column %s (%d) has ColumnFlag CF_UPDATE set, but the AccessFlag AF_UPDATE is not set on the table %s") % queryName % it->first %m_tableInfo.GetQueryName()));
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
		}
	}


	void Table::CheckSqlTypes(bool removeUnsupported)
	{
		auto it = m_columns.begin();
		while(it != m_columns.end())
		{
			ColumnBufferPtrVariant columnBuffer = it->second;
			const std::wstring& queryName = boost::apply_visitor(QueryNameVisitor(), columnBuffer);
			ColumnFlagsPtr pColumnFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), columnBuffer);
			SQLSMALLINT sqlType = boost::apply_visitor(SqlTypeVisitor(), columnBuffer);
			bool remove = false;
			if (!m_pDb->IsSqlTypeSupported(sqlType))
			{
				if (TestOpenFlag(TableOpenFlag::TOF_SKIP_UNSUPPORTED_COLUMNS))
				{
					LOG_WARNING(boost::str(boost::wformat(L"Defined Column %s (%d) has SQL Type %s (%d) set, but the Database did not report this type as a supported SQL Type. The Column is skipped due to the flag TOF_SKIP_UNSUPPORTED_COLUMNS") % queryName % it->first % SqlType2s(sqlType) % sqlType));
					remove = true;
				}			
				else
				{
					Exception ex(boost::str(boost::wformat(L"Defined Column %s (%d) has SQL Type %s (%d) set, but the Database did not report this type as a supported SQL Type.") % queryName % it->first % SqlType2s(sqlType) % sqlType));
					SET_EXCEPTION_SOURCE(ex);
					throw ex;
				}
			}
			if (remove)
			{
				it = m_columns.erase(it);
			}
			else
			{
				++it;
			}
		}
	}


	std::set<SQLUSMALLINT> Table::GetColumnBufferIndexes() const throw()
	{
		std::set<SQLUSMALLINT> columnIndexes;
		ColumnBufferPtrVariantMap::const_iterator it = m_columns.begin();
		while (it != m_columns.end())
		{
			columnIndexes.insert(it->first);
			++it;
		}
		return columnIndexes;
	}


	SQLUSMALLINT Table::GetColumnBufferIndex(const std::wstring& columnQueryName, bool caseSensitive /* = true */) const
	{
		ColumnBufferPtrVariantMap::const_iterator it = m_columns.begin();
		while (it != m_columns.end())
		{
			const ColumnBufferPtrVariant& column = it->second;
			std::wstring queryName = boost::apply_visitor(QueryNameVisitor(), column);

			if (caseSensitive)
			{		
				if (queryName == columnQueryName)
				{
					return it->first;
				}
			}
			else
			{
				if (boost::iequals(queryName, columnQueryName))
				{
					return it->first;
				}
			}
			++it;
		}

		NotFoundException nfe(boost::str(boost::wformat(L"No ColumnBuffer found with QueryName '%s'") % columnQueryName));
		SET_EXCEPTION_SOURCE(nfe);
		throw nfe;
	}


	SQLUBIGINT Table::Count(const std::wstring& whereStatement)
	{
		exASSERT(IsOpen());
		exASSERT(m_tableAccessFlags.Test(TableAccessFlag::AF_SELECT));
		exASSERT(m_pSelectCountResultBuffer);

		// Prepare the sql to be executed on our internal ExecutableStatement
		std::wstring sqlstmt;
		if ( ! whereStatement.empty())
		{
			sqlstmt = (boost::wformat(L"SELECT COUNT(*) FROM %s WHERE %s") % m_tableInfo.GetQueryName() % whereStatement).str();
		}
		else
		{
			sqlstmt = (boost::wformat(L"SELECT COUNT(*) FROM %s") % m_tableInfo.GetQueryName()).str();
		}

		m_execStmtCount.ExecuteDirect(sqlstmt);
		exASSERT(m_execStmtCount.SelectNext());

		m_execStmtCount.SelectClose();

		return *m_pSelectCountResultBuffer;
	}


	void Table::Select(const std::wstring& whereStatement /* = L"" */)
	{
		exASSERT(IsOpen());

		std::wstring sqlstmt;
		if (!whereStatement.empty())
		{
			sqlstmt = (boost::wformat(L"SELECT %s FROM %s WHERE %s") % BuildSelectFieldsStatement() % m_tableInfo.GetQueryName() % whereStatement).str();
		}
		else
		{
			sqlstmt = (boost::wformat(L"SELECT %s FROM %s") % BuildSelectFieldsStatement() % m_tableInfo.GetQueryName()).str();
		}
		SelectBySqlStmt(sqlstmt);
	}


	void Table::SelectBySqlStmt(const std::wstring& sqlStmt)
	{
		exASSERT(IsOpen());
		exASSERT(m_tableAccessFlags.Test(TableAccessFlag::AF_SELECT));
		exASSERT(!sqlStmt.empty());

		m_execStmtSelect.ExecuteDirect(sqlStmt);
	}


	bool Table::SelectPrev()
	{
		return m_execStmtSelect.SelectPrev();
	}


	bool Table::SelectFirst()
	{
		return m_execStmtSelect.SelectFirst();
	}


	bool Table::SelectLast()
	{
		return m_execStmtSelect.SelectLast();
	}


	bool Table::SelectAbsolute(SQLLEN position)
	{
		return m_execStmtSelect.SelectAbsolute(position);
	}


	bool Table::SelectRelative(SQLLEN offset)
	{
		return m_execStmtSelect.SelectRelative(offset);
	}


	bool Table::SelectNext()
	{
		return m_execStmtSelect.SelectNext();
	}

	
	void Table::SelectClose()
	{
		m_execStmtSelect.SelectClose();
	}


	void Table::Insert() const
	{
		exASSERT(IsOpen());
		exASSERT(TestAccessFlag(TableAccessFlag::AF_INSERT));
		m_execStmtInsert.ExecutePrepared();
	}


	void Table::Delete(bool failOnNoData /* = true */)
	{
		exASSERT(IsOpen());
		exASSERT(TestAccessFlag(TableAccessFlag::AF_DELETE_PK));

		try
		{
			m_execStmtDeletePk.ExecutePrepared();
		}
		catch (const SqlResultException& ex)
		{
			HIDE_UNUSED(ex);
			if (!failOnNoData && ex.GetRet() == SQL_NO_DATA)
			{
				return;
			}
			throw;
		}
	}


	void Table::Delete(const std::wstring& where, bool failOnNoData /* = true */) const
	{
		exASSERT(!m_columns.empty());
		exASSERT(TestAccessFlag(TableAccessFlag::AF_DELETE_WHERE));
		exASSERT(!where.empty());

		ExecutableStatement execStmtDelete;
		execStmtDelete.Init(m_pDb, true);

		// Build a statement that we can directly execute
		wstring stmt = boost::str(boost::wformat(L"DELETE FROM %s WHERE %s") % m_tableInfo.GetQueryName() % where);
		
		try
		{
			execStmtDelete.ExecuteDirect(stmt);
		}
		catch (const SqlResultException& ex)
		{
			HIDE_UNUSED(ex);
			if (!failOnNoData && ex.GetRet() == SQL_NO_DATA)
			{
				return;
			}
			throw;
		}
	}


	void Table::Update()
	{
		exASSERT(IsOpen());
		exASSERT(TestAccessFlag(TableAccessFlag::AF_UPDATE_PK));
		m_execStmtUpdatePk.ExecutePrepared();
	}


	void Table::Update(const std::wstring& where)
	{
		exASSERT(!m_columns.empty());
		exASSERT(TestAccessFlag(TableAccessFlag::AF_UPDATE_WHERE));
		exASSERT(!where.empty());

		ExecutableStatement execStmtUpdate;
		execStmtUpdate.Init(m_pDb, true);

		// Build a statement with parameter-markers
		vector<ColumnBufferPtrVariant> setParamsToBind;
		wstring setMarkers;
		auto it = m_columns.begin();
		while (it != m_columns.end())
		{
			ColumnBufferPtrVariant pVar = it->second;
			ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), pVar);
			if (pFlags->Test(ColumnFlag::CF_UPDATE))
			{
				setMarkers += boost::apply_visitor(QueryNameVisitor(), pVar);
				setMarkers += L" = ?, ";
				setParamsToBind.push_back(pVar);
			}
			++it;
		}
		boost::erase_last(setMarkers, L", ");
		wstring stmt = boost::str(boost::wformat(L"UPDATE %s SET %s WHERE %s") % m_tableInfo.GetQueryName() % setMarkers % where);

		// check that we actually have some columns to update and some for the where part
		exASSERT_MSG(!setParamsToBind.empty(), L"No Columns flaged for UPDATEing");

		// .. and prepare stmt
		execStmtUpdate.Prepare(stmt);

		// and binding of columns... we must do that after calling Prepare - or
		// not use SqlDescribeParam during Bind
		// first come the set params, then the where markers
		SQLSMALLINT paramNr = 1;
		for (auto it = setParamsToBind.begin(); it != setParamsToBind.end(); ++it)
		{
			execStmtUpdate.BindParameter(*it, paramNr);
			++paramNr;
		}

		// And do the update
		execStmtUpdate.ExecutePrepared();
	}


	SQLLEN Table::SelectColumnAttribute(SQLSMALLINT columnIndex, ColumnAttribute attr)
	{
		//exASSERT(IsSelectOpen());
		//SQLLEN value = 0;
		//SQLRETURN ret = SQLColAttribute(m_hStmtSelect, columnIndex + 1, (SQLUSMALLINT)attr, NULL, NULL, NULL, &value);
		//THROW_IFN_SUCCEEDED(SQLColAttribute, ret, SQL_HANDLE_STMT, m_hStmtSelect);
		//return value;

		return 0;
	}


	void Table::SetBinaryValue(SQLSMALLINT columnIndex, const SQLCHAR* buffer, SQLINTEGER bufferSize)
	{
		//ColumnBuffer* pBuff = GetColumnBuffer(columnIndex);
		//pBuff->SetBinaryValue(buffer, bufferSize);
	}


	void Table::SetColumnNull(SQLSMALLINT columnIndex) const
	{
		ColumnBufferPtrVariant var = GetColumnBufferPtrVariant(columnIndex);
		ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), var);
		exASSERT(pFlags->Test(ColumnFlag::CF_NULLABLE));
		ColumnBufferLengthIndicatorPtr pCb = boost::apply_visitor(ColumnBufferLengthIndicatorPtrVisitor(), var);
		pCb->SetNull();
	}

	

	//BufferVariant Table::GetColumnValue(SQLSMALLINT columnIndex) const
	//{
	//	ColumnBuffer* pBuff = GetColumnBuffer(columnIndex);
	//	return pBuff->GetValue();
	//}


	const SQLCHAR* Table::GetBinaryValue(SQLSMALLINT columnIndex, SQLLEN& bufferSize, SQLLEN& lengthIndicator) const
	{
		//const SqlBinaryArray& column = GetNonNullColumn<SqlBinaryArray>(columnIndex);
		//const ColumnBufferPtrVariant& var = GetColumnVariant(columnIndex);
		//try
		//{
		//	const SqlCBufferLengthIndicator& cb = boost::polymorphic_get<SqlCBufferLengthIndicator>(var);
		//	lengthIndicator = cb.GetCb();
		//}
		//catch (const boost::bad_polymorphic_get& ex)
		//{
		//	WrapperException we(ex);
		//	SET_EXCEPTION_SOURCE(we);
		//	throw we;
		//}

		//bufferSize = column.GetBufferLength();
		//return column.GetBuffer().get();
		return NULL;
	}


	bool Table::IsColumnNull(SQLSMALLINT columnIndex) const
	{
		ColumnBufferPtrVariant var = GetColumnBufferPtrVariant(columnIndex);
		ColumnBufferLengthIndicatorPtr pCb = boost::apply_visitor(ColumnBufferLengthIndicatorPtrVisitor(), var);
		return pCb->IsNull();
	}


	bool Table::IsColumnNullable(SQLSMALLINT columnIndex) const
	{
		ColumnBufferPtrVariant var = GetColumnBufferPtrVariant(columnIndex);
		ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), var);
		return pFlags->Test(ColumnFlag::CF_NULLABLE);
	}


	void Table::SetColumn(SQLUSMALLINT columnIndex, ColumnBufferPtrVariant column)
	{
		exASSERT(columnIndex >= 0);
		exASSERT(m_columns.find(columnIndex) == m_columns.end());

		// test if we have a non-empty query name
		const std::wstring& queryName = boost::apply_visitor(QueryNameVisitor(), column);
		exASSERT(!queryName.empty());

		// okay, remember the passed variant
		m_columns[columnIndex] = column;
	}


	void Table::SetColumn(SQLUSMALLINT columnIndex, const std::wstring& queryName, SQLSMALLINT sqlType, SQLPOINTER pBuffer, SQLSMALLINT sqlCType, SQLLEN bufferSize, ColumnFlags flags, SQLINTEGER columnSize /* = 0 */, SQLSMALLINT decimalDigits /* = 0 */)
	{
		SqlCPointerBufferPtr pColumn = std::make_shared<SqlCPointerBuffer>(queryName, sqlType, pBuffer, sqlCType, bufferSize, flags, columnSize, decimalDigits);
		SetColumn(columnIndex, pColumn);
	}


	//void Table::SetColumn(SQLUSMALLINT columnIndex, const std::wstring& queryName, SQLSMALLINT sqlType, BufferPtrVariant pBuffer, SQLSMALLINT sqlCType, SQLLEN bufferSize, OldColumnFlags flags, SQLINTEGER columnSize /* = -1 */, SQLSMALLINT decimalDigits /* = -1 */)
	//{
	//	//exASSERT(columnIndex >= 0);
	//	//exASSERT( ! queryName.empty());
	//	//exASSERT(bufferSize > 0);
	//	//exASSERT(m_columnBuffers.find(columnIndex) == m_columnBuffers.end());
	//	//exASSERT( ! ( (sqlType == SQL_UNKNOWN_TYPE) && ((flags & CF_INSERT) || (flags & CF_UPDATE)) ) );
	//	//exASSERT(!IsOpen());
	//	//// Flag that columns are created manually
	//	//m_autoCreatedColumns = true;
	//	//// Read OdbcVersion from Environment
	//	//const Environment* pEnv = m_pDb->GetEnvironment();
	//	//exASSERT(pEnv != NULL);
	//	//ManualColumnInfo colInfo(sqlType, queryName, columnSize, decimalDigits);
	//	//ColumnBuffer* pColumnBuffer = new ColumnBuffer(colInfo, sqlCType, pBuffer, bufferSize, pEnv->GetOdbcVersion(), flags);
	//	//m_columnBuffers[columnIndex] = pColumnBuffer;
	//}


	void Table::SetColumnPrimaryKeyIndexes(const std::set<SQLUSMALLINT>& columnIndexes)
	{
		exASSERT(!IsOpen());
		m_primaryKeyColumnIndexes = columnIndexes;
	}


	void Table::SetSql2BufferTypeMap(Sql2BufferTypeMapPtr pSql2BufferTypeMap)
	{
		exASSERT(!IsOpen());
		exASSERT(pSql2BufferTypeMap);
		m_pSql2BufferTypeMap = pSql2BufferTypeMap;
	}


	const Sql2BufferTypeMapPtr Table::GetSql2BufferTypeMap() const
	{
		exASSERT(m_pSql2BufferTypeMap);
		return m_pSql2BufferTypeMap;
	}


	void Table::Open(TableOpenFlags openFlags /* = TOF_CHECK_EXISTANCE */)
	{
		exASSERT(m_pDb);
		exASSERT(m_pDb->IsOpen());
		exASSERT(!IsOpen());

		// Init open flags and update them with flags implicitly set by checking db-driver
		m_openFlags = openFlags;

		// Set TOF_DO_NOT_QUERY_PRIMARY_KEYS this flag for Access, Access does not support SQLPrimaryKeys
		if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			m_openFlags.Set(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS);
		}

		// Access and Excel seem to support only forward cursors, so activate that flag
		if (m_pDb->GetDbms() == DatabaseProduct::ACCESS || m_pDb->GetDbms() == DatabaseProduct::EXCEL)
		{
			m_openFlags.Set(TableOpenFlag::TOF_FORWARD_ONLY_CURSORS);
		}

		// Allocate all statements we need
		AllocateStatements(m_openFlags.Test(TableOpenFlag::TOF_FORWARD_ONLY_CURSORS));

		// Nest try/catch the free the buffers created in here if we fail somewhere
		// and to unbind all handles that were bound
		try
		{
			// If we do not already have a TableInfo for our table, we absolutely must find one
			bool searchedTable = false;
			if (!m_haveTableInfo)
			{
				// Finding will throw if not exactly one is found
				m_tableInfo = m_pDb->FindOneTable(m_initialTableName, m_initialSchemaName, m_initialCatalogName, m_initialTypeName);
				m_haveTableInfo = true;
				searchedTable = true;
			}

			// If we are asked to check existence and have not just proved we exist just find a table
			// Search using the info from the now available m_tableInfo
			if (TestOpenFlag(TableOpenFlag::TOF_CHECK_EXISTANCE) && !searchedTable)
			{
				// Will throw if not one is found
				std::wstring catalogName = m_tableInfo.GetCatalog();
				std::wstring typeName = m_tableInfo.GetType();
				if (m_pDb->GetDbms() == DatabaseProduct::EXCEL)
				{
					// workaround for #111
					catalogName = L"";
					typeName = L"";
				}
				m_pDb->FindOneTable(m_tableInfo.GetPureName(), m_tableInfo.GetSchema(), catalogName, typeName);
				searchedTable = true;
			}

			// If no columns have been set so far try to create them automatically
			if (m_columns.empty())
			{
				CreateAutoColumnBufferPtrs(TestOpenFlag(TableOpenFlag::TOF_SKIP_UNSUPPORTED_COLUMNS), true);
			}

			// Set the CF_PRIMARY_KEY flag if information about primary key indexes have been set
			// And implicitly activate the flag TOF_DO_NOT_QUERY_PRIMARY_KEYS
			if(!m_primaryKeyColumnIndexes.empty())
			{
				m_openFlags.Set(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS);
				for (auto it = m_primaryKeyColumnIndexes.begin(); it != m_primaryKeyColumnIndexes.end(); ++it)
				{
					ColumnBufferPtrVariantMap::iterator itCols = m_columns.find(*it);
					if (itCols == m_columns.end())
					{
						THROW_WITH_SOURCE(IllegalArgumentException, boost::str(boost::wformat(L"No ColumnBuffer was found for a manually defined primary key column index (index = %d)") % *it));
					}
					ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), itCols->second);
					pFlags->Set(ColumnFlag::CF_PRIMARY_KEY);
				}
			}

			// If we need the primary keys and are allowed to query them do that now
			if ((TestAccessFlag(TableAccessFlag::AF_UPDATE_PK) || TestAccessFlag(TableAccessFlag::AF_DELETE_PK)) && !TestOpenFlag(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS))
			{
				QueryPrimaryKeysAndUpdateColumns();
			}

			// check that column flags match with table access flags
			CheckColumnFlags();

			// check that data types are supported if columns have been defined manually
			if (!m_autoCreatedColumns && !TestOpenFlag(TableOpenFlag::TOF_IGNORE_DB_TYPE_INFOS))
			{
				CheckSqlTypes(TestOpenFlag(TableOpenFlag::TOF_SKIP_UNSUPPORTED_COLUMNS));
			}

			// Optionally check privileges
			if (TestOpenFlag(TableOpenFlag::TOF_CHECK_PRIVILEGES))
			{
				CheckPrivileges();
			}

			// Bind the Buffer for the Count operations
			if (TestAccessFlag(TableAccessFlag::AF_SELECT))
			{
				exASSERT(m_pSelectCountResultBuffer);
				m_execStmtCount.BindColumn(m_pSelectCountResultBuffer, 1);
			}

			// Bind the member variables for field exchange between
			// the Table object and the ODBC record for Select()
			if (TestAccessFlag(TableAccessFlag::AF_SELECT))
			{
				SQLSMALLINT boundColumnNumber = 1;
				for (ColumnBufferPtrVariantMap::iterator it = m_columns.begin(); it != m_columns.end(); it++)
				{
					ColumnBufferPtrVariant columnBuffer = it->second;
					ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), columnBuffer);
					if (pFlags->Test(ColumnFlag::CF_SELECT))
					{
						m_execStmtSelect.BindColumn( columnBuffer, boundColumnNumber);
						boundColumnNumber++;
					}
				}
			}

			// Create additional CF_UPDATE and DELETE statement-handles to be used with the pk-columns
			// and bind the params. PKs are required.
			// Need to have at least one primary key.
			if (TestAccessFlag(TableAccessFlag::AF_UPDATE_PK) || TestAccessFlag(TableAccessFlag::AF_DELETE_PK))
			{
				if (GetPrimaryKeyColumnBuffers().empty())
				{
					Exception ex((boost::wformat(L"Table '%s' has no primary keys, cannot open with AF_UPDATE_PK or AF_DELETE_PK") % m_tableInfo.GetQueryName()).str());
					SET_EXCEPTION_SOURCE(ex);
					throw ex;
				}

				if (TestAccessFlag(TableAccessFlag::AF_UPDATE_PK))
				{
					BindUpdatePkParameters();
				}

				if (TestAccessFlag(TableAccessFlag::AF_DELETE_PK))
				{
					BindDeleteParameters();
				}
			}

			// Bind CF_INSERT params
			if (TestAccessFlag(TableAccessFlag::AF_INSERT))
			{
				BindInsertParameters();
			}

			// Completed successfully
			m_isOpen = true;
		}
		catch (const Exception& ex)
		{
			HIDE_UNUSED(ex);
			
			// Reset all stmts
			// remove the ColumnBuffers if we have allocated them during this process (if not manual)
			FreeStatements();

			if (m_autoCreatedColumns)
			{
				m_columns.clear();
			}
			// and rethrow
			throw;
		}
	}


	void Table::Close()
	{
		exASSERT(IsOpen());

		// Unbind all Columns
		//m_pHStmtSelect->UnbindColumns();

		// remove the ColumnBuffers if we have allocated them during Open()
		if (m_autoCreatedColumns)
		{
			m_columns.clear();
		}

		// Reset all statements
		FreeStatements();


		//// Unbind ColumnBuffers
		//SQLRETURN ret = SQL_SUCCESS;
		//if (TestAccessFlag(AF_SELECT))
		//{
		//	ret = SQLFreeStmt(m_hStmtSelect, SQL_UNBIND);
		//	THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtSelect);
		//}

		//// And column parameters, if we were bound rw
		//if (TestAccessFlag(AF_INSERT))
		//{
		//	ret = SQLFreeStmt(m_hStmtInsert, SQL_RESET_PARAMS);
		//	THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtInsert);
		//}
		//if (TestAccessFlag(AF_DELETE_PK))
		//{
		//	ret = SQLFreeStmt(m_hStmtDeletePk, SQL_RESET_PARAMS);
		//	THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtDeletePk);
		//}
		//if (TestAccessFlag(AF_DELETE_WHERE))
		//{
		//	ret = SQLFreeStmt(m_hStmtDeleteWhere, SQL_RESET_PARAMS);
		//	THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtDeleteWhere);
		//}
		//if (TestAccessFlag(AF_UPDATE_PK))
		//{
		//	ret = SQLFreeStmt(m_hStmtUpdatePk, SQL_RESET_PARAMS);
		//	THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtUpdatePk);
		//}
		//if (TestAccessFlag(AF_UPDATE_WHERE))
		//{
		//	ret = SQLFreeStmt(m_hStmtUpdateWhere, SQL_RESET_PARAMS);
		//	THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtUpdateWhere);
		//}

		//// Delete ColumnBuffers if they were created automatically
		//if (!m_autoCreatedColumns)
		//{
		//	ColumnBufferPtrMap::iterator it;
		//	for (it = m_columnBuffers.begin(); it != m_columnBuffers.end(); it++)
		//	{
		//		ColumnBuffer* pBuffer = it->second;
		//		delete pBuffer;
		//	}
		//	m_columnBuffers.clear();
		//}

		m_isOpen = false;
	}


	void Table::SetAccessFlag(TableAccessFlag ac)
	{
		exASSERT(!IsOpen());

		if (TestAccessFlag(ac))
		{
			// already set, do nothing
			return;
		}

		TableAccessFlags newFlags(m_tableAccessFlags);
		newFlags.Set(ac);
		SetAccessFlags(newFlags);
	}


	void Table::ClearAccessFlag(TableAccessFlag ac)
	{
		exASSERT(!IsOpen());

		if (!TestAccessFlag(ac))
		{
			// Not set anyway, do nothing
			return;
		}

		TableAccessFlags newFlags(m_tableAccessFlags);
		newFlags.Clear(ac);
		SetAccessFlags(newFlags);
	}


	void Table::SetAccessFlags(TableAccessFlags acs)
	{
		exASSERT(!IsOpen());
		exASSERT(m_pDb->IsOpen());

		if (m_tableAccessFlags == acs)
		{
			// Same flags, return
			return;
		}

		// Free statements, then re-allocate all
		//FreeStatements();
		m_tableAccessFlags = acs;
		//AllocateStatements();
	}


	bool Table::TestAccessFlag(TableAccessFlag af) const noexcept
	{
		return m_tableAccessFlags.Test(af);
	}


	bool Table::TestOpenFlag(TableOpenFlag of) const noexcept
	{
		return m_openFlags.Test(of);
	}


	void Table::SetColumnNTS(SQLSMALLINT columnIndex) const
	{
		SetColumnLengthIndicator(columnIndex, SQL_NTS);
	}


	void Table::SetColumnLengthIndicator(SQLSMALLINT columnIndex, SQLLEN cb) const
	{
		ColumnBufferPtrVariant var = GetColumnBufferPtrVariant(columnIndex);
		ColumnBufferLengthIndicatorPtr pCb = boost::apply_visitor(ColumnBufferLengthIndicatorPtrVisitor(), var);
		pCb->SetCb(cb);
	}


	/*!
	* \brief	Checks if we can only read from this table.
	* \return	True if this table has the flag AF_READ set and none of the flags
	*			AF_UPDATE_PK, AF_UPDATE_WHERE, AF_INSERT, AF_DELETE_PK or AF_DELETE_WHERE are set.
	*/
	bool Table::IsQueryOnly() const throw()  {
		return TestAccessFlag(TableAccessFlag::AF_READ) &&
				! ( TestAccessFlag(TableAccessFlag::AF_UPDATE_PK) || TestAccessFlag(TableAccessFlag::AF_UPDATE_WHERE)
					|| TestAccessFlag(TableAccessFlag::AF_INSERT)
					|| TestAccessFlag(TableAccessFlag::AF_DELETE_PK) || TestAccessFlag(TableAccessFlag::AF_DELETE_PK));
	}
}