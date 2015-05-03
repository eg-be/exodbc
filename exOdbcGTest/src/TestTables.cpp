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


		void ClearIntTable(const exodbc::Database& db, TestTables::NameCase nameCase)
		{
			std::wstring tableName;
			try
			{
				// Create a deletable table and delete on it
				tableName = GetTableName(TestTables::Table::INTEGERTYPES_TMP, nameCase);
				std::wstring idColName = GetIdColumnName(TestTables::Table::INTEGERTYPES_TMP, nameCase);
				exodbc::Table intTable(db, tableName, L"", L"", L"", AF_SELECT | AF_DELETE_WHERE);
				intTable.Open(db);
				std::wstring where = boost::str(boost::wformat(L"%s >= 0 OR %s < 0") % idColName %idColName);
				intTable.Delete(where, false);
				db.CommitTrans();
			}
			catch (exodbc::Exception& ex)
			{
				LOG_ERROR(boost::str(boost::wformat(L"Failed to clear test table '%s': %s") % tableName %ex.ToString()));
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

				// id is never null
				insertableTable.SetColumnValue(0, id);

				// and int works everywhere
				if (i != NULL_INT_VALUE)
					insertableTable.SetColumnValue(2, i);
				else
					insertableTable.SetColumnNull(2);

				// note: access has only integers, try to convert the values
				if (smallInt == NULL_INT_VALUE)
				{
					insertableTable.SetColumnNull(1);
				}
				else if (db.GetDbms() == DatabaseProduct::ACCESS)
				{
					insertableTable.SetColumnValue(1, (SQLINTEGER)smallInt);
				}
				else
				{
					insertableTable.SetColumnValue(1, smallInt);
				}

				if (bigInt == NULL_INT_VALUE)
				{
					insertableTable.SetColumnNull(3);
				}
				else if (db.GetDbms() == DatabaseProduct::ACCESS)
				{
					insertableTable.SetColumnValue(3, (SQLINTEGER)bigInt);
				}
				else
				{
					insertableTable.SetColumnValue(3, bigInt);
				}

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