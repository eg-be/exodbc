/*!
* \file Table.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 25.07.2014
* \brief Source file for the Table class and its helpers.
* \copyright wxWindows Library Licence, Version 3.1
*
* Header file for the Table class and its helpers.
* This file was originally wx/dbtable.cpp from wxWidgets 2.8.
* Most of the code has been rewritten, a lot of functionality
* not needed and not tested so far has been droped.
*
* For completion, here follows the old wxWidgets header:
*
* ///////////////////////////////////////////////////////////////////////////////<br>
* // Name:        src/common/dbtable.cpp<br>
* // Purpose:     Implementation of the wxDbTable class.<br>
* // Author:      Doug Card<br>
* // Modified by: George Tasker<br>
* //              Bart Jourquin<br>
* //              Mark Johnson<br>
* // Created:     9.96<br>
* // RCS-ID:      $Id: dbtable.cpp 48685 2007-09-14 19:02:28Z VZ $<br>
* // Copyright:   (c) 1996 Remstar International, Inc.<br>
* // Licence:     wxWindows licence<br>
* ///////////////////////////////////////////////////////////////////////////////<br>
*/

#include "stdafx.h"

// Own header
#include "Table.h"

// Same component headers
#include "Helpers.h"
#include "ColumnBuffer.h"
#include "Environment.h"
#include "Exception.h"

// Other headers

// Debug
#include "DebugNew.h"

using namespace std;

namespace exodbc
{
	// Construction
	// ------------
	Table::Table(const Database& db, SQLSMALLINT numColumns, const std::wstring& tableName, const std::wstring& schemaName /* = L"" */, const std::wstring& catalogName /* = L"" */, const std::wstring& tableType /* = L"" */, AccessFlags afs /* = AF_READ_WRITE */)
		: m_numCols(numColumns)
		, m_manualColumns(true)
		, m_initialTableName(tableName)
		, m_initialSchemaName(schemaName)
		, m_initialCatalogName(catalogName)
		, m_initialTypeName(tableType)
		, m_haveTableInfo(false)
		, m_accessFlags(AF_NONE)
	{
		exASSERT(db.IsOpen());
		
		Initialize();
		SetAccessFlags(db, afs);
		// statements should now be allocated
		exASSERT(HasAllStatements());
	}


	Table::Table(const Database& db, SQLSMALLINT numColumns, const STableInfo& tableInfo, AccessFlags afs /* = AF_READ_WRITE */)
		: m_numCols(numColumns)
		, m_manualColumns(true)
		, m_initialTableName(L"")
		, m_initialSchemaName(L"")
		, m_initialCatalogName(L"")
		, m_initialTypeName(L"")
		, m_haveTableInfo(true)
		, m_tableInfo(tableInfo)
		, m_accessFlags(AF_NONE)
	{
		exASSERT(db.IsOpen());
		
		Initialize();
		SetAccessFlags(db, afs);
		// statements should now be allocated
		exASSERT(HasAllStatements());
	}


	Table::Table(const Database& db, const std::wstring& tableName, const std::wstring& schemaName /* = L"" */, const std::wstring& catalogName /* = L"" */, const std::wstring& tableType /* = L"" */, AccessFlags afs /* = AF_READ_WRITE */)
		: m_numCols(0)
		, m_manualColumns(false)
		, m_initialTableName(tableName)
		, m_initialSchemaName(schemaName)
		, m_initialCatalogName(catalogName)
		, m_initialTypeName(tableType)
		, m_haveTableInfo(false)
		, m_accessFlags(AF_NONE)
	{
		exASSERT(db.IsOpen());

		Initialize();
		SetAccessFlags(db, afs);
		// statements should now be allocated
		exASSERT(HasAllStatements());
	}


