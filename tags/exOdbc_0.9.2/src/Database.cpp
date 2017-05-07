﻿/*!
* \file Database.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 25.07.2014
* \brief Source file for the Database class and its helpers.
* \copyright GNU Lesser General Public License Version 3
*
* Source file for the Database class and its helpers.
*/

// Own header
#include "Database.h"

// Same component headers
#include "Environment.h"
#include "Helpers.h"
#include "Table.h"
#include "Exception.h"
#include "SqlStatementCloser.h"
#include "LogManagerOdbcMacros.h"

// Other headers
// Debug
#include "DebugNew.h"

using namespace std;

namespace exodbc
{
	// Construction
	// ------------
	Database::Database() noexcept
		: m_pEnv(NULL)
		, m_pSql2BufferTypeMap(NULL)
		, m_pHDbc(std::make_shared<SqlDbcHandle>())
		, m_pHStmt(std::make_shared<SqlStmtHandle>())
		, m_pHStmtExecSql(std::make_shared<SqlStmtHandle>())
		, m_dbmsType(DatabaseProduct::UNKNOWN)
		, m_dbIsOpen(false)
		, m_dbOpenedWithConnectionString(false)
		, m_commitMode(CommitMode::UNKNOWN)
	{
	}


	Database::Database(ConstEnvironmentPtr pEnv)
		: m_pEnv(NULL)
		, m_pSql2BufferTypeMap(NULL)
		, m_pHDbc(std::make_shared<SqlDbcHandle>())
		, m_pHStmt(std::make_shared<SqlStmtHandle>())
		, m_pHStmtExecSql(std::make_shared<SqlStmtHandle>())
		, m_dbmsType(DatabaseProduct::UNKNOWN)
		, m_dbIsOpen(false)
		, m_dbOpenedWithConnectionString(false)
		, m_commitMode(CommitMode::UNKNOWN)
	{
		// Allocate the DBC-Handle and set the member m_pEnv
		Init(pEnv);
	}


	Database::Database(const Database& other)
		: m_pEnv(NULL)
		, m_pSql2BufferTypeMap(NULL)
		, m_pHDbc(std::make_shared<SqlDbcHandle>())
		, m_pHStmt(std::make_shared<SqlStmtHandle>())
		, m_pHStmtExecSql(std::make_shared<SqlStmtHandle>())
		, m_dbmsType(DatabaseProduct::UNKNOWN)
		, m_dbIsOpen(false)
		, m_dbOpenedWithConnectionString(false)
		, m_commitMode(CommitMode::UNKNOWN)
	{
		if (other.GetEnvironment() != NULL)
		{
			Init(other.GetEnvironment());
		}
	}


	// Destructor
	// -----------
	Database::~Database()
	{
		// Do not throw from here
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
	std::shared_ptr<Database> Database::Create(ConstEnvironmentPtr pEnv)
	{
		DatabasePtr pDb = std::make_shared<Database>(pEnv);
		return pDb;
	}


	void Database::Init(ConstEnvironmentPtr pEnv)
	{
		exASSERT(pEnv != NULL);
		exASSERT(pEnv->IsEnvHandleAllocated());
		exASSERT(m_pEnv == NULL);

		// Allocate connection handle from passed environment
		m_pHDbc->AllocateWithParent(pEnv->GetSqlEnvHandle());

		// success, remember environment used
		m_pEnv = pEnv;
	}


	void Database::OpenImpl()
	{
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());
		exASSERT(!m_pHStmt->IsAllocated());
		exASSERT(!m_pHStmtExecSql->IsAllocated());

		// Everything might throw, let this function free the handles in case of failure, 
		// so that we are back in the state before this function was called.
		try
		{
			// Allocate a statement handle from the database connection to be used internally and the exec-handle
			m_pHStmt->AllocateWithParent(m_pHDbc);
			m_pHStmtExecSql->AllocateWithParent(m_pHDbc);

			// Query the data source for info about itself
			m_dbInf = ReadDbInfo();

			// Check that our ODBC-Version matches
			// We do not fail, what counts for us is the version of the
			// environment.
			OdbcVersion connectionVersion = GetDriverOdbcVersion();
			OdbcVersion envVersion = m_pEnv->GetOdbcVersion();
			OdbcVersion maxSupportedVersion = GetMaxSupportedOdbcVersion();
			if (envVersion > connectionVersion)
			{
				LOG_WARNING((boost::format(u8"ODBC Version missmatch: Environment requested %d, but the driver (name: '%s' version: '%s') reported %d ('%s'). The Database ('%s') will be using %d") % (int)envVersion %m_dbInf.GetDriverName() %m_dbInf.GetDriverVersion() % (int)connectionVersion %m_dbInf.GetDriverOdbcVersion() %m_dbInf.GetDbmsName() % (int)connectionVersion).str());
			}
			if (!m_pSql2BufferTypeMap)
			{
				m_pSql2BufferTypeMap = Sql2BufferTypeMapPtr(new DefaultSql2BufferMap(maxSupportedVersion));
			}

			// Try to detect the type - this will update our internal type
			DetectDbms();

			// Set Connection Options
			SetConnectionAttributes();

			// Default to manual commit, if the Database is able to set a commit mode. Anyway read the currently active mode, we need to know that
			m_commitMode = ReadCommitMode();
			if (m_dbInf.GetSupportsTransactions() && m_commitMode != CommitMode::MANUAL)
			{
				SetCommitMode(CommitMode::MANUAL);
			}

			// Query the datatypes
			m_datatypes = ReadDataTypesInfo();

		}
		catch (const Exception& ex)
		{
			HIDE_UNUSED(ex);
			// Try to free what we've allocated and rethrow
			if (m_pHStmt->IsAllocated())
			{
				m_pHStmt->Free();
			}
			if (m_pHStmtExecSql->IsAllocated())
			{
				m_pHStmtExecSql->Free();
			}
			throw;
		}
	}


	std::string Database::Open(const std::string& inConnectStr)
	{
		return Open(inConnectStr, NULL);
	}


