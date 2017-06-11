/*!
* \file Table.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 25.07.2014
* \brief Source file for the Table class and its helpers.
* \copyright GNU Lesser General Public License Version 3
*
* Header file for the Table class and its helpers.
*/

// Own header
#include "Table.h"

// Same component headers
#include "Database.h"
#include "Environment.h"
#include "Exception.h"
#include "Sql2BufferTypeMap.h"
#include "SqlStatementCloser.h"
#include "ColumnBufferVisitors.h"
#include "ExecutableStatement.h"
#include "DatabaseCatalog.h"
#include "Sql2StringHelper.h"

// Other headers
// Debug
#include "DebugNew.h"

using namespace std;

namespace exodbc
{
	// Construction
	// ------------
	Table::Table() noexcept
		: m_autoCreatedColumns(false)
		, m_haveTableInfo(false)
		, m_pDb(NULL)
		, m_pSql2BufferTypeMap(NULL)
		, m_isOpen(false)
		, m_tableAccessFlags(TableAccessFlag::AF_NONE)
		, m_openFlags(TableOpenFlag::TOF_NONE)
	{ }


	Table::Table(ConstDatabasePtr pDb, TableAccessFlags afs, const std::string& tableName, const std::string& schemaName /* = u8"" */, const std::string& catalogName /* = u8"" */, const std::string& tableType /* = u8"" */)
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
		
		// Remember db created from
		exASSERT(m_pDb == NULL);
		m_pDb = pDb;

		// tableinfo passed
		m_haveTableInfo = true;
		m_tableInfo = tableInfo;

