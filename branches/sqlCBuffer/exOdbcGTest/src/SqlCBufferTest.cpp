/*!
* \file VisitorsTests.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 31.03.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "SqlCBufferTest.h"

// Same component headers
#include "exOdbcGTestHelpers.h"

// Other headers
#include "SqlCBuffer.h"
#include "SqlStatementCloser.h"

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
using namespace std;
using namespace exodbctest;

namespace exodbc
{
	// SqlCBufferLengthIndicator
	// -------------
	TEST_F(SqlCBufferLengthIndicatorTest, Construction)
	{
		SqlCBufferLengthIndicator cb;
		EXPECT_EQ(0, cb.GetCb());
		EXPECT_FALSE(cb.IsNull());
	}


	TEST_F(SqlCBufferLengthIndicatorTest, SetAndGetCb)
	{
		SqlCBufferLengthIndicator cb;
		cb.SetCb(13);
		EXPECT_EQ(13, cb.GetCb());
	}


	TEST_F(SqlCBufferLengthIndicatorTest, SetNull)
	{
		SqlCBufferLengthIndicator cb;
		cb.SetNull();
		EXPECT_TRUE(cb.IsNull());
		cb.SetCb(25);
		EXPECT_FALSE(cb.IsNull());
	}


	TEST_F(SqlCBufferLengthIndicatorTest, CopyConstruction)
	{
		// must internally use the same cb-buffer
		SqlCBufferLengthIndicator cb;
		cb.SetCb(25);
		ASSERT_EQ(25, cb.GetCb());

		// create copy
		SqlCBufferLengthIndicator cb2(cb);
		EXPECT_EQ(25, cb2.GetCb());

		// changing either must change the other too
		cb.SetCb(13);
		EXPECT_EQ(13, cb.GetCb());
		EXPECT_EQ(13, cb2.GetCb());

		cb2.SetCb(14);
		EXPECT_EQ(14, cb.GetCb());
		EXPECT_EQ(14, cb2.GetCb());
	}


	// SqlCBuffer
	// -------------
	TEST_F(SqlCBufferTest, Construction)
	{
		// after construction buffer will be Null
		SqlSBigIntBuffer buff;
		EXPECT_TRUE(buff.IsNull());
	}


	TEST_F(SqlCBufferTest, SetAndGetValue)
	{
		SqlSBigIntBuffer buff;
		buff.SetValue(13, buff.GetBufferLength());
		EXPECT_EQ(13, buff.GetValue());
	}


	TEST_F(SqlCBufferTest, CopyConstruction)
	{
		// must internally use the same buffer
		SqlSBigIntBuffer buff;
		buff.SetValue(25, buff.GetBufferLength());
		ASSERT_EQ(25, buff.GetValue());

		// create copy
		SqlSBigIntBuffer buff2(buff);
		EXPECT_EQ(25, buff2.GetValue());

		// changing either must change the other too
		buff.SetValue(13, buff.GetBufferLength());
		EXPECT_EQ(13, buff.GetValue());
		EXPECT_EQ(13, buff.GetValue());

		buff2.SetValue(14, buff.GetBufferLength());
		EXPECT_EQ(14, buff.GetValue());
		EXPECT_EQ(14, buff.GetValue());
	}


	// SqlCArrayBuffer
	// -------------
	TEST_F(SqlCArrayBufferTest, Construction)
	{
		// after construction buffer will be Null
		SqlWCharArray arr(L"ColumnName", 24);
		EXPECT_TRUE(arr.IsNull());
		EXPECT_EQ(24, arr.GetNrOfElements());
		EXPECT_EQ(sizeof(SQLWCHAR) * 24, arr.GetBufferLength());
		EXPECT_EQ(L"ColumnName", arr.GetQueryName());
	}


	TEST_F(SqlCArrayBufferTest, SetAndGetValue)
	{
		SqlWCharArray arr(L"ColumnName", 24);
		wstring s(L"Hello");
		arr.SetValue(std::vector<SQLWCHAR>(s.begin(), s.end()), SQL_NTS);
		wstring v(arr.GetBuffer()->data());
		EXPECT_EQ(s, v);
		EXPECT_EQ(SQL_NTS, arr.GetCb());
	}


	TEST_F(SqlCArrayBufferTest, CopyConstruction)
	{
		// must internally use the same buffer
		SqlWCharArray arr(L"ColumnName", 24);
		wstring s(L"Hello");
		arr.SetValue(std::vector<SQLWCHAR>(s.begin(), s.end()), SQL_NTS);
		wstring v(arr.GetBuffer()->data());
		ASSERT_EQ(s, v);
		ASSERT_EQ(SQL_NTS, arr.GetCb());

		// create copy
		SqlWCharArray arr2(arr);
		wstring v2(arr2.GetBuffer()->data());
		EXPECT_EQ(s, v2);
		EXPECT_EQ(arr.GetQueryName(), arr2.GetQueryName());

		// changing either must change the other too
		s = L"World";
		arr.SetValue(std::vector<SQLWCHAR>(s.begin(), s.end()), SQL_NTS);
		v = arr.GetBuffer()->data();
		v2 = arr2.GetBuffer()->data();
		EXPECT_EQ(s, v);
		EXPECT_EQ(s, v2);

		s = L"Moon";
		arr2.SetValue(std::vector<SQLWCHAR>(s.begin(), s.end()), SQL_NTS);
		v = arr.GetBuffer()->data();
		v2 = arr2.GetBuffer()->data();
		EXPECT_EQ(s, v);
		EXPECT_EQ(s, v2);
	}


	void ColumnTestBase::SetUp()
	{
		m_pDb = OpenTestDb(OdbcVersion::V_3);
		m_pStmt = std::make_shared<SqlStmtHandle>();
		m_pStmt->AllocateWithParent(m_pDb->GetSqlDbcHandle());
	}


	struct FSelectFetcher
	{
		FSelectFetcher(DatabaseProduct dbms, SqlStmtHandlePtr pStmt, exodbctest::TableId tableId, const std::wstring& queryColumnName)
			: m_tableId(tableId)
			, m_queryColumnName(ToDbCase(queryColumnName))
			, m_pStmt(pStmt)
			, m_dbms(dbms)
		{
		}

		void operator()(SQLINTEGER idValue)
		{
			StatementCloser stmtCloser(m_pStmt);

			wstring tableName = GetTableName(TableId::INTEGERTYPES);
			tableName = PrependSchemaOrCatalogName(m_dbms, tableName);
			wstring idColName = GetIdColumnName(TableId::INTEGERTYPES);
			wstring sqlStmt = boost::str(boost::wformat(L"SELECT %s FROM %s WHERE %s = %d") % m_queryColumnName % tableName % idColName % idValue);

			SQLRETURN ret = SQLExecDirect(m_pStmt->GetHandle(), (SQLWCHAR*)sqlStmt.c_str(), SQL_NTS);
			THROW_IFN_SUCCESS(SQLExecDirect, ret, SQL_HANDLE_STMT, m_pStmt->GetHandle());
			ret = SQLFetch(m_pStmt->GetHandle());
			THROW_IFN_SUCCESS(SQLFetch, ret, SQL_HANDLE_STMT, m_pStmt->GetHandle());
		}

		DatabaseProduct m_dbms;
		SqlStmtHandlePtr m_pStmt;
		exodbctest::TableId m_tableId;
		wstring m_queryColumnName;
	};


	TEST_F(ShortColumnTest, ReadValue)
	{
		wstring colName = ToDbCase(L"exodbc.tshort");
		SqlSShortBuffer shortCol(colName);
		shortCol.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::INTEGERTYPES, L"tsmallint");

		f(1);
		EXPECT_EQ(-32768, shortCol.GetValue());
		f(2);
		EXPECT_EQ(32767, shortCol.GetValue());
		f(3);
		EXPECT_TRUE(shortCol.IsNull());
	}


	TEST_F(LongColumnTest, ReadValue)
	{
		wstring colName = ToDbCase(L"exodbc.tshort");
		SqlSLongBuffer longCol(colName);
		longCol.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::INTEGERTYPES, L"tint");

		f(3);
		EXPECT_EQ((-2147483647 -1), longCol.GetValue());
		f(4);
		EXPECT_EQ(2147483647, longCol.GetValue());
		f(5);
		EXPECT_TRUE(longCol.IsNull());
	}


	TEST_F(BigIntColumnTest, ReadValue)
	{
		wstring colName = ToDbCase(L"exodbc.tbigint");
		SqlSBigIntBuffer biCol(colName);
		biCol.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::INTEGERTYPES, L"tbigint");

		f(5);
		EXPECT_EQ((-9223372036854775807 - 1), biCol.GetValue());
		f(6);
		EXPECT_EQ(9223372036854775807, biCol.GetValue());
		f(4);
		EXPECT_TRUE(biCol.IsNull());

	}
} //namespace exodbc