	std::string Database::Open(const std::string& inConnectStr, SQLHWND parentWnd)
	{
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());
		exASSERT(!IsOpen());
		exASSERT(!inConnectStr.empty());
		m_dsn = u8"";
		m_uid = u8"";
		m_authStr = u8"";
		m_inConnectionStr = inConnectStr;

		// Connect to the data source
		SQLAPICHARTYPE outConnectBuffer[DB_MAX_CONNECTSTR_LEN + 1];
		SQLSMALLINT outConnectBufferLen;

		// Note: 
		// StringLength1: [Input] Length of *InConnectionString, in characters if the string is Unicode, or bytes if string is ANSI or DBCS.
		// BufferLength: [Input] Length of the *OutConnectionString buffer, in characters.
		SQLRETURN ret = SQLDriverConnect(m_pHDbc->GetHandle(), parentWnd, 
			EXODBCSTR_TO_SQLAPICHARPTR(m_inConnectionStr),
			(SQLSMALLINT) m_inConnectionStr.length(), 
			(SQLAPICHARTYPE*) outConnectBuffer, 
			DB_MAX_CONNECTSTR_LEN, &outConnectBufferLen, parentWnd == NULL ? SQL_DRIVER_NOPROMPT : SQL_DRIVER_COMPLETE);
		THROW_IFN_SUCCEEDED(SQLDriverConnect, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle());

		outConnectBuffer[outConnectBufferLen] = 0;
//		m_outConnectionStr = SQLRESCHARCONVERT((SQLAPICHARTYPE*)outConnectBuffer);
//		m_outConnectionStr = (SQLCHAR*)outConnectBuffer;
//		m_outConnectionStr = std::string(reinterpret_cast<const char*>(outConnectBuffer));
        m_outConnectionStr = SQLAPICHARPTR_TO_EXODBCSTR(outConnectBuffer);
		m_dbOpenedWithConnectionString = true;

		// Do all the common stuff about opening
		// If we fail, disconnect
		try
		{
			OpenImpl();
		}
		catch (const Exception& ex)
		{
			HIDE_UNUSED(ex);
			// Try to disconnect from the data source
			ret = SQLDisconnect(m_pHDbc->GetHandle());
			if (!SQL_SUCCEEDED(ret))
			{
				ErrorHelper::SErrorInfoVector errs = ErrorHelper::GetAllErrors(SQL_HANDLE_DBC, m_pHDbc->GetHandle());
				ErrorHelper::SErrorInfoVector::const_iterator it;
				for (it = errs.begin(); it != errs.end(); ++it)
				{
					LOG_ERROR(boost::str(boost::format(u8"Failed to Disconnect from datasource with %s (%d): %s") % SqlReturn2s(ret) % ret % it->ToString()));
				}
			}

			// and rethrow what went wrong originally
			throw;
		}

		// Mark database as Open
		m_dbIsOpen = true;
		return m_outConnectionStr;
	}


	void Database::Open(const std::string& dsn, const std::string& uid, const std::string& authStr)
	{
		exASSERT(!IsOpen());
		exASSERT(!dsn.empty());
		exASSERT(HasConnectionHandle());

		m_dsn        = dsn;
		m_uid        = uid;
		m_authStr    = authStr;

		// Not using a connection-string
		m_inConnectionStr = u8"";
		m_outConnectionStr = u8"";

		// Connect to the data source
		SQLRETURN ret = SQLConnect(m_pHDbc->GetHandle(),
			EXODBCSTR_TO_SQLAPICHARPTR(m_dsn), SQL_NTS,
			EXODBCSTR_TO_SQLAPICHARPTR(m_uid), SQL_NTS,
			EXODBCSTR_TO_SQLAPICHARPTR(m_authStr), SQL_NTS);

		// Do not reset an eventually allocated connection handle here.
		// The destructor will reset it if one is allocated
		THROW_IFN_SUCCEEDED(SQLConnect, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle());

		// Do all the common stuff about opening
		// If we fail, disconnect
		try
		{
			OpenImpl();
		}
		catch (const Exception& ex)
		{
			HIDE_UNUSED(ex);
			// Try to disconnect from the data source
			ret = SQLDisconnect(m_pHDbc->GetHandle());
			if ( ! SQL_SUCCEEDED(ret))
			{
				ErrorHelper::SErrorInfoVector errs = ErrorHelper::GetAllErrors(SQL_HANDLE_DBC, m_pHDbc->GetHandle());
				ErrorHelper::SErrorInfoVector::const_iterator it;
				for (it = errs.begin(); it != errs.end(); ++it)
				{
					LOG_ERROR(boost::str(boost::format(u8"Failed to Disconnect from datasource with %s (%d): %s") % SqlReturn2s(ret) % ret % it->ToString()));
				}
			}

			// and rethrow what went wrong originally
			throw;
		}
		
		// Mark database as Open
		m_dbIsOpen = true;
	}


	void Database::SetTracefile(const std::string path)
	{
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		// note, from ms-doc: 
		// Character strings pointed to by the ValuePtr argument of SQLSetConnectAttr have a length of StringLength bytes.
		SQLINTEGER cb = 0;

		// windows probably wants wchars here.. (?)
#ifdef _WIN32
		std::wstring wpath = utf8ToUtf16(path);
		SQLRETURN ret = SQLSetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_TRACEFILE, (SQLPOINTER)wpath.c_str(), ((SQLINTEGER)wpath.length()) * sizeof(SQLWCHAR));
#else
		SQLRETURN ret = SQLSetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_TRACEFILE, (SQLPOINTER)path.c_str(), ((SQLINTEGER)path.length()) * sizeof(SQLCHAR));
