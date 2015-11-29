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
#include "ColumnBuffer.h"
#include "Database.h"
#include "Environment.h"
#include "Exception.h"
#include "Sql2BufferTypeMap.h"
#include "SqlStatementCloser.h"

// Other headers

// Debug
#include "DebugNew.h"

using namespace std;

namespace exodbc
{
	// Construction
	// ------------
	Table::Table()
		: m_manualColumns(false)
		, m_haveTableInfo(false)
		//, m_accessFlags(AF_NONE)
		, m_pDb(NULL)
		, m_pSql2BufferTypeMap(NULL)
		, m_isOpen(false)
		, m_pHStmtSelect(make_shared<SqlStmtHandle>())
		, m_pHStmtCount(make_shared<SqlStmtHandle>())
		//, m_hStmtCount(SQL_NULL_HSTMT)
		//, m_hStmtSelect(SQL_NULL_HSTMT)
		//, m_hStmtInsert(SQL_NULL_HSTMT)
		//, m_hStmtUpdatePk(SQL_NULL_HSTMT)
		//, m_hStmtDeletePk(SQL_NULL_HSTMT)
		//, m_hStmtDeleteWhere(SQL_NULL_HSTMT)
		//, m_hStmtUpdateWhere(SQL_NULL_HSTMT)
		, m_selectQueryOpen(false)
		//, m_openFlags(TOF_NONE)
	{ }


	Table::Table(ConstDatabasePtr pDb, TableAccessFlags afs, const std::wstring& tableName, const std::wstring& schemaName /* = L"" */, const std::wstring& catalogName /* = L"" */, const std::wstring& tableType /* = L"" */)
		: m_manualColumns(false)
		, m_haveTableInfo(false)
		//, m_accessFlags(AF_NONE)
		, m_pDb(NULL)
		, m_pSql2BufferTypeMap(NULL)
		, m_isOpen(false)
		, m_pHStmtSelect(make_shared<SqlStmtHandle>())
		, m_pHStmtCount(make_shared<SqlStmtHandle>())
		//, m_hStmtCount(SQL_NULL_HSTMT)
		//, m_hStmtSelect(SQL_NULL_HSTMT)
		//, m_hStmtInsert(SQL_NULL_HSTMT)
		//, m_hStmtUpdatePk(SQL_NULL_HSTMT)
		//, m_hStmtDeletePk(SQL_NULL_HSTMT)
		//, m_hStmtDeleteWhere(SQL_NULL_HSTMT)
		//, m_hStmtUpdateWhere(SQL_NULL_HSTMT)
		, m_selectQueryOpen(false)
		//, m_openFlags(TOF_NONE)
	{
		Init(pDb, afs, tableName, schemaName, catalogName, tableType);
	}


	Table::Table(ConstDatabasePtr pDb, TableAccessFlags afs, const TableInfo& tableInfo)
		: m_manualColumns(false)
		, m_haveTableInfo(false)
		//, m_accessFlags(AF_NONE)
		, m_pDb(NULL)
		, m_pSql2BufferTypeMap(NULL)
		, m_isOpen(false)
		, m_pHStmtSelect(make_shared<SqlStmtHandle>())
		, m_pHStmtCount(make_shared<SqlStmtHandle>())
		//, m_hStmtCount(SQL_NULL_HSTMT)
		//, m_hStmtSelect(SQL_NULL_HSTMT)
		//, m_hStmtInsert(SQL_NULL_HSTMT)
		//, m_hStmtUpdatePk(SQL_NULL_HSTMT)
		//, m_hStmtDeletePk(SQL_NULL_HSTMT)
		//, m_hStmtDeleteWhere(SQL_NULL_HSTMT)
		//, m_hStmtUpdateWhere(SQL_NULL_HSTMT)
		, m_selectQueryOpen(false)
		//, m_openFlags(TOF_NONE)
	{
		Init(pDb, afs, tableInfo);
	}


	Table::Table(const Table& other)
		: m_manualColumns(false)
		, m_haveTableInfo(false)
		//, m_accessFlags(AF_NONE)
		, m_pDb(NULL)
		, m_pSql2BufferTypeMap(NULL)
		, m_isOpen(false)
		, m_pHStmtSelect(make_shared<SqlStmtHandle>())
		, m_pHStmtCount(make_shared<SqlStmtHandle>())
		//, m_hStmtCount(SQL_NULL_HSTMT)
		//, m_hStmtSelect(SQL_NULL_HSTMT)
		//, m_hStmtInsert(SQL_NULL_HSTMT)
		//, m_hStmtUpdatePk(SQL_NULL_HSTMT)
		//, m_hStmtDeletePk(SQL_NULL_HSTMT)
		//, m_hStmtDeleteWhere(SQL_NULL_HSTMT)
		//, m_hStmtUpdateWhere(SQL_NULL_HSTMT)
		, m_selectQueryOpen(false)
		//, m_openFlags(TOF_NONE)
	{
		// note: This constructor will always copy the search-names. Maybe they were set on other,
		// and then the TableInfo was searched. Do not loose the information about the search-names.
		if (other.HasTableInfo())
		{
			Init(other.m_pDb, other.GetAccessFlags(), other.GetTableInfo());
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
			//if (m_manualColumns)
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


	void Table::AllocateStatements()
	{
		exASSERT(!IsOpen());
		exASSERT(m_pDb->IsOpen());

		exASSERT(!m_pHStmtSelect->IsAllocated());
		exASSERT(!m_pHStmtCount->IsAllocated());
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
				m_pHStmtSelect->AllocateWithParent(pHDbc);
				m_pHStmtCount->AllocateWithParent(pHDbc);
			}

			//// Allocate handles needed for writing
			//if (TestAccessFlag(AF_INSERT))
			//{
			//	m_hStmtInsert = AllocateStatementHandle(hdbc);
			//}
			//if (TestAccessFlag(AF_UPDATE_PK))
			//{
			//	m_hStmtUpdatePk = AllocateStatementHandle(hdbc);
			//}
			//if (TestAccessFlag(AF_UPDATE_WHERE))
			//{
			//	m_hStmtUpdateWhere = AllocateStatementHandle(hdbc);
			//}

			//if (TestAccessFlag(AF_DELETE_PK))
			//{
			//	m_hStmtDeletePk = AllocateStatementHandle(hdbc);
			//}
			//if (TestAccessFlag(AF_DELETE_WHERE))
			//{
			//	m_hStmtDeleteWhere = AllocateStatementHandle(hdbc);
			//}
		}
		catch (const Exception& ex)
		{
			HIDE_UNUSED(ex);
			if (m_pHStmtSelect->IsAllocated())
				m_pHStmtSelect->Free();
			if (m_pHStmtCount->IsAllocated())
				m_pHStmtCount->Free();

			// rethrow
			throw;
		}
	}


