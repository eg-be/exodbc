/*!
* \file exOdbcGTestHelpers.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 22.11.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

// Own header
#include "exOdbcGTestHelpers.h"

// Same component headers

// Other headers

// Debug
#include "DebugNew.h"

using namespace std;
using namespace exodbc;

namespace exodbctest
{
	const std::map<TableId, std::string> g_TableNames = {
		{ TableId::BLOBTYPES, u8"blobtypes" },
		{ TableId::BLOBTYPES_TMP, u8"blobtypes_tmp" },
		{ TableId::CHARTABLE, u8"chartable" },
		{ TableId::CHARTYPES, u8"chartypes" },
		{ TableId::CHARTYPES_TMP, u8"chartypes_tmp" },
		{ TableId::DATETYPES, u8"datetypes" },
		{ TableId::DATETYPES_TMP, u8"datetypes_tmp" },
		{ TableId::FLOATTYPES, u8"floattypes" },
		{ TableId::FLOATTYPES_TMP, u8"floattypes_tmp" },
		{ TableId::INTEGERTYPES, u8"integertypes" },
		{ TableId::INTEGERTYPES_TMP, u8"integertypes_tmp" },
		{ TableId::MULTIKEY, u8"multikey" },
		{ TableId::NUMERICTYPES, u8"numerictypes" },
		{ TableId::NUMERICTYPES_TMP, u8"numerictypes_tmp" },
		{ TableId::SELECTONLY, u8"selectonly" },
		{ TableId::NOT_EXISTING, u8"not_existing" },
		{ TableId::NOT_SUPPORTED, u8"not_supported" },
		{ TableId::NOT_SUPPORTED_TMP, u8"not_supported_tmp" }
	};


	const std::map<TableId, std::string> g_IdColumnNames = {
		{ TableId::BLOBTYPES, u8"idblobtypes" },
		{ TableId::BLOBTYPES_TMP, u8"idblobtypes" },
		{ TableId::CHARTABLE, u8"idchartable" },
		{ TableId::CHARTYPES, u8"idchartypes" },
		{ TableId::CHARTYPES_TMP, u8"idchartypes" },
		{ TableId::DATETYPES, u8"iddatetypes" },
		{ TableId::DATETYPES_TMP, u8"iddatetypes" },
		{ TableId::FLOATTYPES, u8"idfloattypes" },
		{ TableId::FLOATTYPES_TMP, u8"idfloattypes" },
		{ TableId::INTEGERTYPES, u8"idintegertypes" },
		{ TableId::INTEGERTYPES_TMP, u8"idintegertypes" },
		{ TableId::MULTIKEY, u8"idmultikey" },
		{ TableId::NUMERICTYPES, u8"idnumerictypes" },
		{ TableId::NUMERICTYPES_TMP, u8"idnumerictypes" },
		{ TableId::SELECTONLY, u8"idselectonly" },
		{ TableId::NOT_EXISTING, u8"idnot_existing" },
		{ TableId::NOT_SUPPORTED, u8"idnot_supported" },
		{ TableId::NOT_SUPPORTED_TMP, u8"idnot_supported" }
	};


	exodbc::EnvironmentPtr CreateEnv(exodbc::OdbcVersion odbcVersion /* = exodbc::OdbcVersion::V_3 */)
	{
		exodbc::EnvironmentPtr pEnv = std::make_shared<exodbc::Environment>(odbcVersion);
		return pEnv;
	}


	exodbc::DatabasePtr OpenTestDb(exodbc::ConstEnvironmentPtr pEnv)
	{
		exodbc::DatabasePtr pDb = std::make_shared<exodbc::Database>(pEnv);

		if (g_odbcInfo.HasConnectionString())
		{
			pDb->Open(g_odbcInfo.m_connectionString);
		}
		else
		{
			pDb->Open(g_odbcInfo.m_dsn, g_odbcInfo.m_username, g_odbcInfo.m_password);
		}
		return pDb;
	}


	exodbc::DatabasePtr OpenTestDb(exodbc::OdbcVersion odbcVersion /* = exodbc::OdbcVersion::V_3 */)
	{
		exodbc::EnvironmentPtr pEnv = CreateEnv(odbcVersion);
		return OpenTestDb(pEnv);
	}


	std::string PrependSchemaOrCatalogName(exodbc::DatabaseProduct dbms, const std::string& name)
	{
		string prefix;
		if (dbms != DatabaseProduct::ACCESS)
		{
			prefix = u8"exodbc.";
		}

		prefix = ToDbCase(prefix);
		return prefix + name;
	}

	std::string GetTableName(TableId table)
	{
		std::map<TableId, std::string>::const_iterator it = g_TableNames.find(table);
		assert(it != g_TableNames.end());
		return ToDbCase(it->second);
	}


	std::string GetIdColumnName(TableId table)
	{
		std::map<TableId, std::string>::const_iterator it = g_IdColumnNames.find(table);
		assert(it != g_IdColumnNames.end());
		return ToDbCase(it->second);
	}


	std::string ToDbCase(const std::string& str)
	{
		if (g_odbcInfo.m_namesCase == Case::UPPER)
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
			LOG_ERROR(u8"Is not a tmpTable");
			return;
		}

		DatabasePtr pDb = OpenTestDb(OdbcVersion::V_3);

		string tableName = GetTableName(tmpTableId);
		string idColName = GetIdColumnName(tmpTableId);
		string schemaName = u8"exodbc.";
		if (pDb->GetDbms() == DatabaseProduct::ACCESS)
		{
			schemaName = u8"";
		}
		schemaName = ToDbCase(schemaName);

		string sql = boost::str(boost::format(u8"DELETE FROM %s%s WHERE %s >= 0 OR %s <= 0") %schemaName %tableName %idColName %idColName);
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


} // namespace exodbc