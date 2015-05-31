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
	namespace test
	{
		std::wstring ConvertNameCase(const std::wstring& columnOrTableName, Case nameCase)
		{
			return (nameCase == Case::UPPER ? boost::algorithm::to_upper_copy(columnOrTableName) : boost::algorithm::to_lower_copy(columnOrTableName));
		}


		const std::map<test::TableId, std::wstring> TableNames = {
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


		const std::map<test::TableId, std::wstring> IdColumnNames = {
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


		std::wstring GetTableName(test::TableId table, test::Case nameCase)
		{
			std::map<test::TableId, std::wstring>::const_iterator it = TableNames.find(table);
			exASSERT(it != TableNames.end());
			return ConvertNameCase(it->second, nameCase);
		}


		std::wstring GetIdColumnName(test::TableId table, test::Case nameCase)
		{
			std::map<test::TableId, std::wstring>::const_iterator it = IdColumnNames.find(table);
			exASSERT(it != IdColumnNames.end());
			return ConvertNameCase(it->second, nameCase);
		}


		void ClearTestTable(test::TableId table, test::Case nameCase, exodbc::Table& testTable, exodbc::Database& db)
		{
			std::wstring idName = GetIdColumnName(table, nameCase);
			std::wstring sqlstmt = (boost::wformat(L"%s >= 0 OR %s < 0") % idName % idName).str();

			// Remove everything, ignoring if there was any data:
			testTable.Delete(sqlstmt, false);
			db.CommitTrans();
		}

		
		template<typename TExp>
		ValueIndicator GetValueIndicator(TExp expected)
		{
			ValueIndicator indicator = ValueIndicator::NO_INDICATOR;
			if (expected.which() == 0)
			{
				indicator = boost::get<ValueIndicator>(expected);
			}
			return indicator;
		}


		template <typename TExp, typename TExpSqlType, typename TBufferSqlType>
		struct FComperator
		{
			::testing::AssertionResult& m_result;
			const Database& m_db;

			bool operator()(TExp expected, BufferVariant value)
			{
				ValueIndicator indicator = GetValueIndicator(expected);

				if (indicator == ValueIndicator::IGNORE_VAL)
				{
					return true;
				}

				if (indicator == ValueIndicator::IS_NULL && value.which() != 0)
				{
					m_result << "Expected NULL, but the value is not." << std::endl;
					return false;
				}

				if (indicator == ValueIndicator::NO_INDICATOR)
				{
					TExpSqlType expI = boost::get<TExpSqlType>(expected);
					if (value.which() == 0)
					{
						m_result << "Expected " << expI << ", but the value is NULL" << std::endl;
						return false;
					}
					TBufferSqlType i = boost::get<TBufferSqlType>(value);
					if (i != expI)
					{
						m_result << "Expected " << expI << ", but the value is " << i << "" << std::endl;
						return false;
					}
				}

				return true;
			}
		};


		::testing::AssertionResult IsIntRecordEqual(const exodbc::Database& db, const exodbc::Table& iTable, Int expId, SmallInt expSmallInt, Int expInt, BigInt expBigInt)
		{
			try
			{
				::testing::AssertionResult failure = ::testing::AssertionFailure();
				bool failed = false;

				BufferVariant id = iTable.GetColumnValue(0);
				BufferVariant tsmallInt = iTable.GetColumnValue(1);
				BufferVariant tint = iTable.GetColumnValue(2);
				BufferVariant tbigInt = iTable.GetColumnValue(3);

				FComperator<Int, SQLINTEGER, SQLINTEGER> idComperator = { failure, db };
				if (!idComperator(expId, id))
				{
					failed = true;
				}

				if (db.GetDbms() == DatabaseProduct::ACCESS)
				{
					// Access has no BigInt, we simply ignore that column
					expBigInt = ValueIndicator::IGNORE_VAL;
					// Also Access has no Smallints. we still have those values in our tests, but we stored them as INT
					FComperator<SmallInt, SQLSMALLINT, SQLINTEGER> tSmallIntComperator = { failure, db };
					if (!tSmallIntComperator(expSmallInt, tsmallInt))
					{
						failed = true;
					}
				}
				else
				{
					FComperator<SmallInt, SQLSMALLINT, SQLSMALLINT> tSmallIntComperator = { failure, db };
					if (!tSmallIntComperator(expSmallInt, tsmallInt))
					{
						failed = true;
					}
				}

				FComperator<Int, SQLINTEGER, SQLINTEGER> tIntComperator = { failure, db };
				FComperator<BigInt, SQLBIGINT, SQLBIGINT> tBigIntComperator = { failure, db };

				if ( ! (
						tIntComperator(expInt, tint)
					&&	tBigIntComperator(expBigInt, tbigInt)
					))
				{
					failed = true;
				}

				if (failed)
				{
					std::string top = boost::str(boost::format("Records are not equal:"));
					std::string hed = boost::str(boost::format("          | %18s | %18s | %18s | %18s") % "idintegertypes" %"tsmalint" %"tint" %"tbigint");
					std::string dat;
					try
					{
								dat = boost::str(boost::format("  values: | %18d | %18d | %18d | %18d") % iTable.GetString(0) % iTable.GetString(1) % iTable.GetString(2) % iTable.GetString(3));
					}
					catch (Exception& ex)
					{
						dat = boost::str(boost::format("  values: ERROR - failed printing row as str: %s") % ex.what());
					}
					failure << top << std::endl << hed << std::endl << dat << std::endl;
					return failure;
				}
			}
			catch (boost::bad_get& ex)
			{
				return ::testing::AssertionFailure() << "ERROR: boost::bad_get thrown while comparing column values: " << ex.what();
			}
			catch (Exception& ex)
			{
				return ::testing::AssertionFailure() << "ERROR: exodbc::Exception thrown while comparing column values: " << ex.what();
			}

			return ::testing::AssertionSuccess();

		}


		void ClearIntTable(const exodbc::Database& db, test::Case nameCase)
		{
			std::wstring tableName;
			try
			{
				// Create a deletable table and delete on it
				tableName = GetTableName(test::TableId::INTEGERTYPES_TMP, nameCase);
				std::wstring idColName = GetIdColumnName(test::TableId::INTEGERTYPES_TMP, nameCase);
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


		void InsertIntTypes(test::Case nameCase, const exodbc::Database& db, SQLINTEGER id, SQLSMALLINT smallInt, SQLINTEGER i, SQLBIGINT bigInt, bool commit /* = true */ )
		{
			// Create a insertable table
			std::wstring tableName = GetTableName(test::TableId::INTEGERTYPES_TMP, nameCase);
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