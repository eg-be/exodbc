/*!
* \file TestTables.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 26.12.2014
* \copyright GNU Lesser General Public License Version 3
*
*  Defines some helpers for TestTables, like converting names to upper/lower case, etc.
*/

#include "stdafx.h"

// Own header
#include "TestTables.h"

// Same component headers
#include "gtest/gtest.h"

// Other headers
#include "Database.h"

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

			//bool operator()(TExp expected, BufferVariant value)
			//{
			//	//ValueIndicator indicator = GetValueIndicator(expected);

			//	//if (indicator == ValueIndicator::IGNORE_VAL)
			//	//{
			//	//	return true;
			//	//}

			//	//if (indicator == ValueIndicator::IS_NULL && value.which() != 0)
			//	//{
			//	//	m_result << "Expected NULL, but the value is not." << std::endl;
			//	//	return false;
			//	//}

			//	//if (indicator == ValueIndicator::NO_INDICATOR)
			//	//{
			//	//	TExpSqlType expI = boost::get<TExpSqlType>(expected);
			//	//	if (value.which() == 0)
			//	//	{
			//	//		m_result << "Expected " << expI << ", but the value is NULL" << std::endl;
			//	//		return false;
			//	//	}
			//	//	TBufferSqlType i = boost::get<TBufferSqlType>(value);
			//	//	if (i != expI)
			//	//	{
			//	//		m_result << "Expected " << expI << ", but the value is " << i << "" << std::endl;
			//	//		return false;
			//	//	}
			//	//}

			//	return false;
			//}
		};


		//::testing::AssertionResult IsIntRecordEqual(const exodbc::Database& db, const exodbc::Table& iTable, Int expId, SmallInt expSmallInt, Int expInt, BigInt expBigInt)
		//{
		//	try
		//	{
		//		::testing::AssertionResult failure = ::testing::AssertionFailure();
		//		bool failed = false;

		//		BufferVariant id = iTable.GetColumnValue(0);
		//		BufferVariant tsmallInt = iTable.GetColumnValue(1);
		//		BufferVariant tint = iTable.GetColumnValue(2);
		//		BufferVariant tbigInt = iTable.GetColumnValue(3);

		//		FComperator<Int, SQLINTEGER, SQLINTEGER> idComperator = { failure, db };
		//		if (!idComperator(expId, id))
		//		{
		//			failed = true;
		//		}

		//		if (db.GetDbms() == DatabaseProduct::ACCESS)
		//		{
		//			// Access has no BigInt, we simply ignore that column
		//			expBigInt = ValueIndicator::IGNORE_VAL;
		//			// Also Access has no Smallints. we still have those values in our tests, but we stored them as INT
		//			FComperator<SmallInt, SQLSMALLINT, SQLINTEGER> tSmallIntComperator = { failure, db };
		//			if (!tSmallIntComperator(expSmallInt, tsmallInt))
		//			{
		//				failed = true;
		//			}
		//		}
		//		else
		//		{
		//			FComperator<SmallInt, SQLSMALLINT, SQLSMALLINT> tSmallIntComperator = { failure, db };
		//			if (!tSmallIntComperator(expSmallInt, tsmallInt))
		//			{
		//				failed = true;
		//			}
		//		}

		//		FComperator<Int, SQLINTEGER, SQLINTEGER> tIntComperator = { failure, db };
		//		FComperator<BigInt, SQLBIGINT, SQLBIGINT> tBigIntComperator = { failure, db };

		//		if ( ! (
		//				tIntComperator(expInt, tint)
		//			&&	tBigIntComperator(expBigInt, tbigInt)
		//			))
		//		{
		//			failed = true;
		//		}

		//		if (failed)
		//		{
		//			std::string top = boost::str(boost::format("Records are not equal:"));
		//			std::string hed = boost::str(boost::format("          | %18s | %18s | %18s | %18s") % "idintegertypes" %"tsmalint" %"tint" %"tbigint");
		//			std::string dat;
		//			try
		//			{
		//						dat = boost::str(boost::format("  values: | %18d | %18d | %18d | %18d") % iTable.GetString(0) % iTable.GetString(1) % iTable.GetString(2) % iTable.GetString(3));
		//			}
		//			catch (Exception& ex)
		//			{
		//				dat = boost::str(boost::format("  values: ERROR - failed printing row as str: %s") % ex.what());
		//			}
		//			failure << top << std::endl << hed << std::endl << dat << std::endl;
		//			return failure;
		//		}
		//	}
		//	catch (boost::bad_get& ex)
		//	{
		//		return ::testing::AssertionFailure() << "ERROR: boost::bad_get thrown while comparing column values: " << ex.what();
		//	}
		//	catch (Exception& ex)
		//	{
		//		return ::testing::AssertionFailure() << "ERROR: exodbc::Exception thrown while comparing column values: " << ex.what();
		//	}

		//	return ::testing::AssertionSuccess();

		//}


		//void InsertIntTypesTmp(test::Case nameCase, const exodbc::Database& db, Int id, SmallInt tSmallInt, Int tInt, BigInt tBigInt, bool commitTrans /* = true */)
		//{
		//	std::wstring tableName;
		//	try
		//	{
		//		// Note: We only allow the indicator to be set to NULL
		//		ValueIndicator idInd = GetValueIndicator(id);
		//		ValueIndicator smallIntInd = GetValueIndicator(tSmallInt);
		//		ValueIndicator intInd = GetValueIndicator(tInt);
		//		ValueIndicator bigIntInd = GetValueIndicator(tBigInt);
		//		exASSERT(idInd == ValueIndicator::NO_INDICATOR || idInd == ValueIndicator::IS_NULL);
		//		exASSERT(smallIntInd == ValueIndicator::NO_INDICATOR || smallIntInd == ValueIndicator::IS_NULL);
		//		exASSERT(intInd == ValueIndicator::NO_INDICATOR || intInd == ValueIndicator::IS_NULL);
		//		exASSERT(bigIntInd == ValueIndicator::NO_INDICATOR || bigIntInd == ValueIndicator::IS_NULL);

		//		// Get an insertable IntTypesTmp Table
		//		tableName = GetTableName(test::TableId::INTEGERTYPES_TMP, nameCase);
		//		std::wstring idColName = GetIdColumnName(test::TableId::INTEGERTYPES_TMP, nameCase);
		//		exodbc::Table intTable(&db, AF_SELECT | AF_INSERT, tableName, L"", L"", L"");
		//		intTable.Open();

		//		// Insert given values
		//		// Note: BufferVariant defaults to NULL value
		//		BufferVariant idVal;
		//		BufferVariant tSmallIntVal;
		//		BufferVariant tIntVal;
		//		BufferVariant tBigIntVal;
		//		if (idInd != ValueIndicator::IS_NULL)
		//		{
		//			idVal = boost::get<SQLINTEGER>(id);
		//		}
		//		if (smallIntInd != ValueIndicator::IS_NULL)
		//		{
		//			if (db.GetDbms() == DatabaseProduct::ACCESS)
		//			{
		//				tSmallIntVal = (SQLINTEGER) boost::get<SQLSMALLINT>(tSmallInt);
		//			}
		//			else
		//			{
		//				tSmallIntVal = boost::get<SQLSMALLINT>(tSmallInt);
		//			}
		//		}
		//		if (intInd != ValueIndicator::IS_NULL)
		//		{
		//			tIntVal = boost::get<SQLINTEGER>(tInt);
		//		}
		//		if (bigIntInd != ValueIndicator::IS_NULL)
		//		{
		//			// on access, we stay on the null value
		//			if (db.GetDbms() != DatabaseProduct::ACCESS)
		//			{
		//				tBigIntVal = boost::get<SQLBIGINT>(tBigInt);
		//			}
		//		}
		//		intTable.SetColumnValue(0, idVal);
		//		intTable.SetColumnValue(1, tSmallIntVal);
		//		intTable.SetColumnValue(2, tIntVal);
		//		intTable.SetColumnValue(3, tBigIntVal);
		//		intTable.Insert();
		//		if (commitTrans)
		//		{
		//			db.CommitTrans();
		//		}
		//	}
		//	catch (exodbc::Exception& ex)
		//	{
		//		LOG_ERROR(boost::str(boost::wformat(L"Failed to clear test table '%s': %s") % tableName %ex.ToString()));
		//		throw;
		//	}

		//}


		//void ClearIntTypesTmpTable(const exodbc::Database& db, test::Case nameCase)
		//{
		//	std::wstring tableName;
		//	try
		//	{
		//		// Create a deletable table and delete on it
		//		tableName = GetTableName(test::TableId::INTEGERTYPES_TMP, nameCase);
		//		std::wstring idColName = GetIdColumnName(test::TableId::INTEGERTYPES_TMP, nameCase);
		//		exodbc::Table intTable(&db, AF_SELECT | AF_DELETE_WHERE, tableName, L"", L"", L"");
		//		intTable.Open();
		//		std::wstring where = boost::str(boost::wformat(L"%s >= 0 OR %s < 0") % idColName %idColName);
		//		intTable.Delete(where, false);
		//		db.CommitTrans();
		//	}
		//	catch (exodbc::Exception& ex)
		//	{
		//		LOG_ERROR(boost::str(boost::wformat(L"Failed to clear test table '%s': %s") % tableName %ex.ToString()));
		//		throw;
		//	}
		//}


		//void ClearDateTypesTmpTable(const exodbc::Database& db, test::Case nameCase)
		//{
		//	std::wstring tableName;
		//	try
		//	{
		//		// Create a deletable table and delete on it
		//		tableName = GetTableName(test::TableId::DATETYPES_TMP, nameCase);
		//		std::wstring idColName = GetIdColumnName(test::TableId::DATETYPES_TMP, nameCase);
		//		exodbc::Table dTable(&db, AF_SELECT | AF_DELETE_WHERE, tableName, L"", L"", L"");
		//		dTable.Open();
		//		std::wstring where = boost::str(boost::wformat(L"%s >= 0 OR %s < 0") % idColName %idColName);
		//		dTable.Delete(where, false);
		//		db.CommitTrans();
		//	}
		//	catch (exodbc::Exception& ex)
		//	{
		//		LOG_ERROR(boost::str(boost::wformat(L"Failed to clear test table '%s': %s") % tableName %ex.ToString()));
		//		throw;
		//	}
		//}


		//void ClearCharTypesTmpTable(const exodbc::Database& db, test::Case nameCase)
		//{
		//	std::wstring tableName;
		//	try
		//	{
		//		// Create a deletable table and delete on it
		//		tableName = GetTableName(test::TableId::CHARTYPES_TMP, nameCase);
		//		std::wstring idColName = GetIdColumnName(test::TableId::CHARTYPES_TMP, nameCase);
		//		exodbc::Table dTable(&db, AF_SELECT | AF_DELETE_WHERE, tableName, L"", L"", L"");
		//		dTable.Open();
		//		std::wstring where = boost::str(boost::wformat(L"%s >= 0 OR %s < 0") % idColName %idColName);
		//		dTable.Delete(where, false);
		//		db.CommitTrans();
		//	}
		//	catch (exodbc::Exception& ex)
		//	{
		//		LOG_ERROR(boost::str(boost::wformat(L"Failed to clear test table '%s': %s") % tableName %ex.ToString()));
		//		throw;
		//	}
		//}


		//void ClearNumericTypesTmpTable(const exodbc::Database& db, test::Case nameCase)
		//{
		//	std::wstring tableName;
		//	try
		//	{
		//		// Create a deletable table and delete on it
		//		tableName = GetTableName(test::TableId::NUMERICTYPES_TMP, nameCase);
		//		std::wstring idColName = GetIdColumnName(test::TableId::NUMERICTYPES_TMP, nameCase);
		//		exodbc::Table dTable(&db, AF_SELECT | AF_DELETE_WHERE, tableName, L"", L"", L"");
		//		dTable.Open();
		//		std::wstring where = boost::str(boost::wformat(L"%s >= 0 OR %s < 0") % idColName %idColName);
		//		dTable.Delete(where, false);
		//		db.CommitTrans();
		//	}
		//	catch (exodbc::Exception& ex)
		//	{
		//		LOG_ERROR(boost::str(boost::wformat(L"Failed to clear test table '%s': %s") % tableName %ex.ToString()));
		//		throw;
		//	}
		//}


		//void ClearBlobTypesTmpTable(const exodbc::Database& db, test::Case nameCase)
		//{
		//	std::wstring tableName;
		//	try
		//	{
		//		// Create a deletable table and delete on it
		//		tableName = GetTableName(test::TableId::BLOBTYPES_TMP, nameCase);
		//		std::wstring idColName = GetIdColumnName(test::TableId::BLOBTYPES_TMP, nameCase);
		//		exodbc::Table dTable(&db, AF_SELECT | AF_DELETE_WHERE, tableName, L"", L"", L"");
		//		dTable.Open();
		//		std::wstring where = boost::str(boost::wformat(L"%s >= 0 OR %s < 0") % idColName %idColName);
		//		dTable.Delete(where, false);
		//		db.CommitTrans();
		//	}
		//	catch (exodbc::Exception& ex)
		//	{
		//		LOG_ERROR(boost::str(boost::wformat(L"Failed to clear test table '%s': %s") % tableName %ex.ToString()));
		//		throw;
		//	}
		//}


		//void ClearFloatTypesTmpTable(const exodbc::Database& db, test::Case nameCase)
		//{
		//	std::wstring tableName;
		//	try
		//	{
		//		// Create a deletable table and delete on it
		//		tableName = GetTableName(test::TableId::FLOATTYPES_TMP, nameCase);
		//		std::wstring idColName = GetIdColumnName(test::TableId::FLOATTYPES_TMP, nameCase);
		//		exodbc::Table dTable(&db, AF_SELECT | AF_DELETE_WHERE, tableName, L"", L"", L"");
		//		dTable.Open();
		//		std::wstring where = boost::str(boost::wformat(L"%s >= 0 OR %s < 0") % idColName %idColName);
		//		dTable.Delete(where, false);
		//		db.CommitTrans();
		//	}
		//	catch (exodbc::Exception& ex)
		//	{
		//		LOG_ERROR(boost::str(boost::wformat(L"Failed to clear test table '%s': %s") % tableName %ex.ToString()));
		//		throw;
		//	}
		//}
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