		// remember buffertype map and acess flags
		m_pSql2BufferTypeMap = m_pDb->GetSql2BufferTypeMap();
		SetAccessFlags(afs);
	}


	void Table::Init(ConstDatabasePtr pDb, TableAccessFlags afs, const std::string& tableName, const std::string& schemaName /* = u8"" */, const std::string& catalogName /* = u8"" */, const std::string& tableType /* = u8"" */)
	{
		exASSERT(pDb);
		exASSERT(pDb->IsOpen());

		// Remember db created from
		exASSERT(m_pDb == NULL);
		m_pDb = pDb;

		// no tableinfo passed
		m_haveTableInfo = false;

		// but table search params
		m_initialTableName = tableName;
		m_initialSchemaName = schemaName;
		m_initialCatalogName = catalogName;
		m_initialTypeName = tableType;

		// remember buffertype map, set AccessFlags
		m_pSql2BufferTypeMap = m_pDb->GetSql2BufferTypeMap();
		SetAccessFlags(afs);
	}


	void Table::AllocateStatements(bool forwardOnlyCursors)
	{
		exASSERT(!IsOpen());
		exASSERT(m_pDb->IsOpen());

		ConstSqlDbcHandlePtr pHDbc = m_pDb->GetSqlDbcHandle();

		// Allocate handles needed, on failure try to deallocate everything
		try
		{
			if (TestAccessFlag(TableAccessFlag::AF_SELECT_WHERE) || TestAccessFlag(TableAccessFlag::AF_SELECT_PK))
			{
				m_execStmtSelect.Init(m_pDb, forwardOnlyCursors);
			}
			if (TestAccessFlag(TableAccessFlag::AF_COUNT_WHERE))
			{
				// note: The count statements never needs scrollable cursors. If we enable them and then execute a
				// SELECT COUNT, ms sql server will report a warning saying 'Cursor type changed'.
				// and the inserts and updates do not need to be scrollable.
				m_execStmtCountWhere.Init(m_pDb, true);
				// Create the buffer required for counts
				m_pSelectCountResultBuffer = UBigIntColumnBuffer::Create(u8"", SQL_UNKNOWN_TYPE, ColumnFlag::CF_SELECT);
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
			m_execStmtCountWhere.Reset();
			m_execStmtSelect.Reset();
			m_execStmtInsert.Reset();
			m_execStmtUpdatePk.Reset();
			m_execStmtDeletePk.Reset();

			// rethrow
			throw;
		}
	}


	std::vector<ColumnBufferPtrVariant> Table::CreateAutoColumnBufferPtrs(bool skipUnsupportedColumns, bool setAsTableColumns, bool queryPrimaryKeys)
	{
		exASSERT(m_pDb->IsOpen());

		const TableInfo& tableInfo = ReadTableInfo();

		// Query Columns and create SqlCBuffers
		DatabaseCatalogPtr pDbCat = m_pDb->GetDbCatalog();
		ColumnInfoVector columnInfos = pDbCat->ReadColumnInfo(tableInfo);
		exASSERT(columnInfos.size() <= SHRT_MAX);
		SQLSMALLINT numCols = (SQLSMALLINT)columnInfos.size();
		if (numCols == 0)
		{
			Exception ex((boost::format(u8"No columns found for table '%s'") % tableInfo.GetQueryName()).str());
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}
		// Derive the ColumnFlags from the TableAccessFlags
		ColumnFlags flags;
		if (TestAccessFlag(TableAccessFlag::AF_SELECT_WHERE) || TestAccessFlag(TableAccessFlag::AF_SELECT_PK))
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
				if (!colInfo.IsIsNullableNull() && boost::algorithm::iequals(colInfo.GetIsNullable(), u8"YES"))
				{
					flags.Set(ColumnFlag::CF_NULLABLE);
				}
				std::shared_ptr<ColumnFlags> pColumnFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), columnPtrVariant);
				pColumnFlags->Set(flags);
				std::shared_ptr<ColumnProperties> pExtendedProps = boost::apply_visitor(ColumnPropertiesPtrVisitor(), columnPtrVariant);
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
			catch (const NotSupportedException& nse)
			{
				if (skipUnsupportedColumns)
				{
					// Ignore unsupported column. (note: If it has thrown from the constructor, memory is already deleted)
					LOG_WARNING(boost::str(boost::format(u8"Failed to create ColumnBuffer for column '%s': %s") % colInfo.GetQueryName() % nse.ToString()));
					continue;
				}
				else
				{
					// rethrow
					throw;
				}
			}
		}

		// Prepare a map for later storing or updating with pk-info
		// \todo: why start with a vector.. use a map from begin
		ColumnBufferPtrVariantMap columnMap;
		for (size_t i = 0; i < columns.size(); ++i)
		{
			columnMap[(SQLUSMALLINT)i] = columns[i];
		}

		if (queryPrimaryKeys)
		{
			QueryPrimaryKeysAndUpdateColumns(columnMap);
		}

		// Do this as last step, it cannot fail but it will modify this object
		if (setAsTableColumns)
		{
			m_columns = columnMap;
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


	const TableInfo& Table::ReadTableInfo()
	{
		if (!m_haveTableInfo)
		{
			DatabaseCatalogPtr pDbCatalog = m_pDb->GetDbCatalog();
			m_tableInfo = pDbCatalog->FindOneTable(m_initialTableName, m_initialSchemaName, m_initialCatalogName, m_initialTypeName);
			m_haveTableInfo = true;
		}

		return m_tableInfo;
	}


	const TableInfo& Table::GetTableInfo() const
	{
		exASSERT(m_haveTableInfo);

		return m_tableInfo;
	}


	void Table::FreeStatements()
	{
		// Do NOT check for IsOpen() here. If Open() fails it will call FreeStatements to do its cleanup

		m_pSelectCountResultBuffer.reset();

		m_execStmtCountWhere.Reset();
		m_execStmtSelect.Reset();
		m_execStmtInsert.Reset();
		m_execStmtUpdatePk.Reset();
		m_execStmtDeletePk.Reset();
	}


	std::string Table::BuildSelectFieldsStatement() const
	{
		exASSERT(m_columns.size() > 0);

		std::string fields = u8"";
		ColumnBufferPtrVariantMap::const_iterator it = m_columns.begin();
		while (it != m_columns.end())
		{
			ColumnBufferPtrVariant var = it->second;
			ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), var);
			if (pFlags->Test(ColumnFlag::CF_SELECT))
			{
				std::string queryName = boost::apply_visitor(QueryNameVisitor(), var);
				fields += queryName;
				fields += u8", ";
			}
			++it;
		}
		boost::algorithm::erase_last(fields, u8", ");

		return fields;
	}


	void Table::BindDeletePkParameters()
	{
		exASSERT(!m_columns.empty());
		exASSERT(TestAccessFlag(TableAccessFlag::AF_DELETE_PK));
		exASSERT(m_execStmtDeletePk.IsInitialized());

		// Build a statement with parameter-markers
		vector<ColumnBufferPtrVariant> whereParamsToBind;
		string whereMarkers;
		auto it = m_columns.begin();
		while (it != m_columns.end())
		{
			ColumnBufferPtrVariant pVar = it->second;
			ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), pVar);
			if (pFlags->Test(ColumnFlag::CF_PRIMARY_KEY))
			{
				whereMarkers += boost::apply_visitor(QueryNameVisitor(), pVar);
				whereMarkers += u8" = ?, ";
				whereParamsToBind.push_back(pVar);
			}
			++it;
		}
		boost::erase_last(whereMarkers, u8", ");
		string stmt = boost::str(boost::format(u8"DELETE FROM %s WHERE %s") % m_tableInfo.GetQueryName() % whereMarkers);

		// check that we actually have some columns to update and some for the where part
		exASSERT_MSG(!whereParamsToBind.empty(), u8"No Columns usable to construct WHERE statement");

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
		string setMarkers;
		string whereMarkers;
		auto it = m_columns.begin();
		while (it != m_columns.end())
		{
			ColumnBufferPtrVariant pVar = it->second;
			ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), pVar);
			if (pFlags->Test(ColumnFlag::CF_PRIMARY_KEY))
			{
				whereMarkers += boost::apply_visitor(QueryNameVisitor(), pVar);
				whereMarkers += u8" = ?, ";
				whereParamsToBind.push_back(pVar);
			}
			else if (pFlags->Test(ColumnFlag::CF_UPDATE))
			{				
				setMarkers += boost::apply_visitor(QueryNameVisitor(), pVar);
				setMarkers += u8" = ?, ";
				setParamsToBind.push_back(pVar);
			}
			++it;
		}
		boost::erase_last(setMarkers, u8", ");
		boost::erase_last(whereMarkers, u8", ");
		string stmt = boost::str(boost::format(u8"UPDATE %s SET %s WHERE %s") % m_tableInfo.GetQueryName() % setMarkers % whereMarkers);

		// check that we actually have some columns to update and some for the where part
		exASSERT_MSG(!setParamsToBind.empty(), u8"No Columns flaged for UPDATEing");
		exASSERT_MSG(!whereParamsToBind.empty(), u8"No Columns usable to construct WHERE statement");

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
		string markers;
		string fields;
		auto it = m_columns.begin();
		while (it != m_columns.end())
		{
			ColumnBufferPtrVariant pVar = it->second;
			ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), pVar);
			if (pFlags->Test(ColumnFlag::CF_INSERT))
			{
				fields+= boost::apply_visitor(QueryNameVisitor(), pVar);
				fields += u8", ";
				markers+= u8"?, ";
				colsToBind.push_back(pVar);
			}
			++it;
		}
		boost::erase_last(fields, u8", ");
		boost::erase_last(markers, u8", ");
		string stmt = boost::str(boost::format(u8"INSERT INTO %s (%s) VALUES(%s)") % m_tableInfo.GetQueryName() % fields % markers);

		// check that we actually have some column to insert
		exASSERT_MSG( ! colsToBind.empty(), u8"No Columns flaged for INSERTing");

		// .. and prepare stmt
		m_execStmtInsert.Prepare(stmt);

		// and binding of columns... we must do that after calling Prepare - or
		// not use SqlDescribeParam during Bind
		for (size_t i = 0; i < colsToBind.size(); ++i)
		{
			m_execStmtInsert.BindParameter(colsToBind[i], (SQLSMALLINT)(i + 1));
		}
	}


	void Table::BindSelectPkParameters()
	{
		exASSERT(!m_columns.empty());
		exASSERT(TestAccessFlag(TableAccessFlag::AF_SELECT_PK));
		exASSERT(m_execStmtSelect.IsInitialized());

		// determine a where clause 
		vector<ColumnBufferPtrVariant> whereParamsToBind;
		string whereMarkers;
		auto it = m_columns.begin();
		while (it != m_columns.end())
		{
			ColumnBufferPtrVariant pVar = it->second;
			ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), pVar);
			if (pFlags->Test(ColumnFlag::CF_PRIMARY_KEY))
			{
				whereMarkers += boost::apply_visitor(QueryNameVisitor(), pVar);
				whereMarkers += u8" = ?, ";
				whereParamsToBind.push_back(pVar);
			}
			++it;
		}
		boost::erase_last(whereMarkers, u8", ");

		// check that we actually have some columns for the where part
		exASSERT_MSG(!whereParamsToBind.empty(), u8"No Columns usable to construct WHERE statement");

		// .. and prepare stmt
		string stmt = boost::str(boost::format(u8"SELECT %s FROM %s WHERE %s") % BuildSelectFieldsStatement() % m_tableInfo.GetQueryName() % whereMarkers);
		m_execStmtSelect.Prepare(stmt);

		// and binding of columns... we must do that after calling Prepare - or
		// not use SqlDescribeParam during Bind
		SQLSMALLINT paramNr = 1;
		for (auto it = whereParamsToBind.begin(); it != whereParamsToBind.end(); ++it)
		{
			m_execStmtSelect.BindParameter(*it, paramNr);
			++paramNr;
		}
	}


	bool Table::ColumnBufferExists(SQLSMALLINT columnIndex) const noexcept
	{
		ColumnBufferPtrVariantMap::const_iterator it = m_columns.find(columnIndex);
		return it != m_columns.end();
	}


	const ColumnBufferPtrVariant& Table::GetColumnBufferPtrVariant(SQLSMALLINT columnIndex) const
	{
		ColumnBufferPtrVariantMap::const_iterator it = m_columns.find(columnIndex);
		if (it == m_columns.end())
		{
			IllegalArgumentException ex(boost::str(boost::format(u8"ColumnIndex %d is not a zero-based bound or manually defined ColumnBuffer") % columnIndex));
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
			std::string queryName = boost::apply_visitor(QueryNameVisitor(), columnPtrVariant);
			NullValueException ex(queryName);
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}
		return columnPtrVariant;
	}


	void Table::QueryPrimaryKeysAndUpdateColumns(const ColumnBufferPtrVariantMap& columns) const
	{
		exASSERT(m_haveTableInfo);
		exASSERT(m_pDb);
		
		DatabaseCatalogPtr pDbCat = m_pDb->GetDbCatalog();
		PrimaryKeyInfoVector keys = pDbCat->ReadPrimaryKeyInfo(m_tableInfo);
		for (auto itKey = keys.begin(); itKey != keys.end(); ++itKey)
		{
			const PrimaryKeyInfo& keyInfo = *itKey;
			SQLUSMALLINT colIndex = GetColumnBufferIndex(keyInfo.GetQueryName(), columns);
			auto itCol = columns.find(colIndex);
			exASSERT(itCol != columns.end());
			ColumnBufferPtrVariant column = itCol->second;
			ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), column);
			pFlags->Set(ColumnFlag::CF_PRIMARY_KEY);
		}
	}


	void Table::CheckColumnFlags() const
	{
		for (auto it = m_columns.begin(); it != m_columns.end(); ++it)
		{
			ColumnBufferPtrVariant columnBuffer = it->second;
			const std::string& queryName = boost::apply_visitor(QueryNameVisitor(), columnBuffer);
			ColumnFlagsPtr pColumnFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), columnBuffer);
			if (pColumnFlags->Test(ColumnFlag::CF_SELECT) && !(TestAccessFlag(TableAccessFlag::AF_SELECT_PK) || TestAccessFlag(TableAccessFlag::AF_SELECT_WHERE)))
			{
				Exception ex(boost::str(boost::format(u8"Defined Column %s (%d) has ColumnFlag CF_SELECT set, but the AccessFlag AF_SELECT_PK or AF_SELECT_WHERE is not set on the table %s") % queryName % it->first %m_tableInfo.GetQueryName()));
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
			if (pColumnFlags->Test(ColumnFlag::CF_INSERT) && !TestAccessFlag(TableAccessFlag::AF_INSERT))
			{
				Exception ex(boost::str(boost::format(u8"Defined Column %s (%d) has ColumnFlag CF_INSERT set, but the AccessFlag AF_INSERT is not set on the table %s") % queryName % it->first %m_tableInfo.GetQueryName()));
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
			if (pColumnFlags->Test(ColumnFlag::CF_UPDATE) && !(TestAccessFlag(TableAccessFlag::AF_UPDATE_PK) || TestAccessFlag(TableAccessFlag::AF_UPDATE_WHERE)))
			{
				Exception ex(boost::str(boost::format(u8"Defined Column %s (%d) has ColumnFlag CF_UPDATE set, but the AccessFlag AF_UPDATE_PK or AF_UPDATE_WHERE is not set on the table %s") % queryName % it->first %m_tableInfo.GetQueryName()));
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
			const std::string& queryName = boost::apply_visitor(QueryNameVisitor(), columnBuffer);
			ColumnFlagsPtr pColumnFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), columnBuffer);
			SQLSMALLINT sqlType = boost::apply_visitor(SqlTypeVisitor(), columnBuffer);
			bool remove = false;
			if (!m_pDb->IsSqlTypeSupported(sqlType))
			{
				if (TestOpenFlag(TableOpenFlag::TOF_SKIP_UNSUPPORTED_COLUMNS))
				{
					LOG_WARNING(boost::str(boost::format(u8"Defined Column %s (%d) has SQL Type %s (%d) set, but the Database did not report this type as a supported SQL Type. The Column is skipped due to the flag TOF_SKIP_UNSUPPORTED_COLUMNS") % queryName % it->first % Sql2StringHelper::SqlType2s(sqlType) % sqlType));
					remove = true;
				}			
				else
				{
					Exception ex(boost::str(boost::format(u8"Defined Column %s (%d) has SQL Type %s (%d) set, but the Database did not report this type as a supported SQL Type.") % queryName % it->first % Sql2StringHelper::SqlType2s(sqlType) % sqlType));
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


	std::set<SQLUSMALLINT> Table::GetColumnBufferIndexes() const noexcept
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


	SQLUSMALLINT Table::GetColumnBufferIndex(const std::string& columnQueryName, bool caseSensitive /* = true */) const
	{
		return GetColumnBufferIndex(columnQueryName, m_columns, caseSensitive);
	}


	SQLUSMALLINT Table::GetColumnBufferIndex(const std::string& columnQueryName, const ColumnBufferPtrVariantMap& columns, bool caseSensitive /* = true */) const
	{
		ColumnBufferPtrVariantMap::const_iterator it = columns.begin();
		while (it != columns.end())
		{
			const ColumnBufferPtrVariant& column = it->second;
			std::string queryName = boost::apply_visitor(QueryNameVisitor(), column);

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

		NotFoundException nfe(boost::str(boost::format(u8"No ColumnBuffer found with QueryName '%s'") % columnQueryName));
		SET_EXCEPTION_SOURCE(nfe);
		throw nfe;
	}


	SQLUBIGINT Table::Count(const std::string& whereStatement)
	{
		exASSERT(IsOpen());
		exASSERT(m_tableAccessFlags.Test(TableAccessFlag::AF_COUNT_WHERE));
		exASSERT(m_pSelectCountResultBuffer);

		// Prepare the sql to be executed on our internal ExecutableStatement
		std::string sqlstmt;
		if ( ! whereStatement.empty())
		{
			sqlstmt = (boost::format(u8"SELECT COUNT(*) FROM %s WHERE %s") % m_tableInfo.GetQueryName() % whereStatement).str();
		}
		else
		{
			sqlstmt = (boost::format(u8"SELECT COUNT(*) FROM %s") % m_tableInfo.GetQueryName()).str();
		}

		m_execStmtCountWhere.ExecuteDirect(sqlstmt);
		exASSERT(m_execStmtCountWhere.SelectNext());

		m_execStmtCountWhere.SelectClose();

		return *m_pSelectCountResultBuffer;
	}


	void Table::Select(const std::string& whereStatement /* = u8"" */, const std::string& orderStatement /* = u8"" */)
	{
		exASSERT(IsOpen());

		stringstream ws;
		ws << u8"SELECT " << BuildSelectFieldsStatement() << u8" FROM " << m_tableInfo.GetQueryName();

		if (!whereStatement.empty())
		{
			ws << u8" WHERE " << whereStatement;
		}

		if (!orderStatement.empty())
		{
			ws << u8" ORDER BY " << orderStatement;
		}

		SelectBySqlStmt(ws.str());
	}


	void Table::SelectByPkValues()
	{
		exASSERT(IsOpen());

		m_execStmtSelect.ExecutePrepared();
	}


	void Table::SelectBySqlStmt(const std::string& sqlStmt)
	{
		exASSERT(IsOpen());
		exASSERT(m_tableAccessFlags.Test(TableAccessFlag::AF_SELECT_WHERE));
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


	void Table::DeleteByPkValues(bool failOnNoData /* = true */)
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


	void Table::Delete(const std::string& where, bool failOnNoData /* = true */) const
	{
		exASSERT(!m_columns.empty());
		exASSERT(TestAccessFlag(TableAccessFlag::AF_DELETE_WHERE));
		exASSERT(!where.empty());

		ExecutableStatement execStmtDelete;
		execStmtDelete.Init(m_pDb, true);

		// Build a statement that we can directly execute
		string stmt = boost::str(boost::format(u8"DELETE FROM %s WHERE %s") % m_tableInfo.GetQueryName() % where);
		
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


	void Table::UpdateByPkValues()
	{
		exASSERT(IsOpen());
		exASSERT(TestAccessFlag(TableAccessFlag::AF_UPDATE_PK));
		m_execStmtUpdatePk.ExecutePrepared();
	}


	void Table::Update(const std::string& where)
	{
		exASSERT(!m_columns.empty());
		exASSERT(TestAccessFlag(TableAccessFlag::AF_UPDATE_WHERE));
		exASSERT(!where.empty());

		ExecutableStatement execStmtUpdate;
		execStmtUpdate.Init(m_pDb, true);

		// Build a statement with parameter-markers
		vector<ColumnBufferPtrVariant> setParamsToBind;
		string setMarkers;
		auto it = m_columns.begin();
		while (it != m_columns.end())
		{
			ColumnBufferPtrVariant pVar = it->second;
			ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), pVar);
			if (pFlags->Test(ColumnFlag::CF_UPDATE))
			{
				setMarkers += boost::apply_visitor(QueryNameVisitor(), pVar);
				setMarkers += u8" = ?, ";
				setParamsToBind.push_back(pVar);
			}
			++it;
		}
		boost::erase_last(setMarkers, u8", ");
		string stmt = boost::str(boost::format(u8"UPDATE %s SET %s WHERE %s") % m_tableInfo.GetQueryName() % setMarkers % where);

		// check that we actually have some columns to update and some for the where part
		exASSERT_MSG(!setParamsToBind.empty(), u8"No Columns flaged for UPDATEing");

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


	void Table::SetColumnNull(SQLSMALLINT columnIndex) const
	{
		ColumnBufferPtrVariant var = GetColumnBufferPtrVariant(columnIndex);
		ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), var);
		exASSERT(pFlags->Test(ColumnFlag::CF_NULLABLE));
		LengthIndicatorPtr pCb = boost::apply_visitor(LengthIndicatorPtrVisitor(), var);
		pCb->SetNull();
	}


	bool Table::IsColumnNull(SQLSMALLINT columnIndex) const
	{
		ColumnBufferPtrVariant var = GetColumnBufferPtrVariant(columnIndex);
		LengthIndicatorPtr pCb = boost::apply_visitor(LengthIndicatorPtrVisitor(), var);
		return pCb->IsNull();
	}


	bool Table::IsColumnNullable(SQLSMALLINT columnIndex) const
	{
		ColumnBufferPtrVariant var = GetColumnBufferPtrVariant(columnIndex);
		ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), var);
		return pFlags->Test(ColumnFlag::CF_NULLABLE);
	}


	void Table::ClearColumns()
	{
		exASSERT(!IsOpen());
		m_columns.clear();
	}


	void Table::SetColumn(SQLUSMALLINT columnIndex, ColumnBufferPtrVariant column)
	{
		exASSERT(!IsOpen());
		exASSERT(columnIndex >= 0);
		exASSERT(m_columns.find(columnIndex) == m_columns.end());

		// test if we have a non-empty query name
		const std::string& queryName = boost::apply_visitor(QueryNameVisitor(), column);
		exASSERT(!queryName.empty());

		// okay, remember the passed variant
		m_columns[columnIndex] = column;
	}


	void Table::SetColumn(SQLUSMALLINT columnIndex, const std::string& queryName, SQLSMALLINT sqlType, SQLPOINTER pBuffer, SQLSMALLINT sqlCType, SQLLEN bufferSize, ColumnFlags flags, SQLINTEGER columnSize /* = 0 */, SQLSMALLINT decimalDigits /* = 0 */)
	{
		SqlCPointerBufferPtr pColumn = std::make_shared<SqlCPointerBuffer>(queryName, sqlType, pBuffer, sqlCType, bufferSize, flags, columnSize, decimalDigits);
		SetColumn(columnIndex, pColumn);
	}


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


	ConstSql2BufferTypeMapPtr Table::GetSql2BufferTypeMap() const
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

		// Set TOF_DO_NOT_QUERY_PRIMARY_KEYS this flag for Access and Excel, driver does not support SQLPrimaryKeys
		if (m_pDb->GetDbms() == DatabaseProduct::ACCESS || m_pDb->GetDbms() == DatabaseProduct::EXCEL)
		{
			m_openFlags.Set(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS);
		}

		// If the Database does not support scrollable cursors, do not even try to enable them
		if ( ! m_pDb->SupportsScrollableCursor())
		{
			LOG_DEBUG(boost::str(boost::format(u8"Enabling flag TOF_FORWARD_ONLY_CURSORS, because Database does not suppport scrollable cursors")));
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
				ReadTableInfo();
				searchedTable = true;
			}

			// If we are asked to check existence and have not just proved we exist just find a table
			// Search using the info from the now available m_tableInfo
			if (TestOpenFlag(TableOpenFlag::TOF_CHECK_EXISTANCE) && !searchedTable)
			{
				// Will throw if not one is found
				std::string catalogName = m_tableInfo.GetCatalog();
				std::string typeName = m_tableInfo.GetType();
				if (m_pDb->GetDbms() == DatabaseProduct::EXCEL)
				{
					// workaround for #111
					catalogName = u8"";
					typeName = u8"";
				}
				DatabaseCatalogPtr pDbCatalog = m_pDb->GetDbCatalog();
				pDbCatalog->FindOneTable(m_tableInfo.GetName(), m_tableInfo.GetSchema(), catalogName, typeName);
				searchedTable = true;
			}

			// If no columns have been set so far and we need any access, try to create ColumnBuffers automatically
			// Do not create any buffers if no flags are set, or count where is the only flag set. all other ops need columns
			if (m_columns.empty() && !(m_tableAccessFlags == TableAccessFlag::AF_NONE || m_tableAccessFlags == TableAccessFlag::AF_COUNT_WHERE))
			{
				CreateAutoColumnBufferPtrs(TestOpenFlag(TableOpenFlag::TOF_SKIP_UNSUPPORTED_COLUMNS), true, false);
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
						THROW_WITH_SOURCE(IllegalArgumentException, boost::str(boost::format(u8"No ColumnBuffer was found for a manually defined primary key column index (index = %d)") % *it));
					}
					ColumnFlagsPtr pFlags = boost::apply_visitor(ColumnFlagsPtrVisitor(), itCols->second);
					pFlags->Set(ColumnFlag::CF_PRIMARY_KEY);
				}
			}

			// If we need the primary keys and are allowed to query them do that now
			if ((TestAccessFlag(TableAccessFlag::AF_UPDATE_PK) 
				|| TestAccessFlag(TableAccessFlag::AF_DELETE_PK)
				|| TestAccessFlag(TableAccessFlag::AF_SELECT_PK)) 
				&& !TestOpenFlag(TableOpenFlag::TOF_DO_NOT_QUERY_PRIMARY_KEYS))
			{
				QueryPrimaryKeysAndUpdateColumns(m_columns);
			}

			// check that column flags match with table access flags
			CheckColumnFlags();

			// check that data types are supported if columns have been defined manually
			if (!m_autoCreatedColumns && !TestOpenFlag(TableOpenFlag::TOF_IGNORE_DB_TYPE_INFOS))
			{
				CheckSqlTypes(TestOpenFlag(TableOpenFlag::TOF_SKIP_UNSUPPORTED_COLUMNS));
			}

			// Bind the Buffer for the Count operations
			if (TestAccessFlag(TableAccessFlag::AF_COUNT_WHERE))
			{
				exASSERT(m_pSelectCountResultBuffer);
				m_execStmtCountWhere.BindColumn(m_pSelectCountResultBuffer, 1);
			}

			// Bind all columns with flag CF_SELECT set to the Select-where and/or Select-pk statement:
			if (TestAccessFlag(TableAccessFlag::AF_SELECT_WHERE) || TestAccessFlag(TableAccessFlag::AF_SELECT_PK))
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
			if (TestAccessFlag(TableAccessFlag::AF_UPDATE_PK) 
				|| TestAccessFlag(TableAccessFlag::AF_DELETE_PK)
				|| TestAccessFlag(TableAccessFlag::AF_SELECT_PK))
			{
				if (GetPrimaryKeyColumnBuffers().empty())
				{
					Exception ex((boost::format(u8"Table '%s' has no primary keys, cannot open with AF_UPDATE_PK or AF_DELETE_PK") % m_tableInfo.GetQueryName()).str());
					SET_EXCEPTION_SOURCE(ex);
					throw ex;
				}
				if (TestAccessFlag(TableAccessFlag::AF_UPDATE_PK))
				{
					BindUpdatePkParameters();
				}
				if (TestAccessFlag(TableAccessFlag::AF_DELETE_PK))
				{
					BindDeletePkParameters();
				}
				if (TestAccessFlag(TableAccessFlag::AF_SELECT_PK))
				{
					BindSelectPkParameters();
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

		// remove the ColumnBuffers if we have allocated them during Open()
		if (m_autoCreatedColumns)
		{
			// Do not call ClearColumns() here, because ClearColumns asserts
			// that IsOpen() returns false.
			m_columns.clear();
		}

		// Reset all statements
		FreeStatements();

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

		m_tableAccessFlags = acs;
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
		LengthIndicatorPtr pCb = boost::apply_visitor(LengthIndicatorPtrVisitor(), var);
		pCb->SetCb(cb);
	}


	/*!
	* \brief	Checks if we can only read from this table.
	* \return	True if at least one SELECT flags is set and no update, insert or delete flags are set.
	*/
	bool Table::IsQueryOnly() const noexcept  {
		return (TestAccessFlag(TableAccessFlag::AF_SELECT_PK) || TestAccessFlag(TableAccessFlag::AF_SELECT_WHERE)) 
			&& !TestAccessFlag(TableAccessFlag::AF_UPDATE_PK) && !TestAccessFlag(TableAccessFlag::AF_UPDATE_WHERE)
			&& !TestAccessFlag(TableAccessFlag::AF_DELETE_PK) && !TestAccessFlag(TableAccessFlag::AF_DELETE_WHERE)
			&& !TestAccessFlag(TableAccessFlag::AF_INSERT);
	}
}