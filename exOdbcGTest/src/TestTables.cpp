/*!
* \file TestTables.cpp
* \author Elias Gerber <eg@zame.ch>
* \date 26.12.2014
* \copyright wxWindows Library Licence, Version 3.1
*
*  Defines some helpers for TestTables, like converting names to upper/lower case, etc.
*/

#include "stdafx.h"

// Own header
#include "TestTables.h"

// Same component headers
#include "gtest/gtest.h"

// Other headers

// Debug
#include "DebugNew.h"

// Static consts
// -------------

// Construction
// -------------

// Destructor
// -----------

// Implementation
// --------------
namespace exodbc
{
	namespace TestTables
	{
		std::wstring ConvertNameCase(const std::wstring& columnOrTableName, NameCase nameCase)
		{
			return (nameCase == NameCase::UPPER ? boost::algorithm::to_upper_copy(columnOrTableName) : boost::algorithm::to_lower_copy(columnOrTableName));
		}


		const std::map<TestTables::Table, std::wstring> TableNames = {
			{ Table::BLOBTYPES, L"blobtypes" },
			{ Table::BLOBTYPES_TMP, L"blobtypes_tmp" },
			{ Table::CHARTABLE, L"chartable" },
			{ Table::CHARTYPES, L"chartypes" },
			{ Table::CHARTYPES_TMP, L"chartypes_tmp" },
			{ Table::DATETYPES, L"datetypes" },
			{ Table::DATETYPES_TMP, L"datetypes_tmp" },
			{ Table::FLOATTYPES, L"floattypes" },
			{ Table::FLOATTYPES_TMP, L"floattypes_tmp" },
			{ Table::INTEGERTYPES, L"integertypes" },
			{ Table::INTEGERTYPES_TMP, L"integertypes_tmp" },
			{ Table::MULTIKEY, L"multikey" },
			{ Table::NUMERICTYPES, L"numerictypes" },
			{ Table::NUMERICTYPES_TMP, L"numerictypes_tmp" },
			{ Table::SELECTONLY, L"selectonly" },
			{ Table::NOT_EXISTING, L"not_existing" },
			{ Table::NOT_SUPPORTED, L"not_supported" },
			{ Table::NOT_SUPPORTED_TMP, L"not_supported_tmp" }
		};


		const std::map<TestTables::Table, std::wstring> IdColumnNames = {
			{ Table::BLOBTYPES, L"idblobtypes" },
			{ Table::BLOBTYPES_TMP, L"idblobtypes" },
			{ Table::CHARTABLE, L"idchartable" },
			{ Table::CHARTYPES, L"idchartypes" },
			{ Table::CHARTYPES_TMP, L"idchartypes" },
			{ Table::DATETYPES, L"iddatetypes" },
			{ Table::DATETYPES_TMP, L"iddatetypes" },
			{ Table::FLOATTYPES, L"idfloattypes" },
			{ Table::FLOATTYPES_TMP, L"idfloattypes" },
			{ Table::INTEGERTYPES, L"idintegertypes" },
			{ Table::INTEGERTYPES_TMP, L"idintegertypes" },
			{ Table::MULTIKEY, L"idmultikey" },
			{ Table::NUMERICTYPES, L"idnumerictypes" },
			{ Table::NUMERICTYPES_TMP, L"idnumerictypes" },
			{ Table::SELECTONLY, L"idselectonly" },
			{ Table::NOT_EXISTING, L"idnot_existing" },
			{ Table::NOT_SUPPORTED, L"idnot_supported" },
			{ Table::NOT_SUPPORTED_TMP, L"idnot_supported" }
		};


		std::wstring GetTableName(TestTables::Table table, TestTables::NameCase nameCase)
		{
			std::map<TestTables::Table, std::wstring>::const_iterator it = TableNames.find(table);
			exASSERT(it != TableNames.end());
			return ConvertNameCase(it->second, nameCase);
		}


		std::wstring GetIdColumnName(TestTables::Table table, TestTables::NameCase nameCase)
		{
			std::map<TestTables::Table, std::wstring>::const_iterator it = IdColumnNames.find(table);
			exASSERT(it != IdColumnNames.end());
			return ConvertNameCase(it->second, nameCase);
		}


		void ClearTestTable(TestTables::Table table, TestTables::NameCase nameCase, exodbc::Table& testTable, exodbc::Database& db)
		{
			std::wstring idName = GetIdColumnName(table, nameCase);
			std::wstring sqlstmt = (boost::wformat(L"%s >= 0 OR %s < 0") % idName % idName).str();

			// Remove everything, ignoring if there was any data:
			testTable.Delete(sqlstmt, false);
			db.CommitTrans();
		}


		// \todo: See ticket #82
		//exodbc::Table GetEmptyTestTable(TestTables::Table table, TestTables::NameCase nameCase, exodbc::Database& db)
		//{
		//	std::wstring tableName = GetTableName(table, nameCase);
		//	exodbc::Table t(db, tableName, L"", L"", L"", AF_READ_WRITE);
		//	t.Open(db);
		//	ClearTestTable(table, nameCase, t, db);
		//	return t;
		//}
	}
}