	Table::Table(const Database& db, const STableInfo& tableInfo, AccessFlags afs /* = AF_READ_WRITE */)
		: m_numCols(0)
		, m_manualColumns(false)
		, m_initialTableName(L"")
		, m_initialSchemaName(L"")
		, m_initialCatalogName(L"")
		, m_initialTypeName(L"")
		, m_haveTableInfo(true)
		, m_tableInfo(tableInfo)
		, m_accessFlags(AF_NONE)
	{
		exASSERT(db.IsOpen());
		
		Initialize();
		SetAccessFlags(db, afs);
		// statements should now be allocated
		exASSERT(HasAllStatements());
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
			if (m_manualColumns)
			{
				for (ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin(); it != m_columnBuffers.end(); ++it)
				{
					ColumnBuffer* pBuffer = it->second;
					delete pBuffer;
				}
				m_columnBuffers.clear();
				m_numCols = 0;
			}

			FreeStatements();
			exASSERT(m_numCols == 0);
			exASSERT(m_columnBuffers.size() == 0);
		}
		catch (Exception ex)
		{
			LOG_ERROR(ex.ToString());
		}
	}


	// Implementation
	// --------------
	void Table::Initialize()
	{
		// Initializing member variables
		// Note: m_haveTableInfo must have been set before this function is called - it is not modified by this Initialize
		m_isOpen = false;

		m_hStmtCount = SQL_NULL_HSTMT;
		m_hStmtSelect = SQL_NULL_HSTMT;
		m_hStmtInsert = SQL_NULL_HSTMT;
		m_hStmtUpdate = SQL_NULL_HSTMT;
		m_hStmtDelete = SQL_NULL_HSTMT;
		m_hStmtDeleteWhere = SQL_NULL_HSTMT;
		m_hStmtUpdateWhere = SQL_NULL_HSTMT;
		m_selectQueryOpen = false;
		m_fieldsStatement = L"";
		m_autoBindingMode = AutoBindingMode::BIND_AS_REPORTED;
		m_openFlags = TOF_NONE;
		m_accessFlags = AF_NONE;
	}


	void Table::AllocateStatements(const Database& db)
	{
		exASSERT(!IsOpen());
		exASSERT(db.IsOpen());

		exASSERT(SQL_NULL_HSTMT == m_hStmtCount);
		exASSERT(SQL_NULL_HSTMT == m_hStmtSelect);
		exASSERT(SQL_NULL_HSTMT == m_hStmtInsert);
		exASSERT(SQL_NULL_HSTMT == m_hStmtUpdate);
		exASSERT(SQL_NULL_HSTMT == m_hStmtDeleteWhere);
		exASSERT(SQL_NULL_HSTMT == m_hStmtUpdateWhere);

		// Allocate handles needed
		if (TestAccessFlag(AF_SELECT))
		{
			m_hStmtCount = AllocateStatementHandle(db.GetConnectionHandle());
			m_hStmtSelect = AllocateStatementHandle(db.GetConnectionHandle());
		}

		// Allocate handles needed for writing
		if (TestAccessFlag(AF_INSERT))
		{
			m_hStmtInsert = AllocateStatementHandle(db.GetConnectionHandle());
		}
		if (TestAccessFlag(AF_UPDATE_PK))
		{
			m_hStmtUpdate = AllocateStatementHandle(db.GetConnectionHandle());
		}
		if (TestAccessFlag(AF_UPDATE_WHERE))
		{
			m_hStmtUpdateWhere = AllocateStatementHandle(db.GetConnectionHandle());
		}

		if (TestAccessFlag(AF_DELETE_PK))
		{
			m_hStmtDelete = AllocateStatementHandle(db.GetConnectionHandle());
		}
		if (TestAccessFlag(AF_DELETE_WHERE))
		{
			m_hStmtDeleteWhere = AllocateStatementHandle(db.GetConnectionHandle());
		}
	}


	bool Table::HasAllStatements() const throw()
	{
		bool haveAll = true;
		if (haveAll && TestAccessFlag(AF_SELECT))
		{
			haveAll = (SQL_NULL_HSTMT != m_hStmtSelect) && (SQL_NULL_HSTMT != m_hStmtCount);
		}
		if (haveAll && TestAccessFlag(AF_UPDATE_PK))
		{
			haveAll = (SQL_NULL_HSTMT != m_hStmtUpdate);
		}
		if (haveAll && TestAccessFlag(AF_UPDATE_WHERE))
		{
			haveAll = (SQL_NULL_HSTMT != m_hStmtUpdateWhere);
		}
		if (haveAll && TestAccessFlag(AF_INSERT))
		{
			haveAll = (SQL_NULL_HSTMT != m_hStmtInsert);
		}
		if (haveAll && TestAccessFlag(AF_DELETE_PK))
		{
			haveAll = (SQL_NULL_HSTMT != m_hStmtDelete);
		}
		if (haveAll && TestAccessFlag(AF_DELETE_WHERE))
		{
			haveAll = (SQL_NULL_HSTMT != m_hStmtDeleteWhere);
		}
		return haveAll;
	}


	void Table::CreateAutoColumnBuffers(const Database& db, const STableInfo& tableInfo, bool skipUnsupportedColumns)
	{
		exASSERT(m_manualColumns == false);
		exASSERT(m_columnBuffers.size() == 0);
		exASSERT(m_numCols == 0);

		// Nest try-catch to always free eventually allocated buffers
		try
		{
			// Will throw if fails
			ColumnInfosVector columns = db.ReadTableColumnInfo(tableInfo);
			// Remember column sizes and create ColumnBuffers
			m_numCols = columns.size();
			if (m_numCols == 0)
			{
				Exception ex((boost::wformat(L"No columns found for table '%s'") % tableInfo.GetSqlName()).str());
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
			// We need to know which ODBC version we are using, might throw
			OdbcVersion odbcVersion = db.GetMaxSupportedOdbcVersion();
			// And how to open this column
			ColumnFlags columnFlags = CF_NONE;
			if (TestAccessFlag(AF_SELECT))
			{
				columnFlags |= CF_SELECT;
			}
			if (TestAccessFlag(AF_UPDATE))
			{
				columnFlags |= CF_UPDATE;
			}
			if (TestAccessFlag(AF_INSERT))
			{
				columnFlags |= CF_INSERT;
			}
			int bufferIndex = 0;
			for (int columnIndex = 0; columnIndex < (SQLSMALLINT)columns.size(); columnIndex++)
			{
				SColumnInfo colInfo = columns[columnIndex];
				try
				{
					ColumnBuffer* pColBuff = new ColumnBuffer(colInfo, m_autoBindingMode, odbcVersion, columnFlags);
					m_columnBuffers[bufferIndex] = pColBuff;
					++bufferIndex;
				}
				catch (NotSupportedException nse)
				{
					if (skipUnsupportedColumns)
					{
						// Ignore unsupported column. (note: If it has thrown from the constructor, memory is already deleted)
						LOG_WARNING(boost::str(boost::wformat(L"Failed to create ColumnBuffer for column '%s': %s") % colInfo.GetSqlName() % nse.ToString()));
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
		catch (Exception ex)
		{
			// Free allocated buffers and rethrow
			for (ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin(); it != m_columnBuffers.end(); ++it)
			{
				ColumnBuffer* pColumnBuffer = it->second;
				delete pColumnBuffer;
			}
			m_columnBuffers.clear();
			m_numCols = 0;
			throw;
		}
	}


	void Table::FreeStatements()
	{
		exASSERT(!IsOpen());

		// Free allocated statements
		if (m_hStmtCount != SQL_NULL_HSTMT)
		{
			m_hStmtCount = FreeStatementHandle(m_hStmtCount);
		}
		if (m_hStmtSelect != SQL_NULL_HSTMT)
		{
			m_hStmtSelect = FreeStatementHandle(m_hStmtSelect);
		}

		// And then those needed for writing
		if (m_hStmtInsert != SQL_NULL_HSTMT)
		{
			m_hStmtInsert = FreeStatementHandle(m_hStmtInsert);
		}
		if (m_hStmtDelete != SQL_NULL_HSTMT)
		{
			m_hStmtDelete = FreeStatementHandle(m_hStmtDelete);
		}
		if (m_hStmtDeleteWhere != SQL_NULL_HSTMT)
		{
			m_hStmtDeleteWhere = FreeStatementHandle(m_hStmtDeleteWhere);
		}
		if (m_hStmtUpdate != SQL_NULL_HSTMT)
		{
			m_hStmtUpdate = FreeStatementHandle(m_hStmtUpdate);
		}
		if (m_hStmtUpdateWhere != SQL_NULL_HSTMT)
		{
			m_hStmtUpdateWhere = FreeStatementHandle(m_hStmtUpdateWhere);
		}
	}


	std::wstring Table::BuildFieldsStatement() const
	{
		exASSERT(m_columnBuffers.size() > 0);

		std::wstring fields = L"";
		ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin();
		while (it != m_columnBuffers.end())
		{
			ColumnBuffer* pBuffer = it->second;
			if (pBuffer->IsColumnFlagSet(CF_SELECT))
			{
				fields += pBuffer->GetQueryName();
				fields += L", ";
			}
			++it;
		}
		boost::algorithm::erase_last(fields, L", ");

		return fields;
	}


	void Table::BindDeleteParameters()
	{
		exASSERT(m_columnBuffers.size() > 0);
		exASSERT(TestAccessFlag(AF_DELETE_PK));
		exASSERT(m_hStmtDelete != SQL_NULL_HSTMT);
		exASSERT(m_tablePrimaryKeys.AreAllPrimaryKeysBound(m_columnBuffers));

		// Build statement
		// note: parem-number reflects here the number of the param in the created prepared statement, so we
		// cannot use the values from the ColumnBuffer column index.
		int paramNr = 1;
		wstring deleteStmt = (boost::wformat(L"DELETE FROM %s WHERE ") %m_tableInfo.GetSqlName()).str();
		for (ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin(); it != m_columnBuffers.end(); it++)
		{
			ColumnBuffer* pBuffer = it->second;
			if (pBuffer->IsColumnFlagSet(CF_PRIMARY_KEY))
			{
				// Bind this parameter as primary key and include it in the statement
				pBuffer->BindParameter(m_hStmtDelete, paramNr);
				deleteStmt += (boost::wformat(L"%s = ? AND ") % pBuffer->GetQueryName()).str();
				paramNr++;
			}
		}
		boost::erase_last(deleteStmt, L" AND ");
		// ensure that we have something in our where clause
		if (paramNr <= 1)
		{
			Exception ex(boost::str(boost::wformat(L"Table '%s' was requested to prepare a DELETE statement (probably because flag AF_DELETE_PK is set), but no ColumnBuffers were bound as PrimaryKeys to build a WHERE clause") % m_tableInfo.GetSqlName()));
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}

		// Prepare to delete
		SQLRETURN ret = SQLPrepare(m_hStmtDelete, (SQLWCHAR*)deleteStmt.c_str(), SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLPrepare, ret, SQL_HANDLE_STMT, m_hStmtDelete);
	}


	void Table::BindUpdateParameters()
	{
		exASSERT(m_columnBuffers.size() > 0);
		exASSERT(TestAccessFlag(AF_UPDATE_PK));
		exASSERT(m_hStmtUpdate != SQL_NULL_HSTMT);
		exASSERT(m_tablePrimaryKeys.AreAllPrimaryKeysBound(m_columnBuffers));

		// Build statement..
		wstring updateStmt = (boost::wformat(L"UPDATE %s SET ") % m_tableInfo.GetSqlName()).str();
		// .. first the values to update
		// note: parem-number reflects here the number of the param in the created prepared statement, so we
		// cannot use the values from the ColumnBuffer column index.
		int paramNr = 1;
		for (ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin(); it != m_columnBuffers.end(); it++)
		{
			ColumnBuffer* pBuffer = it->second;
			if (pBuffer->IsColumnFlagSet(CF_UPDATE) && !pBuffer->IsColumnFlagSet(CF_PRIMARY_KEY))
			{
				// Bind this parameter as update parameter and include it in the statement
				// The update params come first, so the numbering works
				pBuffer->BindParameter(m_hStmtUpdate, paramNr);
				updateStmt += (boost::wformat(L"%s = ?, ") % pBuffer->GetQueryName()).str();
				paramNr++;
			}
		}
		boost::erase_last(updateStmt, L",");
		// ensure that we have something to update
		if (paramNr <= 1)
		{	
			Exception ex(boost::str(boost::wformat(L"Table '%s' was requested to bind update parameters (probably because flag AF_UPDATE_PK is set), but no ColumnBuffers were bound for UPDATing") % m_tableInfo.GetSqlName()));
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}
		updateStmt += L"WHERE ";
		bool haveWhereParam = false;
		for (ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin(); it != m_columnBuffers.end(); it++)
		{
			ColumnBuffer* pBuffer = it->second;
			if (pBuffer->IsColumnFlagSet(CF_PRIMARY_KEY))
			{
				// Bind this parameter as primary key and include it in the where part of the statement
				// The update params come first, so the numbering works
				pBuffer->BindParameter(m_hStmtUpdate, paramNr);
				updateStmt += (boost::wformat(L"%s = ? AND ") % pBuffer->GetQueryName()).str();
				paramNr++;
				haveWhereParam = true;
			}
		}
		boost::erase_last(updateStmt, L"AND ");
		// and ensure that we have something as where
		exASSERT(haveWhereParam);

		// Prepare to update
		SQLRETURN ret = SQLPrepare(m_hStmtUpdate, (SQLWCHAR*)updateStmt.c_str(), SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLPrepare, ret, SQL_HANDLE_STMT, m_hStmtUpdate);
	}


	void Table::BindInsertParameters()
	{
		exASSERT(m_columnBuffers.size() > 0);
		exASSERT(TestAccessFlag(AF_INSERT));
		exASSERT(m_hStmtInsert != SQL_NULL_HSTMT);

		// Build a statement with parameter-markers
		// note: parem-number reflects here the number of the param in the created prepared statement, so we
		// cannot use the values from the ColumnBuffer column index.
		SQLSMALLINT paramNr = 1;
		std::wstring insertStmt = L"INSERT INTO " + m_tableInfo.GetSqlName() + L" (";
		ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin();
		while (it != m_columnBuffers.end())
		{
			ColumnBuffer* pBuffer = it->second;
			// Bind parameter if it is marked an INSERTable
			if (pBuffer->IsColumnFlagSet(CF_INSERT))
			{
				pBuffer->BindParameter(m_hStmtInsert, paramNr);
				// prepare statement
				insertStmt += pBuffer->GetQueryName() + L", ";
				paramNr++;
			}
			++it;
		}
		// ensure that we have something to insert
		if (paramNr <= 1)
		{
			Exception ex(boost::str(boost::wformat(L"Table '%s' was requested to bind insert parameters (probably because flag AF_INSERT is set), but no ColumnBuffers were bound for INSERTing") % m_tableInfo.GetSqlName()));
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}
		boost::erase_last(insertStmt, L", ");
		// and set markers for the values
		insertStmt += L") VALUES(";
		for (int i = 0; i < paramNr - 2; i++)
		{
			insertStmt += L"?, ";
		}
		insertStmt += L"?)";

		// Prepare to update
		SQLRETURN ret = SQLPrepare(m_hStmtInsert, (SQLWCHAR*) insertStmt.c_str(), SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLPrepare, ret, SQL_HANDLE_STMT, m_hStmtInsert);
	}


	ColumnBuffer* Table::GetColumnBuffer(SQLSMALLINT columnIndex) const
	{
		exASSERT(IsOpen());
		exASSERT(columnIndex < m_numCols);

		ColumnBufferPtrMap::const_iterator it = m_columnBuffers.find(columnIndex);
		if (it == m_columnBuffers.end())
		{
			IllegalArgumentException ex(boost::str(boost::wformat(L"ColumnIndex %d is not a zero-based bound or manually defined ColumnBuffer") % columnIndex));
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}

		return it->second;
	}


	ColumnBuffer* Table::GetNonNullColumnBuffer(SQLSMALLINT columnIndex) const
	{
		ColumnBuffer* pBuffer = GetColumnBuffer(columnIndex);
		if (pBuffer->IsNull())
		{
			NullValueException ex(pBuffer->GetQueryName());
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}
		return pBuffer;
	}


	std::set<SQLSMALLINT> Table::GetColumnBufferIndexes() const throw()
	{
		std::set<SQLSMALLINT> columnIndexes;
		ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin();
		while (it != m_columnBuffers.end())
		{
			columnIndexes.insert(it->first);
			++it;
		}
		return columnIndexes;
	}


	STableInfo Table::GetTableInfo() const
	{
		exASSERT(m_haveTableInfo);
		return m_tableInfo;
	}


	SQLUBIGINT Table::Count(const std::wstring& whereStatement)
	{
		exASSERT(IsOpen());

		// Close Statement handle on exit
		StatementCloser stmtCloser(m_hStmtCount, false, true);

		SQLUBIGINT count = 0;
		std::wstring sqlstmt;
		if ( ! whereStatement.empty())
		{
			sqlstmt = (boost::wformat(L"SELECT COUNT(*) FROM %s WHERE %s") % m_tableInfo.GetSqlName() % whereStatement).str();
		}
		else
		{
			sqlstmt = (boost::wformat(L"SELECT COUNT(*) FROM %s") % m_tableInfo.GetSqlName()).str();
		}

		SQLRETURN ret = SQLExecDirect(m_hStmtCount, (SQLWCHAR*)sqlstmt.c_str(), SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLExecDirect, ret, SQL_HANDLE_STMT, m_hStmtCount);
			
		ret = SQLFetch(m_hStmtCount);
		THROW_IFN_SUCCEEDED(SQLFetch, ret, SQL_HANDLE_STMT, m_hStmtCount);

		bool isNull = false;
		SQLINTEGER cb = 0;
		GetData(m_hStmtCount, 1, SQL_C_UBIGINT, &count, sizeof(count), &cb, &isNull);
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
			sqlstmt = (boost::wformat(L"SELECT %s FROM %s WHERE %s") % m_fieldsStatement % m_tableInfo.GetSqlName() % whereStatement).str();
		}
		else
		{
			sqlstmt = (boost::wformat(L"SELECT %s FROM %s") % m_fieldsStatement % m_tableInfo.GetSqlName()).str();
		}
		SelectBySqlStmt(sqlstmt);
	}


	void Table::SelectBySqlStmt(const std::wstring& sqlStmt)
	{
		exASSERT(IsOpen());
		exASSERT(!sqlStmt.empty());

		if (IsSelectOpen())
		{
			SelectClose();
		}

		SQLRETURN ret = SQLExecDirect(m_hStmtSelect, (SQLWCHAR*)sqlStmt.c_str(), SQL_NTS);
		THROW_IFN_SUCCESS(SQLExecDirect, ret, SQL_HANDLE_STMT, m_hStmtSelect);
		m_selectQueryOpen = true;
	}


	bool Table::SelectNext()
	{
		exASSERT(IsSelectOpen());
		
		SQLRETURN ret = SQLFetch(m_hStmtSelect);
		if ( ! (SQL_SUCCEEDED(ret) || ret == SQL_NO_DATA))
		{
			LOG_ERROR_STMT(m_hStmtSelect, ret, SQLFetch);
		}
		if (ret == SQL_SUCCESS_WITH_INFO)
		{
			LOG_WARNING_STMT(m_hStmtSelect, ret, SQLFetch);
		}

		return SQL_SUCCEEDED(ret);
	}

	
	void Table::SelectClose()
	{
		CloseStmtHandle(m_hStmtSelect, StmtCloseMode::IgnoreNotOpen);
		m_selectQueryOpen = false;
	}


	void Table::Insert() const
	{
		exASSERT(IsOpen());
		exASSERT(TestAccessFlag(AF_INSERT));
		exASSERT(m_hStmtInsert != SQL_NULL_HSTMT);
		SQLRETURN ret = SQLExecute(m_hStmtInsert);
		THROW_IFN_SUCCEEDED(SQLExecute, ret, SQL_HANDLE_STMT, m_hStmtInsert);
	}


	void Table::Delete(bool failOnNoData /* = true */)
	{
		exASSERT(IsOpen());
		exASSERT(TestAccessFlag(AF_DELETE_PK));
		exASSERT(m_hStmtDelete != SQL_NULL_HSTMT);
		SQLRETURN ret = SQLExecute(m_hStmtDelete);
		if (failOnNoData && ret == SQL_NO_DATA)
		{
			SqlResultException ex(L"SQLExecute", ret, L"Did not expect a SQL_NO_DATA while executing a delete-query");
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}
		if (!(!failOnNoData && ret == SQL_NO_DATA))
		{
			THROW_IFN_SUCCEEDED(SQLExecute, ret, SQL_HANDLE_STMT, m_hStmtDelete);
		}
	}


	void Table::Delete(const std::wstring& where, bool failOnNoData /* = true */) const
	{
		exASSERT(IsOpen());
		exASSERT(TestAccessFlag(AF_DELETE_WHERE));
		exASSERT(m_hStmtDeleteWhere != SQL_NULL_HSTMT);
		exASSERT(!where.empty());

		wstring sqlstmt = (boost::wformat(L"DELETE FROM %s WHERE %s") % m_tableInfo.GetSqlName() % where).str();
		SQLRETURN ret = SQLExecDirect(m_hStmtDeleteWhere, (SQLWCHAR*)sqlstmt.c_str(), sqlstmt.length());
		if (failOnNoData && ret == SQL_NO_DATA)
		{
			SqlResultException ex(L"SQLExecute", ret, boost::str(boost::wformat(L"Did not expect a SQL_NO_DATA while executing the delete-query '%s'") %sqlstmt));
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}
		if ( ! (!failOnNoData && ret == SQL_NO_DATA))
		{
			THROW_IFN_SUCCEEDED(SQLExecute, ret, SQL_HANDLE_STMT, m_hStmtDeleteWhere);
		}
	}


	void Table::Update()
	{
		exASSERT(IsOpen());
		exASSERT(TestAccessFlag(AF_UPDATE_PK));
		exASSERT(m_hStmtUpdate != SQL_NULL_HSTMT);
		SQLRETURN ret = SQLExecute(m_hStmtUpdate);
		THROW_IFN_SUCCEEDED(SQLExecute, ret, SQL_HANDLE_STMT, m_hStmtUpdate);
	}


	void Table::Update(const std::wstring& where)
	{
		exASSERT(IsOpen());
		exASSERT(TestAccessFlag(AF_UPDATE_WHERE));
		exASSERT(m_hStmtUpdateWhere != SQL_NULL_HSTMT);
		exASSERT(!where.empty());
		// Format an update-statement that updates all Bound columns that have the flag CF_UPDATE set
		wstring updateStmt = (boost::wformat(L"UPDATE %s SET ") % m_tableInfo.GetSqlName()).str();
		// .. first the values to update
		// note: parem-number reflects here the number of the param in the created prepared statement, so we
		// cannot use the values from the ColumnBuffer column index.
		int paramNr = 1;
		for (ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin(); it != m_columnBuffers.end(); it++)
		{
			ColumnBuffer* pBuffer = it->second;
			if (pBuffer->IsColumnFlagSet(CF_UPDATE))
			{
				// Bind this parameter as update parameter and include it in the statement
				pBuffer->BindParameter(m_hStmtUpdateWhere, paramNr);
				updateStmt += (boost::wformat(L"%s = ?, ") % pBuffer->GetQueryName()).str();
				paramNr++;
			}
		}
		boost::erase_last(updateStmt, L",");
		exASSERT_MSG(paramNr > 1, L"No columns are bound that have the flag CF_UPDATE set");
		updateStmt += (L"WHERE " + where);

		// Prepare to update
		SQLRETURN ret = SQLPrepare(m_hStmtUpdateWhere, (SQLWCHAR*)updateStmt.c_str(), SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLPrepare, ret, SQL_HANDLE_STMT, m_hStmtUpdateWhere);

		// And Execute
		ret = SQLExecute(m_hStmtUpdateWhere);
		THROW_IFN_SUCCEEDED(SQLExecute, ret, SQL_HANDLE_STMT, m_hStmtUpdateWhere);

		// Always unbind all parameters at the end
		ret = SQLFreeStmt(m_hStmtUpdateWhere, SQL_UNBIND);
		THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtUpdateWhere);
	}


	SQLINTEGER Table::SelectColumnAttribute(SQLSMALLINT columnIndex, ColumnAttribute attr)
	{
		exASSERT(IsSelectOpen());
		SQLINTEGER value = 0;
		SQLRETURN ret = SQLColAttribute(m_hStmtSelect, columnIndex + 1, (SQLUSMALLINT)attr, NULL, NULL, NULL, &value);
		THROW_IFN_SUCCEEDED(SQLColAttribute, ret, SQL_HANDLE_STMT, m_hStmtSelect);
		return value;
	}


	void Table::SetColumnValue(SQLSMALLINT columnIndex, const BufferVariant& value) const
	{
		ColumnBuffer* pBuff = GetColumnBuffer(columnIndex);
		*pBuff = value;
	}


	void Table::SetBinaryValue(SQLSMALLINT columnIndex, const SQLCHAR* buffer, SQLINTEGER bufferSize)
	{
		ColumnBuffer* pBuff = GetColumnBuffer(columnIndex);
		pBuff->SetBinaryValue(buffer, bufferSize);
	}


	void Table::SetColumnNull(SQLSMALLINT columnIndex) const
	{
		ColumnBuffer* pBuff = GetColumnBuffer(columnIndex);
		pBuff->SetNull();
	}


	SQLSMALLINT Table::GetSmallInt(SQLSMALLINT columnIndex) const
	{
		ColumnBuffer* pBuff = GetNonNullColumnBuffer(columnIndex);
		SQLSMALLINT smallInt = *pBuff;
		return smallInt;
	}


	SQLINTEGER Table::GetInt(SQLSMALLINT columnIndex) const
	{
		ColumnBuffer* pBuff = GetNonNullColumnBuffer(columnIndex);
		SQLINTEGER i = *pBuff;
		return i;
	}


	SQLBIGINT Table::GetBigInt(SQLSMALLINT columnIndex) const
	{
		ColumnBuffer* pBuff = GetNonNullColumnBuffer(columnIndex);
		SQLBIGINT bigInt = *pBuff;
		return bigInt;
	}


	SQL_DATE_STRUCT Table::GetDate(SQLSMALLINT columnIndex) const
	{
		ColumnBuffer* pBuff = GetNonNullColumnBuffer(columnIndex);
		SQL_DATE_STRUCT date = *pBuff;
		return date;
	}


	SQL_TIME_STRUCT Table::GetTime(SQLSMALLINT columnIndex) const
	{
		ColumnBuffer* pBuff = GetNonNullColumnBuffer(columnIndex);
		SQL_TIME_STRUCT time = *pBuff;
		return time;
	}

#if HAVE_MSODBCSQL_H
	SQL_SS_TIME2_STRUCT Table::GetTime2(SQLSMALLINT columnIndex) const
	{
		ColumnBuffer* pBuff = GetNonNullColumnBuffer(columnIndex);
		SQL_SS_TIME2_STRUCT time2 = *pBuff;
		return time2;
	}
#endif


	SQL_TIMESTAMP_STRUCT Table::GetTimeStamp(SQLSMALLINT columnIndex) const
	{
		ColumnBuffer* pBuff = GetNonNullColumnBuffer(columnIndex);
		SQL_TIMESTAMP_STRUCT timestamp = *pBuff;
		return timestamp;
	}


	std::string Table::GetString(SQLSMALLINT columnIndex) const
	{
		ColumnBuffer* pBuff = GetNonNullColumnBuffer(columnIndex);
		std::string s = *pBuff;

		if (TestOpenFlag(TOF_CHAR_TRIM_LEFT))
		{
			boost::trim_left(s);
		}
		if (TestOpenFlag(TOF_CHAR_TRIM_RIGHT))
		{
			boost::trim_right(s);
		}

		return s;
	}


	std::wstring Table::GetWString(SQLSMALLINT columnIndex) const
	{
		ColumnBuffer* pBuff = GetNonNullColumnBuffer(columnIndex);
		std::wstring ws = *pBuff;

		if (TestOpenFlag(TOF_CHAR_TRIM_LEFT))
		{
			boost::trim_left(ws);
		}
		if (TestOpenFlag(TOF_CHAR_TRIM_RIGHT))
		{
			boost::trim_right(ws);
		}

		return ws;
	}


	SQLDOUBLE Table::GetDouble(SQLSMALLINT columnIndex) const
	{
		ColumnBuffer* pBuff = GetNonNullColumnBuffer(columnIndex);
		SQLDOUBLE d = *pBuff;
		return d;
	}


	SQL_NUMERIC_STRUCT Table::GetNumeric(SQLSMALLINT columnIndex) const
	{
		ColumnBuffer* pBuff = GetNonNullColumnBuffer(columnIndex);
		SQL_NUMERIC_STRUCT num = *pBuff;
		return num;
	}


	BufferVariant Table::GetColumnValue(SQLSMALLINT columnIndex) const
	{
		ColumnBuffer* pBuff = GetColumnBuffer(columnIndex);
		return pBuff->GetValue();
	}


	const SQLCHAR* Table::GetBinaryValue(SQLSMALLINT columnIndex, SQLINTEGER& bufferSize, SQLINTEGER& lengthIndicator) const
	{
		ColumnBuffer* pBuff = GetColumnBuffer(columnIndex);
		exASSERT(!pBuff->IsNull());
		const SQLCHAR* pBuffer = *pBuff;
		bufferSize = pBuff->GetBufferSize();
		lengthIndicator = pBuff->GetCb();
		return pBuffer;
	}


	bool Table::IsColumnNull(SQLSMALLINT columnIndex) const
	{
		const ColumnBuffer* pBuff = GetColumnBuffer(columnIndex);		
		return pBuff->IsNull();
	}


	bool Table::IsColumnNullable(SQLSMALLINT columnIndex) const
	{
		const ColumnBuffer* pBuff = GetColumnBuffer(columnIndex);
		return pBuff->IsNullable();
	}


	void Table::SetColumn(SQLUSMALLINT columnIndex, const std::wstring& queryName, SQLSMALLINT sqlType, BufferPtrVariant pBuffer, SQLSMALLINT sqlCType, SQLLEN bufferSize, ColumnFlags flags /* = CF_SELECT */, SQLINTEGER columnSize /* = -1 */, SQLSMALLINT decimalDigits /* = -1 */)
	{
		exASSERT(m_manualColumns);
		exASSERT(columnIndex >= 0);
		exASSERT(columnIndex < m_numCols);
		exASSERT( ! queryName.empty());
		exASSERT(bufferSize > 0);
		exASSERT(m_columnBuffers.find(columnIndex) == m_columnBuffers.end());
		exASSERT( ! ( (sqlType == SQL_UNKNOWN_TYPE) && ((flags & CF_INSERT) || (flags & CF_UPDATE)) ) );
		// \see #129 and #133. Once the db is back, we can determine the version from there (or via environment)
		ColumnBuffer* pColumnBuffer = new ColumnBuffer(sqlCType, pBuffer, bufferSize, sqlType, queryName, OdbcVersion::UNKNOWN, flags, columnSize, decimalDigits);
		m_columnBuffers[columnIndex] = pColumnBuffer;
	}


	void Table::SetColumn(SQLUSMALLINT columnIndex, const std::wstring& queryName, BufferPtrVariant pBuffer, SQLSMALLINT sqlCType, SQLLEN bufferSize, ColumnFlags flags /* = CF_SELECT */, SQLINTEGER columnSize /* = -1 */, SQLSMALLINT decimalDigits /* = -1 */)
	{
		SetColumn(columnIndex, queryName, SQL_UNKNOWN_TYPE, pBuffer, sqlCType, bufferSize, flags, columnSize, decimalDigits);
	}


	void Table::SetColumnPrimaryKeyIndexes(const std::set<SQLUSMALLINT>& columnIndexes)
	{
		exASSERT(!IsOpen());
		m_primaryKeyColumnIndexes = columnIndexes;
	}


	void Table::Open(const Database& db, TableOpenFlags openFlags /* = TOF_CHECK_EXISTANCE */)
	{
		exASSERT(db.IsOpen());
		exASSERT(!IsOpen());
		exASSERT(HasAllStatements());
		// \note: We do not force a user to define columns.

		// Set TOF_DO_NOT_QUERY_PRIMARY_KEYS this flag for Access, Access does not support SQLPrimaryKeys
		if (db.GetDbms() == DatabaseProduct::ACCESS)
		{
			openFlags |= TOF_DO_NOT_QUERY_PRIMARY_KEYS;
		}

		// Nest try/catch the free the buffers created in here if we fail somewhere
		try
		{

			std::wstring sqlStmt;
			std::wstring s;
			s.empty();

			// If we do not already have a STableInfo for our table, we absolutely must find one
			bool searchedTable = false;
			if (!m_haveTableInfo)
			{
				// Finding will throw if not exactly one is found
				m_tableInfo = db.FindOneTable(m_initialTableName, m_initialSchemaName, m_initialCatalogName, m_initialTypeName);
				m_haveTableInfo = true;
				searchedTable = true;
			}

			// If we are asked to check existence and have not just proved we exist just find a table
			// Search using the info from the now available m_tableInfo
			if (((openFlags & TOF_CHECK_EXISTANCE) == TOF_CHECK_EXISTANCE) && !searchedTable)
			{
				// Will throw if not one is found
				std::wstring catalogName = m_tableInfo.m_catalogName;
				std::wstring typeName = m_tableInfo.m_tableType;
				if (db.GetDbms() == DatabaseProduct::EXCEL)
				{
					// workaround for #111
					catalogName = L"";
					typeName = L"";
				}
				db.FindOneTable(m_tableInfo.m_tableName, m_tableInfo.m_schemaName, catalogName, typeName);
				searchedTable = true;
			}

			// Set the hints for the Table-Query name depending on the Database
			// If this is an excel-db, we must search for a table named 'foo$', but query using '[foo$]'. See Ticket #111. Set the corresponding hint on the tableInfo-object
			// maybe excel also has the full path
			if (db.GetDbms() == DatabaseProduct::EXCEL)
			{
				m_tableInfo.SetSqlNameHint(TableQueryNameHint::EXCEL);
			}
			if (db.GetDbms() == DatabaseProduct::ACCESS)
			{
				m_tableInfo.SetSqlNameHint(TableQueryNameHint::TABLE_ONLY);
			}


			// If we are asked to create our columns automatically, read the column information and create the buffers
			if (!m_manualColumns)
			{
				CreateAutoColumnBuffers(db, m_tableInfo, (openFlags & TOF_SKIP_UNSUPPORTED_COLUMNS) == TOF_SKIP_UNSUPPORTED_COLUMNS);
			}
			else
			{
				// Check that the manually defined columns do not violate our access-flags
				for (ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin(); it != m_columnBuffers.end(); ++it)
				{
					ColumnBuffer* pBuffer = it->second;
					if (pBuffer->IsColumnFlagSet(CF_SELECT) && !TestAccessFlag(AF_SELECT))
					{
						Exception ex(boost::str(boost::wformat(L"Defined Column %s (%d) has ColumnFlag CF_SELECT set, but the AccessFlag AF_SELECT is not set on the table %s") % pBuffer->GetQueryName() % it->first %m_tableInfo.GetSqlName()));
						SET_EXCEPTION_SOURCE(ex);
						throw ex;
					}
					if (pBuffer->IsColumnFlagSet(CF_INSERT) && !TestAccessFlag(AF_INSERT))
					{
						Exception ex(boost::str(boost::wformat(L"Defined Column %s (%d) has ColumnFlag CF_INSERT set, but the AccessFlag AF_INSERT is not set on the table %s") % pBuffer->GetQueryName() % it->first %m_tableInfo.GetSqlName()));
						SET_EXCEPTION_SOURCE(ex);
						throw ex;
					}
					if (pBuffer->IsColumnFlagSet(CF_UPDATE) && !TestAccessFlag(AF_UPDATE))
					{
						Exception ex(boost::str(boost::wformat(L"Defined Column %s (%d) has ColumnFlag CF_UPDATE set, but the AccessFlag AF_UPDATE is not set on the table %s") % pBuffer->GetQueryName() % it->first %m_tableInfo.GetSqlName()));
						SET_EXCEPTION_SOURCE(ex);
						throw ex;
					}
				}
			}

			// Prepare the FieldStatement to be used for selects
			m_fieldsStatement = BuildFieldsStatement();

			// Optionally check privileges
			if ((openFlags & TOF_CHECK_PRIVILEGES) == TOF_CHECK_PRIVILEGES)
			{
				m_tablePrivileges.Initialize(db, m_tableInfo);
				// We always need to be able to select, but the rest only if we want to write
				if ((TestAccessFlag(AF_SELECT) && !m_tablePrivileges.IsSet(TP_SELECT))
					|| (TestAccessFlag(AF_UPDATE_PK) && !m_tablePrivileges.IsSet(TP_UPDATE))
					|| (TestAccessFlag(AF_UPDATE_WHERE) && !m_tablePrivileges.IsSet(TP_UPDATE))
					|| (TestAccessFlag(AF_INSERT) && !m_tablePrivileges.IsSet(TP_INSERT))
					|| (TestAccessFlag(AF_DELETE_PK) && !m_tablePrivileges.IsSet(TP_DELETE))
					|| (TestAccessFlag(AF_DELETE_WHERE) && !m_tablePrivileges.IsSet(TP_DELETE))
					)
				{
					Exception ex((boost::wformat(L"Not sufficient Privileges to Open Table '%s'") % m_tableInfo.GetSqlName()).str());
					SET_EXCEPTION_SOURCE(ex);
					throw ex;
				}
			}

			// Maybe the primary keys have been set manually?
			if (m_primaryKeyColumnIndexes.size() > 0)
			{
				for (std::set<SQLUSMALLINT>::const_iterator it = m_primaryKeyColumnIndexes.begin(); it != m_primaryKeyColumnIndexes.end(); ++it)
				{
					// Get corresponding Buffer and set the flag CF_PRIMARY_KEY
					ColumnBufferPtrMap::const_iterator itBuffs = m_columnBuffers.find(*it);
					if (itBuffs == m_columnBuffers.end())
					{
						Exception ex(boost::str(boost::wformat(L"No ColumnBuffer was found for a manually defined primary key column index (index = %d)") % *it));
						SET_EXCEPTION_SOURCE(ex);
						throw ex;
					}
					itBuffs->second->SetColumnFlag(CF_PRIMARY_KEY);
				}
				// And implicitly activate the flag TOF_DO_NOT_QUERY_PRIMARY_KEYS
				openFlags |= TOF_DO_NOT_QUERY_PRIMARY_KEYS;
			}

			// Bind the member variables for field exchange between
			// the Table object and the ODBC record for Select()
			if (TestAccessFlag(AF_SELECT))
			{
				SQLSMALLINT boundColumnNumber = 1;
				for (ColumnBufferPtrMap::iterator it = m_columnBuffers.begin(); it != m_columnBuffers.end(); it++)
				{
					// Only bind those marked for select - no danger if primary keys are not bound, it is checked later
					// if the table is opened for writing that all pks are bound
					ColumnBuffer* pBuffer = it->second;
					if (pBuffer->IsColumnFlagSet(CF_SELECT))
					{
						SQLUSMALLINT colIndex = it->first;
						pBuffer->Bind(m_hStmtSelect, boundColumnNumber);
						++boundColumnNumber;
					}
				}
			}

			// Create additional UPDATE and DELETE statement-handles to be used with the pk-columns
			// and bind the params. PKs are required.
			if (TestAccessFlag(AF_UPDATE_PK) || TestAccessFlag(AF_DELETE_PK))
			{
				// We need the primary keys
				// Do not query them from the db is corresponding flag is set
				if (openFlags & TOF_DO_NOT_QUERY_PRIMARY_KEYS)
				{
					m_tablePrimaryKeys.Initialize(m_tableInfo, m_columnBuffers);
				}
				else
				{
					m_tablePrimaryKeys.Initialize(db, m_tableInfo);
				}

				// And we need to have a primary key
				if ( (TestAccessFlag(AF_UPDATE_PK) || TestAccessFlag(AF_DELETE_PK)) && m_tablePrimaryKeys.GetPrimaryKeysCount() == 0)
				{
					Exception ex((boost::wformat(L"Table '%s' has no primary keys") % m_tableInfo.GetSqlName()).str());
					SET_EXCEPTION_SOURCE(ex);
					throw ex;
				}

				// And a Buffer for every primary key
				if (!m_tablePrimaryKeys.AreAllPrimaryKeysInMap(m_columnBuffers))
				{
					Exception ex((boost::wformat(L"Not all primary Keys of table '%s' have a corresponding ColumnBuffer") % m_tableInfo.GetSqlName()).str());
					SET_EXCEPTION_SOURCE(ex);
					throw ex;
				}

				// Test that all primary keys are bound
				if (!m_tablePrimaryKeys.AreAllPrimaryKeysBound(m_columnBuffers))
				{
					Exception ex((boost::wformat(L"Not all primary Keys of table '%s' are bound") % m_tableInfo.GetSqlName()).str());
					SET_EXCEPTION_SOURCE(ex);
					throw ex;
				}

				// Set the primary key flags on the bound Columns
				// but only if corresponding flag is not set - else we have initialized the primary keys from the ColumnBuffers - makes no sense
				if (!(openFlags & TOF_DO_NOT_QUERY_PRIMARY_KEYS))
				{
					m_tablePrimaryKeys.SetPrimaryKeyFlags(m_columnBuffers);
				}

				if (TestAccessFlag(AF_DELETE_PK))
				{
					BindDeleteParameters();
				}
				if (TestAccessFlag(AF_UPDATE_PK))
				{
					BindUpdateParameters();
				}
			}

			// Bind INSERT params
			if (TestAccessFlag(AF_INSERT))
			{
				BindInsertParameters();
			}

			// Completed successfully
			m_openFlags = openFlags;
			m_isOpen = true;
		}
		catch (const Exception& ex)
		{
			// delete the ColumnBuffers if we have allocated them during this process (if not manual)
			HIDE_UNUSED(ex);
			if (!m_manualColumns)
			{
				for (ColumnBufferPtrMap::const_iterator it = m_columnBuffers.begin(); it != m_columnBuffers.end(); ++it)
				{
					ColumnBuffer* pBuffer = it->second;
					delete pBuffer;
				}
				m_columnBuffers.clear();
				m_numCols = 0;
			}
			// and rethrow
			throw;
		}
	}


	void Table::Close()
	{
		exASSERT(IsOpen());

		// Unbind ColumnBuffers
		SQLRETURN ret = SQL_SUCCESS;
		if (TestAccessFlag(AF_SELECT))
		{
			ret = SQLFreeStmt(m_hStmtSelect, SQL_UNBIND);
			THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtSelect);
		}

		// And column parameters, if we were bound rw
		if (TestAccessFlag(AF_INSERT))
		{
			ret = SQLFreeStmt(m_hStmtInsert, SQL_RESET_PARAMS);
			THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtInsert);
		}
		if (TestAccessFlag(AF_DELETE))
		{
			ret = SQLFreeStmt(m_hStmtDelete, SQL_RESET_PARAMS);
			THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtDelete);

			ret = SQLFreeStmt(m_hStmtDeleteWhere, SQL_RESET_PARAMS);
			THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtDeleteWhere);
		}
		if (TestAccessFlag(AF_UPDATE))
		{
			ret = SQLFreeStmt(m_hStmtUpdate, SQL_RESET_PARAMS);
			THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtUpdate);

			ret = SQLFreeStmt(m_hStmtUpdateWhere, SQL_RESET_PARAMS);
			THROW_IFN_SUCCEEDED(SQLFreeStmt, ret, SQL_HANDLE_STMT, m_hStmtUpdateWhere);
		}

		// Delete ColumnBuffers if they were created automatically
		if (!m_manualColumns)
		{
			ColumnBufferPtrMap::iterator it;
			for (it = m_columnBuffers.begin(); it != m_columnBuffers.end(); it++)
			{
				ColumnBuffer* pBuffer = it->second;
				delete pBuffer;
			}
			m_columnBuffers.clear();
			m_numCols = 0;
		}

		m_isOpen = false;
	}


	void Table::SetAccessFlag(const Database& db, AccessFlag ac)
	{
		exASSERT(!IsOpen());

		if (TestAccessFlag(ac))
		{
			// already set, do nothing
			return;
		}

		AccessFlags newFlags = ( m_accessFlags | ac );
		SetAccessFlags(db, newFlags);
	}


	void Table::ClearAccessFlag(const Database& db, AccessFlag ac)
	{
		exASSERT(!IsOpen());

		if (!TestAccessFlag(ac))
		{
			// Not set anyway, do nothing
			return;
		}

		AccessFlags newFlags = ( m_accessFlags & ~ac );
		SetAccessFlags(db, newFlags);
	}


	void Table::SetAccessFlags(const Database& db, AccessFlags acs)
	{
		exASSERT(!IsOpen());
		exASSERT(db.IsOpen());

		if (m_accessFlags == acs)
		{
			// Same flags, return
			return;
		}

		// Free statements, then re-allocate all
		FreeStatements();
		m_accessFlags = acs;
		AllocateStatements(db);
	}


	void Table::SetCharTrimLeft(bool trimLeft) throw()
	{
		if (trimLeft)
		{
			m_openFlags |= TOF_CHAR_TRIM_LEFT;
		}
		else
		{
			m_openFlags &= ~TOF_CHAR_TRIM_LEFT;
		}
	}


	void Table::SetCharTrimRight(bool trimRight) throw()
	{
		if (trimRight)
		{
			m_openFlags |= TOF_CHAR_TRIM_RIGHT;
		}
		else
		{
			m_openFlags &= ~TOF_CHAR_TRIM_RIGHT;
		}
	}


	/*!
	* \brief	Checks if we can only read from this table.
	* \return	True if this table has the flag AF_READ set and none of the flags
	*			AF_UPDATE_PK, AF_UPDATE_WHERE, AF_INSERT, AF_DELETE_PK or AF_DELETE_WHERE are set.
	*/
	bool Table::IsQueryOnly() const throw()  {
		return TestAccessFlag(AF_READ) &&
				! ( TestAccessFlag(AF_UPDATE_PK) || TestAccessFlag(AF_UPDATE_WHERE)
					|| TestAccessFlag(AF_INSERT)
					|| TestAccessFlag(AF_DELETE_PK) || TestAccessFlag(AF_DELETE_PK));
	}


	void Table::SetAutoBindingMode(AutoBindingMode mode)
	{
		exASSERT(!IsOpen());
		m_autoBindingMode = mode;
	}

	// OLD STUF DOWN HERE
	// ==================


	//void Table::setCbValueForColumn(int columnIndex)
	//{
	//	switch (m_colDefs[columnIndex].m_dbDataType)
	//	{
	//	case DB_DATA_TYPE_VARCHAR:
	//	case DB_DATA_TYPE_MEMO:
	//		if (m_colDefs[columnIndex].m_null)
	//			m_colDefs[columnIndex].m_cbValue = SQL_NULL_DATA;
	//		else
	//			m_colDefs[columnIndex].m_cbValue = SQL_NTS;
	//		break;
	//	case DB_DATA_TYPE_INTEGER:
	//		if (m_colDefs[columnIndex].m_null)
	//			m_colDefs[columnIndex].m_cbValue = SQL_NULL_DATA;
	//		else
	//			m_colDefs[columnIndex].m_cbValue = 0;
	//		break;
	//	case DB_DATA_TYPE_FLOAT:
	//		if (m_colDefs[columnIndex].m_null)
	//			m_colDefs[columnIndex].m_cbValue = SQL_NULL_DATA;
	//		else
	//			m_colDefs[columnIndex].m_cbValue = 0;
	//		break;
	//	case DB_DATA_TYPE_DATE:
	//		if (m_colDefs[columnIndex].m_null)
	//			m_colDefs[columnIndex].m_cbValue = SQL_NULL_DATA;
	//		else
	//			m_colDefs[columnIndex].m_cbValue = 0;
	//		break;
	//	case DB_DATA_TYPE_BLOB:
	//		if (m_colDefs[columnIndex].m_null)
	//			m_colDefs[columnIndex].m_cbValue = SQL_NULL_DATA;
	//		else
	//			if (m_colDefs[columnIndex].m_sqlCtype == SQL_C_WXCHAR)
	//				m_colDefs[columnIndex].m_cbValue = SQL_NTS;
	//			else
	//				m_colDefs[columnIndex].m_cbValue = SQL_LEN_DATA_AT_EXEC(m_colDefs[columnIndex].m_szDataObj);
	//		break;
	//	}
	//}

	/********** wxDbTable::bindParams() **********/
	//bool Table::bindParams(bool forUpdate)
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return false;
	//
	//    SWORD   fSqlType    = 0;
	//    SDWORD  precision   = 0;
	//    SWORD   scale       = 0;
	//
	//    // Bind each column of the table that should be bound
	//    // to a parameter marker
	//    int i;
	//    UWORD colNumber;
	//
	//    for (i=0, colNumber=1; i < m_numCols; i++)
	//    {
	//        if (forUpdate)
	//        {
	//            if (!m_colDefs[i].m_updateable)
	//                continue;
	//        }
	//        else
	//        {
	//            if (!m_colDefs[i].m_insertAllowed)
	//                continue;
	//        }
	//
	//        switch(m_colDefs[i].m_dbDataType)
	//        {
	//            case DB_DATA_TYPE_VARCHAR:
	//                fSqlType = m_pDb->GetTypeInfVarchar().FsqlType;
	//                precision = m_colDefs[i].m_szDataObj;
	//                scale = 0;
	//                break;
	//            case DB_DATA_TYPE_MEMO:
	//                fSqlType = m_pDb->GetTypeInfMemo().FsqlType;
	//                precision = m_colDefs[i].m_szDataObj;
	//                scale = 0;
	//                break;
	//            case DB_DATA_TYPE_INTEGER:
	//                fSqlType = m_pDb->GetTypeInfInteger().FsqlType;
	//                precision = m_pDb->GetTypeInfInteger().Precision;
	//                scale = 0;
	//                break;
	//            case DB_DATA_TYPE_FLOAT:
	//                fSqlType = m_pDb->GetTypeInfFloat().FsqlType;
	//                precision = m_pDb->GetTypeInfFloat().Precision;
	//                scale = m_pDb->GetTypeInfFloat().MaximumScale;
	//                // SQL Sybase Anywhere v5.5 returned a negative number for the
	//                // MaxScale.  This caused ODBC to kick out an error on ibscale.
	//                // I check for this here and set the scale = precision.
	//                //if (scale < 0)
	//                // scale = (short) precision;
	//                break;
	//            case DB_DATA_TYPE_DATE:
	//                fSqlType = m_pDb->GetTypeInfDate().FsqlType;
	//                precision = m_pDb->GetTypeInfDate().Precision;
	//                scale = 0;
	//                break;
	//            case DB_DATA_TYPE_BLOB:
	//                fSqlType = m_pDb->GetTypeInfBlob().FsqlType;
	//                precision = m_colDefs[i].m_szDataObj;
	//                scale = 0;
	//                break;
	//        }
	//
	//        setCbValueForColumn(i);
	//
	//        if (forUpdate)
	//        {
	//            if (SQLBindParameter(m_hstmtUpdate, colNumber++, SQL_PARAM_INPUT, m_colDefs[i].m_sqlCtype,
	//                                 fSqlType, precision, scale, (UCHAR*) m_colDefs[i].m_ptrDataObj,
	//                                 precision+1, &m_colDefs[i].m_cbValue) != SQL_SUCCESS)
	//            {
	//                return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtUpdate));
	//            }
	//        }
	//        else
	//        {
	//            if (SQLBindParameter(m_hstmtInsert, colNumber++, SQL_PARAM_INPUT, m_colDefs[i].m_sqlCtype,
	//                                 fSqlType, precision, scale, (UCHAR*) m_colDefs[i].m_ptrDataObj,
	//                                 precision+1, &m_colDefs[i].m_cbValue) != SQL_SUCCESS)
	//            {
	//                return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtInsert));
	//            }
	//        }
	//    }
	//
	//    // Completed successfully
	//    return true;
	//
	//}  // wxDbTable::bindParams()


	/********** wxDbTable::bindInsertParams() **********/
	//bool Table::bindInsertParams()
	//{
	//    return bindParams(false);
	//}  // wxDbTable::bindInsertParams()
	//
	//
	///********** wxDbTable::bindUpdateParams() **********/
	//bool Table::bindUpdateParams()
	//{
	//    return bindParams(true);
	//}  // wxDbTable::bindUpdateParams()


	///********** wxDbTable::bindCols() **********/
	//bool Table::bindCols(SQLHSTMT cursor)
	//{
	//	// Bind each column of the table to a memory address for fetching data
	//	UWORD i;
	//	for (i = 0; i < m_numCols; i++)
	//	{
	//		ColumnDefinition def = m_colDefs[i];
	//		if (SQLBindCol(cursor, (UWORD)(i + 1), m_colDefs[i].m_sqlCtype, (UCHAR*)m_colDefs[i].m_ptrDataObj,
	//			m_colDefs[i].m_szDataObj, &m_colDefs[i].m_cbValue) != SQL_SUCCESS)
	//			return (m_pDb->DispAllErrors(NULL, m_hdbc, cursor));
	//	}

	//	// Completed successfully
	//	return true;
	//}  // wxDbTable::bindCols()


	///********** wxDbTable::getRec() **********/
	//bool Table::getRec(UWORD fetchType)
	//{
	//	RETCODE retcode;

	//	if (!m_pDb->FwdOnlyCursors())
	//	{
	//		// Fetch the NEXT, PREV, FIRST or LAST record, depending on fetchType
	//		SQLULEN cRowsFetched;
	//		UWORD   rowStatus;

	//		retcode = SQLExtendedFetch(m_hstmt, fetchType, 0, &cRowsFetched, &rowStatus);
	//		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
	//		{
	//			if (retcode == SQL_NO_DATA_FOUND)
	//				return false;
	//			else
	//				return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmt));
	//		}
	//		else
	//		{
	//			// Set the Null member variable to indicate the Null state
	//			// of each column just read in.
	//			size_t i;
	//			for (i = 0; i < m_numCols; i++)
	//				m_colDefs[i].m_null = (m_colDefs[i].m_cbValue == SQL_NULL_DATA);
	//		}
	//	}
	//	else
	//	{
	//		// Fetch the next record from the record set
	//		retcode = SQLFetch(m_hstmt);
	//		if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
	//		{
	//			if (retcode == SQL_NO_DATA_FOUND)
	//				return false;
	//			else
	//				return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmt));
	//		}
	//		else
	//		{
	//			// Set the Null member variable to indicate the Null state
	//			// of each column just read in.
	//			size_t i;
	//			for (i = 0; i < m_numCols; i++)
	//				m_colDefs[i].m_null = (m_colDefs[i].m_cbValue == SQL_NULL_DATA);
	//		}
	//	}

	//	// Completed successfully
	//	return true;

	//}  // wxDbTable::getRec()


	///********** wxDbTable::execDelete() **********/
	//bool Table::execDelete(const std::wstring &pSqlStmt)
	//{
	//	RETCODE retcode;

	//	// Execute the DELETE statement
	//	retcode = SQLExecDirect(m_hstmtDelete, (SQLWCHAR*) pSqlStmt.c_str(), SQL_NTS);

	//	if (retcode == SQL_SUCCESS ||
	//		retcode == SQL_NO_DATA_FOUND ||
	//		retcode == SQL_SUCCESS_WITH_INFO)
	//	{
	//		// Record deleted successfully
	//		return true;
	//	}

	//	// Problem deleting record
	//	return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtDelete));

	//}  // wxDbTable::execDelete()


	///********** wxDbTable::execUpdate() **********/
	//bool Table::execUpdate(const std::wstring &pSqlStmt)
	//{
	//    RETCODE retcode;
	//
	//    // Execute the UPDATE statement
	//    retcode = SQLExecDirect(m_hstmtUpdate, (SQLTCHAR FAR *) pSqlStmt.c_str(), SQL_NTS);
	//
	//    if (retcode == SQL_SUCCESS ||
	//        retcode == SQL_NO_DATA_FOUND ||
	//        retcode == SQL_SUCCESS_WITH_INFO)
	//    {
	//        // Record updated successfully
	//        return true;
	//    }
	//    else if (retcode == SQL_NEED_DATA)
	//    {
	//        PTR pParmID;
	//        retcode = SQLParamData(m_hstmtUpdate, &pParmID);
	//        while (retcode == SQL_NEED_DATA)
	//        {
	//            // Find the parameter
	//            int i;
	//            for (i=0; i < m_numCols; i++)
	//            {
	//                if (m_colDefs[i].m_ptrDataObj == pParmID)
	//                {
	//                    // We found it.  Store the parameter.
	//                    retcode = SQLPutData(m_hstmtUpdate, pParmID, m_colDefs[i].m_szDataObj);
	//                    if (retcode != SQL_SUCCESS)
	//                    {
	//                        m_pDb->DispNextError();
	//                        return m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtUpdate);
	//                    }
	//                    break;
	//                }
	//            }
	//            retcode = SQLParamData(m_hstmtUpdate, &pParmID);
	//        }
	//        if (retcode == SQL_SUCCESS ||
	//            retcode == SQL_NO_DATA_FOUND ||
	//            retcode == SQL_SUCCESS_WITH_INFO)
	//        {
	//            // Record updated successfully
	//            return true;
	//        }
	//    }
	//
	//    // Problem updating record
	//    return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtUpdate));
	//
	//}  // wxDbTable::execUpdate()


	//// ********** wxDbTable::query() **********/
	//bool Table::query(int queryType, bool forUpdate, bool distinct, const std::wstring &pSqlStmt)
	//{
	//	std::wstring sqlStmt;

	//	if (forUpdate)
	//		// The user may wish to select for update, but the DBMS may not be capable
	//		m_selectForUpdate = CanSelectForUpdate();
	//	else
	//		m_selectForUpdate = false;

	//	// Set the SQL SELECT string
	//	if (queryType != DB_SELECT_STATEMENT)               // A select statement was not passed in,
	//	{                                                   // so generate a select statement.
	//		exASSERT(false);
	//		//        BuildSelectStmt(sqlStmt, queryType, distinct);
	//		//        m_pDb->WriteSqlLog(sqlStmt);
	//	}

	//	// Make sure the cursor is closed first
	//	if (!CloseCursor(m_hstmt))
	//		return false;

	//	// Execute the SQL SELECT statement
	//	int retcode;
	//	retcode = SQLExecDirect(m_hstmt, (SQLTCHAR FAR *) (queryType == DB_SELECT_STATEMENT ? pSqlStmt.c_str() : sqlStmt.c_str()), SQL_NTS);
	//	if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
	//		return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmt));

	//	// Completed successfully
	//	return true;

	//}  // wxDbTable::query()


	///********** wxDbColDef::wxDbColDef() Constructor **********/
	//ColumnDefinition::ColumnDefinition()
	//{
	//	Initialize();
	//}  // Constructor


	//bool ColumnDefinition::Initialize()
	//{
	//	m_colName[0] = 0;
	//	m_dbDataType = DB_DATA_TYPE_INTEGER;
	//	m_sqlCtype = SQL_C_LONG;
	//	m_ptrDataObj = NULL;
	//	m_szDataObj = 0;
	//	m_keyField = false;
	//	m_updateable = false;
	//	m_insertAllowed = false;
	//	m_derivedCol = false;
	//	m_cbValue = 0;
	//	m_null = false;

	//	return true;
	//}  // wxDbColDef::Initialize()


	///********** wxDbTable::Query() **********/
	//bool Table::Query(bool forUpdate, bool distinct)
	//{
	//
	//    return(query(DB_SELECT_WHERE, forUpdate, distinct));
	//
	//}  // wxDbTable::Query()


	///********** wxDbTable::QueryBySqlStmt() **********/
	//bool Table::QueryBySqlStmt(const std::wstring &pSqlStmt)
	//{
	//	//    m_pDb->WriteSqlLog(pSqlStmt);

	//	return(query(DB_SELECT_STATEMENT, false, false, pSqlStmt));

	//}  // wxDbTable::QueryBySqlStmt()


	///********** wxDbTable::QueryMatching() **********/
	//bool Table::QueryMatching(bool forUpdate, bool distinct)
	//{

	//	return(query(DB_SELECT_MATCHING, forUpdate, distinct));

	//}  // wxDbTable::QueryMatching()


	///********** wxDbTable::QueryOnKeyFields() **********/
	//bool Table::QueryOnKeyFields(bool forUpdate, bool distinct)
	//{

	//	return(query(DB_SELECT_KEYFIELDS, forUpdate, distinct));

	//}  // wxDbTable::QueryOnKeyFields()


	///********** wxDbTable::GetPrev() **********/
	//bool Table::GetPrev()
	//{
	//	if (m_pDb->FwdOnlyCursors())
	//	{
	//		exFAIL_MSG(L"GetPrev()::Backward scrolling cursors are not enabled for this instance of wxDbTable");
	//		return false;
	//	}
	//	else
	//		return(getRec(SQL_FETCH_PRIOR));

	//}  // wxDbTable::GetPrev()


	///********** wxDbTable::operator-- **********/
	//bool Table::operator--(int)
	//{
	//	if (m_pDb->FwdOnlyCursors())
	//	{
	//		exFAIL_MSG(L"operator--:Backward scrolling cursors are not enabled for this instance of wxDbTable");
	//		return false;
	//	}
	//	else
	//		return(getRec(SQL_FETCH_PRIOR));

	//}  // wxDbTable::operator--


	///********** wxDbTable::GetFirst() **********/
	//bool Table::GetFirst()
	//{
	//	if (m_pDb->FwdOnlyCursors())
	//	{
	//		exFAIL_MSG(L"GetFirst():Backward scrolling cursors are not enabled for this instance of wxDbTable");
	//		return false;
	//	}
	//	else
	//		return(getRec(SQL_FETCH_FIRST));

	//}  // wxDbTable::GetFirst()


	///********** wxDbTable::GetLast() **********/
	//bool Table::GetLast()
	//{
	//	if (m_pDb->FwdOnlyCursors())
	//	{
	//		exFAIL_MSG(L"GetLast()::Backward scrolling cursors are not enabled for this instance of wxDbTable");
	//		return false;
	//	}
	//	else
	//		return(getRec(SQL_FETCH_LAST));

	//}  // wxDbTable::GetLast()


	///********** wxDbTable::BuildDeleteStmt() **********/
	//void Table::BuildDeleteStmt(std::wstring &pSqlStmt, int typeOfDel, const std::wstring &pWhereClause)
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return;
	//
	//    std::wstring whereClause;
	//
	//    whereClause.empty();
	//
	//    // Handle the case of DeleteWhere() and the where clause is blank.  It should
	//    // delete all records from the database in this case.
	//    if (typeOfDel == DB_DEL_WHERE && (pWhereClause.length() == 0))
	//    {
	//		pSqlStmt = (boost::wformat(L"DELETE FROM %s") %m_pDb->SQLTableName(m_tableName.c_str())).str();
	//        return;
	//    }
	//
	//	pSqlStmt = (boost::wformat(L"DELETE FROM %s WHERE ") % m_pDb->SQLTableName(m_tableName.c_str())).str();
	//
	//    // Append the WHERE clause to the SQL DELETE statement
	//    switch(typeOfDel)
	//    {
	//        case DB_DEL_KEYFIELDS:
	//            // If the datasource supports the ROWID column, build
	//            // the where on ROWID for efficiency purposes.
	//            // e.g. DELETE FROM PARTS WHERE ROWID = '111.222.333'
	//            if (CanUpdateByROWID())
	//            {
	//                SQLLEN cb;
	//                wchar_t   rowid[wxDB_ROWID_LEN+1];
	//
	//                // Get the ROWID value.  If not successful retreiving the ROWID,
	//                // simply fall down through the code and build the WHERE clause
	//                // based on the key fields.
	//                if (SQLGetData(m_hstmt, (UWORD)(m_numCols+1), SQL_C_WXCHAR, (UCHAR*) rowid, sizeof(rowid), &cb) == SQL_SUCCESS)
	//                {
	//                    pSqlStmt += L"ROWID = '";
	//                    pSqlStmt += rowid;
	//                    pSqlStmt += L"'";
	//                    break;
	//                }
	//            }
	//            // Unable to delete by ROWID, so build a WHERE
	//            // clause based on the keyfields.
	//            BuildWhereClause(whereClause, DB_WHERE_KEYFIELDS);
	//            pSqlStmt += whereClause;
	//            break;
	//        case DB_DEL_WHERE:
	//            pSqlStmt += pWhereClause;
	//            break;
	//        case DB_DEL_MATCHING:
	//            BuildWhereClause(whereClause, DB_WHERE_MATCHING);
	//            pSqlStmt += whereClause;
	//            break;
	//    }
	//
	//}  // BuildDeleteStmt()


	///***** DEPRECATED: use wxDbTable::BuildDeleteStmt(std::wstring &....) form *****/
	//void Table::BuildDeleteStmt(wchar_t *pSqlStmt, int typeOfDel, const std::wstring &pWhereClause)
	//{
	//    std::wstring tempSqlStmt;
	//    BuildDeleteStmt(tempSqlStmt, typeOfDel, pWhereClause);
	//    wcscpy(pSqlStmt, tempSqlStmt.c_str());
	//}  // wxDbTable::BuildDeleteStmt()


	///********** wxDbTable::BuildSelectStmt() **********/
	//void Table::BuildSelectStmt(std::wstring &pSqlStmt, int typeOfSelect, bool distinct)
	//{
	//    std::wstring whereClause;
	//    whereClause.empty();
	//
	//    // Build a select statement to query the database
	//    pSqlStmt = L"SELECT ";
	//
	//    // SELECT DISTINCT values only?
	//    if (distinct)
	//        pSqlStmt += L"DISTINCT ";
	//
	//    // Was a FROM clause specified to join tables to the base table?
	//    // Available for ::Query() only!!!
	//    bool appendFromClause = false;
	//
	//    if (typeOfSelect == DB_SELECT_WHERE && m_from.length())
	//        appendFromClause = true;
	//
	//    // Add the column list
	//    int i;
	//    std::wstring tStr;
	//    for (i = 0; i < m_numCols; i++)
	//    {
	//        tStr = m_colDefs[i].m_colName;
	//        // If joining tables, the base table column names must be qualified to avoid ambiguity
	//        if ((appendFromClause || m_pDb->GetDbms() == dbmsACCESS) && tStr.find(L'.') == std::wstring::npos)
	//        {
	//            pSqlStmt += m_pDb->SQLTableName(m_queryTableName.c_str());
	//            pSqlStmt += L".";
	//        }
	//        pSqlStmt += m_pDb->SQLColumnName(m_colDefs[i].m_colName);
	//        if (i + 1 < m_numCols)
	//            pSqlStmt += L",";
	//    }
	//
	//    // If the datasource supports ROWID, get this column as well.  Exception: Don't retrieve
	//    // the ROWID if querying distinct records.  The rowid will always be unique.
	//    if (!distinct && CanUpdateByROWID())
	//    {
	//        // If joining tables, the base table column names must be qualified to avoid ambiguity
	//        if (appendFromClause || m_pDb->GetDbms() == dbmsACCESS)
	//        {
	//            pSqlStmt += L",";
	//            pSqlStmt += m_pDb->SQLTableName(m_queryTableName.c_str());
	//            pSqlStmt += L".ROWID";
	//        }
	//        else
	//            pSqlStmt += L",ROWID";
	//    }
	//
	//    // Append the FROM tablename portion
	//    pSqlStmt += L" FROM ";
	//    pSqlStmt += m_pDb->SQLTableName(m_queryTableName.c_str());
	////    pSqlStmt += queryTableName;
	//
	//    // Sybase uses the HOLDLOCK keyword to lock a record during query.
	//    // The HOLDLOCK keyword follows the table name in the from clause.
	//    // Each table in the from clause must specify HOLDLOCK or
	//    // NOHOLDLOCK (the default).  Note: The "FOR UPDATE" clause
	//    // is parsed but ignored in SYBASE Transact-SQL.
	//    if (m_selectForUpdate && (m_pDb->GetDbms() == dbmsSYBASE_ASA || m_pDb->GetDbms() == dbmsSYBASE_ASE))
	//        pSqlStmt += L" HOLDLOCK";
	//
	//    if (appendFromClause)
	//        pSqlStmt += m_from;
	//
	//    // Append the WHERE clause.  Either append the where clause for the class
	//    // or build a where clause.  The typeOfSelect determines this.
	//    switch(typeOfSelect)
	//    {
	//        case DB_SELECT_WHERE:
	//
	//            if (m_where.length())   // May not want a where clause!!!
	//            {
	//                pSqlStmt += L" WHERE ";
	//                pSqlStmt += m_where;
	//            }
	//            break;
	//        case DB_SELECT_KEYFIELDS:
	//            BuildWhereClause(whereClause, DB_WHERE_KEYFIELDS);
	//            if (whereClause.length())
	//            {
	//                pSqlStmt += L" WHERE ";
	//                pSqlStmt += whereClause;
	//            }
	//            break;
	//        case DB_SELECT_MATCHING:
	//            BuildWhereClause(whereClause, DB_WHERE_MATCHING);
	//            if (whereClause.length())
	//            {
	//                pSqlStmt += L" WHERE ";
	//                pSqlStmt += whereClause;
	//            }
	//            break;
	//    }
	//
	//    // Append the ORDER BY clause
	//    if (m_orderBy.length())
	//    {
	//        pSqlStmt += L" ORDER BY ";
	//        pSqlStmt += m_orderBy;
	//    }
	//
	//    // SELECT FOR UPDATE if told to do so and the datasource is capable.  Sybase
	//    // parses the FOR UPDATE clause but ignores it.  See the comment above on the
	//    // HOLDLOCK for Sybase.
	//    if (m_selectForUpdate && CanSelectForUpdate())
	//        pSqlStmt += L" FOR UPDATE";
	//
	//}  // wxDbTable::BuildSelectStmt()


	///***** DEPRECATED: use wxDbTable::BuildSelectStmt(std::wstring &....) form *****/
	//void Table::BuildSelectStmt(wchar_t *pSqlStmt, int typeOfSelect, bool distinct)
	//{
	//    std::wstring tempSqlStmt;
	//    BuildSelectStmt(tempSqlStmt, typeOfSelect, distinct);
	//    wcscpy(pSqlStmt, tempSqlStmt.c_str());
	//}  // wxDbTable::BuildSelectStmt()


	///********** wxDbTable::BuildUpdateStmt() **********/
	//void Table::BuildUpdateStmt(std::wstring &pSqlStmt, int typeOfUpdate, const std::wstring &pWhereClause)
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return;
	//
	//    std::wstring whereClause;
	//    whereClause.empty();
	//
	//    bool firstColumn = true;
	//
	//	pSqlStmt = (boost::wformat(L"UPDATE %s SET ") % m_pDb->SQLTableName(m_tableName.c_str())).str();
	//
	//    // Append a list of columns to be updated
	//    int i;
	//    for (i = 0; i < m_numCols; i++)
	//    {
	//        // Only append Updateable columns
	//        if (m_colDefs[i].m_updateable)
	//        {
	//            if (!firstColumn)
	//                pSqlStmt += L",";
	//            else
	//                firstColumn = false;
	//
	//            pSqlStmt += m_pDb->SQLColumnName(m_colDefs[i].m_colName);
	////            pSqlStmt += colDefs[i].ColName;
	//            pSqlStmt += L" = ?";
	//        }
	//    }
	//
	//    // Append the WHERE clause to the SQL UPDATE statement
	//    pSqlStmt += L" WHERE ";
	//    switch(typeOfUpdate)
	//    {
	//        case DB_UPD_KEYFIELDS:
	//            // If the datasource supports the ROWID column, build
	//            // the where on ROWID for efficiency purposes.
	//            // e.g. UPDATE PARTS SET Col1 = ?, Col2 = ? WHERE ROWID = '111.222.333'
	//            if (CanUpdateByROWID())
	//            {
	//                SQLLEN cb;
	//                wchar_t rowid[wxDB_ROWID_LEN+1];
	//
	//                // Get the ROWID value.  If not successful retreiving the ROWID,
	//                // simply fall down through the code and build the WHERE clause
	//                // based on the key fields.
	//                if (SQLGetData(m_hstmt, (UWORD)(m_numCols+1), SQL_C_WXCHAR, (UCHAR*) rowid, sizeof(rowid), &cb) == SQL_SUCCESS)
	//                {
	//                    pSqlStmt += L"ROWID = '";
	//                    pSqlStmt += rowid;
	//                    pSqlStmt += L"'";
	//                    break;
	//                }
	//            }
	//            // Unable to delete by ROWID, so build a WHERE
	//            // clause based on the keyfields.
	//            BuildWhereClause(whereClause, DB_WHERE_KEYFIELDS);
	//            pSqlStmt += whereClause;
	//            break;
	//        case DB_UPD_WHERE:
	//            pSqlStmt += pWhereClause;
	//            break;
	//    }
	//}  // BuildUpdateStmt()


	///***** DEPRECATED: use wxDbTable::BuildUpdateStmt(std::wstring &....) form *****/
	//void Table::BuildUpdateStmt(wchar_t *pSqlStmt, int typeOfUpdate, const std::wstring &pWhereClause)
	//{
	//    std::wstring tempSqlStmt;
	//    BuildUpdateStmt(tempSqlStmt, typeOfUpdate, pWhereClause);
	//    wcscpy(pSqlStmt, tempSqlStmt.c_str());
	//}  // BuildUpdateStmt()


	///********** wxDbTable::BuildWhereClause() **********/
	//void Table::BuildWhereClause(std::wstring &pWhereClause, int typeOfWhere,
	//                                 const std::wstring &qualTableName, bool useLikeComparison)
	///*
	// * Note: BuildWhereClause() currently ignores timestamp columns.
	// *       They are not included as part of the where clause.
	// */
	//{
	//    bool moreThanOneColumn = false;
	//    std::wstring colValue;
	//
	//    // Loop through the columns building a where clause as you go
	//    int colNumber;
	//    for (colNumber = 0; colNumber < m_numCols; colNumber++)
	//    {
	//        // Determine if this column should be included in the WHERE clause
	//        if ((typeOfWhere == DB_WHERE_KEYFIELDS && m_colDefs[colNumber].m_keyField) ||
	//             (typeOfWhere == DB_WHERE_MATCHING  && (!IsColNull((UWORD)colNumber))))
	//        {
	//            // Skip over timestamp columns
	//            if (m_colDefs[colNumber].m_sqlCtype == SQL_C_TIMESTAMP)
	//                continue;
	//            // If there is more than 1 column, join them with the keyword "AND"
	//            if (moreThanOneColumn)
	//                pWhereClause += L" AND ";
	//            else
	//                moreThanOneColumn = true;
	//
	//            // Concatenate where phrase for the column
	//            std::wstring tStr = m_colDefs[colNumber].m_colName;
	//
	//            if (qualTableName.length() && tStr.find(L'.') == std::wstring::npos)
	//            {
	//                pWhereClause += m_pDb->SQLTableName(qualTableName.c_str());
	//                pWhereClause += L".";
	//            }
	//            pWhereClause += m_pDb->SQLColumnName(m_colDefs[colNumber].m_colName);
	//
	//            if (useLikeComparison && (m_colDefs[colNumber].m_sqlCtype == SQL_C_WXCHAR))
	//                pWhereClause += L" LIKE ";
	//            else
	//                pWhereClause += L" = ";
	//
	//            switch(m_colDefs[colNumber].m_sqlCtype)
	//            {
	//                case SQL_C_CHAR:
	//#ifdef SQL_C_WCHAR
	//                case SQL_C_WCHAR:
	//#endif
	//                //case SQL_C_WXCHAR:  SQL_C_WXCHAR is covered by either SQL_C_CHAR or SQL_C_WCHAR
	//					colValue = (boost::wformat(L"'%s'") % GetDb()->EscapeSqlChars((wchar_t *)m_colDefs[colNumber].m_ptrDataObj)).str();
	//                    break;
	//                case SQL_C_SHORT:
	//                case SQL_C_SSHORT:
	//					colValue = (boost::wformat(L"%hi") % *((SWORD *) m_colDefs[colNumber].m_ptrDataObj)).str();
	//                    break;
	//                case SQL_C_USHORT:
	//					colValue = (boost::wformat(L"%hu") % *((UWORD *) m_colDefs[colNumber].m_ptrDataObj)).str();
	//                    break;
	//                case SQL_C_LONG:
	//                case SQL_C_SLONG:
	//					colValue = (boost::wformat(L"%li") % *((SDWORD *) m_colDefs[colNumber].m_ptrDataObj)).str();
	//                    break;
	//                case SQL_C_ULONG:
	//					colValue = (boost::wformat(L"%lu") % *((UDWORD *) m_colDefs[colNumber].m_ptrDataObj)).str();
	//                    break;
	//                case SQL_C_FLOAT:
	//					colValue = (boost::wformat(L"%.6f") % *((SFLOAT *) m_colDefs[colNumber].m_ptrDataObj)).str();
	//                    break;
	//                case SQL_C_DOUBLE:
	//					colValue = (boost::wformat(L"%.6f") % *((SDOUBLE *) m_colDefs[colNumber].m_ptrDataObj)).str();
	//                    break;
	//                default:
	//                    {
	//                        std::wstring strMsg;
	//						strMsg = (boost::wformat(L"wxDbTable::bindParams(): Unknown column type for colDefs %d colName %s") % colNumber % m_colDefs[colNumber].m_colName).str();
	//                        exFAIL_MSG(strMsg.c_str());
	//                    }
	//                    break;
	//            }
	//            pWhereClause += colValue;
	//        }
	//    }
	//}  // wxDbTable::BuildWhereClause()


	///***** DEPRECATED: use wxDbTable::BuildWhereClause(std::wstring &....) form *****/
	//void Table::BuildWhereClause(wchar_t *pWhereClause, int typeOfWhere,
	//                                 const std::wstring &qualTableName, bool useLikeComparison)
	//{
	//    std::wstring tempSqlStmt;
	//    BuildWhereClause(tempSqlStmt, typeOfWhere, qualTableName, useLikeComparison);
	//    wcscpy(pWhereClause, tempSqlStmt.c_str());
	//}  // wxDbTable::BuildWhereClause()


	///********** wxDbTable::GetRowNum() **********/
	//UWORD Table::GetRowNum()
	//{
	//	UDWORD rowNum;

	//	if (SQLGetStmtOption(m_hstmt, SQL_ROW_NUMBER, (UCHAR*)&rowNum) != SQL_SUCCESS)
	//	{
	//		m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmt);
	//		return(0);
	//	}

	//	// Completed successfully
	//	return((UWORD)rowNum);

	//}  // wxDbTable::GetRowNum()


	///********** wxDbTable::CloseCursor() **********/
	//bool Table::CloseCursor(SQLHSTMT cursor)
	//{
	//	if (SQLFreeStmt(cursor, SQL_CLOSE) != SQL_SUCCESS)
	//		return(m_pDb->DispAllErrors(NULL, m_hdbc, cursor));

	//	// Completed successfully
	//	return true;

	//}  // wxDbTable::CloseCursor()


	/********** wxDbTable::CreateTable() **********/
	//bool Table::CreateTable(bool attemptDrop)
	//{
	//    if (!m_pDb)
	//        return false;
	//
	//    int i, j;
	//    std::wstring sqlStmt;
	//
	//#ifdef DBDEBUG_CONSOLE
	//    std::wcout << L"Creating Table " << m_tableName << L"..." << std::endl;
	//#endif
	//
	//    // Drop table first
	//    if (attemptDrop && !DropTable())
	//        return false;
	//
	//    // Create the table
	//#ifdef DBDEBUG_CONSOLE
	//    for (i = 0; i < m_numCols; i++)
	//    {
	//        // Exclude derived columns since they are NOT part of the base table
	//        if (m_colDefs[i].m_derivedCol)
	//            continue;
	//        std::wcout << i + 1 << L": " << m_colDefs[i].m_colName << L"; ";
	//        switch(m_colDefs[i].m_dbDataType)
	//        {
	//            case DB_DATA_TYPE_VARCHAR:
	//                std::wcout << m_pDb->GetTypeInfVarchar().TypeName << L"(" << (int)(m_colDefs[i].m_szDataObj / sizeof(wchar_t)) << L")";
	//                break;
	//            case DB_DATA_TYPE_MEMO:
	//                std::wcout << m_pDb->GetTypeInfMemo().TypeName;
	//                break;
	//            case DB_DATA_TYPE_INTEGER:
	//                std::wcout << m_pDb->GetTypeInfInteger().TypeName;
	//                break;
	//            case DB_DATA_TYPE_FLOAT:
	//                std::wcout << m_pDb->GetTypeInfFloat().TypeName;
	//                break;
	//            case DB_DATA_TYPE_DATE:
	//                std::wcout << m_pDb->GetTypeInfDate().TypeName;
	//                break;
	//            case DB_DATA_TYPE_BLOB:
	//                std::wcout << m_pDb->GetTypeInfBlob().TypeName;
	//                break;
	//        }
	//        std::wcout << std::endl;
	//    }
	//#endif
	//
	//    // Build a CREATE TABLE string from the colDefs structure.
	//    bool needComma = false;
	//
	//	sqlStmt = (boost::wformat(L"CREATE TABLE %s (") % m_pDb->SQLTableName(m_tableName.c_str())).str();
	//
	//    for (i = 0; i < m_numCols; i++)
	//    {
	//        // Exclude derived columns since they are NOT part of the base table
	//        if (m_colDefs[i].m_derivedCol)
	//            continue;
	//        // Comma Delimiter
	//        if (needComma)
	//            sqlStmt += L",";
	//        // Column Name
	//        sqlStmt += m_pDb->SQLColumnName(m_colDefs[i].m_colName);
	////        sqlStmt += colDefs[i].ColName;
	//        sqlStmt += L" ";
	//        // Column Type
	//        switch(m_colDefs[i].m_dbDataType)
	//        {
	//            case DB_DATA_TYPE_VARCHAR:
	//                sqlStmt += m_pDb->GetTypeInfVarchar().TypeName;
	//                break;
	//            case DB_DATA_TYPE_MEMO:
	//                sqlStmt += m_pDb->GetTypeInfMemo().TypeName;
	//                break;
	//            case DB_DATA_TYPE_INTEGER:
	//                sqlStmt += m_pDb->GetTypeInfInteger().TypeName;
	//                break;
	//            case DB_DATA_TYPE_FLOAT:
	//                sqlStmt += m_pDb->GetTypeInfFloat().TypeName;
	//                break;
	//            case DB_DATA_TYPE_DATE:
	//                sqlStmt += m_pDb->GetTypeInfDate().TypeName;
	//                break;
	//            case DB_DATA_TYPE_BLOB:
	//                sqlStmt += m_pDb->GetTypeInfBlob().TypeName;
	//                break;
	//        }
	//        // For varchars, append the size of the string
	//        if (m_colDefs[i].m_dbDataType == DB_DATA_TYPE_VARCHAR &&
	//            (m_pDb->GetDbms() != dbmsMY_SQL || m_pDb->GetTypeInfVarchar().TypeName != L"text"))// ||
	////            colDefs[i].DbDataType == DB_DATA_TYPE_BLOB)
	//        {
	//            std::wstring s;
	//			s = (boost::wformat(L"(%d)") % (int)(m_colDefs[i].m_szDataObj / sizeof(wchar_t))).str();
	//            sqlStmt += s;
	//        }
	//
	//        if (m_pDb->GetDbms() == dbmsDB2 ||
	//            m_pDb->GetDbms() == dbmsMY_SQL ||
	//            m_pDb->GetDbms() == dbmsSYBASE_ASE  ||
	//            m_pDb->GetDbms() == dbmsINTERBASE  ||
	//            m_pDb->GetDbms() == dbmsFIREBIRD  ||
	//            m_pDb->GetDbms() == dbmsMS_SQL_SERVER)
	//        {
	//            if (m_colDefs[i].m_keyField)
	//            {
	//                sqlStmt += L" NOT NULL";
	//            }
	//        }
	//
	//        needComma = true;
	//    }
	//    // If there is a primary key defined, include it in the create statement
	//    for (i = j = 0; i < m_numCols; i++)
	//    {
	//        if (m_colDefs[i].m_keyField)
	//        {
	//            j++;
	//            break;
	//        }
	//    }
	//    if ( j && (m_pDb->GetDbms() != dbmsDBASE)
	//        && (m_pDb->GetDbms() != dbmsXBASE_SEQUITER) )  // Found a keyfield
	//    {
	//        switch (m_pDb->GetDbms())
	//        {
	//            case dbmsACCESS:
	//            case dbmsINFORMIX:
	//            case dbmsSYBASE_ASA:
	//            case dbmsSYBASE_ASE:
	//            case dbmsMY_SQL:
	//            case dbmsFIREBIRD:
	//            {
	//                // MySQL goes out on this one. We also declare the relevant key NON NULL above
	//                sqlStmt += L",PRIMARY KEY (";
	//                break;
	//            }
	//            default:
	//            {
	//                sqlStmt += L",CONSTRAINT ";
	//                //  DB2 is limited to 18 characters for index names
	//                if (m_pDb->GetDbms() == dbmsDB2)
	//                {
	//                    exASSERT_MSG(m_tableName.length() <= 13, "DB2 table/index names must be no longer than 13 characters in length.\n\nTruncating table name to 13 characters.");
	//                    sqlStmt += m_pDb->SQLTableName(m_tableName.substr(0, 13).c_str());
	////                    sqlStmt += tableName.substr(0, 13);
	//                }
	//                else
	//                    sqlStmt += m_pDb->SQLTableName(m_tableName.c_str());
	////                    sqlStmt += tableName;
	//
	//                sqlStmt += L"_PIDX PRIMARY KEY (";
	//                break;
	//            }
	//        }
	//
	//        // List column name(s) of column(s) comprising the primary key
	//        for (i = j = 0; i < m_numCols; i++)
	//        {
	//            if (m_colDefs[i].m_keyField)
	//            {
	//                if (j++) // Multi part key, comma separate names
	//                    sqlStmt += L",";
	//                sqlStmt += m_pDb->SQLColumnName(m_colDefs[i].m_colName);
	//
	//                if (m_pDb->GetDbms() == dbmsMY_SQL &&
	//                    m_colDefs[i].m_dbDataType ==  DB_DATA_TYPE_VARCHAR)
	//                {
	//                    std::wstring s;
	//					s = (boost::wformat(L"(%d)") % (int)(m_colDefs[i].m_szDataObj / sizeof(wchar_t))).str();
	//                    sqlStmt += s;
	//                }
	//            }
	//        }
	//        sqlStmt += L")";
	//
	//        if (m_pDb->GetDbms() == dbmsINFORMIX ||
	//            m_pDb->GetDbms() == dbmsSYBASE_ASA ||
	//            m_pDb->GetDbms() == dbmsSYBASE_ASE)
	//        {
	//            sqlStmt += L" CONSTRAINT ";
	//            sqlStmt += m_pDb->SQLTableName(m_tableName.c_str());
	////            sqlStmt += tableName;
	//            sqlStmt += L"_PIDX";
	//        }
	//    }
	//    // Append the closing parentheses for the create table statement
	//    sqlStmt += L")";
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//#ifdef DBDEBUG_CONSOLE
	//    std::wcout << std::endl << sqlStmt.c_str() << std::endl;
	//#endif
	//
	//    // Execute the CREATE TABLE statement
	//    RETCODE retcode = SQLExecDirect(m_hstmt, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS);
	//    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
	//    {
	//        m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmt);
	//        m_pDb->RollbackTrans();
	//        CloseCursor(m_hstmt);
	//        return false;
	//    }
	//
	//    // Commit the transaction and close the cursor
	//    if (!m_pDb->CommitTrans())
	//        return false;
	//    if (!CloseCursor(m_hstmt))
	//        return false;
	//
	//    // Database table created successfully
	//    return true;
	//
	//} // wxDbTable::CreateTable()


	/********** wxDbTable::DropTable() **********/
	//bool Table::DropTable()
	//{
	//    // NOTE: This function returns true if the Table does not exist, but
	//    //       only for identified databases.  Code will need to be added
	//    //       below for any other databases when those databases are defined
	//    //       to handle this situation consistently
	//
	//    std::wstring sqlStmt;
	//
	//	sqlStmt = (boost::wformat(L"DROP TABLE %s") % m_pDb->SQLTableName(m_tableName.c_str())).str();
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//#ifdef DBDEBUG_CONSOLE
	//    std::wcout << std::endl << sqlStmt.c_str() << std::endl;
	//#endif
	//
	//    RETCODE retcode = SQLExecDirect(m_hstmt, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS);
	//    if (retcode != SQL_SUCCESS)
	//    {
	//        // Check for "Base table not found" error and ignore
	//        m_pDb->GetNextError(m_henv, m_hdbc, m_hstmt);
	//        if (wcscmp(m_pDb->sqlState, L"S0002") /*&&
	//            wcscmp(pDb->sqlState, L"S1000")*/)  // "Base table not found"
	//        {
	//            // Check for product specific error codes
	//            if (!((m_pDb->GetDbms() == dbmsSYBASE_ASA    && !wcscmp(m_pDb->sqlState, L"42000"))   ||  // 5.x (and lower?)
	//                  (m_pDb->GetDbms() == dbmsSYBASE_ASE    && !wcscmp(m_pDb->sqlState, L"37000"))   ||
	//                  (m_pDb->GetDbms() == dbmsPERVASIVE_SQL && !wcscmp(m_pDb->sqlState, L"S1000"))   ||  // Returns an S1000 then an S0002
	//                  (m_pDb->GetDbms() == dbmsPOSTGRES      && !wcscmp(m_pDb->sqlState, L"08S01"))))
	//            {
	//                m_pDb->DispNextError();
	//                m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmt);
	//                m_pDb->RollbackTrans();
	////                CloseCursor(hstmt);
	//                return false;
	//            }
	//        }
	//    }
	//
	//    // Commit the transaction and close the cursor
	//    if (! m_pDb->CommitTrans())
	//        return false;
	//    if (! CloseCursor(m_hstmt))
	//        return false;
	//
	//    return true;
	//}  // wxDbTable::DropTable()


	/********** wxDbTable::CreateIndex() **********/
	//bool Table::CreateIndex(const std::wstring &indexName, bool unique, UWORD numIndexColumns,
	//                                     SIndexDefinition *pIndexDefs, bool attemptDrop)
	//{
	//    std::wstring sqlStmt;
	//
	//    // Drop the index first
	//    if (attemptDrop && !DropIndex(indexName))
	//        return false;
	//
	//    // MySQL (and possibly Sybase ASE?? - gt) require that any columns which are used as portions
	//    // of an index have the columns defined as "NOT NULL".  During initial table creation though,
	//    // it may not be known which columns are necessarily going to be part of an index (e.g. the
	//    // table was created, then months later you determine that an additional index while
	//    // give better performance, so you want to add an index).
	//    //
	//    // The following block of code will modify the column definition to make the column be
	//    // defined with the "NOT NULL" qualifier.
	//    if (m_pDb->GetDbms() == dbmsMY_SQL)
	//    {
	//        std::wstring sqlStmt;
	//        int i;
	//        bool ok = true;
	//        for (i = 0; i < numIndexColumns && ok; i++)
	//        {
	//            int   j = 0;
	//            bool  found = false;
	//            // Find the column definition that has the ColName that matches the
	//            // index column name.  We need to do this to get the DB_DATA_TYPE of
	//            // the index column, as MySQL's syntax for the ALTER column requires
	//            // this information
	//            while (!found && (j < this->m_numCols))
	//            {
	//                if (wcscmp(m_colDefs[j].m_colName,pIndexDefs[i].ColName) == 0)
	//                    found = true;
	//                if (!found)
	//                    j++;
	//            }
	//
	//            if (found)
	//            {
	//                ok = m_pDb->ModifyColumn(m_tableName, pIndexDefs[i].ColName,
	//                                        m_colDefs[j].m_dbDataType, (int)(m_colDefs[j].m_szDataObj / sizeof(wchar_t)),
	//                                        L"NOT NULL");
	//
	//                if (!ok)
	//                {
	//                    #if 0
	//                    // retcode is not used
	//                    wxODBC_ERRORS retcode;
	//                    // Oracle returns a DB_ERR_GENERAL_ERROR if the column is already
	//                    // defined to be NOT NULL, but reportedly MySQL doesn't mind.
	//                    // This line is just here for debug checking of the value
	//                    retcode = (wxODBC_ERRORS)pDb->DB_STATUS;
	//                    #endif
	//                }
	//            }
	//            else
	//                ok = false;
	//        }
	//        if (ok)
	//            m_pDb->CommitTrans();
	//        else
	//        {
	//            m_pDb->RollbackTrans();
	//            return false;
	//        }
	//    }
	//
	//    // Build a CREATE INDEX statement
	//    sqlStmt = L"CREATE ";
	//    if (unique)
	//        sqlStmt += L"UNIQUE ";
	//
	//    sqlStmt += L"INDEX ";
	//    sqlStmt += m_pDb->SQLTableName(indexName.c_str());
	//    sqlStmt += L" ON ";
	//
	//    sqlStmt += m_pDb->SQLTableName(m_tableName.c_str());
	////    sqlStmt += tableName;
	//    sqlStmt += L" (";
	//
	//    // Append list of columns making up index
	//    int i;
	//    for (i = 0; i < numIndexColumns; i++)
	//    {
	//        sqlStmt += m_pDb->SQLColumnName(pIndexDefs[i].ColName);
	////        sqlStmt += pIndexDefs[i].ColName;
	//
	//        // MySQL requires a key length on VARCHAR keys
	//        if ( m_pDb->GetDbms() == dbmsMY_SQL )
	//        {
	//            // Find the details on this column
	//            int j;
	//            for ( j = 0; j < m_numCols; ++j )
	//            {
	//                if ( wcscmp( pIndexDefs[i].ColName, m_colDefs[j].m_colName ) == 0 )
	//                {
	//                    break;
	//                }
	//            }
	//            if ( m_colDefs[j].m_dbDataType ==  DB_DATA_TYPE_VARCHAR)
	//            {
	//                std::wstring s;
	//				s = (boost::wformat(L"(%d)") % (int)(m_colDefs[i].m_szDataObj / sizeof(wchar_t))).str();
	//                sqlStmt += s;
	//            }
	//        }
	//
	//        // Postgres and SQL Server 7 do not support the ASC/DESC keywords for index columns
	//        if (!((m_pDb->GetDbms() == dbmsMS_SQL_SERVER) && (wcsncmp(m_pDb->m_dbInf.dbmsVer, L"07", 2)==0)) &&
	//            !(m_pDb->GetDbms() == dbmsFIREBIRD) &&
	//            !(m_pDb->GetDbms() == dbmsPOSTGRES))
	//        {
	//            if (pIndexDefs[i].Ascending)
	//                sqlStmt += L" ASC";
	//            else
	//                sqlStmt += L" DESC";
	//        }
	//        else
	//            exASSERT_MSG(pIndexDefs[i].Ascending, "Datasource does not support DESCending index columns");
	//
	//        if ((i + 1) < numIndexColumns)
	//            sqlStmt += L",";
	//    }
	//
	//    // Append closing parentheses
	//    sqlStmt += L")";
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//#ifdef DBDEBUG_CONSOLE
	//    std::wcout << std::endl << sqlStmt.c_str() << std::endl << std::endl;
	//#endif
	//
	//    // Execute the CREATE INDEX statement
	//    RETCODE retcode = SQLExecDirect(m_hstmt, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS);
	//    if (retcode != SQL_SUCCESS)
	//    {
	//        m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmt);
	//        m_pDb->RollbackTrans();
	//        CloseCursor(m_hstmt);
	//        return false;
	//    }
	//
	//    // Commit the transaction and close the cursor
	//    if (! m_pDb->CommitTrans())
	//        return false;
	//    if (! CloseCursor(m_hstmt))
	//        return false;
	//
	//    // Index Created Successfully
	//    return true;
	//
	//}  // wxDbTable::CreateIndex()


	/********** wxDbTable::DropIndex() **********/
	//bool Table::DropIndex(const std::wstring &indexName)
	//{
	//    // NOTE: This function returns true if the Index does not exist, but
	//    //       only for identified databases.  Code will need to be added
	//    //       below for any other databases when those databases are defined
	//    //       to handle this situation consistently
	//
	//    std::wstring sqlStmt;
	//
	//    if (m_pDb->GetDbms() == dbmsACCESS || m_pDb->GetDbms() == dbmsMY_SQL ||
	//        m_pDb->GetDbms() == dbmsDBASE /*|| Paradox needs this syntax too when we add support*/)
	//		sqlStmt = (boost::wformat(L"DROP INDEX %s ON %s") % m_pDb->SQLTableName(indexName.c_str()) % m_pDb->SQLTableName(m_tableName.c_str())).str();
	//    else if ((m_pDb->GetDbms() == dbmsMS_SQL_SERVER) ||
	//             (m_pDb->GetDbms() == dbmsSYBASE_ASE) ||
	//             (m_pDb->GetDbms() == dbmsXBASE_SEQUITER))
	//		sqlStmt = (boost::wformat(L"DROP INDEX %s.%s") % m_pDb->SQLTableName(m_tableName.c_str()) % m_pDb->SQLTableName(indexName.c_str())).str();
	//    else
	//		sqlStmt = (boost::wformat(L"DROP INDEX %s") % m_pDb->SQLTableName(indexName.c_str())).str();
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//#ifdef DBDEBUG_CONSOLE
	//    std::wcout << std::endl << sqlStmt.c_str() << std::endl;
	//#endif
	//    RETCODE retcode = SQLExecDirect(m_hstmt, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS);
	//    if (retcode != SQL_SUCCESS)
	//    {
	//        // Check for "Index not found" error and ignore
	//        m_pDb->GetNextError(m_henv, m_hdbc, m_hstmt);
	//        if (wcscmp(m_pDb->sqlState, L"S0012"))  // "Index not found"
	//        {
	//            // Check for product specific error codes
	//            if (!((m_pDb->GetDbms() == dbmsSYBASE_ASA    && !wcscmp(m_pDb->sqlState, L"42000")) ||  // v5.x (and lower?)
	//                  (m_pDb->GetDbms() == dbmsSYBASE_ASE    && !wcscmp(m_pDb->sqlState, L"37000")) ||
	//                  (m_pDb->GetDbms() == dbmsMS_SQL_SERVER && !wcscmp(m_pDb->sqlState, L"S1000")) ||
	//                  (m_pDb->GetDbms() == dbmsINTERBASE     && !wcscmp(m_pDb->sqlState, L"S1000")) ||
	//                  (m_pDb->GetDbms() == dbmsMAXDB         && !wcscmp(m_pDb->sqlState, L"S1000")) ||
	//                  (m_pDb->GetDbms() == dbmsFIREBIRD      && !wcscmp(m_pDb->sqlState, L"HY000")) ||
	//                  (m_pDb->GetDbms() == dbmsSYBASE_ASE    && !wcscmp(m_pDb->sqlState, L"S0002")) ||  // Base table not found
	//                  (m_pDb->GetDbms() == dbmsMY_SQL        && !wcscmp(m_pDb->sqlState, L"42S12")) ||  // tested by Christopher Ludwik Marino-Cebulski using v3.23.21beta
	//                  (m_pDb->GetDbms() == dbmsPOSTGRES      && !wcscmp(m_pDb->sqlState, L"08S01"))
	//               ))
	//            {
	//                m_pDb->DispNextError();
	//                m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmt);
	//                m_pDb->RollbackTrans();
	//                CloseCursor(m_hstmt);
	//                return false;
	//            }
	//        }
	//    }
	//
	//    // Commit the transaction and close the cursor
	//    if (! m_pDb->CommitTrans())
	//        return false;
	//    if (! CloseCursor(m_hstmt))
	//        return false;
	//
	//    return true;
	//}  // wxDbTable::DropIndex()


	///********** wxDbTable::SetOrderByColNums() **********/
	//bool Table::SetOrderByColNums(UWORD first, ...)
	//{
	//	int         colNumber = first;  // using 'int' to be able to look for wxDB_NO_MORE_COLUN_NUMBERS
	//	va_list     argptr;

	//	bool        abort = false;
	//	std::wstring    tempStr;

	//	va_start(argptr, first);     /* Initialize variable arguments. */
	//	while (!abort && (colNumber != wxDB_NO_MORE_COLUMN_NUMBERS))
	//	{
	//		// Make sure the passed in column number
	//		// is within the valid range of columns
	//		//
	//		// Valid columns are 0 thru m_numCols-1
	//		if (colNumber >= m_numCols || colNumber < 0)
	//		{
	//			abort = true;
	//			continue;
	//		}

	//		if (colNumber != first)
	//			tempStr += L",";

	//		tempStr += m_colDefs[colNumber].m_colName;
	//		colNumber = va_arg(argptr, int);
	//	}
	//	va_end(argptr);              /* Reset variable arguments.      */

	//	SetOrderByClause(tempStr);

	//	return (!abort);
	//}  // wxDbTable::SetOrderByColNums()


	/********** wxDbTable::Insert() **********/
	//int Table::Insert()
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly || !m_insertable)
	//        return(DB_FAILURE);
	//
	//    bindInsertParams();
	//
	//    // Insert the record by executing the already prepared insert statement
	//    RETCODE retcode;
	//    retcode = SQLExecute(m_hstmtInsert);
	//    if (retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO &&
	//        retcode != SQL_NEED_DATA)
	//    {
	//        // Check to see if integrity constraint was violated
	//        m_pDb->GetNextError(m_henv, m_hdbc, m_hstmtInsert);
	//        if (! wcscmp(m_pDb->sqlState, L"23000"))  // Integrity constraint violated
	//            return(DB_ERR_INTEGRITY_CONSTRAINT_VIOL);
	//        else
	//        {
	//            m_pDb->DispNextError();
	//            m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtInsert);
	//            return(DB_FAILURE);
	//        }
	//    }
	//    if (retcode == SQL_NEED_DATA)
	//    {
	//        PTR pParmID;
	//        retcode = SQLParamData(m_hstmtInsert, &pParmID);
	//        while (retcode == SQL_NEED_DATA)
	//        {
	//            // Find the parameter
	//            int i;
	//            for (i=0; i < m_numCols; i++)
	//            {
	//                if (m_colDefs[i].m_ptrDataObj == pParmID)
	//                {
	//                    // We found it.  Store the parameter.
	//                    retcode = SQLPutData(m_hstmtInsert, pParmID, m_colDefs[i].m_szDataObj);
	//                    if (retcode != SQL_SUCCESS)
	//                    {
	//                        m_pDb->DispNextError();
	//                        m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtInsert);
	//                        return(DB_FAILURE);
	//                    }
	//                    break;
	//                }
	//            }
	//            retcode = SQLParamData(m_hstmtInsert, &pParmID);
	//            if (retcode != SQL_SUCCESS &&
	//                retcode != SQL_SUCCESS_WITH_INFO)
	//            {
	//                // record was not inserted
	//                m_pDb->DispNextError();
	//                m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtInsert);
	//                return(DB_FAILURE);
	//            }
	//        }
	//    }
	//
	//    // Record inserted into the datasource successfully
	//    return(DB_SUCCESS);
	//
	//}  // wxDbTable::Insert()


	///********** wxDbTable::Update() **********/
	//bool Table::Update()
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return false;
	//
	//    std::wstring sqlStmt;
	//
	//    // Build the SQL UPDATE statement
	//    BuildUpdateStmt(sqlStmt, DB_UPD_KEYFIELDS);
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//#ifdef DBDEBUG_CONSOLE
	//    std::wcout << std::endl << sqlStmt.c_str() << std::endl << std::endl;
	//#endif
	//
	//    // Execute the SQL UPDATE statement
	//    return(execUpdate(sqlStmt));
	//
	//}  // wxDbTable::Update()


	///********** wxDbTable::Update(pSqlStmt) **********/
	//bool Table::Update(const std::wstring &pSqlStmt)
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return false;
	//
	//    m_pDb->WriteSqlLog(pSqlStmt);
	//
	//    return(execUpdate(pSqlStmt));
	//
	//}  // wxDbTable::Update(pSqlStmt)


	///********** wxDbTable::UpdateWhere() **********/
	//bool Table::UpdateWhere(const std::wstring &pWhereClause)
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return false;
	//
	//    std::wstring sqlStmt;
	//
	//    // Build the SQL UPDATE statement
	//    BuildUpdateStmt(sqlStmt, DB_UPD_WHERE, pWhereClause);
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//#ifdef DBDEBUG_CONSOLE
	//    std::wcout << std::endl << sqlStmt.c_str() << std::endl << std::endl;
	//#endif
	//
	//    // Execute the SQL UPDATE statement
	//    return(execUpdate(sqlStmt));
	//
	//}  // wxDbTable::UpdateWhere()


	///********** wxDbTable::Delete() **********/
	//bool Table::Delete()
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return false;
	//
	//    std::wstring sqlStmt;
	//    sqlStmt.empty();
	//
	//    // Build the SQL DELETE statement
	//    BuildDeleteStmt(sqlStmt, DB_DEL_KEYFIELDS);
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//    // Execute the SQL DELETE statement
	//    return(execDelete(sqlStmt));
	//
	//}  // wxDbTable::Delete()


	///********** wxDbTable::DeleteWhere() **********/
	//bool Table::DeleteWhere(const std::wstring &pWhereClause)
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return false;
	//
	//    std::wstring sqlStmt;
	//    sqlStmt.empty();
	//
	//    // Build the SQL DELETE statement
	//    BuildDeleteStmt(sqlStmt, DB_DEL_WHERE, pWhereClause);
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//    // Execute the SQL DELETE statement
	//    return(execDelete(sqlStmt));
	//
	//}  // wxDbTable::DeleteWhere()


	///********** wxDbTable::DeleteMatching() **********/
	//bool Table::DeleteMatching()
	//{
	//    exASSERT(!m_queryOnly);
	//    if (m_queryOnly)
	//        return false;
	//
	//    std::wstring sqlStmt;
	//    sqlStmt.empty();
	//
	//    // Build the SQL DELETE statement
	//    BuildDeleteStmt(sqlStmt, DB_DEL_MATCHING);
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//    // Execute the SQL DELETE statement
	//    return(execDelete(sqlStmt));
	//
	//}  // wxDbTable::DeleteMatching()


	///********** wxDbTable::IsColNull() **********/
	//bool Table::IsColNull(UWORD colNumber) const
	//{
	//	/*
	//		This logic is just not right.  It would indicate true
	//		if a numeric field were set to a value of 0.

	//		switch(colDefs[colNumber].SqlCtype)
	//		{
	//		case SQL_C_CHAR:
	//		case SQL_C_WCHAR:
	//		//case SQL_C_WXCHAR:  SQL_C_WXCHAR is covered by either SQL_C_CHAR or SQL_C_WCHAR
	//		return(((UCHAR FAR *) colDefs[colNumber].PtrDataObj)[0] == 0);
	//		case SQL_C_SSHORT:
	//		return((  *((SWORD *) colDefs[colNumber].PtrDataObj))   == 0);
	//		case SQL_C_USHORT:
	//		return((   *((UWORD*) colDefs[colNumber].PtrDataObj))   == 0);
	//		case SQL_C_SLONG:
	//		return(( *((SDWORD *) colDefs[colNumber].PtrDataObj))   == 0);
	//		case SQL_C_ULONG:
	//		return(( *((UDWORD *) colDefs[colNumber].PtrDataObj))   == 0);
	//		case SQL_C_FLOAT:
	//		return(( *((SFLOAT *) colDefs[colNumber].PtrDataObj))   == 0);
	//		case SQL_C_DOUBLE:
	//		return((*((SDOUBLE *) colDefs[colNumber].PtrDataObj))   == 0);
	//		case SQL_C_TIMESTAMP:
	//		TIMESTAMP_STRUCT *pDt;
	//		pDt = (TIMESTAMP_STRUCT *) colDefs[colNumber].PtrDataObj;
	//		if (pDt->year == 0 && pDt->month == 0 && pDt->day == 0)
	//		return true;
	//		else
	//		return false;
	//		default:
	//		return true;
	//		}
	//		*/
	//	return (m_colDefs[colNumber].m_null);
	//}  // wxDbTable::IsColNull()


	///********** wxDbTable::CanSelectForUpdate() **********/
	//bool Table::CanSelectForUpdate()
	//{
	//	// Broken
	//	//if (IsQueryOnly())
	//	//    return false;

	//	//if (m_pDb->GetDbms() == dbmsMY_SQL)
	//	//    return false;

	//	//if (/*(m_pDb->GetDbms() == dbmsORACLE) ||*/
	//	//    (m_pDb->m_dbInf.posStmts & SQL_PS_SELECT_FOR_UPDATE))
	//	//    return true;
	//	//else
	//	return false;

	//}  // wxDbTable::CanSelectForUpdate()


	///********** wxDbTable::CanUpdateByROWID() **********/
	//bool Table::CanUpdateByROWID()
	//{
	//	/*
	//	 * NOTE: Returning false for now until this can be debugged,
	//	 *        as the ROWID is not getting updated correctly
	//	 */
	//	return false;
	//	/*
	//		if (pDb->GetDbms() == dbmsORACLE)
	//		return true;
	//		else
	//		return false;
	//		*/
	//}  // wxDbTable::CanUpdateByROWID()


	///********** wxDbTable::IsCursorClosedOnCommit() **********/
	//bool Table::IsCursorClosedOnCommit()
	//{
	//	//if (m_pDb->m_dbInf.cursorCommitBehavior == SQL_CB_PRESERVE)
	//	//    return false;
	//	//else
	//	return true;

	//}  // wxDbTable::IsCursorClosedOnCommit()



