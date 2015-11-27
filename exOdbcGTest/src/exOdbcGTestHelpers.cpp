/*!
* \file exOdbcGTestHelpers.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 22.11.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "exOdbcGTestHelpers.h"

// Same component headers
#include "TestTables.h"

// Other headers

// Debug
#include "DebugNew.h"

using namespace std;
using namespace exodbc;

namespace exodbctest
{
	const std::map<TableId, std::wstring> g_TableNames = {
		{ TableId::BLOBTYPES, L"blobtypes" },
		{ TableId::BLOBTYPES_TMP, L"blobtypes_tmp" },
		{ TableId::CHARTABLE, L"chartable" },
		{ TableId::CHARTYPES, L"chartypes" },
		{ TableId::CHARTYPES_TMP, L"chartypes_tmp" },
		{ TableId::DATETYPES, L"datetypes" },
		{ TableId::DATETYPES_TMP, L"datetypes_tmp" },
		{ TableId::FLOATTYPES, L"floattypes" },
		{ TableId::FLOATTYPES_TMP, L"floattypes_tmp" },
		{ TableId::INTEGERTYPES, L"integertypes" },
		{ TableId::INTEGERTYPES_TMP, L"integertypes_tmp" },
		{ TableId::MULTIKEY, L"multikey" },
		{ TableId::NUMERICTYPES, L"numerictypes" },
		{ TableId::NUMERICTYPES_TMP, L"numerictypes_tmp" },
		{ TableId::SELECTONLY, L"selectonly" },
		{ TableId::NOT_EXISTING, L"not_existing" },
		{ TableId::NOT_SUPPORTED, L"not_supported" },
		{ TableId::NOT_SUPPORTED_TMP, L"not_supported_tmp" }
	};


	const std::map<TableId, std::wstring> g_IdColumnNames = {
		{ TableId::BLOBTYPES, L"idblobtypes" },
		{ TableId::BLOBTYPES_TMP, L"idblobtypes" },
		{ TableId::CHARTABLE, L"idchartable" },
		{ TableId::CHARTYPES, L"idchartypes" },
		{ TableId::CHARTYPES_TMP, L"idchartypes" },
		{ TableId::DATETYPES, L"iddatetypes" },
		{ TableId::DATETYPES_TMP, L"iddatetypes" },
		{ TableId::FLOATTYPES, L"idfloattypes" },
		{ TableId::FLOATTYPES_TMP, L"idfloattypes" },
		{ TableId::INTEGERTYPES, L"idintegertypes" },
		{ TableId::INTEGERTYPES_TMP, L"idintegertypes" },
		{ TableId::MULTIKEY, L"idmultikey" },
		{ TableId::NUMERICTYPES, L"idnumerictypes" },
		{ TableId::NUMERICTYPES_TMP, L"idnumerictypes" },
		{ TableId::SELECTONLY, L"idselectonly" },
		{ TableId::NOT_EXISTING, L"idnot_existing" },
		{ TableId::NOT_SUPPORTED, L"idnot_supported" },
		{ TableId::NOT_SUPPORTED_TMP, L"idnot_supported" }
	};


	exodbc::EnvironmentPtr CreateEnv(exodbc::OdbcVersion odbcVersion /* = exodbc::OdbcVersion::V_3 */)
	{
		exodbc::EnvironmentPtr pEnv = std::make_shared<exodbc::Environment>(odbcVersion);
		return pEnv;
	}


	exodbc::DatabasePtr OpenTestDb(exodbc::ConstEnvironmentPtr pEnv)
	{
		exodbc::DatabasePtr pDb = std::make_shared<exodbc::Database>(pEnv);

		if (exodbc::g_odbcInfo.HasConnectionString())
		{
			pDb->Open(exodbc::g_odbcInfo.m_connectionString);
		}
		else
		{
			pDb->Open(exodbc::g_odbcInfo.m_dsn, exodbc::g_odbcInfo.m_username, exodbc::g_odbcInfo.m_password);
		}
		return pDb;
	}


	exodbc::DatabasePtr OpenTestDb(exodbc::OdbcVersion odbcVersion /* = exodbc::OdbcVersion::V_3 */)
	{
		exodbc::EnvironmentPtr pEnv = CreateEnv(odbcVersion);
		return OpenTestDb(pEnv);
	}


	std::wstring GetTableName(TableId table)
	{
		std::map<TableId, std::wstring>::const_iterator it = g_TableNames.find(table);
		assert(it != g_TableNames.end());
		return ToDbCase(it->second);
	}


	std::wstring GetIdColumnName(TableId table)
	{
		std::map<TableId, std::wstring>::const_iterator it = g_IdColumnNames.find(table);
		assert(it != g_IdColumnNames.end());
		return ToDbCase(it->second);
	}


	std::wstring ToDbCase(const std::wstring& str)
	{
		if (exodbc::g_odbcInfo.m_namesCase == exodbc::test::Case::UPPER)
		{
			return boost::algorithm::to_upper_copy(str);
		}
		else
		{
			return boost::algorithm::to_lower_copy(str);
		}
	}


	void ClearTmpTable(TableId tmpTableId)
	{
		bool isTmp = (tmpTableId == TableId::BLOBTYPES_TMP
			|| tmpTableId == TableId::CHARTYPES_TMP
			|| tmpTableId == TableId::DATETYPES_TMP
			|| tmpTableId == TableId::FLOATTYPES_TMP
			|| tmpTableId == TableId::INTEGERTYPES_TMP
			|| tmpTableId == TableId::NUMERICTYPES_TMP
			|| tmpTableId == TableId::NOT_SUPPORTED_TMP);

		if (!isTmp)
		{
			LOG_ERROR(L"Is not a tmpTable");
			return;
		}

		DatabasePtr pDb = OpenTestDb(OdbcVersion::V_3);

		wstring tableName = GetTableName(tmpTableId);
		wstring idColName = GetIdColumnName(tmpTableId);
		wstring schemaName = L"exodbc.";
		if (pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			wstring schemaName = L"";
		}
		schemaName = ToDbCase(schemaName);

		wstring sql = boost::str(boost::wformat(L"DELETE FROM %s%s WHERE %s >= 0 OR %s <= 0") %schemaName %tableName %idColName %idColName);
		pDb->ExecSql(sql);
		pDb->CommitTrans();
	}


	// Static consts
	// -------------

	// Construction
	// -------------

	// Destructor
	// -----------

	// Implementation
	// --------------



	// Interfaces
	// ----------

} // namespace exodbc