#endif
		THROW_IFN_SUCCEEDED(SQLSetConnectAttr, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle());
	}


	std::string Database::GetTracefile() const
	{
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		// Assume some max length, querying the driver about length did not really work
		SQLINTEGER cb = 0;
		SQLINTEGER charBuffSize = MAX_PATH + 1;
		SQLINTEGER byteBuffSize = sizeof(SQLAPICHARTYPE) * charBuffSize;
		std::unique_ptr<SQLAPICHARTYPE[]> buffer(new SQLAPICHARTYPE[charBuffSize]);
		memset(buffer.get(), 0, byteBuffSize);
		SQLRETURN ret = SQLGetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_TRACEFILE, (SQLPOINTER)buffer.get(), byteBuffSize, &cb);
		THROW_IFN_SUCCEEDED(SQLGetConnectAttr, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle());
        std::string tracefile(SQLAPICHARPTR_TO_EXODBCSTR(buffer.get()));
		return tracefile;
	}


	void Database::SetTrace(bool enable)
	{
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		if (enable)
		{
			SQLRETURN ret = SQLSetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_TRACE, (SQLPOINTER)SQL_OPT_TRACE_ON, 0);
			THROW_IFN_SUCCEEDED_MSG(SQLSetConnectAttr, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle(), u8"Failed to set Attribute SQL_ATTR_TRACE to SQL_OPT_TRACE_ON");
		}
		else
		{
			SQLRETURN ret = SQLSetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_TRACE, (SQLPOINTER)SQL_OPT_TRACE_OFF, 0);
			THROW_IFN_SUCCEEDED_MSG(SQLSetConnectAttr, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle(), u8"Failed to set Attribute SQL_ATTR_TRACE to SQL_OPT_TRACE_OFF");
		}
	}


	bool Database::GetTrace() const
	{
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		SQLUINTEGER value = 0;
		SQLRETURN ret = SQLGetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_TRACE, &value, 0, 0);
		THROW_IFN_SUCCEEDED_MSG(SQLGetConnectAttr, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle(), u8"Failed to read Attribute SQL_ATTR_TRACE");

		return value == SQL_OPT_TRACE_ON;
	}


	bool Database::IsSqlTypeSupported(SQLSMALLINT sqlType) const
	{
		exASSERT(IsOpen());
		SqlTypeInfosVector::const_iterator it = m_datatypes.begin();
		while (it != m_datatypes.end())
		{
			if (it->m_sqlDataType == sqlType || it->m_sqlType == sqlType)
			{
				return true;
			}
			++it;
		}
		return false;
	}


	void Database::SetSql2BufferTypeMap(Sql2BufferTypeMapPtr pSql2BufferTypeMap) noexcept
	{
		m_pSql2BufferTypeMap = pSql2BufferTypeMap;
	}


	Sql2BufferTypeMapPtr Database::GetSql2BufferTypeMap() const
	{
		exASSERT(m_pSql2BufferTypeMap);
		return m_pSql2BufferTypeMap;
	}


	SqlTypeInfosVector Database::GetTypeInfos() const
	{
		exASSERT(IsOpen());
		return m_datatypes;
	}


	void Database::SetConnectionAttributes()
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		// but we need to have at least the connection handle
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		SQLRETURN ret = SQLSetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_TRACE, (SQLPOINTER) SQL_OPT_TRACE_OFF, 0);
		THROW_IFN_SUCCEEDED_MSG(SQLSetConnectAttr, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle(), u8"Cannot set SQL_ATTR_TRACE to SQL_OPT_TRACE_OFF");

		// Note: This is unsupported SQL_ATTR_METADATA_ID by most drivers. It should default to OFF
		//ret = SQLSetConnectAttr(m_hdbc, SQL_ATTR_METADATA_ID, (SQLPOINTER) SQL_TRUE, NULL);
		//if(ret != SQL_SUCCESS)
		//{
		//	LOG_ERROR_DBC_MSG(m_hdbc, ret, SQLSetConnectAttr, u8"Cannot set ATTR_METADATA_ID to SQL_FALSE");
		//	ok = false;
		//}
	}


	DatabaseInfo Database::ReadDbInfo()
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		DatabaseInfo dbInf;
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::StringProperty::AccessibleTables);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::StringProperty::DatabaseName);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::StringProperty::DbmsName);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::StringProperty::DbmsVersion);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::StringProperty::DriverName);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::StringProperty::DriverOdbcVersion);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::StringProperty::DriverVersion);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::StringProperty::OdbcSupportIEF);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::StringProperty::OdbcVersion);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::StringProperty::OuterJoins);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::StringProperty::ProcedureSupport);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::StringProperty::SearchPatternEscape);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::StringProperty::ServerName);

		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::USmallIntProperty::CursorCommitBehavior);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::USmallIntProperty::CursorRollbackBehavior);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::USmallIntProperty::MaxCatalogNameLen);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::USmallIntProperty::MaxColumnNameLen);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::USmallIntProperty::MaxConcurrentActivs);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::USmallIntProperty::MaxConnections);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::USmallIntProperty::MaxSchemaNameLen);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::USmallIntProperty::MaxTableNameLen);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::USmallIntProperty::NonNullableColumns);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::USmallIntProperty::OdbcSagCliConformance);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::USmallIntProperty::TxnCapable);

		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::UIntProperty::DefaultTxnIsolation);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::UIntProperty::ScrollOptions);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::UIntProperty::TxnIsolationOption);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::UIntProperty::CursorSensitity);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::UIntProperty::DynamicCursorAttributes1);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::UIntProperty::ForwardOnlyCursorAttributes1);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::UIntProperty::KeysetCursorAttributes1);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::UIntProperty::StaticCursorAttributes1);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::UIntProperty::KeysetCursorAttributes2);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::UIntProperty::StaticCursorAttributes2);

		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::IntProperty::PositionedStatements);
		dbInf.ReadAndStoryProperty(m_pHDbc, DatabaseInfo::IntProperty::PosOperations);

		return dbInf;
	}


	std::vector<std::string> Database::ReadCatalogInfo(ReadCatalogInfoMode mode)
	{
		exASSERT(IsOpen());

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_pHStmt, true, true);

		std::vector<std::string> results;

		std::string catalogName = u8"";
		std::string schemaName = u8"";
		std::string tableTypeName = u8"";
		std::string tableName = u8"";

		SQLSMALLINT colNr = 0;
		SQLUSMALLINT charLen = 0;

		switch(mode)
		{
		case ReadCatalogInfoMode::AllCatalogs:
			catalogName = SQL_ALL_CATALOGS;
			colNr = 1;
			charLen = m_dbInf.GetMaxCatalogNameLen();
			break;
		case ReadCatalogInfoMode::AllSchemas:
			schemaName = SQL_ALL_SCHEMAS;
			colNr = 2;
			charLen = m_dbInf.GetMaxSchemaNameLen();
			break;
		case ReadCatalogInfoMode::AllTableTypes:
			tableTypeName = SQL_ALL_TABLE_TYPES;
			colNr = 4;
			charLen = m_dbInf.GetMaxTableTypeNameLen();
			break;
		default:
			exASSERT(false);
		}
		
		std::unique_ptr<SQLAPICHARTYPE[]> buffer(new SQLAPICHARTYPE[charLen]);
		SQLRETURN ret = SQLTables(m_pHStmt->GetHandle(),
			EXODBCSTR_TO_SQLAPICHARPTR(catalogName), SQL_NTS,		// catalog name                 
			EXODBCSTR_TO_SQLAPICHARPTR(schemaName), SQL_NTS,		// schema name
			EXODBCSTR_TO_SQLAPICHARPTR(tableName), SQL_NTS,			// table name
			EXODBCSTR_TO_SQLAPICHARPTR(tableTypeName), SQL_NTS);

		THROW_IFN_SUCCEEDED(SQLTables, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		// Read data
		SQLLEN cb;
		while ((ret = SQLFetch(m_pHStmt->GetHandle())) == SQL_SUCCESS)   // Table Information
		{
			GetData(m_pHStmt, colNr, SQLAPICHARTYPENAME, buffer.get(), charLen * sizeof(SQLAPICHARTYPE), &cb, NULL);
			results.push_back(SQLAPICHARPTR_TO_EXODBCSTR(buffer.get()));
		}

		THROW_IFN_NO_DATA(SQLFetch, ret);

		return results;
	}


	TableInfo Database::FindOneTable(const std::string& tableName, const std::string& schemaName, const std::string& catalogName, const std::string& tableType) const
	{
		// Query the tables that match
		TableInfosVector tables = FindTables(tableName, schemaName, catalogName, tableType);

		if(tables.size() == 0)
		{
			Exception ex((boost::format(u8"No tables found while searching for: tableName: '%s', schemName: '%s', catalogName: '%s', typeName : '%s'") %tableName %schemaName %catalogName %tableType).str());
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}
		if(tables.size() != 1)
		{
			Exception ex((boost::format(u8"Not exactly one table found while searching for: tableName: '%s', schemName: '%s', catalogName: '%s', typeName : '%s'") %tableName %schemaName %catalogName %tableType).str());
			SET_EXCEPTION_SOURCE(ex);
			throw ex;
		}

		TableInfo table = tables[0];
		return table;
	}


	SqlTypeInfosVector Database::ReadDataTypesInfo()
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
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
		while(ret == SQL_SUCCESS)
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


	void Database::Close()
	{
		// Note: We assume that we are fully opened, all statements are allocated, etc.
		// Open() will do its own cleanup in case of failure.
		if (!IsOpen())
		{
			return;
		}

		// Rollback any ongoing Transaction if in Manual mode
		if (GetCommitMode() == CommitMode::MANUAL)
		{
			RollbackTrans();
		}

		// Free statement handles - just reset them, that should be enough
		m_pHStmt.reset();
		m_pHStmtExecSql.reset();

		// Try to disconnect from the data source
		SQLRETURN ret = SQLDisconnect(m_pHDbc->GetHandle());
		THROW_IFN_SUCCEEDED(SQLDisconnect, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle());

		m_dbIsOpen = false;
	}


	void Database::CommitTrans() const
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		// Commit the transaction
		SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, m_pHDbc->GetHandle(), SQL_COMMIT);
		THROW_IFN_SUCCEEDED_MSG(SQLEndTran, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle(), u8"Failed to Commit Transaction");
	}


	void Database::RollbackTrans() const
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		// Rollback the transaction
		SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, m_pHDbc->GetHandle(), SQL_ROLLBACK);
		THROW_IFN_SUCCEEDED_MSG(SQLEndTran, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle(), u8"Failed to Rollback Transaction");
	}


	void Database::ExecSql(const std::string& sqlStmt, ExecFailMode mode /* = NotFailOnNoData */)
	{
		exASSERT(IsOpen());

		RETCODE retcode;

		StatementCloser::CloseStmtHandle(m_pHStmtExecSql, StatementCloser::Mode::IgnoreNotOpen);

		retcode = SQLExecDirect(m_pHStmtExecSql->GetHandle(),  EXODBCSTR_TO_SQLAPICHARPTR(sqlStmt), SQL_NTS);
		if ( ! SQL_SUCCEEDED(retcode))
		{
			if (!(mode == ExecFailMode::NotFailOnNoData && retcode == SQL_NO_DATA))
			{
				SqlResultException ex(u8"SQLExecDirect", retcode, SQL_HANDLE_STMT, m_pHStmtExecSql->GetHandle(), (boost::format(u8"Failed to execute Stmt '%s'") % sqlStmt).str());
				SET_EXCEPTION_SOURCE(ex);
				throw ex;
			}
		}
	}


	TableInfosVector Database::FindTables(const std::string& tableName, const std::string& schemaName, const std::string& catalogName, const std::string& tableType) const
	{
		exASSERT(IsOpen());

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_pHStmt, true, true);

		TableInfosVector tables;

		std::unique_ptr<SQLAPICHARTYPE[]> buffCatalog(new SQLAPICHARTYPE[m_dbInf.GetMaxCatalogNameLen()]);
		std::unique_ptr<SQLAPICHARTYPE[]> buffSchema(new SQLAPICHARTYPE[m_dbInf.GetMaxSchemaNameLen()]);
		std::unique_ptr<SQLAPICHARTYPE[]> buffTableName(new SQLAPICHARTYPE[m_dbInf.GetMaxTableNameLen()]);
		std::unique_ptr<SQLAPICHARTYPE[]> buffTableType(new SQLAPICHARTYPE[DB_MAX_TABLE_TYPE_LEN]);
		std::unique_ptr<SQLAPICHARTYPE[]> buffTableRemarks(new SQLAPICHARTYPE[DB_MAX_TABLE_REMARKS_LEN]);
		bool isCatalogNull = false;
		bool isSchemaNull = false;

		// Query db
		SQLRETURN ret = SQLTables(m_pHStmt->GetHandle(),
			catalogName.empty() ? NULL :  EXODBCSTR_TO_SQLAPICHARPTR(catalogName), catalogName.empty() ? 0 : SQL_NTS,   // catname                 
			schemaName.empty() ? NULL :  EXODBCSTR_TO_SQLAPICHARPTR(schemaName), schemaName.empty() ? 0 : SQL_NTS,   // schema name
			tableName.empty() ? NULL :  EXODBCSTR_TO_SQLAPICHARPTR(tableName), tableName.empty() ? 0 : SQL_NTS,							// table name
			tableType.empty() ? NULL :  EXODBCSTR_TO_SQLAPICHARPTR(tableType), tableType.empty() ? 0 : SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLTables, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		while ((ret = SQLFetch(m_pHStmt->GetHandle())) == SQL_SUCCESS)
		{
			buffCatalog[0] = 0;
			buffSchema[0] = 0;
			buffTableName[0] = 0;
			buffTableType[0] = 0;
			buffTableRemarks[0] = 0;

			SQLLEN cb;
			GetData(m_pHStmt, 1, SQLAPICHARTYPENAME, buffCatalog.get(), m_dbInf.GetMaxCatalogNameLen() * sizeof(SQLAPICHARTYPE), &cb, &isCatalogNull);
			GetData(m_pHStmt, 2, SQLAPICHARTYPENAME, buffSchema.get(), m_dbInf.GetMaxSchemaNameLen() * sizeof(SQLAPICHARTYPE), &cb, &isSchemaNull);
			GetData(m_pHStmt, 3, SQLAPICHARTYPENAME, buffTableName.get(), m_dbInf.GetMaxTableNameLen() * sizeof(SQLAPICHARTYPE), &cb, NULL);
			GetData(m_pHStmt, 4, SQLAPICHARTYPENAME, buffTableType.get(), DB_MAX_TABLE_TYPE_LEN * sizeof(SQLAPICHARTYPE), &cb, NULL);
			GetData(m_pHStmt, 5, SQLAPICHARTYPENAME, buffTableRemarks.get(), DB_MAX_TABLE_REMARKS_LEN * sizeof(SQLAPICHARTYPE), &cb, NULL);

			TableInfo table(SQLAPICHARPTR_TO_EXODBCSTR(buffTableName.get()), SQLAPICHARPTR_TO_EXODBCSTR(buffTableType.get()), 
                            SQLAPICHARPTR_TO_EXODBCSTR(buffTableRemarks.get()), SQLAPICHARPTR_TO_EXODBCSTR(buffCatalog.get()), 
                            SQLAPICHARPTR_TO_EXODBCSTR(buffSchema.get()), isCatalogNull, isSchemaNull, GetDbms());
			tables.push_back(table);
		}
		THROW_IFN_NO_DATA(SQLFetch, ret);

		return tables;
	}


	int Database::ReadColumnCount(const TableInfo& table)
	{
		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_pHStmt, true, true);

		// Note: The schema and table name arguments are Pattern Value arguments
		// The catalog name is an ordinary argument. if we do not have one in the
		// DbCatalogTable, we set it to an empty string
		std::string catalogQueryName = u8"";
		if (table.HasCatalog())
		{
			catalogQueryName = table.GetCatalog();
		}

		// Query columns, note:
		// we always have a tablename, but only sometimes a schema
		int colCount = 0;
		SQLRETURN ret = SQLColumns(m_pHStmt->GetHandle(),
				 EXODBCSTR_TO_SQLAPICHARPTR(catalogQueryName), SQL_NTS,	// catalog
				table.HasSchema() ?  EXODBCSTR_TO_SQLAPICHARPTR(table.GetSchema()) : NULL, table.HasSchema() ? SQL_NTS : 0,	// schema
				 EXODBCSTR_TO_SQLAPICHARPTR(table.GetPureName()), SQL_NTS,		// tablename
				NULL, 0);						// All columns

		THROW_IFN_SUCCEEDED(SQLColumns, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		// Count the columns
		while ((ret = SQLFetch(m_pHStmt->GetHandle())) == SQL_SUCCESS)
		{
			++colCount;
		}
		THROW_IFN_NO_DATA(SQLColumns, ret);

		return colCount;
	}


	int Database::ReadColumnCount(const std::string& tableName, const std::string& schemaName, const std::string& catalogName, const std::string& tableType)
	{
		// Find one matching table
		TableInfo table = FindOneTable(tableName, schemaName, catalogName, tableType);

		// Forward the call		
		return ReadColumnCount(table);
	}


	TablePrimaryKeysVector Database::ReadTablePrimaryKeys(const TableInfo& table) const
	{
		exASSERT(IsOpen());
		// Access returns 'SQLSTATE IM001; Native Error: 0; [Microsoft][ODBC Driver Manager] Driver does not support this function' for SQLPrimaryKeys
		exASSERT(GetDbms() != DatabaseProduct::ACCESS);

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_pHStmt, true, true);

		TablePrimaryKeysVector primaryKeys;

		SQLRETURN ret = SQLPrimaryKeys(m_pHStmt->GetHandle(),
			table.HasCatalog() ?  EXODBCSTR_TO_SQLAPICHARPTR(table.GetCatalog()) : NULL, table.HasCatalog() ? SQL_NTS : 0,
			table.HasSchema() ?  EXODBCSTR_TO_SQLAPICHARPTR(table.GetSchema()) : NULL, table.HasSchema() ? SQL_NTS : 0,
			 EXODBCSTR_TO_SQLAPICHARPTR(table.GetPureName()), SQL_NTS);

		THROW_IFN_SUCCEEDED(SQLPrimaryKeys, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		while ((ret = SQLFetch(m_pHStmt->GetHandle())) == SQL_SUCCESS)
		{
			SQLLEN cb;
			std::string catalogName, schemaName, tableName, columnName, keyName;
			bool isCatalogNull, isSchemaNull, isKeyNameNull;
			SQLSMALLINT keySequence;
			GetData(m_pHStmt, 1, m_dbInf.GetMaxCatalogNameLen(), catalogName, &isCatalogNull);
			GetData(m_pHStmt, 2, m_dbInf.GetMaxSchemaNameLen(), schemaName, &isSchemaNull);
			GetData(m_pHStmt, 3, m_dbInf.GetMaxTableNameLen(), tableName);
			GetData(m_pHStmt, 4, m_dbInf.GetMaxColumnNameLen(), columnName);
			GetData(m_pHStmt, 5, SQL_C_SHORT, &keySequence, sizeof(keySequence), &cb, NULL);
			GetData(m_pHStmt, 6, DB_MAX_PRIMARY_KEY_NAME_LEN, keyName, &isKeyNameNull);
			TablePrimaryKeyInfo pk(catalogName, schemaName, tableName, columnName, keySequence, keyName, isCatalogNull, isSchemaNull, isKeyNameNull);
			primaryKeys.push_back(pk);
		}
		THROW_IFN_NO_DATA(SQLFetch, ret);

		return primaryKeys;
	}


	TablePrivilegesVector Database::ReadTablePrivileges(const std::string& tableName, const std::string& schemaName, const std::string& catalogName, const std::string& tableType) const
	{
		exASSERT(IsOpen());

		// Find one matching table
		TableInfo table = FindOneTable(tableName, schemaName, catalogName, tableType);

		// Forward the call		
		return ReadTablePrivileges(table);
	}


	TablePrivilegesVector Database::ReadTablePrivileges(const TableInfo& table) const
	{
		exASSERT(IsOpen());
		exASSERT_MSG(GetDbms() != DatabaseProduct::ACCESS, u8"Access reports 'SQLSTATE IM001; Driver does not support this function' for SQLTablePrivileges");

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_pHStmt, true, true);

		TablePrivilegesVector privileges;

		// Note: The schema and table name arguments are Pattern Value arguments
		// The catalog name is an ordinary argument. if we do not have one in the
		// DbCatalogTable, we set it to an empty string
		std::string catalogQueryName = u8"";
		if (table.HasCatalog())
		{
			catalogQueryName = table.GetCatalog();
		}

		// Query privs
		// we always have a tablename, but only sometimes a schema
		SQLRETURN ret = SQLTablePrivileges(m_pHStmt->GetHandle(),
			 EXODBCSTR_TO_SQLAPICHARPTR(catalogQueryName), SQL_NTS,
			table.HasSchema() ?  EXODBCSTR_TO_SQLAPICHARPTR(table.GetSchema()) : NULL, table.HasSchema() ? SQL_NTS : 0,
			 EXODBCSTR_TO_SQLAPICHARPTR(table.GetPureName()), SQL_NTS);
		THROW_IFN_SUCCEEDED(SQLTablePrivileges, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

		while ((ret = SQLFetch(m_pHStmt->GetHandle())) == SQL_SUCCESS)
		{

			STablePrivilegesInfo priv;
			GetData(m_pHStmt, 1, m_dbInf.GetMaxCatalogNameLen(), priv.m_catalogName, &priv.m_isCatalogNull);
			GetData(m_pHStmt, 2, m_dbInf.GetMaxSchemaNameLen(), priv.m_schemaName, &priv.m_isSchemaNull);
			GetData(m_pHStmt, 3, m_dbInf.GetMaxTableNameLen(), priv.m_tableName);
			GetData(m_pHStmt, 4, DB_MAX_GRANTOR_LEN, priv.m_grantor, &priv.m_isGrantorNull);
			GetData(m_pHStmt, 5, DB_MAX_GRANTEE_LEN, priv.m_grantee);
			GetData(m_pHStmt, 6, DB_MAX_PRIVILEGES_LEN, priv.m_privilege);
			GetData(m_pHStmt, 7, DB_MAX_IS_GRANTABLE_LEN, priv.m_grantable, &priv.m_isGrantableNull);

			privileges.push_back(priv);
		}

		THROW_IFN_NO_DATA(SQLFetch, ret);

		return privileges;
	}


	SpecialColumnInfosVector Database::ReadSpecialColumns(const TableInfo& table, IdentifierType idType, RowIdScope scope, bool includeNullableColumns /* = true */) const
	{
		exASSERT(IsOpen());

		SpecialColumnInfosVector columns;

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_pHStmt, true, true);

		string catalogName;
		string schemaName;
		string tableName = table.GetPureName();
		if (table.HasCatalog())
		{
			catalogName = table.GetCatalog();
		}
		if (table.HasSchema())
		{
			schemaName = table.GetSchema();
		}

		SQLSMALLINT nullable = SQL_NO_NULLS;
		if (includeNullableColumns)
		{
			nullable = SQL_NULLABLE;
		}

		SQLRETURN ret = SQLSpecialColumns(m_pHStmt->GetHandle(), (SQLSMALLINT)idType,
			 EXODBCSTR_TO_SQLAPICHARPTR(catalogName), (SQLSMALLINT) catalogName.length(),
			 EXODBCSTR_TO_SQLAPICHARPTR(schemaName), (SQLSMALLINT) schemaName.length(),
			 EXODBCSTR_TO_SQLAPICHARPTR(tableName), (SQLSMALLINT) tableName.length(),
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
			GetData(m_pHStmt, 2, m_dbInf.GetMaxColumnNameLen(), columnName);
			GetData(m_pHStmt, 3, SQL_C_SSHORT, &sqlType, sizeof(sqlType), &cb, NULL);
			GetData(m_pHStmt, 4, DB_MAX_TYPE_NAME_LEN, sqlTypeName);
			GetData(m_pHStmt, 5, SQL_C_SLONG, &columnSize, sizeof(columnSize), &cb, NULL);
			GetData(m_pHStmt, 6, SQL_C_SLONG, &bufferLength, sizeof(bufferLength), &cb, NULL);
			GetData(m_pHStmt, 7, SQL_C_SSHORT, &decimalDigits, sizeof(decimalDigits), &cb, NULL);
			GetData(m_pHStmt, 8, SQL_C_SSHORT, &pseudoColVal, sizeof(pseudoColVal), &cb, NULL);

			PseudoColumn pseudoCol = PseudoColumn::UNKNOWN;
			if (pseudoColVal == (SQLSMALLINT) PseudoColumn::NOT_PSEUDO)
				pseudoCol = PseudoColumn::NOT_PSEUDO;
			else if (pseudoColVal == (SQLSMALLINT)PseudoColumn::PSEUDO)
				pseudoCol = PseudoColumn::PSEUDO;

			RowIdScope scope;
			if (!scopeIsNull)
			{
				switch (scopeVal)
				{
				case (SQLSMALLINT) RowIdScope::CURSOR:
					scope = RowIdScope::CURSOR;
					break;
				case (SQLSMALLINT) RowIdScope::SESSION:
					scope = RowIdScope::SESSION;
					break;
				case (SQLSMALLINT) RowIdScope::TRANSCATION:
					scope = RowIdScope::TRANSCATION;
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


	ColumnInfosVector Database::ReadTableColumnInfo(const std::string& tableName, const std::string& schemaName, const std::string& catalogName, const std::string& tableType) const
	{
		exASSERT(IsOpen());

		// Find one matching table
		TableInfo table = FindOneTable(tableName, schemaName, catalogName, tableType);

		// Forward the call		
		return ReadTableColumnInfo(table);
	}


	ColumnInfosVector Database::ReadTableColumnInfo(const TableInfo& table) const
	{
		exASSERT(IsOpen());

		// Close Statement and make sure it closes upon exit
		StatementCloser stmtCloser(m_pHStmt, true, true);

		// Clear result
		ColumnInfosVector columns;

		// Note: The schema and table name arguments are Pattern Value arguments
		// The catalog name is an ordinary argument. if we do not have one in the
		// DbCatalogTable, we set it to an empty string
		std::string catalogQueryName = u8"";
		if (table.HasCatalog())
		{
			catalogQueryName = table.GetCatalog();
		}

		// Query columns
		// we always have a tablename, but only sometimes a schema		
		int colCount = 0;
		SQLRETURN ret = SQLColumns(m_pHStmt->GetHandle(),
			 EXODBCSTR_TO_SQLAPICHARPTR(catalogQueryName), SQL_NTS,	// catalog
			table.HasSchema() ?  EXODBCSTR_TO_SQLAPICHARPTR(table.GetSchema()) : NULL, table.HasSchema() ? SQL_NTS : 0,	// schema
			 EXODBCSTR_TO_SQLAPICHARPTR(table.GetPureName()), SQL_NTS,		// tablename
			NULL, 0);						// All columns

		THROW_IFN_SUCCEEDED(SQLColumns, ret, SQL_HANDLE_STMT, m_pHStmt->GetHandle());

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
			GetData(m_pHStmt, 1, m_dbInf.GetMaxCatalogNameLen(), catalogName, &isCatalogNull);
			GetData(m_pHStmt, 2, m_dbInf.GetMaxSchemaNameLen(), schemaName, &isSchemaNull);
			GetData(m_pHStmt, 3, m_dbInf.GetMaxTableNameLen(), tableName);
			GetData(m_pHStmt, 4, m_dbInf.GetMaxColumnNameLen(), columnName);
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


	SDbCatalogInfo Database::ReadCompleteCatalog()
	{
		SDbCatalogInfo dbInf;

		dbInf.m_tables = FindTables(u8"", u8"", u8"", u8"");

		TableInfosVector::const_iterator it;
		for(it = dbInf.m_tables.begin(); it != dbInf.m_tables.end(); it++)
		{
			const TableInfo& table = *it;
			if (table.HasCatalog())
			{
				dbInf.m_catalogs.insert(table.GetCatalog());
			}
			if (table.HasSchema())
			{
				dbInf.m_schemas.insert(table.GetSchema());
			}
		}
		return dbInf;
	}


	CommitMode Database::ReadCommitMode()
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		CommitMode mode = CommitMode::UNKNOWN;

		SQLULEN modeValue = 0;
		SQLINTEGER cb;
		SQLRETURN ret = SQLGetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_AUTOCOMMIT, &modeValue, sizeof(modeValue), &cb);
		THROW_IFN_SUCCEEDED_MSG(SQLGetConnectAttr, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle(), u8"Failed to read Attr SQL_ATTR_AUTOCOMMIT");

		if(modeValue == SQL_AUTOCOMMIT_OFF)
			mode = CommitMode::MANUAL;
		else if(modeValue == SQL_AUTOCOMMIT_ON)
			mode = CommitMode::AUTO;

		m_commitMode = mode;

		return mode;
	}


	TransactionIsolationMode Database::ReadTransactionIsolationMode()
	{
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		SQLULEN modeValue = 0;
		SQLINTEGER cb;
		SQLRETURN ret = SQLGetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_TXN_ISOLATION, &modeValue, sizeof(modeValue), &cb);
		THROW_IFN_SUCCEEDED_MSG(SQLGetConnectAttr, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle(), u8"Failed to read Attr SQL_ATTR_TXN_ISOLATION");

		switch (modeValue)
		{
		case SQL_TXN_READ_UNCOMMITTED:
			return TransactionIsolationMode::READ_UNCOMMITTED;
		case SQL_TXN_READ_COMMITTED:
			return TransactionIsolationMode::READ_COMMITTED;
		case SQL_TXN_REPEATABLE_READ:
			return TransactionIsolationMode::REPEATABLE_READ;
		case SQL_TXN_SERIALIZABLE:
			return TransactionIsolationMode::SERIALIZABLE;
		}

		return TransactionIsolationMode::UNKNOWN;
	}

	void Database::SetCommitMode(CommitMode mode)
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		// If Autocommit is off, we need to rollback any ongoing transaction
		// Else at least MS SQL Server will complain that an ongoing transaction has been committed.
		if (GetCommitMode() != CommitMode::AUTO)
		{
			RollbackTrans();
		}

		SQLRETURN ret;
		std::string errStringMode;
		if (mode == CommitMode::MANUAL)
		{
			ret = SQLSetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_OFF, 0);
			errStringMode = u8"SQL_AUTOCOMMIT_OFF";
		}
		else
		{
			ret = SQLSetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_AUTOCOMMIT, (SQLPOINTER) SQL_AUTOCOMMIT_ON, 0);
			errStringMode = u8"SQL_AUTOCOMMIT_ON";
		}
		THROW_IFN_SUCCEEDED_MSG(SQLSetConnectAttr, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle(), (boost::format(u8"Setting ATTR_AUTOCOMMIT to %s failed") % errStringMode).str());

		m_commitMode = mode;
	}


	void Database::SetTransactionIsolationMode(TransactionIsolationMode mode)
	{	
		exASSERT(m_pHDbc);
		exASSERT(m_pHDbc->IsAllocated());

		// We need to ensure cursors are closed:
		// The internal statement should be closed, the exec-statement could be open
		StatementCloser::CloseStmtHandle(m_pHStmt, StatementCloser::Mode::IgnoreNotOpen);
		StatementCloser::CloseStmtHandle(m_pHStmtExecSql, StatementCloser::Mode::IgnoreNotOpen);

		// If Autocommit is off, we need to Rollback any ongoing transaction
		// Else at least MS SQL Server will complain that an ongoing transaction has been committed.
		if (GetCommitMode() != CommitMode::AUTO)
		{
			RollbackTrans();
		}

		SQLRETURN ret;
		std::string errStringMode;
		{
			ret = SQLSetConnectAttr(m_pHDbc->GetHandle(), SQL_ATTR_TXN_ISOLATION, (SQLPOINTER)mode, 0);
		}
		THROW_IFN_SUCCEEDED_MSG(SQLSetConnectAttr, ret, SQL_HANDLE_DBC, m_pHDbc->GetHandle(), (boost::format(u8"Cannot set SQL_ATTR_TXN_ISOLATION to %d") % (int) mode).str());

	}

	
	bool Database::CanSetTransactionIsolationMode(TransactionIsolationMode mode) const
	{
		SQLUINTEGER txnIsolationOpts = m_dbInf.GetUIntProperty(DatabaseInfo::UIntProperty::TxnIsolationOption);
		return (txnIsolationOpts & (SQLUINTEGER)mode) != 0;
	}


	OdbcVersion Database::GetDriverOdbcVersion() const
	{
		// Note: On purpose we do not check for IsOpen() here, because we need to read that during OpenIml()
		std::string driverOdbcVersion = m_dbInf.GetStringProperty(DatabaseInfo::StringProperty::DriverOdbcVersion);
		exASSERT( ! driverOdbcVersion.empty());

		OdbcVersion ov = OdbcVersion::UNKNOWN;
		std::vector<std::string> versions;
		boost::split(versions, driverOdbcVersion, boost::is_any_of(u8"."));
		if (versions.size() == 2)
		{
			try
			{
				short major = boost::lexical_cast<short>(versions[0]);
				short minor = boost::lexical_cast<short>(versions[1]);
				if (major >= 3 && minor >= 80)
				{
					ov = OdbcVersion::V_3_8;
				}
				else if (major >= 3)
				{
					ov = OdbcVersion::V_3;
				}
				else if (major >= 2)
				{
					ov = OdbcVersion::V_2;
				}
			}
			catch (boost::bad_lexical_cast& e)
			{
				HIDE_UNUSED(e);
				THROW_WITH_SOURCE(Exception, (boost::format(u8"Failed to determine odbc version from string '%s'") % driverOdbcVersion).str());
			}
		}
		return ov;
	}


	OdbcVersion Database::GetMaxSupportedOdbcVersion() const
	{
		OdbcVersion driverVersion = GetDriverOdbcVersion();
		if (driverVersion >= m_pEnv->GetOdbcVersion())
		{
			return m_pEnv->GetOdbcVersion();
		}
		return driverVersion;
	}


	void Database::DetectDbms()
	{
		std::string name = m_dbInf.GetDbmsName();
		if (boost::algorithm::contains(name, u8"Microsoft SQL Server"))
		{
			m_dbmsType = DatabaseProduct::MS_SQL_SERVER;
		}
		else if (boost::algorithm::contains(name, u8"MySQL"))
		{
			m_dbmsType = DatabaseProduct::MY_SQL;
		}
		else if (boost::algorithm::contains(name, u8"DB2"))
		{
			m_dbmsType = DatabaseProduct::DB2;
		}
		else if (boost::algorithm::contains(name, u8"EXCEL"))
		{
			m_dbmsType = DatabaseProduct::EXCEL;
		}
		else if (boost::algorithm::contains(name, u8"ACCESS"))
		{
			m_dbmsType = DatabaseProduct::ACCESS;
		}

		if (m_dbmsType == DatabaseProduct::UNKNOWN)
		{
			LOG_WARNING((boost::format(u8"Unknown database: %s") % m_dbInf.GetDbmsName()).str());
		}
	}
}