//	/********** wxDbTable::ClearMemberVar() **********/
//	void Table::ClearMemberVar(UWORD colNumber, bool setToNull)
//	{
//		exASSERT(colNumber < m_numCols);
//
//		switch (m_colDefs[colNumber].m_sqlCtype)
//		{
//		case SQL_C_CHAR:
//#ifdef SQL_C_WCHAR
//		case SQL_C_WCHAR:
//#endif
//			//case SQL_C_WXCHAR:  SQL_C_WXCHAR is covered by either SQL_C_CHAR or SQL_C_WCHAR
//			((UCHAR FAR *) m_colDefs[colNumber].m_ptrDataObj)[0] = 0;
//			break;
//		case SQL_C_SSHORT:
//			*((SWORD *)m_colDefs[colNumber].m_ptrDataObj) = 0;
//			break;
//		case SQL_C_USHORT:
//			*((UWORD*)m_colDefs[colNumber].m_ptrDataObj) = 0;
//			break;
//		case SQL_C_LONG:
//		case SQL_C_SLONG:
//			*((SDWORD *)m_colDefs[colNumber].m_ptrDataObj) = 0;
//			break;
//		case SQL_C_ULONG:
//			*((UDWORD *)m_colDefs[colNumber].m_ptrDataObj) = 0;
//			break;
//		case SQL_C_FLOAT:
//			*((SFLOAT *)m_colDefs[colNumber].m_ptrDataObj) = 0.0f;
//			break;
//		case SQL_C_DOUBLE:
//			*((SDOUBLE *)m_colDefs[colNumber].m_ptrDataObj) = 0.0f;
//			break;
//		case SQL_C_TIMESTAMP:
//			TIMESTAMP_STRUCT *pDt;
//			pDt = (TIMESTAMP_STRUCT *)m_colDefs[colNumber].m_ptrDataObj;
//			pDt->year = 0;
//			pDt->month = 0;
//			pDt->day = 0;
//			pDt->hour = 0;
//			pDt->minute = 0;
//			pDt->second = 0;
//			pDt->fraction = 0;
//			break;
//		case SQL_C_DATE:
//			DATE_STRUCT *pDtd;
//			pDtd = (DATE_STRUCT *)m_colDefs[colNumber].m_ptrDataObj;
//			pDtd->year = 0;
//			pDtd->month = 0;
//			pDtd->day = 0;
//			break;
//		case SQL_C_TIME:
//			TIME_STRUCT *pDtt;
//			pDtt = (TIME_STRUCT *)m_colDefs[colNumber].m_ptrDataObj;
//			pDtt->hour = 0;
//			pDtt->minute = 0;
//			pDtt->second = 0;
//			break;
//		}
//
//		if (setToNull)
//			SetColNull(colNumber);
//	}  // wxDbTable::ClearMemberVar()


	///********** wxDbTable::ClearMemberVars() **********/
	//void Table::ClearMemberVars(bool setToNull)
	//{
	//	size_t i;

	//	// Loop through the columns setting each member variable to zero
	//	for (i = 0; i < m_numCols; i++)
	//		ClearMemberVar((UWORD)i, setToNull);

	//}  // wxDbTable::ClearMemberVars()


	///********** wxDbTable::SetQueryTimeout() **********/
	//bool Table::SetQueryTimeout(UDWORD nSeconds)
	//{
	//	if (SQLSetStmtOption(m_hstmtInsert, SQL_QUERY_TIMEOUT, nSeconds) != SQL_SUCCESS)
	//		return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtInsert));
	//	if (SQLSetStmtOption(m_hstmtUpdate, SQL_QUERY_TIMEOUT, nSeconds) != SQL_SUCCESS)
	//		return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtUpdate));
	//	if (SQLSetStmtOption(m_hstmtDelete, SQL_QUERY_TIMEOUT, nSeconds) != SQL_SUCCESS)
	//		return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtDelete));
	//	if (SQLSetStmtOption(m_hstmtInternal, SQL_QUERY_TIMEOUT, nSeconds) != SQL_SUCCESS)
	//		return(m_pDb->DispAllErrors(NULL, m_hdbc, m_hstmtInternal));

	//	// Completed Successfully
	//	return true;

	//}  // wxDbTable::SetQueryTimeout()


	///********** wxDbTable::SetColDefs() **********/
	//bool Table::SetColDefs(UWORD index, const std::wstring &fieldName, int dataType, void *pData,
	//	SWORD cType, int size, bool keyField, bool updateable,
	//	bool insertAllowed, bool derivedColumn)
	//{
	//	std::wstring tmpStr;
	//	if (index >= m_numCols)  // Columns numbers are zero based....
	//	{
	//		tmpStr = (boost::wformat(L"Specified column index (%d) exceeds the maximum number of columns (%d) registered for this table definition.  Column definition not added.") % index % m_numCols).str();
	//		exFAIL_MSG(tmpStr.c_str());
	//		return false;
	//	}

	//	if (!m_colDefs)  // May happen if the database connection fails
	//		return false;

	//	if (fieldName.length() > (unsigned int)DB_MAX_COLUMN_NAME_LEN)
	//	{
	//		wcsncpy(m_colDefs[index].m_colName, fieldName.c_str(), DB_MAX_COLUMN_NAME_LEN);
	//		m_colDefs[index].m_colName[DB_MAX_COLUMN_NAME_LEN] = 0;  // Prevent buffer overrun

	//		tmpStr = (boost::wformat(L"Column name '%s' is too long. Truncated to '%s'.") % fieldName % m_colDefs[index].m_colName).str();
	//		exFAIL_MSG(tmpStr.c_str());
	//	}
	//	else
	//		wcscpy(m_colDefs[index].m_colName, fieldName.c_str());

	//	m_colDefs[index].m_dbDataType = dataType;
	//	m_colDefs[index].m_ptrDataObj = pData;
	//	m_colDefs[index].m_sqlCtype = cType;
	//	m_colDefs[index].m_szDataObj = size;  //TODO: glt ??? * sizeof(wchar_t) ???
	//	m_colDefs[index].m_keyField = keyField;
	//	m_colDefs[index].m_derivedCol = derivedColumn;
	//	// Derived columns by definition would NOT be "Insertable" or "Updateable"
	//	if (derivedColumn)
	//	{
	//		m_colDefs[index].m_updateable = false;
	//		m_colDefs[index].m_insertAllowed = false;
	//	}
	//	else
	//	{
	//		m_colDefs[index].m_updateable = updateable;
	//		m_colDefs[index].m_insertAllowed = insertAllowed;
	//	}

	//	m_colDefs[index].m_null = false;

	//	return true;

	//}  // wxDbTable::SetColDefs()


	///********** wxDbTable::SetColDefs() **********/
	//SColumnDataPtr* Table::SetColDefs(ColumnInfo *pColInfs, UWORD numCols)
	//{
	//	exASSERT(pColInfs);
	//	SColumnDataPtr *pColDataPtrs = NULL;

	//	if (pColInfs)
	//	{
	//		UWORD index;

	//		pColDataPtrs = new SColumnDataPtr[numCols + 1];

	//		for (index = 0; index < numCols; index++)
	//		{
	//			// Process the fields
	//			switch (pColInfs[index].m_dbDataType)
	//			{
	//			case DB_DATA_TYPE_VARCHAR:
	//				pColDataPtrs[index].PtrDataObj = new wchar_t[pColInfs[index].m_bufferSize + (1 * sizeof(wchar_t))];
	//				pColDataPtrs[index].SzDataObj = pColInfs[index].m_bufferSize + (1 * sizeof(wchar_t));
	//				pColDataPtrs[index].SqlCtype = SQL_C_WXCHAR;
	//				break;
	//			case DB_DATA_TYPE_MEMO:
	//				pColDataPtrs[index].PtrDataObj = new wchar_t[pColInfs[index].m_bufferSize + (1 * sizeof(wchar_t))];
	//				pColDataPtrs[index].SzDataObj = pColInfs[index].m_bufferSize + (1 * sizeof(wchar_t));
	//				pColDataPtrs[index].SqlCtype = SQL_C_WXCHAR;
	//				break;
	//			case DB_DATA_TYPE_INTEGER:
	//				// Can be long or short
	//				if (pColInfs[index].m_bufferSize == sizeof(long))
	//				{
	//					pColDataPtrs[index].PtrDataObj = new long;
	//					pColDataPtrs[index].SzDataObj = sizeof(long);
	//					pColDataPtrs[index].SqlCtype = SQL_C_SLONG;
	//				}
	//				else
	//				{
	//					pColDataPtrs[index].PtrDataObj = new short;
	//					pColDataPtrs[index].SzDataObj = sizeof(short);
	//					pColDataPtrs[index].SqlCtype = SQL_C_SSHORT;
	//				}
	//				break;
	//			case DB_DATA_TYPE_FLOAT:
	//				// Can be float or double
	//				if (pColInfs[index].m_bufferSize == sizeof(float))
	//				{
	//					pColDataPtrs[index].PtrDataObj = new float;
	//					pColDataPtrs[index].SzDataObj = sizeof(float);
	//					pColDataPtrs[index].SqlCtype = SQL_C_FLOAT;
	//				}
	//				else
	//				{
	//					pColDataPtrs[index].PtrDataObj = new double;
	//					pColDataPtrs[index].SzDataObj = sizeof(double);
	//					pColDataPtrs[index].SqlCtype = SQL_C_DOUBLE;
	//				}
	//				break;
	//			case DB_DATA_TYPE_DATE:
	//				pColDataPtrs[index].PtrDataObj = new TIMESTAMP_STRUCT;
	//				pColDataPtrs[index].SzDataObj = sizeof(TIMESTAMP_STRUCT);
	//				pColDataPtrs[index].SqlCtype = SQL_C_TIMESTAMP;
	//				break;
	//			case DB_DATA_TYPE_BLOB:
	//				exFAIL_MSG(L"This form of ::SetColDefs() cannot be used with BLOB columns");
	//				pColDataPtrs[index].PtrDataObj = /*BLOB ADDITION NEEDED*/NULL;
	//				pColDataPtrs[index].SzDataObj = /*BLOB ADDITION NEEDED*/sizeof(void *);
	//				pColDataPtrs[index].SqlCtype = SQL_VARBINARY;
	//				break;
	//			}
	//			if (pColDataPtrs[index].PtrDataObj != NULL)
	//				SetColDefs(index, pColInfs[index].m_colName, pColInfs[index].m_dbDataType, pColDataPtrs[index].PtrDataObj, pColDataPtrs[index].SqlCtype, pColDataPtrs[index].SzDataObj);
	//			else
	//			{
	//				// Unable to build all the column definitions, as either one of
	//				// the calls to "new" failed above, or there was a BLOB field
	//				// to have a column definition for.  If BLOBs are to be used,
	//				// the other form of ::SetColDefs() must be used, as it is impossible
	//				// to know the maximum size to create the PtrDataObj to be.
	//				delete[] pColDataPtrs;
	//				return NULL;
	//			}
	//		}
	//	}

	//	return (pColDataPtrs);

	//} // wxDbTable::SetColDefs()


	///********** wxDbTable::SetCursor() **********/
	//void Table::SetCursor(SQLHSTMT *hstmtActivate)
	//{
	//	if (hstmtActivate == wxDB_DEFAULT_CURSOR)
	//		m_hstmt = *m_hstmtDefault;
	//	else
	//		m_hstmt = *hstmtActivate;

	//}  // wxDbTable::SetCursor()


	///********** wxDbTable::Count(const std::wstring &) **********/
	//ULONG Table::Count(const std::wstring &args)
	//{
	//    ULONG count;
	//    std::wstring sqlStmt;
	//    SQLLEN cb;
	//
	//    // Build a "SELECT COUNT(*) FROM queryTableName [WHERE whereClause]" SQL Statement
	//    sqlStmt  = L"SELECT COUNT(";
	//    sqlStmt += args;
	//    sqlStmt += L") FROM ";
	//    sqlStmt += m_pDb->SQLTableName(m_queryTableName.c_str());
	////    sqlStmt += queryTableName;
	//    if (m_from.length())
	//        sqlStmt += m_from;
	//
	//    // Add the where clause if one is provided
	//    if (m_where.length())
	//    {
	//        sqlStmt += L" WHERE ";
	//        sqlStmt += m_where;
	//    }
	//
	//    m_pDb->WriteSqlLog(sqlStmt);
	//
	//    // Initialize the Count cursor if it's not already initialized
	//    if (!m_hstmtCount)
	//    {
	//        m_hstmtCount = GetNewCursor(false,false);
	//        exASSERT(m_hstmtCount);
	//        if (!m_hstmtCount)
	//            return(0);
	//    }
	//
	//    // Execute the SQL statement
	//    if (SQLExecDirect(*m_hstmtCount, (SQLTCHAR FAR *) sqlStmt.c_str(), SQL_NTS) != SQL_SUCCESS)
	//    {
	//        m_pDb->DispAllErrors(m_henv, m_hdbc, *m_hstmtCount);
	//        return(0);
	//    }
	//
	//    // Fetch the record
	//    if (SQLFetch(*m_hstmtCount) != SQL_SUCCESS)
	//    {
	//        m_pDb->DispAllErrors(m_henv, m_hdbc, *m_hstmtCount);
	//        return(0);
	//    }
	//
	//    // Obtain the result
	//    if (SQLGetData(*m_hstmtCount, (UWORD)1, SQL_C_ULONG, &count, sizeof(count), &cb) != SQL_SUCCESS)
	//    {
	//        m_pDb->DispAllErrors(m_henv, m_hdbc, *m_hstmtCount);
	//        return(0);
	//    }
	//
	//    // Free the cursor
	//    if (SQLFreeStmt(*m_hstmtCount, SQL_CLOSE) != SQL_SUCCESS)
	//        m_pDb->DispAllErrors(m_henv, m_hdbc, *m_hstmtCount);
	//
	//    // Return the record count
	//    return(count);
	//
	//}  // wxDbTable::Count()


	///********** wxDbTable::Refresh() **********/
	//bool Table::Refresh()
	//{
	//    bool result = true;
	//
	//    // Switch to the internal cursor so any active cursors are not corrupted
	//    HSTMT currCursor = GetCursor();
	//    m_hstmt = m_hstmtInternal;
	//    std::wstring saveWhere = m_where;
	//    std::wstring saveOrderBy = m_orderBy;
	//
	//    // Build a where clause to refetch the record with.  Try and use the
	//    // ROWID if it's available, ow use the key fields.
	//    std::wstring whereClause;
	//    whereClause.empty();
	//
	//    if (CanUpdateByROWID())
	//    {
	//        SQLLEN cb;
	//        wchar_t rowid[wxDB_ROWID_LEN+1];
	//
	//        // Get the ROWID value.  If not successful retreiving the ROWID,
	//        // simply fall down through the code and build the WHERE clause
	//        // based on the key fields.
	//        if (SQLGetData(m_hstmt, (UWORD)(m_numCols+1), SQL_C_WXCHAR, (UCHAR*) rowid, sizeof(rowid), &cb) == SQL_SUCCESS)
	//        {
	//            whereClause += m_pDb->SQLTableName(m_queryTableName.c_str());
	////            whereClause += queryTableName;
	//            whereClause += L".ROWID = '";
	//            whereClause += rowid;
	//            whereClause += L"'";
	//        }
	//    }
	//
	//    // If unable to use the ROWID, build a where clause from the keyfields
	//    if (whereClause.empty())
	//        BuildWhereClause(whereClause, DB_WHERE_KEYFIELDS, m_queryTableName);
	//
	//    // Requery the record
	//    m_where = whereClause;
	//    m_orderBy.empty();
	//    if (!Query())
	//        result = false;
	//
	//    if (result && !GetNext())
	//        result = false;
	//
	//    // Switch back to original cursor
	//    SetCursor(&currCursor);
	//
	//    // Free the internal cursor
	//    if (SQLFreeStmt(m_hstmtInternal, SQL_CLOSE) != SQL_SUCCESS)
	//        m_pDb->DispAllErrors(m_henv, m_hdbc, m_hstmtInternal);
	//
	//    // Restore the original where and order by clauses
	//    m_where   = saveWhere;
	//    m_orderBy = saveOrderBy;
	//
	//    return(result);
	//
	//}  // wxDbTable::Refresh()


	///********** wxDbTable::SetColNull() **********/
	//bool Table::SetColNull(UWORD colNumber, bool set)
	//{
	//	if (colNumber < m_numCols)
	//	{
	//		m_colDefs[colNumber].m_null = set;
	//		if (set)  // Blank out the values in the member variable
	//			ClearMemberVar(colNumber, false);  // Must call with false here, or infinite recursion will happen

	//		setCbValueForColumn(colNumber);

	//		return true;
	//	}
	//	else
	//		return false;

	//}  // wxDbTable::SetColNull()


	///********** wxDbTable::SetColNull() **********/
	//bool Table::SetColNull(const std::wstring &colName, bool set)
	//{
	//	size_t colNumber;
	//	for (colNumber = 0; colNumber < m_numCols; colNumber++)
	//	{
	//		if (!_wcsicmp(colName.c_str(), m_colDefs[colNumber].m_colName))
	//			break;
	//	}

	//	if (colNumber < m_numCols)
	//	{
	//		m_colDefs[colNumber].m_null = set;
	//		if (set)  // Blank out the values in the member variable
	//			ClearMemberVar((UWORD)colNumber, false);  // Must call with false here, or infinite recursion will happen

	//		setCbValueForColumn(colNumber);

	//		return true;
	//	}
	//	else
	//		return false;

	//}  // wxDbTable::SetColNull()


	///********** wxDbTable::GetNewCursor() **********/
	//HSTMT *Table::GetNewCursor(bool setCursor, bool bindColumns)
	//{
	//	HSTMT *newHSTMT = new HSTMT;
	//	exASSERT(newHSTMT);
	//	if (!newHSTMT)
	//		return(0);

	//	if (SQLAllocStmt(m_hdbc, newHSTMT) != SQL_SUCCESS)
	//	{
	//		m_pDb->DispAllErrors(NULL, m_hdbc);
	//		delete newHSTMT;
	//		return(0);
	//	}

	//	if (SQLSetStmtOption(*newHSTMT, SQL_CURSOR_TYPE, m_cursorType) != SQL_SUCCESS)
	//	{
	//		m_pDb->DispAllErrors(NULL, m_hdbc, *newHSTMT);
	//		delete newHSTMT;
	//		return(0);
	//	}

	//	if (bindColumns)
	//	{
	//		if (!bindCols(*newHSTMT))
	//		{
	//			delete newHSTMT;
	//			return(0);
	//		}
	//	}

	//	if (setCursor)
	//		SetCursor(newHSTMT);

	//	return(newHSTMT);

	//}   // wxDbTable::GetNewCursor()


	///********** wxDbTable::DeleteCursor() **********/
	//bool Table::DeleteCursor(SQLHSTMT *hstmtDel)
	//{
	//	bool result = true;

	//	if (!hstmtDel)  // Cursor already deleted
	//		return(result);

	//	/*
	//	ODBC 3.0 says to use this form
	//	if (SQLFreeHandle(*hstmtDel, SQL_DROP) != SQL_SUCCESS)

	//	*/
	//	if (SQLFreeStmt(*hstmtDel, SQL_DROP) != SQL_SUCCESS)
	//	{
	//		m_pDb->DispAllErrors(NULL, m_hdbc);
	//		result = false;
	//	}

	//	delete hstmtDel;

	//	return(result);

	//}  // wxDbTable::DeleteCursor()



	//wxVariant wxDbTable::GetColumn(const int colNumber) const
	//{
	//    wxVariant val;
	//    if ((colNumber < m_numCols) && (!IsColNull((UWORD)colNumber)))
	//    {
	//        switch (colDefs[colNumber].SqlCtype)
	//        {
	//#if wxUSE_UNICODE
	//    #if defined(SQL_WCHAR)
	//            case SQL_WCHAR:
	//    #endif
	//    #if defined(SQL_WVARCHAR)
	//            case SQL_WVARCHAR:
	//    #endif
	//#endif
	//            case SQL_CHAR:
	//            case SQL_VARCHAR:
	//                val = (wchar_t *)(colDefs[colNumber].PtrDataObj);
	//                break;
	//            case SQL_C_LONG:
	//            case SQL_C_SLONG:
	//                val = *(long *)(colDefs[colNumber].PtrDataObj);
	//                break;
	//            case SQL_C_SHORT:
	//            case SQL_C_SSHORT:
	//                val = (long int )(*(short *)(colDefs[colNumber].PtrDataObj));
	//                break;
	//            case SQL_C_ULONG:
	//                val = (long)(*(unsigned long *)(colDefs[colNumber].PtrDataObj));
	//                break;
	//            case SQL_C_TINYINT:
	//                val = (long)(*(wchar_t *)(colDefs[colNumber].PtrDataObj));
	//                break;
	//            case SQL_C_UTINYINT:
	//                val = (long)(*(wchar_t *)(colDefs[colNumber].PtrDataObj));
	//                break;
	//            case SQL_C_USHORT:
	//                val = (long)(*(UWORD *)(colDefs[colNumber].PtrDataObj));
	//                break;
	//            case SQL_C_DATE:
	//                val = (DATE_STRUCT *)(colDefs[colNumber].PtrDataObj);
	//                break;
	//            case SQL_C_TIME:
	//                val = (TIME_STRUCT *)(colDefs[colNumber].PtrDataObj);
	//                break;
	//            case SQL_C_TIMESTAMP:
	//                val = (TIMESTAMP_STRUCT *)(colDefs[colNumber].PtrDataObj);
	//                break;
	//            case SQL_C_DOUBLE:
	//                val = *(double *)(colDefs[colNumber].PtrDataObj);
	//                break;
	//            default:
	//                assert(0);
	//        }
	//    }
	//    return val;
	//}  // wxDbTable::GetCol()


	//void wxDbTable::SetColumn(const int colNumber, const wxVariant val)
	//{
	//    //FIXME: Add proper wxDateTime support to wxVariant..
	//    wxDateTime dateval;
	//
	//    SetColNull((UWORD)colNumber, val.IsNull());
	//
	//    if (!val.IsNull())
	//    {
	//        if ((colDefs[colNumber].SqlCtype == SQL_C_DATE)
	//            || (colDefs[colNumber].SqlCtype == SQL_C_TIME)
	//            || (colDefs[colNumber].SqlCtype == SQL_C_TIMESTAMP))
	//        {
	//            //Returns null if invalid!
	//            if (!dateval.ParseDate(val.GetString()))
	//                SetColNull((UWORD)colNumber, true);
	//        }
	//
	//        switch (colDefs[colNumber].SqlCtype)
	//        {
	//#if wxUSE_UNICODE
	//    #if defined(SQL_WCHAR)
	//            case SQL_WCHAR:
	//    #endif
	//    #if defined(SQL_WVARCHAR)
	//            case SQL_WVARCHAR:
	//    #endif
	//#endif
	//            case SQL_CHAR:
	//            case SQL_VARCHAR:
	//                csstrncpyt((wchar_t *)(colDefs[colNumber].PtrDataObj),
	//                           val.GetString().c_str(),
	//                           colDefs[colNumber].SzDataObj-1);  //TODO: glt ??? * sizeof(wchar_t) ???
	//                break;
	//            case SQL_C_LONG:
	//            case SQL_C_SLONG:
	//                *(long *)(colDefs[colNumber].PtrDataObj) = val;
	//                break;
	//            case SQL_C_SHORT:
	//            case SQL_C_SSHORT:
	//                *(short *)(colDefs[colNumber].PtrDataObj) = (short)val.GetLong();
	//                break;
	//            case SQL_C_ULONG:
	//                *(unsigned long *)(colDefs[colNumber].PtrDataObj) = val.GetLong();
	//                break;
	//            case SQL_C_TINYINT:
	//                *(wchar_t *)(colDefs[colNumber].PtrDataObj) = val.GetChar();
	//                break;
	//            case SQL_C_UTINYINT:
	//                *(wchar_t *)(colDefs[colNumber].PtrDataObj) = val.GetChar();
	//                break;
	//            case SQL_C_USHORT:
	//                *(unsigned short *)(colDefs[colNumber].PtrDataObj) = (unsigned short)val.GetLong();
	//                break;
	//            //FIXME: Add proper wxDateTime support to wxVariant..
	//            case SQL_C_DATE:
	//                {
	//                    DATE_STRUCT *dataptr =
	//                        (DATE_STRUCT *)colDefs[colNumber].PtrDataObj;
	//
	//                    dataptr->year   = (SWORD)dateval.GetYear();
	//                    dataptr->month  = (UWORD)(dateval.GetMonth()+1);
	//                    dataptr->day    = (UWORD)dateval.GetDay();
	//                }
	//                break;
	//            case SQL_C_TIME:
	//                {
	//                    TIME_STRUCT *dataptr =
	//                        (TIME_STRUCT *)colDefs[colNumber].PtrDataObj;
	//
	//                    dataptr->hour   = dateval.GetHour();
	//                    dataptr->minute = dateval.GetMinute();
	//                    dataptr->second = dateval.GetSecond();
	//                }
	//                break;
	//            case SQL_C_TIMESTAMP:
	//                {
	//                    TIMESTAMP_STRUCT *dataptr =
	//                        (TIMESTAMP_STRUCT *)colDefs[colNumber].PtrDataObj;
	//                    dataptr->year   = (SWORD)dateval.GetYear();
	//                    dataptr->month  = (UWORD)(dateval.GetMonth()+1);
	//                    dataptr->day    = (UWORD)dateval.GetDay();
	//
	//                    dataptr->hour   = dateval.GetHour();
	//                    dataptr->minute = dateval.GetMinute();
	//                    dataptr->second = dateval.GetSecond();
	//                }
	//                break;
	//            case SQL_C_DOUBLE:
	//                *(double *)(colDefs[colNumber].PtrDataObj) = val;
	//                break;
	//            default:
	//                assert(0);
	//        }  // switch
	//    }  // if (!val.IsNull())
	//}  // wxDbTable::SetCol()

}