	bool Table::HasAllStatements() const throw()
	{
		bool haveAll = true;
		if (haveAll && TestAccessFlag(TableAccessFlag::AF_SELECT))
		{
			haveAll = m_pHStmtSelect->IsAllocated() && m_pHStmtCount->IsAllocated();
		}
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


	void Table::SetCursorOptions(bool forwardOnlyCursors)
	{
		exASSERT(m_pHStmtSelect->IsAllocated());

		SQLRETURN ret = 0;
		if (forwardOnlyCursors || m_pDb->GetDbInfo().GetForwardOnlyCursors())
		{
			ret = SQLSetStmtAttr(m_pHStmtSelect->GetHandle(), SQL_ATTR_CURSOR_SCROLLABLE, (SQLPOINTER) SQL_NONSCROLLABLE, NULL);
			THROW_IFN_SUCCEEDED_MSG(SQLSetStmtAttr, ret, SQL_HANDLE_STMT, m_pHStmtSelect->GetHandle(), L"Failed to set Statement Attr SQL_ATTR_CURSOR_SCROLLABLE to SQL_NONSCROLLABLE");
		}
		else
		{
			ret = SQLSetStmtAttr(m_pHStmtSelect->GetHandle(), SQL_ATTR_CURSOR_SCROLLABLE, (SQLPOINTER) SQL_SCROLLABLE, NULL);
			THROW_IFN_SUCCEEDED_MSG(SQLSetStmtAttr, ret, SQL_HANDLE_STMT, m_pHStmtSelect->GetHandle(), L"Failed to set Statement Attr SQL_ATTR_CURSOR_SCROLLABLE to SQL_SCROLLABLE");
		}
	}


	void Table::CreateAutoColumnBuffers(const TableInfo& tableInfo, bool skipUnsupportedColumns)
	{
		exASSERT(m_manualColumns == false);
		exASSERT(m_columns.size() == 0);
		exASSERT(m_pDb->IsOpen());

		// Nest try-catch to always free eventually allocated buffers
		try
		{
			// Will throw if fails
			ColumnInfosVector columns = m_pDb->ReadTableColumnInfo(tableInfo);
			// Remember column sizes and create ColumnBuffers
			exASSERT(columns.size() <= SHRT_MAX);
			SQLSMALLINT numCols = (SQLSMALLINT) columns.size();
			if (numCols == 0)
			{
				Exception ex((boost::wformat(L"No columns found for table '%s'") % tableInfo.GetQueryName()).str());
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
			// We need to know which ODBC version we are using, might throw
			OdbcVersion odbcVersion = m_pDb->GetMaxSupportedOdbcVersion();
			// And how to open this column
			OldColumnFlags columnFlags = CF_NONE;
			ColumnFlags flags;
			if (TestAccessFlag(TableAccessFlag::AF_SELECT))
			{
				columnFlags |= CF_SELECT;
				flags.Set(ColumnFlag::CF_SELECT);
			}
			if (TestAccessFlag(TableAccessFlag::AF_UPDATE_WHERE) || TestAccessFlag(TableAccessFlag::AF_UPDATE_PK))
			{
				columnFlags |= CF_UPDATE;
				flags.Set(ColumnFlag::CF_UPDATE);
			}
			if (TestAccessFlag(TableAccessFlag::AF_INSERT))
			{
				columnFlags |= CF_INSERT;
				flags.Set(ColumnFlag::CF_INSERT);
			}
			int bufferIndex = 0;

			for (int columnIndex = 0; columnIndex < (SQLUSMALLINT)columns.size(); columnIndex++)
			{
				const ColumnInfo& colInfo = columns[columnIndex];
				try
				{
					SQLSMALLINT sqlCType = m_pSql2BufferTypeMap->GetBufferType(colInfo.GetSqlType());
					SqlCBufferVariant sqlCBuffer;
					if (IsArrayType(sqlCType))
					{
						sqlCBuffer = CreateArrayBuffer(sqlCType, colInfo);
					}
					else
					{
						sqlCBuffer = CreateBuffer(sqlCType, colInfo);
					}
					ColumnFlags& columnFlags = boost::polymorphic_get<ColumnFlags>(sqlCBuffer);
					columnFlags = flags;
					ExtendedColumnPropertiesHolder& extendedColumnProperties = boost::polymorphic_get<ExtendedColumnPropertiesHolder>(sqlCBuffer);
					extendedColumnProperties.SetObjectName(std::make_shared<ColumnInfo>(colInfo));
					extendedColumnProperties.SetSqlType(colInfo.GetSqlType());
					if(!colInfo.IsColumnSizeNull())
					{
						extendedColumnProperties.SetColumnSize(colInfo.GetColumnSize());
					}
					if (!colInfo.IsDecimalDigitsNull())
					{
						extendedColumnProperties.SetDecimalDigits(colInfo.GetDecimalDigits());
					}
					m_columns[bufferIndex] = sqlCBuffer;
					++bufferIndex;
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
						++bufferIndex;
						continue;
					}
					else
					{
						// rethrow
						throw;
					}

				}
			}
		}
		catch (Exception& ex)
		{
			HIDE_UNUSED(ex);
			// Rollback all buffers created and rethrow
			m_columns.clear();
			throw;
		}
	}


	void Table::FreeStatements()
	{
		exASSERT(!IsOpen());

		// Don no call free on the statements, but create new shared_ptrs.
		// If someone managed somehow to still have a reference to an allocated stmt
		// the shared_ptr will manage destruction once the last client is gone.
		// This is not public, and should only happen if not open,
		// when TableAccessFlags are modified before the table is opened.
		m_pHStmtSelect = std::make_shared<SqlStmtHandle>();
		m_pHStmtCount = std::make_shared<SqlStmtHandle>();

		//if (m_pHStmtSelect->IsAllocated())
		//	m_pHStmtSelect->Free();

		//if (m_pHStmtCount->IsAllocated())
		//	m_pHStmtCount->Free();

		//if (m_hStmtCount != SQL_NULL_HSTMT)
		//{
		//	m_hStmtCount = FreeStatementHandle(m_hStmtCount);
		//}
		//if (m_pHStmtSelect->IsAllocated())
		//{
		//	m_hStmtSelect = FreeStatementHandle(m_hStmtSelect);
		//}

		//// And then those needed for writing
		//if (m_hStmtInsert != SQL_NULL_HSTMT)
		//{
		//	m_hStmtInsert = FreeStatementHandle(m_hStmtInsert);
		//}
		//if (m_hStmtDeletePk != SQL_NULL_HSTMT)
		//{
		//	m_hStmtDeletePk = FreeStatementHandle(m_hStmtDeletePk);
		//}
		//if (m_hStmtDeleteWhere != SQL_NULL_HSTMT)
		//{
		//	m_hStmtDeleteWhere = FreeStatementHandle(m_hStmtDeleteWhere);
		//}
		//if (m_hStmtUpdatePk != SQL_NULL_HSTMT)
		//{
		//	m_hStmtUpdatePk = FreeStatementHandle(m_hStmtUpdatePk);
		//}
		//if (m_hStmtUpdateWhere != SQL_NULL_HSTMT)
		//{
		//	m_hStmtUpdateWhere = FreeStatementHandle(m_hStmtUpdateWhere);
		//}
	}


	bool Table::SelectFetchScroll(SQLSMALLINT fetchOrientation, SQLLEN fetchOffset)
	{
		exASSERT(!TestOpenFlag(TableOpenFlag::TOF_FORWARD_ONLY_CURSORS));
		exASSERT(IsSelectOpen());
		exASSERT(m_pHStmtSelect->IsAllocated());

		SQLRETURN ret = SQLFetchScroll(m_pHStmtSelect->GetHandle(), fetchOrientation, fetchOffset);
		if (!(SQL_SUCCEEDED(ret) || ret == SQL_NO_DATA))
		{
			wstring msg = boost::str(boost::wformat(L"Failed in SQLFetchScroll with FetchOrientation %d") % fetchOrientation);
			SqlResultException sre(L"SQLFetchScroll", ret, SQL_HANDLE_STMT, m_pHStmtSelect->GetHandle(), msg);
			SET_EXCEPTION_SOURCE(sre);
			throw sre;
		}
		if (ret == SQL_SUCCESS_WITH_INFO)
		{
			LOG_WARNING_STMT(m_pHStmtSelect->GetHandle(), ret, SQLFetch);
		}

		return SQL_SUCCEEDED(ret);
	}



	std::wstring Table::BuildFieldsStatement() const
	{
		exASSERT(m_columns.size() > 0);

		std::wstring fields = L"";
		SqlCBufferVariantMap::const_iterator it = m_columns.begin();
		while (it != m_columns.end())
		{
			SqlCBufferVariant var = it->second;
			try
			{
				const ExtendedColumnPropertiesHolder& props = boost::polymorphic_get<ExtendedColumnPropertiesHolder>(var);
				const ColumnFlags columnFlags = boost::polymorphic_get<ColumnFlags>(var);
				if (columnFlags.Test(ColumnFlag::CF_SELECT))
				{
					std::shared_ptr<ObjectName> pColumnName = props.GetObjectName();
					exASSERT(pColumnName);
					fields += pColumnName->GetQueryName();
					fields += L", ";
				}
			}
			catch (const boost::bad_polymorphic_get& ex)
			{
				WrapperException we(ex);
				SET_EXCEPTION_SOURCE(we);
				throw we;
			}
			++it;
		}
		boost::algorithm::erase_last(fields, L", ");

		return fields;
	}


	void Table::BindDeleteParameters()
	{
		//exASSERT(m_columnBuffers.size() > 0);
		//exASSERT(TestAccessFlag(AF_DELETE_PK));
		//exASSERT(m_hStmtDeletePk != SQL_NULL_HSTMT);

		//// Build statement
		//// note: parem-number reflects here the number of the param in the created prepared statement, so we
		//// cannot use the values from the ColumnBuffer column index.
		//int paramNr = 1;
		//wstring deleteStmt = (boost::wformat(L"DELETE FROM %s WHERE ") % m_tableInfo.GetQueryName()).str();
		//for (ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin(); it != m_columnBuffers.end(); it++)
		//{
		//	ColumnBuffer* pBuffer = it->second;
		//	if (pBuffer->IsColumnFlagSet(CF_PRIMARY_KEY))
		//	{
		//		// Bind this parameter as primary key and include it in the statement
		//		pBuffer->BindParameter(m_hStmtDeletePk, paramNr);
		//		deleteStmt += (boost::wformat(L"%s = ? AND ") % pBuffer->GetQueryName()).str();
		//		paramNr++;
		//	}
		//}
		//boost::erase_last(deleteStmt, L" AND ");
		//// ensure that we have something in our where clause
		//if (paramNr <= 1)
		//{
		//	Exception ex(boost::str(boost::wformat(L"Table '%s' was requested to prepare a DELETE statement (probably because flag AF_DELETE_PK is set), but no ColumnBuffers were bound as PrimaryKeys to build a WHERE clause") % m_tableInfo.GetQueryName()));
		//	SET_EXCEPTION_SOURCE(ex);
		//	throw ex;
		//}

		//// Prepare to delete
		//SQLRETURN ret = SQLPrepare(m_hStmtDeletePk, (SQLWCHAR*)deleteStmt.c_str(), SQL_NTS);
		//THROW_IFN_SUCCEEDED(SQLPrepare, ret, SQL_HANDLE_STMT, m_hStmtDeletePk);
	}


	void Table::BindUpdateParameters()
	{
		//exASSERT(m_columnBuffers.size() > 0);
		//exASSERT(TestAccessFlag(AF_UPDATE_PK));
		//exASSERT(m_hStmtUpdatePk != SQL_NULL_HSTMT);

		//// Build statement..
		//wstring updateStmt = (boost::wformat(L"UPDATE %s SET ") % m_tableInfo.GetQueryName()).str();
		//// .. first the values to update
		//// note: parem-number reflects here the number of the param in the created prepared statement, so we
		//// cannot use the values from the ColumnBuffer column index.
		//int paramNr = 1;
		//for (ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin(); it != m_columnBuffers.end(); it++)
		//{
		//	ColumnBuffer* pBuffer = it->second;
		//	if (pBuffer->IsColumnFlagSet(CF_UPDATE) && !pBuffer->IsColumnFlagSet(CF_PRIMARY_KEY))
		//	{
		//		// Bind this parameter as update parameter and include it in the statement
		//		// The update params come first, so the numbering works
		//		pBuffer->BindParameter(m_hStmtUpdatePk, paramNr);
		//		updateStmt += (boost::wformat(L"%s = ?, ") % pBuffer->GetQueryName()).str();
		//		paramNr++;
		//	}
		//}
		//boost::erase_last(updateStmt, L",");
		//// ensure that we have something to update
		//if (paramNr <= 1)
		//{	
		//	Exception ex(boost::str(boost::wformat(L"Table '%s' was requested to bind update parameters (probably because flag AF_UPDATE_PK is set), but no ColumnBuffers were bound for UPDATing") % m_tableInfo.GetQueryName()));
		//	SET_EXCEPTION_SOURCE(ex);
		//	throw ex;
		//}
		//updateStmt += L"WHERE ";
		//bool haveWhereParam = false;
		//for (ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin(); it != m_columnBuffers.end(); it++)
		//{
		//	ColumnBuffer* pBuffer = it->second;
		//	if (pBuffer->IsColumnFlagSet(CF_PRIMARY_KEY))
		//	{
		//		// Bind this parameter as primary key and include it in the where part of the statement
		//		// The update params come first, so the numbering works
		//		pBuffer->BindParameter(m_hStmtUpdatePk, paramNr);
		//		updateStmt += (boost::wformat(L"%s = ? AND ") % pBuffer->GetQueryName()).str();
		//		paramNr++;
		//		haveWhereParam = true;
		//	}
		//}
		//boost::erase_last(updateStmt, L"AND ");
		//// and ensure that we have something as where
		//exASSERT(haveWhereParam);

		//// Prepare to update
		//SQLRETURN ret = SQLPrepare(m_hStmtUpdatePk, (SQLWCHAR*)updateStmt.c_str(), SQL_NTS);
		//THROW_IFN_SUCCEEDED(SQLPrepare, ret, SQL_HANDLE_STMT, m_hStmtUpdatePk);
	}


	void Table::BindInsertParameters()
	{
		//exASSERT(m_columnBuffers.size() > 0);
		//exASSERT(TestAccessFlag(AF_INSERT));
		//exASSERT(m_hStmtInsert != SQL_NULL_HSTMT);

		//// Build a statement with parameter-markers
		//// note: parem-number reflects here the number of the param in the created prepared statement, so we
		//// cannot use the values from the ColumnBuffer column index.
		//SQLSMALLINT paramNr = 1;
		//std::wstring insertStmt = L"INSERT INTO " + m_tableInfo.GetQueryName() + L" (";
		//ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin();
		//while (it != m_columnBuffers.end())
		//{
		//	ColumnBuffer* pBuffer = it->second;
		//	// Bind parameter if it is marked an INSERTable
		//	if (pBuffer->IsColumnFlagSet(CF_INSERT))
		//	{
		//		pBuffer->BindParameter(m_hStmtInsert, paramNr);
		//		// prepare statement
		//		insertStmt += pBuffer->GetQueryName() + L", ";
		//		paramNr++;
		//	}
		//	++it;
		//}
		//// ensure that we have something to insert
		//if (paramNr <= 1)
		//{
		//	Exception ex(boost::str(boost::wformat(L"Table '%s' was requested to bind insert parameters (probably because flag AF_INSERT is set), but no ColumnBuffers were bound for INSERTing") % m_tableInfo.GetQueryName()));
		//	SET_EXCEPTION_SOURCE(ex);
		//	throw ex;
		//}
		//boost::erase_last(insertStmt, L", ");
		//// and set markers for the values
		//insertStmt += L") VALUES(";
		//for (int i = 0; i < paramNr - 2; i++)
		//{
		//	insertStmt += L"?, ";
		//}
		//insertStmt += L"?)";

		//// Prepare to update
		//SQLRETURN ret = SQLPrepare(m_hStmtInsert, (SQLWCHAR*) insertStmt.c_str(), SQL_NTS);
		//THROW_IFN_SUCCEEDED(SQLPrepare, ret, SQL_HANDLE_STMT, m_hStmtInsert);
	}


	bool Table::ColumnBufferExists(SQLSMALLINT columnIndex) const throw()
	{
		SqlCBufferVariantMap::const_iterator it = m_columns.find(columnIndex);
		return it != m_columns.end();
	}


	const SqlCBufferVariant& Table::GetColumnVariant(SQLSMALLINT columnIndex) const
	{
		exASSERT(IsOpen());

		SqlCBufferVariantMap::const_iterator it = m_columns.find(columnIndex);
		if (it == m_columns.end())
		{
			IllegalArgumentException ex(boost::str(boost::wformat(L"ColumnIndex %d is not a zero-based bound or manually defined ColumnBuffer") % columnIndex));
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}

		return it->second;
	}


	const SqlCBufferVariant& Table::GetNonNullColumnVariant(SQLSMALLINT columnIndex) const
	{
		const SqlCBufferVariant& column = GetColumnVariant(columnIndex);
		try
		{
			const SqlCBufferLengthIndicator& cb = boost::polymorphic_get<SqlCBufferLengthIndicator>(column);
			const ExtendedColumnPropertiesHolder& props = boost::polymorphic_get<ExtendedColumnPropertiesHolder>(column);
			if (cb.IsNull())
			{
				std::shared_ptr<ObjectName> objectName = props.GetObjectName();
				NullValueException ex(objectName != NULL ? objectName->GetQueryName() : L"???");
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
			return column;
		}
		catch (const boost::bad_polymorphic_get& ex)
		{
			WrapperException we(ex);
			SET_EXCEPTION_SOURCE(we);
			throw we;
		}
	}


	std::set<SQLSMALLINT> Table::GetColumnBufferIndexes() const throw()
	{
		std::set<SQLSMALLINT> columnIndexes;
		SqlCBufferVariantMap::const_iterator it = m_columns.begin();
		while (it != m_columns.end())
		{
			columnIndexes.insert(it->first);
			++it;
		}
		return columnIndexes;
	}


	SQLSMALLINT Table::GetColumnBufferIndex(const std::wstring& columnQueryName, bool caseSensitive /* = true */) const
	{
		SqlCBufferVariantMap::const_iterator it = m_columns.begin();
		while (it != m_columns.end())
		{
			const SqlCBufferVariant& column = it->second;
			std::shared_ptr<ObjectName> pColumnName(NULL);
			try
			{
				const ExtendedColumnPropertiesHolder& props = boost::polymorphic_get<ExtendedColumnPropertiesHolder>(column);
				pColumnName = props.GetObjectName();
			}
			catch (const boost::bad_polymorphic_get& ex)
			{
				WrapperException we(ex);
				SET_EXCEPTION_SOURCE(we);
				throw we;
			}

			exASSERT(pColumnName != NULL);
			if (caseSensitive)
			{		
				if (pColumnName->GetQueryName() == columnQueryName)
				{
					return it->first;
				}
			}
			else
			{
				if (boost::iequals(pColumnName->GetQueryName(), columnQueryName))
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


	TableInfo Table::GetTableInfo() const
	{
		exASSERT(m_haveTableInfo);
		return m_tableInfo;
	}


	SQLUBIGINT Table::Count(const std::wstring& whereStatement)
	{
		exASSERT(IsOpen());
		exASSERT(m_pHStmtCount->IsAllocated());

		// Close Statement handle on exit
		StatementCloser stmtCloser(m_pHStmtCount, false, true);

		SQLUBIGINT count = 0;
		std::wstring sqlstmt;
		if ( ! whereStatement.empty())
		{
			sqlstmt = (boost::wformat(L"SELECT COUNT(*) FROM %s WHERE %s") % m_tableInfo.GetQueryName() % whereStatement).str();
		}
		else
		{
			sqlstmt = (boost::wformat(L"SELECT COUNT(*) FROM %s") % m_tableInfo.GetQueryName()).str();
		}

		SQLRETURN ret = SQLExecDirect(m_pHStmtCount->GetHandle(), (SQLWCHAR*)sqlstmt.c_str(), SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLExecDirect, ret, SQL_HANDLE_STMT, m_pHStmtCount->GetHandle());
			
		ret = SQLFetch(m_pHStmtCount->GetHandle());
		THROW_IFN_SUCCEEDED(SQLFetch, ret, SQL_HANDLE_STMT, m_pHStmtCount->GetHandle());

		bool isNull = false;
		SQLLEN cb = 0;
		GetData(m_pHStmtCount->GetHandle(), 1, SQL_C_UBIGINT, &count, sizeof(count), &cb, &isNull);
		if (isNull)
		{
			Exception ex(boost::str(boost::wformat(L"Read Value for '%s' is NULL") % sqlstmt));
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}

		return count;
	}


	void Table::Select(const std::wstring& whereStatement /* = L"" */)
	{
		exASSERT(IsOpen());

		std::wstring sqlstmt;
		if (!whereStatement.empty())
		{
			sqlstmt = (boost::wformat(L"SELECT %s FROM %s WHERE %s") % m_fieldsStatement % m_tableInfo.GetQueryName() % whereStatement).str();
		}
		else
		{
			sqlstmt = (boost::wformat(L"SELECT %s FROM %s") % m_fieldsStatement % m_tableInfo.GetQueryName()).str();
		}
		SelectBySqlStmt(sqlstmt);
	}


	void Table::SelectBySqlStmt(const std::wstring& sqlStmt)
	{
		exASSERT(IsOpen());
		exASSERT(!sqlStmt.empty());
		exASSERT(m_pHStmtSelect->IsAllocated());

		if (IsSelectOpen())
		{
			SelectClose();
		}

		SQLRETURN ret = SQLExecDirect(m_pHStmtSelect->GetHandle(), (SQLWCHAR*)sqlStmt.c_str(), SQL_NTS);
		THROW_IFN_SUCCESS(SQLExecDirect, ret, SQL_HANDLE_STMT, m_pHStmtSelect->GetHandle());
		m_selectQueryOpen = true;
	}


	bool Table::SelectPrev()
	{
		return SelectFetchScroll(SQL_FETCH_PREV, NULL);
	}


	bool Table::SelectFirst()
	{
		return SelectFetchScroll(SQL_FETCH_FIRST, NULL);
	}


	bool Table::SelectLast()
	{
		return SelectFetchScroll(SQL_FETCH_LAST, NULL);
	}


	bool Table::SelectAbsolute(SQLLEN position)
	{
		return SelectFetchScroll(SQL_FETCH_ABSOLUTE, position);
	}


	bool Table::SelectRelative(SQLLEN offset)
	{
		return SelectFetchScroll(SQL_FETCH_RELATIVE, offset);
	}


	bool Table::SelectNext()
	{
		exASSERT(IsSelectOpen());
		exASSERT(m_pHStmtSelect->IsAllocated());
			
		SQLRETURN ret = SQLFetch(m_pHStmtSelect->GetHandle());
		if (!(SQL_SUCCEEDED(ret) || ret == SQL_NO_DATA))
		{
			SqlResultException sre(L"SQLFetch", ret, SQL_HANDLE_STMT, m_pHStmtSelect->GetHandle());
			SET_EXCEPTION_SOURCE(sre);
			throw sre;
		}
		if (ret == SQL_SUCCESS_WITH_INFO)
		{
			LOG_WARNING_STMT(m_pHStmtSelect->GetHandle(), ret, SQLFetch);
		}

		return SQL_SUCCEEDED(ret);
	}

	
	void Table::SelectClose()
	{
		StatementCloser::CloseStmtHandle(m_pHStmtSelect, StatementCloser::Mode::IgnoreNotOpen);
		m_selectQueryOpen = false;
	}


	void Table::Insert() const
	{
		exASSERT(IsOpen());
		exASSERT(TestAccessFlag(TableAccessFlag::AF_INSERT));
		//exASSERT(m_hStmtInsert != SQL_NULL_HSTMT);
		//SQLRETURN ret = SQLExecute(m_hStmtInsert);
		//THROW_IFN_SUCCEEDED(SQLExecute, ret, SQL_HANDLE_STMT, m_hStmtInsert);
	}


	void Table::Delete(bool failOnNoData /* = true */)
	{
		exASSERT(IsOpen());
		exASSERT(TestAccessFlag(TableAccessFlag::AF_DELETE_PK));
		//exASSERT(m_hStmtDeletePk != SQL_NULL_HSTMT);
		//SQLRETURN ret = SQLExecute(m_hStmtDeletePk);
		//if (failOnNoData && ret == SQL_NO_DATA)
		//{
		//	SqlResultException ex(L"SQLExecute", ret, L"Did not expect a SQL_NO_DATA while executing a delete-query");
		//	SET_EXCEPTION_SOURCE(ex);
		//	throw ex;
		//}
		//if (!(!failOnNoData && ret == SQL_NO_DATA))
		//{
		//	THROW_IFN_SUCCEEDED(SQLExecute, ret, SQL_HANDLE_STMT, m_hStmtDeletePk);
		//}
	}


	void Table::Delete(const std::wstring& where, bool failOnNoData /* = true */) const
	{
		exASSERT(IsOpen());
		exASSERT(TestAccessFlag(TableAccessFlag::AF_DELETE_WHERE));
		//exASSERT(m_hStmtDeleteWhere != SQL_NULL_HSTMT);
		//exASSERT(!where.empty());

		//wstring sqlstmt = (boost::wformat(L"DELETE FROM %s WHERE %s") % m_tableInfo.GetQueryName() % where).str();
		//exASSERT(sqlstmt.length() < INT_MAX);
		//SQLRETURN ret = SQLExecDirect(m_hStmtDeleteWhere, (SQLWCHAR*)sqlstmt.c_str(), (SQLINTEGER) sqlstmt.length());
		//if (failOnNoData && ret == SQL_NO_DATA)
		//{
		//	SqlResultException ex(L"SQLExecute", ret, boost::str(boost::wformat(L"Did not expect a SQL_NO_DATA while executing the delete-query '%s'") %sqlstmt));
		//	SET_EXCEPTION_SOURCE(ex);
		//	throw ex;
		//}
		//if ( ! (!failOnNoData && ret == SQL_NO_DATA))
		//{
		//	THROW_IFN_SUCCEEDED(SQLExecute, ret, SQL_HANDLE_STMT, m_hStmtDeleteWhere);
		//}
	}


	void Table::Update()
	{
		exASSERT(IsOpen());
		exASSERT(TestAccessFlag(TableAccessFlag::AF_UPDATE_PK));
		//exASSERT(m_hStmtUpdatePk != SQL_NULL_HSTMT);
		//SQLRETURN ret = SQLExecute(m_hStmtUpdatePk);
		//THROW_IFN_SUCCEEDED(SQLExecute, ret, SQL_HANDLE_STMT, m_hStmtUpdatePk);
	}


	void Table::Update(const std::wstring& where)
	{
		//exASSERT(IsOpen());
		//exASSERT(TestAccessFlag(AF_UPDATE_WHERE));
		//exASSERT(m_hStmtUpdateWhere != SQL_NULL_HSTMT);
		//exASSERT(!where.empty());

		//try
		//{
		//	// Format an update-statement that updates all Bound columns that have the flag CF_UPDATE set
		//	wstring updateStmt = (boost::wformat(L"UPDATE %s SET ") % m_tableInfo.GetQueryName()).str();
		//	// .. first the values to update
		//	// note: parem-number reflects here the number of the param in the created prepared statement, so we
		//	// cannot use the values from the ColumnBuffer column index.
		//	int paramNr = 1;
		//	for (ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin(); it != m_columnBuffers.end(); it++)
		//	{
		//		ColumnBuffer* pBuffer = it->second;
		//		if (pBuffer->IsColumnFlagSet(CF_UPDATE))
		//		{
		//			// Bind this parameter as update parameter and include it in the statement
		//			pBuffer->BindParameter(m_hStmtUpdateWhere, paramNr);
		//			updateStmt += (boost::wformat(L"%s = ?, ") % pBuffer->GetQueryName()).str();
		//			paramNr++;
		//		}
		//	}
		//	boost::erase_last(updateStmt, L",");
		//	exASSERT_MSG(paramNr > 1, L"No columns are bound that have the flag CF_UPDATE set");
		//	updateStmt += (L"WHERE " + where);

		//	// Prepare to update
		//	SQLRETURN ret = SQLPrepare(m_hStmtUpdateWhere, (SQLWCHAR*)updateStmt.c_str(), SQL_NTS);
		//	THROW_IFN_SUCCEEDED(SQLPrepare, ret, SQL_HANDLE_STMT, m_hStmtUpdateWhere);

		//	// And Execute
		//	ret = SQLExecute(m_hStmtUpdateWhere);
		//	THROW_IFN_SUCCEEDED(SQLExecute, ret, SQL_HANDLE_STMT, m_hStmtUpdateWhere);

		//	// Always unbind all parameters at the end
		//	ret = SQLFreeStmt(m_hStmtUpdateWhere, SQL_UNBIND);
		//	THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtUpdateWhere);
		//}
		//catch (const Exception& ex)
		//{
		//	// Always unbind all parameters at the end
		//	SQLRETURN ret = SQLFreeStmt(m_hStmtUpdateWhere, SQL_UNBIND);
		//	WARN_IFN_SUCCEEDED_MSG(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtUpdateWhere, boost::str(boost::wformat(L"Failed to unbind UpdateWhere-handle during cleanup of exception '%s") % ex.ToString()));
		//	// rethrow
		//	throw;
		//}

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


	void Table::SetColumnValue(SQLSMALLINT columnIndex, const BufferVariant& value) const
	{
		//ColumnBuffer* pBuff = GetColumnBuffer(columnIndex);
		//*pBuff = value;
	}


	void Table::SetBinaryValue(SQLSMALLINT columnIndex, const SQLCHAR* buffer, SQLINTEGER bufferSize)
	{
		//ColumnBuffer* pBuff = GetColumnBuffer(columnIndex);
		//pBuff->SetBinaryValue(buffer, bufferSize);
	}


	void Table::SetColumnNull(SQLSMALLINT columnIndex) const
	{
		SqlCBufferVariant var = GetColumnVariant(columnIndex);
		try
		{
			SqlCBufferLengthIndicator& cb = boost::polymorphic_get<SqlCBufferLengthIndicator>(var);
			ColumnFlags& flags = boost::polymorphic_get<ColumnFlags>(var);
			exASSERT(flags.Test(ColumnFlag::CF_NULLABLE));
			cb.SetNull();
		}
		catch (const boost::bad_polymorphic_get& ex)
		{
			WrapperException we(ex);
			SET_EXCEPTION_SOURCE(we);
			throw we;
		}
	}


	SQLSMALLINT Table::GetSmallInt(SQLSMALLINT columnIndex) const
	{
		const SqlSShortBuffer& buff = GetNonNullColumn<SqlSShortBuffer>(columnIndex);
		return buff.GetValue();
	}


	SQLINTEGER Table::GetInt(SQLSMALLINT columnIndex) const
	{
		const SqlSLongBuffer& buff = GetNonNullColumn<SqlSLongBuffer>(columnIndex);
		return buff.GetValue();
	}


	SQLBIGINT Table::GetBigInt(SQLSMALLINT columnIndex) const
	{
		const SqlSBigIntBuffer& buff = GetNonNullColumn<SqlSBigIntBuffer>(columnIndex);
		return buff.GetValue();
	}


	SQL_DATE_STRUCT Table::GetDate(SQLSMALLINT columnIndex) const
	{
		const SqlTypeDateStructBuffer& buff = GetNonNullColumn<SqlTypeDateStructBuffer>(columnIndex);
		return buff.GetValue();
	}


	SQL_TIME_STRUCT Table::GetTime(SQLSMALLINT columnIndex) const
	{
		const SqlTypeTimeStructBuffer& buff = GetNonNullColumn<SqlTypeTimeStructBuffer>(columnIndex);
		return buff.GetValue();
	}


	SQL_TIMESTAMP_STRUCT Table::GetTimeStamp(SQLSMALLINT columnIndex) const
	{
		const SqlTypeTimestampStructBuffer& buff = GetNonNullColumn<SqlTypeTimestampStructBuffer>(columnIndex);
		return buff.GetValue();
	}


	//std::string Table::GetString(SQLSMALLINT columnIndex) const
	//{
	//	const SqlCharArray& buff = GetNonNullColumn<SqlCharArray>(columnIndex);
	//	std::shared_ptr<SQLCHAR> pChar = buff.GetBuffer();
	//	exASSERT(pChar != NULL);
	//	std::string s(reinterpret_cast<const char*>(pChar.get()));
	//	if (TestOpenFlag(TOF_CHAR_TRIM_LEFT))
	//	{
	//		boost::trim_left(s);
	//	}
	//	if (TestOpenFlag(TOF_CHAR_TRIM_RIGHT))
	//	{
	//		boost::trim_right(s);
	//	}

	//	return s;
	//}


	//std::wstring Table::GetWString(SQLSMALLINT columnIndex) const
	//{
	//	const SqlWCharArray& buff = GetNonNullColumn<SqlWCharArray>(columnIndex);
	//	std::shared_ptr<SQLWCHAR> pChar = buff.GetBuffer();
	//	exASSERT(pChar != NULL);
	//	std::wstring ws(pChar.get());
	//	if (TestOpenFlag(TOF_CHAR_TRIM_LEFT))
	//	{
	//		boost::trim_left(ws);
	//	}
	//	if (TestOpenFlag(TOF_CHAR_TRIM_RIGHT))
	//	{
	//		boost::trim_right(ws);
	//	}

	//	return ws;
	//}


	SQLDOUBLE Table::GetDouble(SQLSMALLINT columnIndex) const
	{
		const SqlDoubleBuffer& buff = GetNonNullColumn<SqlDoubleBuffer>(columnIndex);
		return buff.GetValue();
	}


	SQLREAL Table::GetReal(SQLSMALLINT columnIndex) const
	{
		const SqlRealBuffer& buff = GetNonNullColumn<SqlRealBuffer>(columnIndex);
		return buff.GetValue();
	}


	SQL_NUMERIC_STRUCT Table::GetNumeric(SQLSMALLINT columnIndex) const
	{
		const SqlNumericStructBuffer& buff = GetNonNullColumn<SqlNumericStructBuffer>(columnIndex);
		return buff.GetValue();
	}


	//BufferVariant Table::GetColumnValue(SQLSMALLINT columnIndex) const
	//{
	//	ColumnBuffer* pBuff = GetColumnBuffer(columnIndex);
	//	return pBuff->GetValue();
	//}


	const SQLCHAR* Table::GetBinaryValue(SQLSMALLINT columnIndex, SQLLEN& bufferSize, SQLLEN& lengthIndicator) const
	{
		const SqlBinaryArray& column = GetNonNullColumn<SqlBinaryArray>(columnIndex);
		const SqlCBufferVariant& var = GetColumnVariant(columnIndex);
		try
		{
			const SqlCBufferLengthIndicator& cb = boost::polymorphic_get<SqlCBufferLengthIndicator>(var);
			lengthIndicator = cb.GetCb();
		}
		catch (const boost::bad_polymorphic_get& ex)
		{
			WrapperException we(ex);
			SET_EXCEPTION_SOURCE(we);
			throw we;
		}

		bufferSize = column.GetBufferLength();
		return column.GetBuffer().get();
	}


	bool Table::IsColumnNull(SQLSMALLINT columnIndex) const
	{
		const SqlCBufferVariant& var = GetColumnVariant(columnIndex);
		try
		{
			const SqlCBufferLengthIndicator& cb = boost::polymorphic_get<SqlCBufferLengthIndicator>(var);
			return cb.IsNull();
		}
		catch (const boost::bad_polymorphic_get& ex)
		{
			WrapperException we(ex);
			SET_EXCEPTION_SOURCE(we);
			throw we;
		}
	}


	bool Table::IsColumnNullable(SQLSMALLINT columnIndex) const
	{
		const SqlCBufferVariant& var = GetColumnVariant(columnIndex);
		try
		{
			const ColumnFlags& flags = boost::polymorphic_get<ColumnFlags>(var);
			return flags.Test(ColumnFlag::CF_NULLABLE);
		}
		catch (const boost::bad_polymorphic_get& ex)
		{
			WrapperException we(ex);
			SET_EXCEPTION_SOURCE(we);
			throw we;
		}
	}


	void Table::SetColumn(SQLUSMALLINT columnIndex, SqlCBufferVariant column)
	{
		exASSERT(columnIndex >= 0);
		exASSERT(m_columns.find(columnIndex) == m_columns.end());

		// test if we have a non-empty query name
		try
		{
			const ExtendedColumnPropertiesHolder& props = boost::polymorphic_get<ExtendedColumnPropertiesHolder>(column);
			std::shared_ptr<ObjectName> pName = props.GetObjectName();
			exASSERT(pName);
			exASSERT(!pName->GetQueryName().empty());
		}
		catch (const boost::bad_polymorphic_get& ex)
		{
			WrapperException we(ex);
			SET_EXCEPTION_SOURCE(we);
			throw we;
		}

		// okay, remember the passed variant
		m_columns[columnIndex] = column;

		// Flag that columns are created manually
		m_manualColumns = true;
	}


	void Table::SetColumn(SQLUSMALLINT columnIndex, const std::wstring& queryName, SQLSMALLINT sqlType, SQLPOINTER pBuffer, SQLSMALLINT sqlCType, SQLLEN bufferSize, ColumnFlags flags, SQLINTEGER columnSize /* = 0 */, SQLSMALLINT decimalDigits /* = 0 */)
	{
		SqlCPointerBuffer column(queryName, sqlType, pBuffer, sqlCType, bufferSize, flags, columnSize, decimalDigits);
		SetColumn(columnIndex, column);
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
	//	//m_manualColumns = true;
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
		exASSERT(HasAllStatements());
		// \note: We do not force a user to define columns.

		// Init open flags
		m_openFlags = openFlags;

		// Set TOF_DO_NOT_QUERY_PRIMARY_KEYS this flag for Access, Access does not support SQLPrimaryKeys
		if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			m_openFlags.Set(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS);
		}

		// Nest try/catch the free the buffers created in here if we fail somewhere
		// and to unbind all handles that were bound
		bool boundSelect = false;
		bool boundUpdatePk = false;
		bool boundDeletePk = false;
		bool boundInsert = false;
		try
		{
			// Try to set Cursor-Options
			// Do not try to set on Access or Excel, they report 'Optional feature not implemented' (even if trying to set forward-only)
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS || m_pDb->GetDbms() == DatabaseProduct::EXCEL)
			{
				m_openFlags.Set(TableOpenFlag::TOF_FORWARD_ONLY_CURSORS);
			}
			else
			{
				SetCursorOptions(TestOpenFlag(TableOpenFlag::TOF_FORWARD_ONLY_CURSORS));
			}

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

			// If we are asked to create our columns automatically, read the column information and create the buffers
			if (!m_manualColumns)
			{
				CreateAutoColumnBuffers(m_tableInfo, TestOpenFlag(TableOpenFlag::TOF_SKIP_UNSUPPORTED_COLUMNS));
			}
			else
			{
				// Check that the manually defined columns do not violate our access-flags
				// and check that manually defined types are supported by the database
				SqlCBufferVariantMap::iterator it = m_columns.begin();
				while(it != m_columns.end())
				{
					SqlCBufferVariant& columnBuffer = it->second;
					ColumnFlags columnFlags;
					wstring queryName;
					SQLSMALLINT sqlType = SQL_UNKNOWN_TYPE;
					try
					{
						columnFlags = boost::polymorphic_get<ColumnFlags>(columnBuffer);
						const ExtendedColumnPropertiesHolder& props = boost::polymorphic_get<ExtendedColumnPropertiesHolder>(columnBuffer);
						sqlType = props.GetSqlType();
						std::shared_ptr<const ObjectName> pName = props.GetObjectName();
						exASSERT(pName);
						queryName = pName->GetQueryName();
					}
					catch (const boost::bad_polymorphic_get& ex)
					{
						WrapperException we(ex);
						SET_EXCEPTION_SOURCE(we);
						throw we;
					}
					if (columnFlags.Test(ColumnFlag::CF_SELECT) && !TestAccessFlag(TableAccessFlag::AF_SELECT))
					{
						Exception ex(boost::str(boost::wformat(L"Defined Column %s (%d) has ColumnFlag CF_SELECT set, but the AccessFlag AF_SELECT is not set on the table %s") % queryName % it->first %m_tableInfo.GetQueryName()));
						SET_EXCEPTION_SOURCE(ex);
						throw ex;
					}
					if (columnFlags.Test(ColumnFlag::CF_INSERT) && !TestAccessFlag(TableAccessFlag::AF_INSERT))
					{
						Exception ex(boost::str(boost::wformat(L"Defined Column %s (%d) has ColumnFlag CF_INSERT set, but the AccessFlag AF_INSERT is not set on the table %s") % queryName % it->first %m_tableInfo.GetQueryName()));
						SET_EXCEPTION_SOURCE(ex);
						throw ex;
					}
					if (columnFlags.Test(ColumnFlag::CF_UPDATE) && !(TestAccessFlag(TableAccessFlag::AF_UPDATE_PK) || TestAccessFlag(TableAccessFlag::AF_UPDATE_WHERE)))
					{
						Exception ex(boost::str(boost::wformat(L"Defined Column %s (%d) has ColumnFlag CF_UPDATE set, but the AccessFlag AF_UPDATE is not set on the table %s") % queryName % it->first %m_tableInfo.GetQueryName()));
						SET_EXCEPTION_SOURCE(ex);
						throw ex;
					}
					// type-check
					bool remove = false;
					if (!TestOpenFlag(TableOpenFlag::TOF_IGNORE_DB_TYPE_INFOS) && sqlType != SQL_UNKNOWN_TYPE && !m_pDb->IsSqlTypeSupported(sqlType))
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

			// Prepare the FieldStatement to be used for selects
			m_fieldsStatement = BuildFieldsStatement();

			// Optionally check privileges
			if (TestOpenFlag(TableOpenFlag::TOF_CHECK_PRIVILEGES))
			{
				m_tablePrivileges.Init(m_pDb, m_tableInfo);
				// We always need to be able to select, but the rest only if we want to write
				if ((TestAccessFlag(TableAccessFlag::AF_SELECT) && !m_tablePrivileges.Test(TablePrivilege::SELECT))
					|| (TestAccessFlag(TableAccessFlag::AF_UPDATE_PK) && !m_tablePrivileges.Test(TablePrivilege::UPDATE))
					|| (TestAccessFlag(TableAccessFlag::AF_UPDATE_WHERE) && !m_tablePrivileges.Test(TablePrivilege::UPDATE))
					|| (TestAccessFlag(TableAccessFlag::AF_INSERT) && !m_tablePrivileges.Test(TablePrivilege::INSERT))
					|| (TestAccessFlag(TableAccessFlag::AF_DELETE_PK) && !m_tablePrivileges.Test(TablePrivilege::DEL))
					|| (TestAccessFlag(TableAccessFlag::AF_DELETE_WHERE) && !m_tablePrivileges.Test(TablePrivilege::DEL))
					)
				{
					Exception ex((boost::wformat(L"Not sufficient Privileges to Open Table '%s'") % m_tableInfo.GetQueryName()).str());
					SET_EXCEPTION_SOURCE(ex);
					throw ex;
				}
			}

			// Maybe the primary keys have been set manually? Then just forward them to the ColumnBuffers
			if (m_primaryKeyColumnIndexes.size() > 0)
			{
				// And implicitly activate the flag TOF_DO_NOT_QUERY_PRIMARY_KEYS
				m_openFlags.Set(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS);

				for (std::set<SQLUSMALLINT>::const_iterator it = m_primaryKeyColumnIndexes.begin(); it != m_primaryKeyColumnIndexes.end(); ++it)
				{
					// Get corresponding Buffer and set the flag CF_PRIMARY_KEY
					SqlCBufferVariantMap::iterator itCols = m_columns.find(*it);
					if (itCols == m_columns.end())
					{
						Exception ex(boost::str(boost::wformat(L"No ColumnBuffer was found for a manually defined primary key column index (index = %d)") % *it));
						SET_EXCEPTION_SOURCE(ex);
						throw ex;
					}
					try
					{
						ColumnFlags& flags = boost::polymorphic_get<ColumnFlags>(itCols->second);
						flags.Set(ColumnFlag::CF_PRIMARY_KEY);
					}
					catch (const boost::bad_polymorphic_get& ex)
					{
						WrapperException we(ex);
						SET_EXCEPTION_SOURCE(we);
						throw we;
					}
				}
			}
			else
			{
				// If we need the primary keys and are allowed to query them, try to fetch them from the database
				if ((TestAccessFlag(TableAccessFlag::AF_UPDATE_PK) || TestAccessFlag(TableAccessFlag::AF_DELETE_PK)) && !TestOpenFlag(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS))
				{
					TablePrimaryKeysVector primaryKeys = m_pDb->ReadTablePrimaryKeys(m_tableInfo);
					// Match them against the ColumnBuffers
					for (TablePrimaryKeysVector::const_iterator itKeys = primaryKeys.begin(); itKeys != primaryKeys.end(); ++itKeys)
					{
						const TablePrimaryKeyInfo& keyInfo = *itKeys;
						const ObjectName* pKeyName = dynamic_cast<const ObjectName*>(&keyInfo);
						// Find corresponding ColumnBuffer
						bool settedFlagOnBuffer = false;
						for (SqlCBufferVariantMap::iterator itCols = m_columns.begin(); itCols != m_columns.end(); ++itCols)
						{
							SqlCBufferVariant& column = itCols->second;
							try
							{
								ExtendedColumnPropertiesHolder& props = boost::polymorphic_get<ExtendedColumnPropertiesHolder>(column);
								std::shared_ptr<ObjectName> pName = props.GetObjectName();
								exASSERT(pName != NULL);
								if (*pKeyName == *pName)
								{
									ColumnFlags& flags = boost::polymorphic_get<ColumnFlags>(column);
									flags.Set(ColumnFlag::CF_PRIMARY_KEY);
									settedFlagOnBuffer = true;
									break;
								}
							}
							catch (const boost::bad_polymorphic_get& ex)
							{
								WrapperException we(ex);
								SET_EXCEPTION_SOURCE(we);
								throw we;
							}
						}
						if (!settedFlagOnBuffer)
						{
							Exception ex((boost::wformat(L"Not all primary Keys of table '%s' have a corresponding ColumnBuffer. No ColumnBuffer found for PrimaryKey '%s'") % m_tableInfo.GetQueryName() % keyInfo.GetQueryName()).str());
							SET_EXCEPTION_SOURCE(ex);
							throw ex;
						}
					}
				}
			}

			// Bind the member variables for field exchange between
			// the Table object and the ODBC record for Select()
			if (TestAccessFlag(TableAccessFlag::AF_SELECT))
			{
				SQLSMALLINT boundColumnNumber = 1;
				for (SqlCBufferVariantMap::iterator it = m_columns.begin(); it != m_columns.end(); it++)
				{
					SqlCBufferVariant& columnBuffer = it->second;
					const ColumnFlags& columnFlags = boost::polymorphic_get<ColumnFlags>(columnBuffer);
					if (columnFlags.Test(ColumnFlag::CF_SELECT))
					{
						BindSelectVisitor bindSelect(boundColumnNumber, m_pHStmtSelect);
						boost::apply_visitor(bindSelect, columnBuffer);
						boundColumnNumber++;
					}
				}

				boundSelect = true;
			}

			// Create additional CF_UPDATE and DELETE statement-handles to be used with the pk-columns
			// and bind the params. PKs are required.
			if (TestAccessFlag(TableAccessFlag::AF_UPDATE_PK) || TestAccessFlag(TableAccessFlag::AF_DELETE_PK))
			{
				// Need to have at least one primary key, and all primary key buffers must be bound
				// \todo: Need to have pk is true, but the check if they are bound is useless, not?
				// we must check when preparing the statements that we are bound, not here ??
				//int primaryKeyCount = 0;
				//for (SqlCBufferVariantMap::const_iterator itCols = m_columns.begin(); itCols != m_columns.end(); ++itCols)
				//{
				//	const SqlCBufferVariant& column = itCols->second;

				//	if (pBuffer->IsPrimaryKey())
				//	{
				//		primaryKeyCount++;
				//		if ( ! pBuffer->IsBound())
				//		{
				//			Exception ex((boost::wformat(L"PrimaryKey ColumnBuffer '%s' of Table '%s' is not Bound().") %pBuffer->GetQueryNameNoThrow() % m_tableInfo.GetQueryName()).str());
				//			SET_EXCEPTION_SOURCE(ex);
				//			throw ex;
				//		}
				//	}
				//}
				//if (primaryKeyCount == 0)
				//{
				//	Exception ex((boost::wformat(L"Table '%s' has no primary keys") % m_tableInfo.GetQueryName()).str());
				//	SET_EXCEPTION_SOURCE(ex);
				//	throw ex;
				//}

				//if (TestAccessFlag(AF_UPDATE_PK))
				//{
				//	BindUpdateParameters();
				//	boundUpdatePk = true;
				//}

				//if (TestAccessFlag(AF_DELETE_PK))
				//{
				//	BindDeleteParameters();
				//	boundDeletePk = true;
				//}
			}

			// Bind CF_INSERT params
			if (TestAccessFlag(TableAccessFlag::AF_INSERT))
			{
				BindInsertParameters();
				boundInsert = true;
			}

			// Completed successfully
			m_isOpen = true;
		}
		catch (const Exception& ex)
		{
			HIDE_UNUSED(ex);
			// Unbind all Buffers
			for (SqlCBufferVariantMap::iterator it = m_columns.begin(); it != m_columns.end(); it++)
			{
				SqlCBufferVariant& columnBuffer = it->second;
				UnbindVisitor unbind;
				boost::apply_visitor(unbind, columnBuffer);
			}

			//if (boundSelect)
			//{
			//	SQLRETURN ret = SQLFreeStmt(m_hStmtSelect, SQL_UNBIND);
			//	WARN_IFN_SUCCEEDED_MSG(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtSelect, boost::str(boost::wformat(L"Failed to unbind from Select-handle during cleanup of Exception '%s': %s") % ex.ToString()));
			//}
			//if (boundDeletePk)
			//{
			//	SQLRETURN ret = SQLFreeStmt(m_hStmtDeletePk, SQL_RESET_PARAMS);
			//	WARN_IFN_SUCCEEDED_MSG(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtDeletePk, boost::str(boost::wformat(L"Failed to unbind from DeletePk-handle during cleanup of Exception '%s': %s") % ex.ToString()));
			//}
			//if (boundUpdatePk)
			//{
			//	SQLRETURN ret = SQLFreeStmt(m_hStmtUpdatePk, SQL_RESET_PARAMS);
			//	WARN_IFN_SUCCEEDED_MSG(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtUpdatePk, boost::str(boost::wformat(L"Failed to unbind from UpdatePk-handle during cleanup of Exception '%s': %s") % ex.ToString()));
			//}
			//if (boundInsert)
			//{
			//	SQLRETURN ret = SQLFreeStmt(m_hStmtInsert, SQL_RESET_PARAMS);
			//	WARN_IFN_SUCCEEDED_MSG(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtInsert, boost::str(boost::wformat(L"Failed to unbind from Insert-handle during cleanup of Exception '%s': %s") % ex.ToString()));
			//}

			// remove the ColumnBuffers if we have allocated them during this process (if not manual)
			if (!m_manualColumns)
			{
				m_columns.clear();
				//for (ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin(); it != m_columnBuffers.end(); ++it)
				//{
				//	ColumnBuffer* pBuffer = it->second;
				//	delete pBuffer;
				//}
				//m_columnBuffers.clear();
			}
			// and rethrow
			throw;
		}
	}


	void Table::Close()
	{
		exASSERT(IsOpen());

		// Unbind all buffers
		for (SqlCBufferVariantMap::iterator it = m_columns.begin(); it != m_columns.end(); it++)
		{
			SqlCBufferVariant& columnBuffer = it->second;
			UnbindVisitor unbind;
			boost::apply_visitor(unbind, columnBuffer);
		}

		// remove the ColumnBuffers if we have allocated them during this process (if not manual)
		if (!m_manualColumns)
		{
			m_columns.clear();
		}


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
		//if (!m_manualColumns)
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
		FreeStatements();
		m_tableAccessFlags = acs;
		AllocateStatements();
	}


	bool Table::TestAccessFlag(TableAccessFlag af) const noexcept
	{
		return m_tableAccessFlags.Test(af);
	}


	bool Table::TestOpenFlag(TableOpenFlag of) const noexcept
	{
		return m_openFlags.Test(of);
	}



	void Table::SetCharTrimLeft(bool trimLeft) noexcept
	{
		if (trimLeft)
		{
			m_openFlags.Set(TableOpenFlag::TOF_CHAR_TRIM_LEFT);
		}
		else
		{
			m_openFlags.Clear(TableOpenFlag::TOF_CHAR_TRIM_LEFT);
		}
	}


	void Table::SetCharTrimRight(bool trimRight) noexcept
	{
		if (trimRight)
		{
			m_openFlags.Set(TableOpenFlag::TOF_CHAR_TRIM_RIGHT);
		}
		else
		{
			m_openFlags.Clear(TableOpenFlag::TOF_CHAR_TRIM_RIGHT);
		}
	}


	void Table::SetColumnNTS(SQLSMALLINT columnIndex) const
	{
		SetColumnLengthIndicator(columnIndex, SQL_NTS);
	}


	void Table::SetColumnLengthIndicator(SQLSMALLINT columnIndex, SQLLEN cb) const
	{
		const SqlCBufferVariant& var = GetColumnVariant(columnIndex);
		try
		{
			SqlCBufferLengthIndicator varCb = boost::polymorphic_get<SqlCBufferLengthIndicator>(var);
			varCb.SetCb(cb);
		}
		catch (const boost::bad_polymorphic_get& ex)
		{
			WrapperException we(ex);
			SET_EXCEPTION_SOURCE(we);
			throw we;
		}
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