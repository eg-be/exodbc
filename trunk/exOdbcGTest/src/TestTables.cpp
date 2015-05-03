/*!
* \file TestTables.cpp
* \author Elias Gerber <eg@elisium.ch>
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


		void ClearTestTable(TestTables::Table table, TestTables::NameCase nameCase, const exodbc::Database& db)
		{
			// Create a deletable table
			std::wstring tableName = GetTableName(table, nameCase);
			exodbc::Table* pDeletableTable = NULL;
			try
			{
				pDeletableTable = new exodbc::Table(db, tableName, L"", L"", L"", AF_DELETE | AF_SELECT);
				pDeletableTable->Open(db);
				ClearTestTable(*pDeletableTable, db);
				pDeletableTable->Close();
			}
			catch (exodbc::Exception& ex)
			{
				LOG_ERROR(boost::str(boost::wformat(L"Failed clearing table '%s': %s") % tableName %ex.ToString()));
				delete pDeletableTable;
				throw;
			}

			// delete and forget
			delete pDeletableTable;
		}


		void ClearTestTable(const exodbc::Table& deletableTable, const exodbc::Database& db)
		{
			bool rollback = false;
			try
			{
				exASSERT_MSG(deletableTable.TestAccessFlag(AF_DELETE), boost::str(boost::wformat(L"Failed to Clear test table '%s', AccessFlag AF_DELETE is not set on the passed table.") % deletableTable.GetTableInfo().GetSqlName()));
				// Determine primary key columns to build some where clause - for test columns we know the id-columns are all numeric
				std::wstringstream ws;
				int primaryKeysCount = 0;
				std::set<SQLSMALLINT> colIndexes = deletableTable.GetColumnBufferIndexes();
				std::set<SQLSMALLINT>::const_iterator it = colIndexes.begin();
				while (it != colIndexes.end())
				{
					ColumnBuffer* pBuff = deletableTable.GetColumnBuffer(*it);
					if (pBuff->IsPrimaryKey())
					{
						if (primaryKeysCount > 0)
						{
							ws << L" AND";
						}
						ws << pBuff->GetQueryName() << L" >= 0 OR " << pBuff->GetQueryName() << L" < 0";
						primaryKeysCount++;
					}
					++it;
				}
				exASSERT_MSG(primaryKeysCount > 0, boost::str(boost::wformat(L"Failed to Clear test table '%s', no ColumnBuffers have the Primary Key flag set.") % deletableTable.GetTableInfo().GetSqlName()));
				deletableTable.Delete(ws.str(), false);
				rollback = true;
				db.CommitTrans();
			}
			catch (exodbc::Exception& ex)
			{
				LOG_ERROR(boost::str(boost::wformat(L"Failed to Clear test table '%s': %s") % deletableTable.GetTableInfo().GetSqlName() % ex.ToString()));
				if (rollback)
				{
					try
					{
						LOG_DEBUG(L"Trying to Rollback..");
						db.RollbackTrans();
						LOG_DEBUG(L"Rollback successfull.");
					}
					catch (exodbc::Exception& ex)
					{
						LOG_ERROR(boost::str(boost::wformat(L"Rollback failed: %s") % ex.ToString()));
					}
				}
				throw;
			}
		}


		void InsertIntTypes(TestTables::NameCase nameCase, const exodbc::Database& db, SQLINTEGER id, SQLSMALLINT smallInt, SQLINTEGER i, SQLBIGINT bigInt, bool commit /* = true */ )
		{
			// Create a insertable table
			std::wstring tableName = GetTableName(TestTables::Table::INTEGERTYPES_TMP, nameCase);
			exodbc::Table* pInsertableTable = NULL;
			try
			{
				pInsertableTable = new exodbc::Table(db, tableName, L"", L"", L"", AF_INSERT | AF_SELECT);
				pInsertableTable->Open(db);
				InsertIntTypes(*pInsertableTable, db, id, smallInt, i, bigInt, commit);
				pInsertableTable->Close();
			}
			catch (exodbc::Exception& ex)
			{
				LOG_ERROR(boost::str(boost::wformat(L"Failed inserting row into test table '%s': %s") % tableName %ex.ToString()));
				delete pInsertableTable;
				throw;
			}

			// delete and forget
			delete pInsertableTable;
		}


		void InsertIntTypes(const exodbc::Table& insertableTable, const exodbc::Database& db, SQLINTEGER id, SQLSMALLINT smallInt, SQLINTEGER i, SQLBIGINT bigInt, bool commit /* = true */)
		{
			bool rollback = false;
			try
			{
				exASSERT_MSG(insertableTable.TestAccessFlag(AF_INSERT), boost::str(boost::wformat(L"Failed to insert row into test table '%s', AccessFlag AF_INSERT is not set on the passed table.") % insertableTable.GetTableInfo().GetSqlName()));
				// Set values on columns
				insertableTable.SetColumnValue(0, id);

				if (smallInt != NULL_INT_VALUE)
					insertableTable.SetColumnValue(1, smallInt);
				else
					insertableTable.SetColumnNull(1);

				if (i != NULL_INT_VALUE)
					insertableTable.SetColumnValue(2, i);
				else
					insertableTable.SetColumnNull(2);

				if (bigInt != NULL_INT_VALUE)
					insertableTable.SetColumnValue(3, bigInt);
				else
					insertableTable.SetColumnNull(3);

				insertableTable.Insert();
				if (commit)
				{
					rollback = true;
					db.CommitTrans();
				}
			}
			catch (exodbc::Exception& ex)
			{
				LOG_ERROR(boost::str(boost::wformat(L"Failed to insert row into test table '%s': %s") % insertableTable.GetTableInfo().GetSqlName() % ex.ToString()));
				if (rollback)
				{
					try
					{
						LOG_DEBUG(L"Trying to Rollback..");
						db.RollbackTrans();
						LOG_DEBUG(L"Rollback successfull.");
					}
					catch (exodbc::Exception& ex)
					{
						LOG_ERROR(boost::str(boost::wformat(L"Rollback failed: %s") % ex.ToString()));
					}
				}
				throw;
			}
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