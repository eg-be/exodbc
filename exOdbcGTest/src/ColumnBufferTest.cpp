/*!
* \file ColumnBufferTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 31.03.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "ColumnBufferTest.h"

// Same component headers
#include "exOdbcGTestHelpers.h"

// Other headers
#include "ColumnBuffer.h"
#include "SqlStatementCloser.h"
#include "ExecutableStatement.h"

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
using namespace exodbc;

namespace exodbctest
{
	// SqlCBufferLengthIndicator
	// -------------
	TEST_F(ColumnBufferLengthIndicatorTest, Construction)
	{
		LengthIndicator cb;
		EXPECT_EQ(0, cb.GetCb());
		EXPECT_FALSE(cb.IsNull());
	}


	TEST_F(ColumnBufferLengthIndicatorTest, SetAndGetCb)
	{
		LengthIndicator cb;
		cb.SetCb(13);
		EXPECT_EQ(13, cb.GetCb());
	}


	TEST_F(ColumnBufferLengthIndicatorTest, SetNull)
	{
		LengthIndicator cb;
		cb.SetNull();
		EXPECT_TRUE(cb.IsNull());
		cb.SetCb(25);
		EXPECT_FALSE(cb.IsNull());
	}


	// SqlCBuffer
	// -------------
	TEST_F(ColumnBufferTest, Construction)
	{
		// after construction buffer will be Null
		BigIntColumnBuffer buff;
		EXPECT_TRUE(buff.IsNull());
		EXPECT_EQ(ColumnFlag::CF_NONE, buff.GetFlags());

		// test that we can construct with flags
		BigIntColumnBuffer buff2(L"SomeName", ColumnFlag::CF_PRIMARY_KEY | ColumnFlag::CF_SELECT);
		EXPECT_TRUE(buff2.IsNull());
		EXPECT_TRUE(buff2.Test(ColumnFlag::CF_PRIMARY_KEY));
		EXPECT_TRUE(buff2.Test(ColumnFlag::CF_SELECT));
		EXPECT_EQ(ColumnFlag::CF_PRIMARY_KEY | ColumnFlag::CF_SELECT, buff2.GetFlags());
		EXPECT_EQ(L"SomeName", buff2.GetQueryName());
	}


	TEST_F(ColumnBufferTest, SetAndGetValue)
	{
		BigIntColumnBuffer buff;
		buff.SetValue(13, buff.GetBufferLength());
		EXPECT_EQ(13, buff.GetValue());
	}


	// SqlCArrayBuffer
	// -------------
	TEST_F(ColumnArrayBufferTest, Construction)
	{
		// after construction buffer will be Null
		WCharColumnArrayBuffer arr(L"ColumnName", 24);
		EXPECT_TRUE(arr.IsNull());
		EXPECT_EQ(24, arr.GetNrOfElements());
		EXPECT_EQ(sizeof(SQLWCHAR) * 24, arr.GetBufferLength());
		EXPECT_EQ(L"ColumnName", arr.GetQueryName());
	}


	TEST_F(ColumnArrayBufferTest, SetAndGetValue)
	{
		WCharColumnArrayBuffer arr(L"ColumnName", 24);
		wstring s(L"Hello");
		arr.SetValue(std::vector<SQLWCHAR>(s.begin(), s.end()), SQL_NTS);
		wstring v(arr.GetBuffer().data());
		EXPECT_EQ(s, v);
		EXPECT_EQ(SQL_NTS, arr.GetCb());
	}


	// Basic Read / Write tests
	// ------------------------
	void ColumnTestBase::SetUp()
	{
		m_pDb = OpenTestDb(OdbcVersion::V_3);
		m_pStmt = std::make_shared<SqlStmtHandle>();
		m_pStmt->AllocateWithParent(m_pDb->GetSqlDbcHandle());
	}


	struct FSelectFetcher
	{
		FSelectFetcher(DatabaseProduct dbms, SqlStmtHandlePtr pStmt, exodbctest::TableId tableId, const std::wstring& columnQueryName)
			: m_tableId(tableId)
			, m_columnQueryName(ToDbCase(columnQueryName))
			, m_pStmt(pStmt)
			, m_dbms(dbms)
		{}

		void operator()(SQLINTEGER idValue)
		{
			StatementCloser stmtCloser(m_pStmt);

			wstring tableName = GetTableName(m_tableId);
			tableName = PrependSchemaOrCatalogName(m_dbms, tableName);
			wstring idColName = GetIdColumnName(m_tableId);
			wstring sqlStmt = boost::str(boost::wformat(L"SELECT %s FROM %s WHERE %s = %d") % m_columnQueryName % tableName % idColName % idValue);

			SQLRETURN ret = SQLExecDirect(m_pStmt->GetHandle(), (SQLWCHAR*)sqlStmt.c_str(), SQL_NTS);
			THROW_IFN_SUCCEEDED(SQLExecDirect, ret, SQL_HANDLE_STMT, m_pStmt->GetHandle());
			ret = SQLFetch(m_pStmt->GetHandle());
			THROW_IFN_SUCCESS(SQLFetch, ret, SQL_HANDLE_STMT, m_pStmt->GetHandle());
		}
	
	protected:
		DatabaseProduct m_dbms;
		SqlStmtHandlePtr m_pStmt;
		exodbctest::TableId m_tableId;
		wstring m_columnQueryName;
	};


	struct FInsert2
	{
		FInserter2(DatabaseProduct dbms, SqlStmtHandlePtr pStmt, exodbctest::TableId tableId, const std::wstring& columnQueryName)
			: m_tableId(tableId)
			, m_columnQueryName(ToDbCase(columnQueryName))
			, m_pStmt(pStmt)
			, m_dbms(dbms)
		{
			// prepare the insert statement
			wstring tableName = GetTableName(m_tableId);
			tableName = PrependSchemaOrCatalogName(m_dbms, tableName);
			wstring idColName = GetIdColumnName(m_tableId);
			wstring sqlstmt = boost::str(boost::wformat(L"INSERT INTO %s (%s, %s) VALUES(?, ?)") % tableName % idColName % m_columnQueryName);
			m_stmt.Prepare(sqlstmt);
			SQLRETURN ret = SQLPrepare(pStmt->GetHandle(), (SQLWCHAR*)sqlstmt.c_str(), SQL_NTS);
			THROW_IFN_SUCCESS(SQLPrepare, ret, SQL_HANDLE_STMT, pStmt->GetHandle());
		}

		void operator()()
		{
			SQLRETURN ret = SQLExecute(m_pStmt->GetHandle());
			THROW_IFN_SUCCESS(SQLExecute, ret, SQL_HANDLE_STMT, m_pStmt->GetHandle());
		}

	protected:
		ExecutableStatement m_stmt;
		DatabaseProduct m_dbms;
		SqlStmtHandlePtr m_pStmt;
		exodbctest::TableId m_tableId;
		wstring m_columnQueryName;
	};


	struct FInserter
	{
		FInserter(DatabaseProduct dbms, SqlStmtHandlePtr pStmt, exodbctest::TableId tableId, const std::wstring& columnQueryName)
			: m_tableId(tableId)
			, m_columnQueryName(ToDbCase(columnQueryName))
			, m_pStmt(pStmt)
			, m_dbms(dbms)
		{
			// prepare the insert statement
			wstring tableName = GetTableName(m_tableId);
			tableName = PrependSchemaOrCatalogName(m_dbms, tableName);
			wstring idColName = GetIdColumnName(m_tableId);
			wstring sqlstmt = boost::str(boost::wformat(L"INSERT INTO %s (%s, %s) VALUES(?, ?)") % tableName % idColName % m_columnQueryName);
			SQLRETURN ret = SQLPrepare(pStmt->GetHandle(), (SQLWCHAR*)sqlstmt.c_str(), SQL_NTS);
			THROW_IFN_SUCCESS(SQLPrepare, ret, SQL_HANDLE_STMT, pStmt->GetHandle());
		}

		void operator()()
		{
			SQLRETURN ret = SQLExecute(m_pStmt->GetHandle());
			THROW_IFN_SUCCESS(SQLExecute, ret, SQL_HANDLE_STMT, m_pStmt->GetHandle());
		}



	protected:
		DatabaseProduct m_dbms;
		SqlStmtHandlePtr m_pStmt;
		exodbctest::TableId m_tableId;
		wstring m_columnQueryName;
	};


	TEST_F(ShortColumnTest, ReadValue)
	{
		wstring colName = ToDbCase(L"tsmallint");
		ShortColumnBuffer shortCol(colName);
		shortCol.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::INTEGERTYPES, colName);

		f(1);
		EXPECT_EQ(-32768, shortCol.GetValue());
		f(2);
		EXPECT_EQ(32767, shortCol.GetValue());
		f(3);
		EXPECT_TRUE(shortCol.IsNull());
	}


	TEST_F(ShortColumnTest, WriteValue)
	{
		TableId tableId = TableId::INTEGERTYPES_TMP;

		wstring colName = ToDbCase(L"tsmallint");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			ShortColumnBuffer shortCol(colName);
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol.SetSqlType(SQL_INTEGER);
				shortCol.SetSqlType(SQL_INTEGER);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);
			shortCol.BindParameter(2, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);

			// insert the default null value
			idCol.SetValue(100);
			i();

			// and some non-null values
			idCol.SetValue(101);
			shortCol.SetValue(-32768);
			i();

			idCol.SetValue(102);
			shortCol.SetValue(32767);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			ShortColumnBuffer shortCol(colName);
			shortCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			EXPECT_EQ(-32768, shortCol.GetValue());
			f(102);
			EXPECT_EQ(32767, shortCol.GetValue());
			f(100);
			EXPECT_TRUE(shortCol.IsNull());
		}
	}


	TEST_F(LongColumnTest, ReadValue)
	{
		wstring colName = ToDbCase(L"tint");
		LongColumnBuffer longCol(colName);
		longCol.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::INTEGERTYPES, colName);

		f(3);
		EXPECT_EQ((-2147483647 -1), longCol.GetValue());
		f(4);
		EXPECT_EQ(2147483647, longCol.GetValue());
		f(5);
		EXPECT_TRUE(longCol.IsNull());
	}


	TEST_F(LongColumnTest, WriteValue)
	{
		TableId tableId = TableId::INTEGERTYPES_TMP;

		wstring colName = ToDbCase(L"tint");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			LongColumnBuffer longCol(colName);
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol.SetSqlType(SQL_INTEGER);
				longCol.SetSqlType(SQL_INTEGER);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);
			longCol.BindParameter(2, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);

			// insert the default null value
			idCol.SetValue(100);
			i();

			// and some non-null values
			idCol.SetValue(101);
			longCol.SetValue((-2147483647 - 1));
			i();

			idCol.SetValue(102);
			longCol.SetValue(2147483647);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			LongColumnBuffer longCol(colName);
			longCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			EXPECT_EQ((-2147483647 - 1), longCol.GetValue());
			f(102);
			EXPECT_EQ(2147483647, longCol.GetValue());
			f(100);
			EXPECT_TRUE(longCol.IsNull());
		}
	}


	TEST_F(BigIntColumnTest, ReadValue)
	{
		wstring colName = ToDbCase(L"tbigint");
		BigIntColumnBuffer biCol(colName);
		biCol.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::INTEGERTYPES, colName);

		f(5);
		EXPECT_EQ((-9223372036854775807 - 1), biCol.GetValue());
		f(6);
		EXPECT_EQ(9223372036854775807, biCol.GetValue());
		f(4);
		EXPECT_TRUE(biCol.IsNull());
	}


	TEST_F(BigIntColumnTest, WriteValue)
	{
		TableId tableId = TableId::INTEGERTYPES_TMP;

		wstring colName = ToDbCase(L"tbigint");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			BigIntColumnBuffer biCol(colName);
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol.SetSqlType(SQL_INTEGER);
				biCol.SetSqlType(SQL_BIGINT);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);
			biCol.BindParameter(2, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);

			// insert the default null value
			idCol.SetValue(100);
			i();

			// and some non-null values
			idCol.SetValue(101);
			biCol.SetValue((-9223372036854775807 - 1));
			i();

			idCol.SetValue(102);
			biCol.SetValue(9223372036854775807);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			BigIntColumnBuffer biCol(colName);
			biCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			EXPECT_EQ((-9223372036854775807 - 1), biCol.GetValue());
			f(102);
			EXPECT_EQ(9223372036854775807, biCol.GetValue());
			f(100);
			EXPECT_TRUE(biCol.IsNull());
		}
	}


	TEST_F(DoubleColumnTest, ReadValue)
	{
		wstring colName = L"tdouble";
		DoubleColumnBuffer doubleCol(colName);
		doubleCol.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::FLOATTYPES, colName);

		f(4);
		EXPECT_EQ((0.0), doubleCol.GetValue());
		f(5);
		EXPECT_EQ(3.141592, doubleCol.GetValue());
		f(6);
		EXPECT_EQ(-3.141592, doubleCol.GetValue());

		f(3);
		EXPECT_TRUE(doubleCol.IsNull());
	}


	TEST_F(DoubleColumnTest, WriteValue)
	{
		TableId tableId = TableId::FLOATTYPES_TMP;

		wstring colName = ToDbCase(L"tdouble");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			DoubleColumnBuffer doubleCol(colName);
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol.SetSqlType(SQL_INTEGER);
				doubleCol.SetSqlType(SQL_DOUBLE);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);
			doubleCol.BindParameter(2, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);

			// insert the default null value
			idCol.SetValue(100);
			i();

			// and some non-null values
			idCol.SetValue(101);
			doubleCol.SetValue((0.0));
			i();

			idCol.SetValue(102);
			doubleCol.SetValue(3.141592);
			i();

			idCol.SetValue(103);
			doubleCol.SetValue(-3.141592);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			DoubleColumnBuffer doubleCol(colName);
			doubleCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			EXPECT_EQ(0.0, doubleCol.GetValue());
			f(102);
			EXPECT_EQ(3.141592, doubleCol.GetValue());
			f(103);
			EXPECT_EQ(-3.141592, doubleCol.GetValue());
			f(100);
			EXPECT_TRUE(doubleCol.IsNull());
		}
	}


	TEST_F(RealColumnTest, ReadValue)
	{
		wstring colName = L"tfloat";
		RealColumnBuffer realCol(colName);
		realCol.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::FLOATTYPES, colName);

		f(1);
		EXPECT_EQ((0.0), realCol.GetValue());
		f(2);
		EXPECT_EQ((int)(3.141 * 1e3), (int)(1e3 * realCol.GetValue()));
		f(3);
		EXPECT_EQ((int)(-3.141 * 1e3), (int)(1e3 * realCol.GetValue()));

		f(4);
		EXPECT_TRUE(realCol.IsNull());
	}


	TEST_F(RealColumnTest, WriteValue)
	{
		// \todo: This is pure luck that the test works, rounding errors might occur already on inserting
		TableId tableId = TableId::FLOATTYPES_TMP;

		wstring colName = ToDbCase(L"tfloat");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			RealColumnBuffer realCol(colName);
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol.SetSqlType(SQL_INTEGER);
				realCol.SetSqlType(SQL_REAL);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);
			realCol.BindParameter(2, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);

			// insert the default null value
			idCol.SetValue(100);
			i();

			// and some non-null values
			idCol.SetValue(101);
			realCol.SetValue((0.0));
			i();

			idCol.SetValue(102);
			realCol.SetValue(3.141f);
			i();

			idCol.SetValue(103);
			realCol.SetValue(-3.141f);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			RealColumnBuffer realCol(colName);
			realCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			EXPECT_EQ(0.0, realCol.GetValue());
			f(102);
			EXPECT_EQ((int)(3.141 * 1e3), (int)(1e3 * realCol.GetValue()));
			f(103);
			EXPECT_EQ((int)(-3.141 * 1e3), (int)(1e3 * realCol.GetValue()));
			f(100);
			EXPECT_TRUE(realCol.IsNull());
		}
	}


	TEST_F(NumericColumnTest, Read_18_0_Value)
	{
		wstring colName = L"tdecimal_18_0";
		NumericColumnBuffer num18_0_Col(colName);
		num18_0_Col.SetColumnSize(18);
		num18_0_Col.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::NUMERICTYPES, colName);

		f(1);
		const SQL_NUMERIC_STRUCT& num = num18_0_Col.GetValue();
		SQLBIGINT* pVal = (SQLBIGINT*)&num.val;
		EXPECT_EQ(18, num.precision);
		EXPECT_EQ(0, num.scale);
		EXPECT_EQ(1, num.sign);
		EXPECT_EQ(0, *pVal);

		f(2);
		EXPECT_EQ(18, num.precision);
		EXPECT_EQ(0, num.scale);
		EXPECT_EQ(1, num.sign);
		EXPECT_EQ(123456789012345678, *pVal);

		f(3);
		EXPECT_EQ(18, num.precision);
		EXPECT_EQ(0, num.scale);
		EXPECT_EQ(0, num.sign);
		EXPECT_EQ(123456789012345678, *pVal);

		f(4);
		EXPECT_TRUE(num18_0_Col.IsNull());
	}


	TEST_F(NumericColumnTest, Write_18_0_Value)
	{
		TableId tableId = TableId::NUMERICTYPES_TMP;

		wstring colName = ToDbCase(L"tdecimal_18_0");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			NumericColumnBuffer num18_0_Col(colName);
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol.SetSqlType(SQL_INTEGER);
				num18_0_Col.SetSqlType(SQL_NUMERIC);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);
			num18_0_Col.BindParameter(2, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);

			// insert the default null value
			idCol.SetValue(100);
			i();

			// and some non-null values
			SQLBIGINT v = 0;
			SQL_NUMERIC_STRUCT num;
			::ZeroMemory(num.val, sizeof(num.val));
			num.precision = 18;
			num.scale = 0;
			num.sign = 1;
			::memcpy(num.val, &v, sizeof(v));

			idCol.SetValue(101);
			num18_0_Col.SetValue(num);
			i();

			num.precision = 18;
			num.scale = 0;
			num.sign = 1;
			v = 123456789012345678;
			::memcpy(num.val, &v, sizeof(v));

			idCol.SetValue(102);
			num18_0_Col.SetValue(num);
			i();

			num.precision = 18;
			num.scale = 0;
			num.sign = 0;
			v = 123456789012345678;
			::memcpy(num.val, &v, sizeof(v));

			idCol.SetValue(103);
			num18_0_Col.SetValue(num);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			NumericColumnBuffer num18_0_Col(colName);
			num18_0_Col.SetColumnSize(18);
			num18_0_Col.SetDecimalDigits(0);
			num18_0_Col.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			const SQL_NUMERIC_STRUCT& num = num18_0_Col.GetValue();
			SQLBIGINT* pVal = (SQLBIGINT*)&num.val;
			EXPECT_EQ(18, num.precision);
			EXPECT_EQ(0, num.scale);
			EXPECT_EQ(1, num.sign);
			EXPECT_EQ(0, *pVal);

			f(102);
			EXPECT_EQ(18, num.precision);
			EXPECT_EQ(0, num.scale);
			EXPECT_EQ(1, num.sign);
			EXPECT_EQ(123456789012345678, *pVal);

			f(103);
			EXPECT_EQ(18, num.precision);
			EXPECT_EQ(0, num.scale);
			EXPECT_EQ(0, num.sign);
			EXPECT_EQ(123456789012345678, *pVal);

			f(100);
			EXPECT_TRUE(num18_0_Col.IsNull());
		}
	}


	TEST_F(NumericColumnTest, Read_18_10_Value)
	{
		wstring colName = L"tdecimal_18_10";
		NumericColumnBuffer num18_10_Col(colName);
		num18_10_Col.SetColumnSize(18);
		num18_10_Col.SetDecimalDigits(10);
		num18_10_Col.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::NUMERICTYPES, colName);

		f(4);
		const SQL_NUMERIC_STRUCT& num = num18_10_Col.GetValue();
		SQLBIGINT* pVal = (SQLBIGINT*)&num.val;
		EXPECT_EQ(18, num.precision);
		EXPECT_EQ(10, num.scale);
		EXPECT_EQ(1, num.sign);
		EXPECT_EQ(0, *pVal);

		f(5);
		EXPECT_EQ(18, num.precision);
		EXPECT_EQ(10, num.scale);
		EXPECT_EQ(1, num.sign);
		EXPECT_EQ(123456789012345678, *pVal);

		f(6);
		EXPECT_EQ(18, num.precision);
		EXPECT_EQ(10, num.scale);
		EXPECT_EQ(0, num.sign);
		EXPECT_EQ(123456789012345678, *pVal);

		f(1);
		EXPECT_TRUE(num18_10_Col.IsNull());
	}


	TEST_F(NumericColumnTest, Write_18_10_Value)
	{
		TableId tableId = TableId::NUMERICTYPES_TMP;

		wstring colName = ToDbCase(L"tdecimal_18_10");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			NumericColumnBuffer num18_10_Col(colName);
			bool queryParameterInfo = ! (		m_pDb->GetDbms() == DatabaseProduct::ACCESS
											||	m_pDb->GetDbms() == DatabaseProduct::MY_SQL);
			if(!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				// MySql implements SqlDescribeParam, but returns columnSize of 255 and decimalDigits 0
				// and a type that indicates it wants varchars for numeric-things.
				// but setting everything manually works fine
				idCol.SetSqlType(SQL_INTEGER);
				num18_10_Col.SetSqlType(SQL_NUMERIC);
				num18_10_Col.SetColumnSize(18);
				num18_10_Col.SetDecimalDigits(10);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			num18_10_Col.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idCol.SetValue(100);
			i();

			// and some non-null values
			SQLBIGINT v = 0;
			SQL_NUMERIC_STRUCT num;
			::ZeroMemory(num.val, sizeof(num.val));
			num.precision = 18;
			num.scale = 10;
			num.sign = 1;
			::memcpy(num.val, &v, sizeof(v));

			idCol.SetValue(101);
			num18_10_Col.SetValue(num);
			i();

			num.precision = 18;
			num.scale = 10;
			num.sign = 1;
			v = 123456789012345678;
			::memcpy(num.val, &v, sizeof(v));

			idCol.SetValue(102);
			num18_10_Col.SetValue(num);
			i();

			num.precision = 18;
			num.scale = 10;
			num.sign = 0;
			v = 123456789012345678;
			::memcpy(num.val, &v, sizeof(v));

			idCol.SetValue(103);
			num18_10_Col.SetValue(num);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			NumericColumnBuffer num18_10_Col(colName);
			num18_10_Col.SetColumnSize(18);
			num18_10_Col.SetDecimalDigits(10);
			num18_10_Col.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			const SQL_NUMERIC_STRUCT& num = num18_10_Col.GetValue();
			SQLBIGINT* pVal = (SQLBIGINT*)&num.val;
			EXPECT_EQ(18, num.precision);
			EXPECT_EQ(10, num.scale);
			EXPECT_EQ(1, num.sign);
			EXPECT_EQ(0, *pVal);

			f(102);
			EXPECT_EQ(18, num.precision);
			EXPECT_EQ(10, num.scale);
			EXPECT_EQ(1, num.sign);
			EXPECT_EQ(123456789012345678, *pVal);

			f(103);
			EXPECT_EQ(18, num.precision);
			EXPECT_EQ(10, num.scale);
			EXPECT_EQ(0, num.sign);
			EXPECT_EQ(123456789012345678, *pVal);

			f(100);
			EXPECT_TRUE(num18_10_Col.IsNull());
		}
	}



	TEST_F(NumericColumnTest, Read_5_3_Value)
	{
		wstring colName = L"tdecimal_5_3";
		NumericColumnBuffer num5_3_Col(colName);
		num5_3_Col.SetColumnSize(5);
		num5_3_Col.SetDecimalDigits(3);
		num5_3_Col.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::NUMERICTYPES, colName);

		f(2);
		const SQL_NUMERIC_STRUCT& num = num5_3_Col.GetValue();
		SQLBIGINT* pVal = (SQLBIGINT*)&num.val;
		EXPECT_EQ(5, num.precision);
		EXPECT_EQ(3, num.scale);
		EXPECT_EQ(1, num.sign);
		EXPECT_EQ(12345, *pVal);

		f(1);
		EXPECT_TRUE(num5_3_Col.IsNull());
	}


	TEST_F(NumericColumnTest, Write_5_3_Value)
	{
		TableId tableId = TableId::NUMERICTYPES_TMP;

		wstring colName = ToDbCase(L"tdecimal_5_3");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			NumericColumnBuffer num5_3_Col(colName);
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS
				|| m_pDb->GetDbms() == DatabaseProduct::MY_SQL);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				// MySql implements SqlDescribeParam, but returns columnSize of 255 and decimalDigits 0
				// and a type that indicates it wants varchars for numeric-things.
				// but setting everything manually works fine
				idCol.SetSqlType(SQL_INTEGER);
				num5_3_Col.SetSqlType(SQL_NUMERIC);
				num5_3_Col.SetColumnSize(5);
				num5_3_Col.SetDecimalDigits(3);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			num5_3_Col.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idCol.SetValue(100);
			i();

			// and some non-null values
			SQLBIGINT v = 0;
			SQL_NUMERIC_STRUCT num;
			::ZeroMemory(num.val, sizeof(num.val));
			num.precision = 5;
			num.scale = 3;
			num.sign = 1;
			::memcpy(num.val, &v, sizeof(v));

			idCol.SetValue(101);
			num5_3_Col.SetValue(num);
			i();

			num.precision = 5;
			num.scale = 3;
			num.sign = 1;
			v = 12345;
			::memcpy(num.val, &v, sizeof(v));

			idCol.SetValue(102);
			num5_3_Col.SetValue(num);
			i();

			num.precision = 5;
			num.scale = 3;
			num.sign = 0;
			v = 12345;
			::memcpy(num.val, &v, sizeof(v));

			idCol.SetValue(103);
			num5_3_Col.SetValue(num);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			NumericColumnBuffer num5_3_Col(colName);
			num5_3_Col.SetColumnSize(5);
			num5_3_Col.SetDecimalDigits(3);
			num5_3_Col.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			const SQL_NUMERIC_STRUCT& num = num5_3_Col.GetValue();
			SQLBIGINT* pVal = (SQLBIGINT*)&num.val;
			EXPECT_EQ(5, num.precision);
			EXPECT_EQ(3, num.scale);
			EXPECT_EQ(1, num.sign);
			EXPECT_EQ(0, *pVal);

			f(102);
			EXPECT_EQ(5, num.precision);
			EXPECT_EQ(3, num.scale);
			EXPECT_EQ(1, num.sign);
			EXPECT_EQ(12345, *pVal);

			f(103);
			EXPECT_EQ(5, num.precision);
			EXPECT_EQ(3, num.scale);
			EXPECT_EQ(0, num.sign);
			EXPECT_EQ(12345, *pVal);

			f(100);
			EXPECT_TRUE(num5_3_Col.IsNull());
		}
	}


	TEST_F(TypeTimeColumnTest, ReadValue)
	{
		wstring colName = L"ttime";
		TypeTimeColumnBuffer time(colName);
		time.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::DATETYPES, colName);

		f(1);
		const SQL_TIME_STRUCT& t = time.GetValue();
		EXPECT_EQ(13, t.hour);
		EXPECT_EQ(55, t.minute);
		EXPECT_EQ(56, t.second);

		f(2);
		EXPECT_TRUE(time.IsNull());
	}


	TEST_F(TypeTimeColumnTest, WriteValue)
	{
		TableId tableId = TableId::DATETYPES_TMP;

		wstring colName = ToDbCase(L"ttime");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			TypeTimeColumnBuffer time(colName);
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				time.SetSqlType(SQL_TYPE_TIME);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			time.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idCol.SetValue(100);
			i();

			// and some non-null values
			SQL_TIME_STRUCT t;
			t.hour = 15;
			t.minute = 22;
			t.second = 45;

			idCol.SetValue(101);
			time.SetValue(t);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			TypeTimeColumnBuffer time(colName);
			time.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			const SQL_TIME_STRUCT& t = time.GetValue();
			EXPECT_EQ(15, t.hour);
			EXPECT_EQ(22, t.minute);
			EXPECT_EQ(45, t.second);

			f(100);
			EXPECT_TRUE(time.IsNull());
		}
	}


	TEST_F(TypeDateColumnTest, ReadValue)
	{
		wstring colName = L"tdate";
		TypeDateColumnBuffer date(colName);
		date.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::DATETYPES, colName);

		f(1);
		const SQL_DATE_STRUCT& d = date.GetValue();
		EXPECT_EQ(1983, d.year);
		EXPECT_EQ(1, d.month);
		EXPECT_EQ(26, d.day);

		f(2);
		EXPECT_TRUE(date.IsNull());
	}


	TEST_F(TypeDateColumnTest, WriteValue)
	{
		TableId tableId = TableId::DATETYPES_TMP;

		wstring colName = ToDbCase(L"tdate");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			TypeDateColumnBuffer date(colName);
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				date.SetSqlType(SQL_TYPE_DATE);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			date.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idCol.SetValue(100);
			i();

			// and some non-null values
			SQL_DATE_STRUCT d;
			d.day = 26;
			d.month = 01;
			d.year = 1983;

			idCol.SetValue(101);
			date.SetValue(d);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			TypeDateColumnBuffer date(colName);
			date.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			const SQL_DATE_STRUCT& d = date.GetValue();
			EXPECT_EQ(26, d.day);
			EXPECT_EQ(1, d.month);
			EXPECT_EQ(1983, d.year);

			f(100);
			EXPECT_TRUE(date.IsNull());
		}
	}


	TEST_F(TypeTimestampColumnTest, ReadValue)
	{
		wstring colName = L"ttimestamp";
		{
			// Test without any fraction
			TypeTimestampColumnBuffer tsCol(colName);
			tsCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::DATETYPES, colName);

			f(1);
			const SQL_TIMESTAMP_STRUCT& ts = tsCol.GetValue();
			EXPECT_EQ(1983, ts.year);
			EXPECT_EQ(1, ts.month);
			EXPECT_EQ(26, ts.day);
			EXPECT_EQ(13, ts.hour);
			EXPECT_EQ(55, ts.minute);
			EXPECT_EQ(56, ts.second);
			EXPECT_EQ(0, ts.fraction);

			f(3);
			EXPECT_TRUE(tsCol.IsNull());
		}

		{
			// Test with fractions, database specific precision
			SQLUINTEGER fraction = 0;
			SQLSMALLINT decimalDigits = 0;
			if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
			{
				// SQL-Server has a max precision of 3 for the fractions
				fraction = 123000000;
				decimalDigits = 3;
			}
			if (m_pDb->GetDbms() == DatabaseProduct::DB2)
			{
				// DB2 has a max precision of 6 for the fraction
				fraction = 123456000;
				decimalDigits = 6;
			}

			TypeTimestampColumnBuffer tsCol(colName);
			tsCol.SetDecimalDigits(decimalDigits);
			tsCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::DATETYPES, colName);

			f(2);
			const SQL_TIMESTAMP_STRUCT& ts = tsCol.GetValue();
			EXPECT_EQ(1983, ts.year);
			EXPECT_EQ(1, ts.month);
			EXPECT_EQ(26, ts.day);
			EXPECT_EQ(13, ts.hour);
			EXPECT_EQ(55, ts.minute);
			EXPECT_EQ(56, ts.second);
			EXPECT_EQ(fraction, ts.fraction);
		}
	}


	TEST_F(TypeTimestampColumnTest, WriteValue)
	{
		TableId tableId = TableId::DATETYPES_TMP;

		wstring colName = ToDbCase(L"ttimestamp");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// test without any fractions
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			TypeTimestampColumnBuffer tsCol(colName);
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				tsCol.SetSqlType(SQL_TYPE_TIMESTAMP);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			tsCol.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idCol.SetValue(100);
			i();

			// and some non-null values
			SQL_TIMESTAMP_STRUCT ts;
			ts.day = 9;
			ts.month = 02;
			ts.year = 1982;
			ts.hour = 23;
			ts.minute = 59;
			ts.second = 59;
			ts.fraction = 0;

			idCol.SetValue(101);
			tsCol.SetValue(ts);
			i();

			m_pDb->CommitTrans();
		}

		{
			// and test with fractions
			SQLUINTEGER fraction = 0;
			SQLSMALLINT decimalDigits = 0;
			if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
			{
				// SQL-Server has a max precision of 3 for the fractions
				fraction = 123000000;
			}
			if (m_pDb->GetDbms() == DatabaseProduct::DB2)
			{
				// DB2 has a max precision of 6 for the fraction
				fraction = 123456000;
			}

			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			TypeTimestampColumnBuffer tsCol(colName);
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				tsCol.SetSqlType(SQL_TYPE_TIMESTAMP);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			tsCol.BindParameter(2, m_pStmt, queryParameterInfo);

			SQL_TIMESTAMP_STRUCT ts;
			ts.day = 9;
			ts.month = 02;
			ts.year = 1982;
			ts.hour = 23;
			ts.minute = 59;
			ts.second = 59;
			ts.fraction = fraction;

			idCol.SetValue(102);
			tsCol.SetValue(ts);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			{
				TypeTimestampColumnBuffer tsCol(colName);
				tsCol.BindSelect(1, m_pStmt);
				FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

				f(101);
				const SQL_TIMESTAMP_STRUCT& ts = tsCol.GetValue();
				EXPECT_EQ(9, ts.day);
				EXPECT_EQ(2, ts.month);
				EXPECT_EQ(1982, ts.year);
				EXPECT_EQ(23, ts.hour);
				EXPECT_EQ(59, ts.minute);
				EXPECT_EQ(59, ts.second);
				EXPECT_EQ(0, ts.fraction);

				f(100);
				EXPECT_TRUE(tsCol.IsNull());
			}
			{
				// and test with fractions
				SQLUINTEGER fraction = 0;
				SQLSMALLINT decimalDigits = 0;
				if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
				{
					// SQL-Server has a max precision of 3 for the fractions
					fraction = 123000000;
					decimalDigits = 3;
				}
				if (m_pDb->GetDbms() == DatabaseProduct::DB2)
				{
					// DB2 has a max precision of 6 for the fraction
					fraction = 123456000;
					decimalDigits = 6;
				}

				TypeTimestampColumnBuffer tsCol(colName);
				tsCol.SetDecimalDigits(decimalDigits);
				tsCol.BindSelect(1, m_pStmt);
				FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

				f(102);
				const SQL_TIMESTAMP_STRUCT& ts = tsCol.GetValue();
				EXPECT_EQ(9, ts.day);
				EXPECT_EQ(2, ts.month);
				EXPECT_EQ(1982, ts.year);
				EXPECT_EQ(23, ts.hour);
				EXPECT_EQ(59, ts.minute);
				EXPECT_EQ(59, ts.second);
				EXPECT_EQ(fraction, ts.fraction);
			}
		}
	}



	TEST_F(WCharColumnTest, ReadCharValues)
	{
		// note that when working witch chars, we add one element for the terminating \0 char.
		{
			wstring colName = L"tvarchar";
			WCharColumnArrayBuffer varcharCol(colName, 128 + 1);
			varcharCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES, colName);

			f(1);
			EXPECT_EQ(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", varcharCol.GetWString());
			f(3);
			EXPECT_EQ(L"הצüאיט", varcharCol.GetWString());
			f(2);
			EXPECT_TRUE(varcharCol.IsNull());
		}

		{
			wstring colName = L"tchar";
			WCharColumnArrayBuffer charCol(colName, 128 + 1);
			charCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES, colName);

			// Note: MySql and Access trim the char values, other DBs do not trim
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS || m_pDb->GetDbms() == DatabaseProduct::MY_SQL)
			{
				f(2);
				EXPECT_EQ(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", charCol.GetWString());
				f(4);
				EXPECT_EQ(L"הצüאיט", charCol.GetWString());
			}
			else
			{
				// Some Databases like DB2 do not offer a nchar type. They use 2 CHAR to store a special char like 'ה'
				// Therefore, the number of preceding whitespaces is not equal on DB2 
				f(2);
				EXPECT_EQ(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~                                 ", charCol.GetWString());
				f(4);
				EXPECT_EQ(L"הצüאיט", boost::trim_copy(charCol.GetWString()));
			}

			f(1);
			EXPECT_TRUE(charCol.IsNull());
		}
	}


	TEST_F(WCharColumnTest, WriteVarcharValue)
	{
		TableId tableId = TableId::CHARTYPES_TMP;

		wstring colName = ToDbCase(L"tvarchar");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);
		
		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			WCharColumnArrayBuffer varcharCol(colName, 128 + 1);
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				varcharCol.SetSqlType(SQL_VARCHAR);
				varcharCol.SetColumnSize(128);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			varcharCol.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idCol.SetValue(100);
			i();

			// and some non-null values
			idCol.SetValue(101);
			varcharCol.SetWString(L"Hello World");
			i();

			idCol.SetValue(102);
			varcharCol.SetWString(L"ה צ ü");
			i();

			idCol.SetValue(103);
			varcharCol.SetWString(L"   ");
			i();

			m_pDb->CommitTrans();
		}

		{
			WCharColumnArrayBuffer varcharCol(colName, 128 + 1);
			varcharCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES_TMP, colName);

			f(101);
			EXPECT_EQ(L"Hello World", varcharCol.GetWString());

			f(102);
			EXPECT_EQ(L"ה צ ü", varcharCol.GetWString());

			f(103);
			EXPECT_EQ(L"   ", varcharCol.GetWString());

			f(100);
			EXPECT_TRUE(varcharCol.IsNull());
		}
	}


	TEST_F(WCharColumnTest, WriteCharValue)
	{
		TableId tableId = TableId::CHARTYPES_TMP;

		wstring colName = ToDbCase(L"tchar_10");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			WCharColumnArrayBuffer charCol(colName, 10 + 1);
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				charCol.SetSqlType(SQL_CHAR);
				charCol.SetColumnSize(10);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			charCol.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idCol.SetValue(100);
			i();

			// and some non-null values
			idCol.SetValue(101);
			charCol.SetWString(L"HelloWorld");
			i();

			idCol.SetValue(102);
			charCol.SetWString(L"ה צ ü");
			i();

			idCol.SetValue(103);
			charCol.SetWString(L"abcdefgh  ");
			i();

			m_pDb->CommitTrans();
		}

		{
			WCharColumnArrayBuffer charCol(colName, 128 + 1);
			charCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES_TMP, colName);

			f(101);
			EXPECT_EQ(L"HelloWorld", charCol.GetWString());

			f(102);
			EXPECT_EQ(L"ה צ ü", boost::trim_right_copy(charCol.GetWString()));

			f(103);
			// It seems like MySql always trims whitespaces - even if we've set them explicitly
			if (m_pDb->GetDbms() == DatabaseProduct::MY_SQL)
			{
				EXPECT_EQ(L"abcdefgh", charCol.GetWString());
			}
			else
			{
				EXPECT_EQ(L"abcdefgh  ", charCol.GetWString());
			}

			f(100);
			EXPECT_TRUE(charCol.IsNull());
		}
	}


	TEST_F(CharColumnTest, ReadCharValues)
	{
		// note that when working witch chars, we add one element for the terminating \0 char.
		{
			wstring colName = L"tvarchar";
			CharColumnArrayBuffer varcharCol(colName, 128 + 1);
			varcharCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES, colName);

			f(1);
			EXPECT_EQ(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", varcharCol.GetString());
			f(3);
			EXPECT_EQ("הצüאיט", varcharCol.GetString());
			f(2);
			EXPECT_TRUE(varcharCol.IsNull());
		}

		{
			wstring colName = L"tchar";
			CharColumnArrayBuffer charCol(colName, 128 + 1);
			charCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES, colName);

			// Note: MySql and Access trim the char values, other DBs do not trim
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS || m_pDb->GetDbms() == DatabaseProduct::MY_SQL)
			{
				f(2);
				EXPECT_EQ(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", charCol.GetString());
				f(4);
				EXPECT_EQ("הצüאיט", charCol.GetString());
			}
			else
			{
				// Some Databases like DB2 do not offer a nchar type. They use 2 CHAR to store a special char like 'ה'
				// Therefore, the number of preceding whitespaces is not equal on DB2 
				f(2);
				EXPECT_EQ(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~                                 ", charCol.GetString());
				f(4);
				EXPECT_EQ("הצüאיט", boost::trim_copy(charCol.GetString()));
			}

			f(1);
			EXPECT_TRUE(charCol.IsNull());
		}
	}


	TEST_F(CharColumnTest, WriteVarcharValue)
	{
		TableId tableId = TableId::CHARTYPES_TMP;

		wstring colName = ToDbCase(L"tvarchar");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			CharColumnArrayBuffer varcharCol(colName, 128 + 1);
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				varcharCol.SetSqlType(SQL_VARCHAR);
				varcharCol.SetColumnSize(128);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			varcharCol.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idCol.SetValue(100);
			i();

			// and some non-null values
			idCol.SetValue(101);
			varcharCol.SetString("Hello World");
			i();

			if (m_pDb->GetDbms() != DatabaseProduct::MY_SQL)
			{
				// MySql fails here, with SQLSTATE HY000
				// Incorrect string value: '\xE4 \xF6 \xFC' for column 'tchar_10' at row 1
				idCol.SetValue(102);
				varcharCol.SetString("ה צ ü");
				i();
			}

			idCol.SetValue(103);
			varcharCol.SetString("   ");
			i();

			m_pDb->CommitTrans();
		}

		{
			CharColumnArrayBuffer varcharCol(colName, 128 + 1);
			varcharCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES_TMP, colName);

			f(101);
			EXPECT_EQ("Hello World", varcharCol.GetString());

			if (m_pDb->GetDbms() != DatabaseProduct::MY_SQL)
			{
				f(102);
				EXPECT_EQ("ה צ ü", varcharCol.GetString());
			}

			f(103);
			EXPECT_EQ("   ", varcharCol.GetString());

			f(100);
			EXPECT_TRUE(varcharCol.IsNull());
		}
	}


	TEST_F(CharColumnTest, WriteCharValue)
	{
		TableId tableId = TableId::CHARTYPES_TMP;

		wstring colName = ToDbCase(L"tchar_10");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			CharColumnArrayBuffer charCol(colName, 10 + 1);
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				charCol.SetSqlType(SQL_CHAR);
				charCol.SetColumnSize(10);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			charCol.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idCol.SetValue(100);
			i();

			// and some non-null values
			idCol.SetValue(101);
			charCol.SetString("HelloWorld");
			i();

			if (m_pDb->GetDbms() != DatabaseProduct::MY_SQL)
			{
				// MySql fails here, with SQLSTATE HY000
				// Incorrect string value: '\xE4 \xF6 \xFC' for column 'tchar_10' at row 1
				idCol.SetValue(102);
				charCol.SetString("ה צ ü");
				i();
			}

			idCol.SetValue(103);
			charCol.SetString("abcdefgh  ");
			i();

			m_pDb->CommitTrans();
		}

		{
			CharColumnArrayBuffer charCol(colName, 128 + 1);
			charCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES_TMP, colName);

			f(101);
			EXPECT_EQ("HelloWorld", charCol.GetString());

			if (m_pDb->GetDbms() != DatabaseProduct::MY_SQL)
			{
				f(102);
				EXPECT_EQ("ה צ ü", boost::trim_right_copy(charCol.GetString()));
			}

			f(103);
			// It seems like MySql always trims whitespaces - even if we've set them explicitly
			if (m_pDb->GetDbms() == DatabaseProduct::MY_SQL)
			{
				EXPECT_EQ("abcdefgh", charCol.GetString());
			}
			else
			{
				EXPECT_EQ("abcdefgh  ", charCol.GetString());
			}

			f(100);
			EXPECT_TRUE(charCol.IsNull());
		}
	}


	TEST_F(BinaryColumnTest, ReadValue)
	{
		// Set up the values we expect to read:

		const vector<SQLCHAR> empty = { 
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		};

		const vector<SQLCHAR> ff = { 
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255
		};

		const vector<SQLCHAR> abc = { 
			0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67
		};

		const vector<SQLCHAR> abc_ff = { 
			0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67,
			0xff, 0xff, 0xff, 0xff
		};

		{
			wstring colName = L"tblob";
			BinaryColumnArrayBuffer blobCol(colName, 16);
			blobCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::BLOBTYPES, colName);

			f(1);
			EXPECT_EQ(16, blobCol.GetCb());
			EXPECT_EQ(empty, blobCol.GetBuffer());

			f(2);
			EXPECT_EQ(16, blobCol.GetCb());
			EXPECT_EQ(ff, blobCol.GetBuffer());

			f(3);
			EXPECT_EQ(16, blobCol.GetCb());
			EXPECT_EQ(abc, blobCol.GetBuffer());

			f(4);
			EXPECT_TRUE(blobCol.IsNull());
		}

		{
			wstring colName = L"tvarblob_20";
			BinaryColumnArrayBuffer varBlobCol(colName, 20);
			varBlobCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::BLOBTYPES, colName);

			f(4);
			// This is a varblob. The buffer is sized to 20, but in this column we read only 16 bytes.
			// Cb must reflect this
			EXPECT_EQ(16, varBlobCol.GetCb());
			vector<SQLCHAR> buff = varBlobCol.GetBuffer();
			EXPECT_EQ(20, buff.size());
			// Only compare first 16 elements
			vector<SQLCHAR> first16Elements(buff.begin(), buff.begin() + 16);
			EXPECT_EQ(abc, first16Elements);

			f(5);
			EXPECT_EQ(20, varBlobCol.GetCb());
			EXPECT_EQ(abc_ff, varBlobCol.GetBuffer());

			f(3);
			EXPECT_TRUE(varBlobCol.IsNull());
		}
	}


	TEST_F(BinaryColumnTest, WriteBlobValue)
	{
		// Set up the values we expect to read/write:

		const vector<SQLCHAR> empty = {
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		};

		const vector<SQLCHAR> ff = {
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255
		};

		const vector<SQLCHAR> abc = {
			0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67
		};

		const vector<SQLCHAR> abc_ff = {
			0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67,
			0xff, 0xff, 0xff, 0xff
		};

		TableId tableId = TableId::BLOBTYPES_TMP;

		wstring colName = ToDbCase(L"tblob");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			BinaryColumnArrayBuffer blobCol(colName, 16);
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				blobCol.SetSqlType(SQL_BINARY);
				blobCol.SetColumnSize(16);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			blobCol.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idCol.SetValue(100);
			i();

			// and some non-null values
			idCol.SetValue(101);
			blobCol.SetValue(empty);
			i();

			idCol.SetValue(102);
			blobCol.SetValue(ff);
			i();

			idCol.SetValue(103);
			blobCol.SetValue(abc);
			i();

			m_pDb->CommitTrans();
		}

		{
			// read back written values
			wstring colName = L"tblob";
			BinaryColumnArrayBuffer blobCol(colName, 16);
			blobCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::BLOBTYPES_TMP, colName);

			f(101);
			EXPECT_EQ(16, blobCol.GetCb());
			EXPECT_EQ(empty, blobCol.GetBuffer());

			f(102);
			EXPECT_EQ(16, blobCol.GetCb());
			EXPECT_EQ(ff, blobCol.GetBuffer());

			f(103);
			EXPECT_EQ(16, blobCol.GetCb());
			EXPECT_EQ(abc, blobCol.GetBuffer());

			f(100);
			EXPECT_TRUE(blobCol.IsNull());
		}
	}


	TEST_F(BinaryColumnTest, WriteVarblobValue)
	{
		// Set up the values we expect to read/write:

		const vector<SQLCHAR> empty = {
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		};

		const vector<SQLCHAR> ff = {
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255
		};

		const vector<SQLCHAR> abc = {
			0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67
		};

		const vector<SQLCHAR> abc_ff = {
			0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67,
			0xff, 0xff, 0xff, 0xff
		};

		TableId tableId = TableId::BLOBTYPES_TMP;

		wstring colName = ToDbCase(L"tvarblob_20");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBuffer idCol(idColName);
			BinaryColumnArrayBuffer varblobCol(colName, 20);
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				varblobCol.SetSqlType(SQL_BINARY);
				varblobCol.SetColumnSize(20);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			varblobCol.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idCol.SetValue(100);
			i();

			idCol.SetValue(101);
			varblobCol.SetValue(empty);
			i();

			idCol.SetValue(102);
			varblobCol.SetValue(ff);
			i();

			idCol.SetValue(103);
			varblobCol.SetValue(abc);
			i();

			idCol.SetValue(104);
			varblobCol.SetValue(abc_ff);
			i();

			m_pDb->CommitTrans();
		}

		{
			// read back written values
			wstring colName = L"tvarblob_20";
			BinaryColumnArrayBuffer varblobCol(colName, 20);
			varblobCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::BLOBTYPES_TMP, colName);

			// This is a varblob. The buffer is sized to 20, but in this column we read only 16 bytes.
			// Cb must reflect this
			const vector<SQLCHAR>& buff = varblobCol.GetBuffer();
			EXPECT_EQ(20, buff.size());
			// Only compare first 16 elements in the following tests, except where we really put in 20 bytes.


			f(101);
			EXPECT_EQ(16, varblobCol.GetCb());
			{
				vector<SQLCHAR> first16Elements(buff.begin(), buff.begin() + 16);
				EXPECT_EQ(empty, first16Elements);
			}

			f(102);
			EXPECT_EQ(16, varblobCol.GetCb());
			{
				vector<SQLCHAR> first16Elements(buff.begin(), buff.begin() + 16);
				EXPECT_EQ(ff, first16Elements);
			}

			f(103);
			EXPECT_EQ(16, varblobCol.GetCb());
			{
				vector<SQLCHAR> first16Elements(buff.begin(), buff.begin() + 16);
				EXPECT_EQ(abc, first16Elements);
			}

			// here we read 20 bytes
			f(104);
			EXPECT_EQ(20, varblobCol.GetCb());
			EXPECT_EQ(abc_ff, varblobCol.GetBuffer());

			f(100);
			EXPECT_TRUE(varblobCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, ReadShortValue)
	{
		wstring colName = ToDbCase(L"tsmallint");
		SQLSMALLINT buffer = 0;
		SqlCPointerBuffer shortCol(colName, SQL_INTEGER, &buffer, SQL_C_SSHORT, sizeof(buffer), ColumnFlag::CF_NONE, 0, 0);
		shortCol.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::INTEGERTYPES, colName);

		f(1);
		EXPECT_EQ(-32768, buffer);
		f(2);
		EXPECT_EQ(32767, buffer);
		f(3);
		EXPECT_TRUE(shortCol.IsNull());
	}


	TEST_F(SqlCPointerTest, ReadLongValue)
	{
		wstring colName = ToDbCase(L"tint");
		SQLINTEGER buffer = 0;
		SqlCPointerBuffer longCol(colName, SQL_INTEGER, &buffer, SQL_C_SLONG, sizeof(buffer), ColumnFlag::CF_NONE, 0, 0);

		longCol.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::INTEGERTYPES, colName);

		f(3);
		EXPECT_EQ((-2147483647 - 1), buffer);
		f(4);
		EXPECT_EQ(2147483647, buffer);
		f(5);
		EXPECT_TRUE(longCol.IsNull());
	}


	TEST_F(SqlCPointerTest, ReadBigIntValue)
	{
		wstring colName = ToDbCase(L"tbigint");
		SQLBIGINT buffer = 0;
		SqlCPointerBuffer biCol(colName, SQL_INTEGER, &buffer, SQL_C_SBIGINT, sizeof(buffer), ColumnFlag::CF_NONE, 0, 0);

		biCol.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::INTEGERTYPES, colName);

		f(5);
		EXPECT_EQ((-9223372036854775807 - 1), buffer);
		f(6);
		EXPECT_EQ(9223372036854775807, buffer);
		f(4);
		EXPECT_TRUE(biCol.IsNull());
	}


	TEST_F(SqlCPointerTest, ReadBlobValue)
	{
		// Set up the values we expect to read:

		const vector<SQLCHAR> empty = {
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		};

		const vector<SQLCHAR> ff = {
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255
		};

		const vector<SQLCHAR> abc = {
			0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67
		};

		const vector<SQLCHAR> abc_ff = {
			0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67,
			0xff, 0xff, 0xff, 0xff
		};

		{
			wstring colName = L"tblob";
			vector<SQLCHAR> buffer(16);
			SqlCPointerBuffer blobCol(colName, SQL_BINARY, &buffer[0], SQL_C_BINARY, 16 * sizeof(SQLCHAR), ColumnFlag::CF_NONE, 0, 0);
			blobCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::BLOBTYPES, colName);

			f(1);
			EXPECT_EQ(16, blobCol.GetCb());
			EXPECT_EQ(empty, buffer);

			f(2);
			EXPECT_EQ(16, blobCol.GetCb());
			EXPECT_EQ(ff, buffer);

			f(3);
			EXPECT_EQ(16, blobCol.GetCb());
			EXPECT_EQ(abc, buffer);

			f(4);
			EXPECT_TRUE(blobCol.IsNull());
		}

		{
			wstring colName = L"tvarblob_20";
			vector<SQLCHAR> buffer(20);
			SqlCPointerBuffer varBlobCol(colName, SQL_BINARY, &buffer[0], SQL_C_BINARY, 20 * sizeof(SQLCHAR), ColumnFlag::CF_NONE, 0, 0);
			varBlobCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::BLOBTYPES, colName);

			f(4);
			// This is a varblob. The buffer is sized to 20, but in this column we read only 16 bytes.
			// Cb must reflect this
			EXPECT_EQ(16, varBlobCol.GetCb());
			EXPECT_EQ(20, buffer.size());
			// Only compare first 16 elements
			vector<SQLCHAR> first16Elements(buffer.begin(), buffer.begin() + 16);
			EXPECT_EQ(abc, first16Elements);

			f(5);
			EXPECT_EQ(20, varBlobCol.GetCb());
			EXPECT_EQ(abc_ff, buffer);

			f(3);
			EXPECT_TRUE(varBlobCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, ReadCharValues)
	{
		// note that when working witch chars, we add one element for the terminating \0 char.
		{
			wstring colName = L"tvarchar";
			std::vector<SQLCHAR> buffer(128 + 1);
			SqlCPointerBuffer varcharCol(colName, SQL_VARCHAR, &buffer[0], SQL_C_CHAR, (128 + 1) * sizeof(SQLCHAR), ColumnFlag::CF_NONE, 128, 0);
			varcharCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES, colName);

			string s;

			f(1);
			s = (char*)buffer.data();
			EXPECT_EQ(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", s);
			f(3);
			s = (char*)buffer.data();
			EXPECT_EQ("הצüאיט", s);
			f(2);
			EXPECT_TRUE(varcharCol.IsNull());
		}

		{
			wstring colName = L"tchar";
			std::vector<SQLCHAR> buffer(128 + 1);
			SqlCPointerBuffer charCol(colName, SQL_CHAR, &buffer[0], SQL_C_CHAR, (128 + 1) * sizeof(SQLCHAR), ColumnFlag::CF_NONE, 128, 0);
			charCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES, colName);

			string s;
			// Note: MySql and Access trim the char values, other DBs do not trim
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS || m_pDb->GetDbms() == DatabaseProduct::MY_SQL)
			{
				f(2);
				s = (char*)buffer.data();
				EXPECT_EQ(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", s);
				f(4);
				s = (char*)buffer.data();
				EXPECT_EQ("הצüאיט", s);
			}
			else
			{
				// Some Databases like DB2 do not offer a nchar type. They use 2 CHAR to store a special char like 'ה'
				// Therefore, the number of preceding whitespaces is not equal on DB2 
				f(2);
				s = (char*)buffer.data();
				EXPECT_EQ(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~                                 ", s);
				f(4);
				s = (char*)buffer.data();
				EXPECT_EQ("הצüאיט", boost::trim_copy(s));
			}

			f(1);
			EXPECT_TRUE(charCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, ReadWCharValues)
	{
		// note that when working witch chars, we add one element for the terminating \0 char.
		{
			wstring colName = L"tvarchar";
			std::vector<SQLWCHAR> buffer(128 + 1);
			SqlCPointerBuffer varcharCol(colName, SQL_VARCHAR, &buffer[0], SQL_C_WCHAR, (128 + 1) * sizeof(SQLWCHAR), ColumnFlag::CF_NONE, 128, 0);
			varcharCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES, colName);

			wstring ws;
			f(1);
			ws = buffer.data();
			EXPECT_EQ(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", ws);
			f(3);
			ws = buffer.data();
			EXPECT_EQ(L"הצüאיט", ws);
			f(2);
			EXPECT_TRUE(varcharCol.IsNull());
		}

		{
			wstring colName = L"tchar";
			std::vector<SQLWCHAR> buffer(128 + 1);
			SqlCPointerBuffer charCol(colName, SQL_CHAR, &buffer[0], SQL_C_WCHAR, (128 + 1) * sizeof(SQLWCHAR), ColumnFlag::CF_NONE, 128, 0);
			charCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES, colName);

			// Note: MySql and Access trim the char values, other DBs do not trim
			wstring ws;
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS || m_pDb->GetDbms() == DatabaseProduct::MY_SQL)
			{
				f(2);
				ws = buffer.data();
				EXPECT_EQ(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", ws);
				f(4);
				ws = buffer.data();
				EXPECT_EQ(L"הצüאיט", ws);
			}
			else
			{
				// Some Databases like DB2 do not offer a nchar type. They use 2 CHAR to store a special char like 'ה'
				// Therefore, the number of preceding whitespaces is not equal on DB2 
				f(2);
				ws = buffer.data();
				EXPECT_EQ(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~                                 ", ws);
				f(4);
				ws = buffer.data();
				EXPECT_EQ(L"הצüאיט", boost::trim_copy(ws));
			}

			f(1);
			EXPECT_TRUE(charCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, ReadDateValues)
	{
		wstring colName = L"tdate";

		SQL_DATE_STRUCT buffer;
		SqlCPointerBuffer date(colName, SQL_DATE, &buffer, SQL_C_TYPE_DATE, sizeof(buffer), ColumnFlag::CF_NONE, 0, 0);
		date.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::DATETYPES, colName);

		f(1);
		EXPECT_EQ(1983, buffer.year);
		EXPECT_EQ(1, buffer.month);
		EXPECT_EQ(26, buffer.day);

		f(2);
		EXPECT_TRUE(date.IsNull());
	}


	TEST_F(SqlCPointerTest, ReadTimeValue)
	{
		wstring colName = L"ttime";
		SQL_TIME_STRUCT buffer;
		SqlCPointerBuffer time(colName, SQL_TIME, &buffer, SQL_C_TYPE_TIME, sizeof(buffer), ColumnFlag::CF_NONE, 0, 0);
		time.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::DATETYPES, colName);

		f(1);
		EXPECT_EQ(13, buffer.hour);
		EXPECT_EQ(55, buffer.minute);
		EXPECT_EQ(56, buffer.second);

		f(2);
		EXPECT_TRUE(time.IsNull());
	}


	TEST_F(SqlCPointerTest, ReadTimestampValue)
	{
		wstring colName = L"ttimestamp";
		{
			// Test without any fraction
			SQL_TIMESTAMP_STRUCT buffer;
			SqlCPointerBuffer tsCol(colName, SQL_TIMESTAMP, &buffer, SQL_C_TYPE_TIMESTAMP, sizeof(buffer), ColumnFlag::CF_NONE, 0, 0);
			tsCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::DATETYPES, colName);

			f(1);
			EXPECT_EQ(1983, buffer.year);
			EXPECT_EQ(1, buffer.month);
			EXPECT_EQ(26, buffer.day);
			EXPECT_EQ(13, buffer.hour);
			EXPECT_EQ(55, buffer.minute);
			EXPECT_EQ(56, buffer.second);
			EXPECT_EQ(0, buffer.fraction);

			f(3);
			EXPECT_TRUE(tsCol.IsNull());
		}

		{
			// Test with fractions, database specific precision
			SQLUINTEGER fraction = 0;
			SQLSMALLINT decimalDigits = 0;
			if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
			{
				// SQL-Server has a max precision of 3 for the fractions
				fraction = 123000000;
				decimalDigits = 3;
			}
			if (m_pDb->GetDbms() == DatabaseProduct::DB2)
			{
				// DB2 has a max precision of 6 for the fraction
				fraction = 123456000;
				decimalDigits = 6;
			}

			SQL_TIMESTAMP_STRUCT buffer;
			SqlCPointerBuffer tsCol(colName, SQL_TIMESTAMP, &buffer, SQL_C_TYPE_TIMESTAMP, sizeof(buffer), ColumnFlag::CF_NONE, 0, decimalDigits);
			tsCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::DATETYPES, colName);

			f(2);
			EXPECT_EQ(1983, buffer.year);
			EXPECT_EQ(1, buffer.month);
			EXPECT_EQ(26, buffer.day);
			EXPECT_EQ(13, buffer.hour);
			EXPECT_EQ(55, buffer.minute);
			EXPECT_EQ(56, buffer.second);
			EXPECT_EQ(fraction, buffer.fraction);
		}
	}


	TEST_F(SqlCPointerTest, ReadDoubleValue)
	{
		wstring colName = L"tdouble";
		SQLDOUBLE buffer;
		SqlCPointerBuffer doubleCol(colName, SQL_DOUBLE, &buffer, SQL_C_DOUBLE, sizeof(buffer), ColumnFlag::CF_NONE, 0, 0);
		doubleCol.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::FLOATTYPES, colName);

		f(4);
		EXPECT_EQ((0.0), buffer);
		f(5);
		EXPECT_EQ(3.141592, buffer);
		f(6);
		EXPECT_EQ(-3.141592, buffer);

		f(3);
		EXPECT_TRUE(doubleCol.IsNull());
	}


	TEST_F(SqlCPointerTest, ReadRealValue)
	{
		wstring colName = L"tfloat";
		SQLREAL buffer;
		SqlCPointerBuffer realCol(colName, SQL_REAL, &buffer, SQL_C_FLOAT, sizeof(buffer), ColumnFlag::CF_NONE, 0, 0);
		realCol.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::FLOATTYPES, colName);

		f(1);
		EXPECT_EQ((0.0), buffer);
		f(2);
		EXPECT_EQ((int)(3.141 * 1e3), (int)(1e3 * buffer));
		f(3);
		EXPECT_EQ((int)(-3.141 * 1e3), (int)(1e3 * buffer));

		f(4);
		EXPECT_TRUE(realCol.IsNull());
	}


	TEST_F(SqlCPointerTest, ReadNumeric_18_0_Value)
	{
		wstring colName = L"tdecimal_18_0";
		SQL_NUMERIC_STRUCT buffer;
		SqlCPointerBuffer num18_0_Col(colName, SQL_NUMERIC, &buffer, SQL_C_NUMERIC, sizeof(buffer), ColumnFlag::CF_NONE, 18, 0);
		num18_0_Col.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::NUMERICTYPES, colName);

		f(1);
		SQLBIGINT* pVal = (SQLBIGINT*)&buffer.val;
		EXPECT_EQ(18, buffer.precision);
		EXPECT_EQ(0, buffer.scale);
		EXPECT_EQ(1, buffer.sign);
		EXPECT_EQ(0, *pVal);

		f(2);
		EXPECT_EQ(18, buffer.precision);
		EXPECT_EQ(0, buffer.scale);
		EXPECT_EQ(1, buffer.sign);
		EXPECT_EQ(123456789012345678, *pVal);

		f(3);
		EXPECT_EQ(18, buffer.precision);
		EXPECT_EQ(0, buffer.scale);
		EXPECT_EQ(0, buffer.sign);
		EXPECT_EQ(123456789012345678, *pVal);

		f(4);
		EXPECT_TRUE(num18_0_Col.IsNull());
	}


	TEST_F(SqlCPointerTest, ReadNumeric_18_10_Value)
	{
		wstring colName = L"tdecimal_18_10";
		SQL_NUMERIC_STRUCT buffer;
		SqlCPointerBuffer num18_10_Col(colName, SQL_NUMERIC, &buffer, SQL_C_NUMERIC, sizeof(buffer), ColumnFlag::CF_NONE, 18, 10);
		num18_10_Col.SetColumnSize(18);
		num18_10_Col.SetDecimalDigits(10);
		num18_10_Col.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::NUMERICTYPES, colName);

		f(4);
		SQLBIGINT* pVal = (SQLBIGINT*)&buffer.val;
		EXPECT_EQ(18, buffer.precision);
		EXPECT_EQ(10, buffer.scale);
		EXPECT_EQ(1, buffer.sign);
		EXPECT_EQ(0, *pVal);

		f(5);
		EXPECT_EQ(18, buffer.precision);
		EXPECT_EQ(10, buffer.scale);
		EXPECT_EQ(1, buffer.sign);
		EXPECT_EQ(123456789012345678, *pVal);

		f(6);
		EXPECT_EQ(18, buffer.precision);
		EXPECT_EQ(10, buffer.scale);
		EXPECT_EQ(0, buffer.sign);
		EXPECT_EQ(123456789012345678, *pVal);

		f(1);
		EXPECT_TRUE(num18_10_Col.IsNull());
	}


	TEST_F(SqlCPointerTest, ReadNumeric_5_3_Value)
	{
		wstring colName = L"tdecimal_5_3";
		SQL_NUMERIC_STRUCT buffer;
		SqlCPointerBuffer num5_3_Col(colName, SQL_NUMERIC, &buffer, SQL_C_NUMERIC, sizeof(buffer), ColumnFlag::CF_NONE, 5, 3);
		num5_3_Col.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::NUMERICTYPES, colName);

		f(2);
		SQLBIGINT* pVal = (SQLBIGINT*)&buffer.val;
		EXPECT_EQ(5, buffer.precision);
		EXPECT_EQ(3, buffer.scale);
		EXPECT_EQ(1, buffer.sign);
		EXPECT_EQ(12345, *pVal);

		f(1);
		EXPECT_TRUE(num5_3_Col.IsNull());
	}


	TEST_F(SqlCPointerTest, WriteBlobValue)
	{
		// Set up the values we expect to read/write:

		const vector<SQLCHAR> empty = {
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		};

		const vector<SQLCHAR> ff = {
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255
		};

		const vector<SQLCHAR> abc = {
			0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67
		};

		const vector<SQLCHAR> abc_ff = {
			0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67,
			0xff, 0xff, 0xff, 0xff
		};

		TableId tableId = TableId::BLOBTYPES_TMP;

		wstring colName = ToDbCase(L"tblob");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQLCHAR buffer[16];
			SqlCPointerBuffer blobCol(colName, SQL_BINARY, (SQLPOINTER)buffer, SQL_C_BINARY, sizeof(buffer), ColumnFlag::CF_NULLABLE, 16, 0);

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				blobCol.SetSqlType(SQL_BINARY);
				blobCol.SetColumnSize(16);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			blobCol.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			// Note that we also need to set Cb
			idBuffer = 101;
			blobCol.SetCb(16);
			memcpy((void*)buffer, (void*)&empty[0], sizeof(buffer));
			i();

			idBuffer = 102;
			memcpy((void*)buffer, (void*)&ff[0], sizeof(buffer));
			i();

			idBuffer = 103;
			memcpy((void*)buffer, (void*)&abc[0], sizeof(buffer));
			i();

			m_pDb->CommitTrans();
		}

		{
			// read back written values
			wstring colName = L"tblob";
			vector<SQLCHAR> buffer(16);
			SqlCPointerBuffer blobCol(colName, SQL_BINARY, &buffer[0], SQL_C_BINARY, 16 * sizeof(SQLCHAR), ColumnFlag::CF_NONE, 16, 0);
			blobCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::BLOBTYPES_TMP, colName);

			f(101);
			EXPECT_EQ(16, blobCol.GetCb());
			EXPECT_EQ(empty, buffer);

			f(102);
			EXPECT_EQ(16, blobCol.GetCb());
			EXPECT_EQ(ff, buffer);

			f(103);
			EXPECT_EQ(16, blobCol.GetCb());
			EXPECT_EQ(abc, buffer);

			f(100);
			EXPECT_TRUE(blobCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, WriteVarblobValue)
	{
		// Set up the values we expect to read/write:

		const vector<SQLCHAR> empty = {
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0
		};

		const vector<SQLCHAR> ff = {
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255,
			255, 255, 255, 255
		};

		const vector<SQLCHAR> abc = {
			0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67
		};

		const vector<SQLCHAR> abc_ff = {
			0xab, 0xcd, 0xef, 0xf0,
			0x12, 0x34, 0x56, 0x78,
			0x90, 0xab, 0xcd, 0xef,
			0x01, 0x23, 0x45, 0x67,
			0xff, 0xff, 0xff, 0xff
		};

		TableId tableId = TableId::BLOBTYPES_TMP;

		wstring colName = ToDbCase(L"tvarblob_20");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQLCHAR buffer[20];
			SqlCPointerBuffer varblobCol(colName, SQL_BINARY, (SQLPOINTER)buffer, SQL_C_BINARY, sizeof(buffer), ColumnFlag::CF_NULLABLE, 20, 0);

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				varblobCol.SetSqlType(SQL_BINARY);
				varblobCol.SetColumnSize(20);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			varblobCol.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			// Note that we also need to set Cb
			idBuffer = 101;
			varblobCol.SetCb(empty.size());
			memcpy((void*)buffer, (void*)&empty[0], empty.size());
			i();

			idBuffer = 102;
			varblobCol.SetCb(ff.size());
			memcpy((void*)buffer, (void*)&ff[0], ff.size());
			i();

			idBuffer = 103;
			varblobCol.SetCb(abc.size());
			memcpy((void*)buffer, (void*)&abc[0], abc.size());
			i();

			idBuffer = 104;
			varblobCol.SetCb(abc_ff.size());
			memcpy((void*)buffer, (void*)&abc_ff[0], abc_ff.size());
			i();

			m_pDb->CommitTrans();
		}

		{
			// read back written values
			wstring colName = L"tvarblob_20";
			BinaryColumnArrayBuffer varblobCol(colName, 20);
			varblobCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::BLOBTYPES_TMP, colName);

			// This is a varblob. The buffer is sized to 20, but in this column we read only 16 bytes.
			// Cb must reflect this
			const vector<SQLCHAR>& buff = varblobCol.GetBuffer();
			EXPECT_EQ(20, buff.size());
			// Only compare first 16 elements in the following tests, except where we really put in 20 bytes.


			f(101);
			EXPECT_EQ(16, varblobCol.GetCb());
			{
				vector<SQLCHAR> first16Elements(buff.begin(), buff.begin() + 16);
				EXPECT_EQ(empty, first16Elements);
			}

			f(102);
			EXPECT_EQ(16, varblobCol.GetCb());
			{
				vector<SQLCHAR> first16Elements(buff.begin(), buff.begin() + 16);
				EXPECT_EQ(ff, first16Elements);
			}

			f(103);
			EXPECT_EQ(16, varblobCol.GetCb());
			{
				vector<SQLCHAR> first16Elements(buff.begin(), buff.begin() + 16);
				EXPECT_EQ(abc, first16Elements);
			}

			// here we read 20 bytes
			f(104);
			EXPECT_EQ(20, varblobCol.GetCb());
			EXPECT_EQ(abc_ff, varblobCol.GetBuffer());

			f(100);
			EXPECT_TRUE(varblobCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, WriteBigIntValue)
	{
		TableId tableId = TableId::INTEGERTYPES_TMP;

		wstring colName = ToDbCase(L"tbigint");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQLBIGINT buffer = 0;
			SqlCPointerBuffer biCol(colName, SQL_BIGINT, (SQLPOINTER)&buffer, SQL_C_SBIGINT, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0);

			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol.SetSqlType(SQL_INTEGER);
				biCol.SetSqlType(SQL_BIGINT);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);
			biCol.BindParameter(2, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			biCol.SetCb(sizeof(buffer));
			buffer = (-9223372036854775807 - 1);
			i();

			idBuffer = 102;
			buffer = 9223372036854775807;
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			BigIntColumnBuffer biCol(colName);
			biCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			EXPECT_EQ((-9223372036854775807 - 1), biCol.GetValue());
			f(102);
			EXPECT_EQ(9223372036854775807, biCol.GetValue());
			f(100);
			EXPECT_TRUE(biCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, WriteLongValue)
	{
		TableId tableId = TableId::INTEGERTYPES_TMP;

		wstring colName = ToDbCase(L"tint");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQLINTEGER buffer = 0;
			SqlCPointerBuffer longCol(colName, SQL_INTEGER, (SQLPOINTER)&buffer, SQL_C_SLONG, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0);

			// Prepare the id-col (required) and the col to insert
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol.SetSqlType(SQL_INTEGER);
				longCol.SetSqlType(SQL_INTEGER);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);
			longCol.BindParameter(2, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			longCol.SetCb(sizeof(buffer));
			buffer = (-2147483647 - 1);
			i();

			idBuffer = 102;
			buffer = 2147483647;
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			LongColumnBuffer longCol(colName);
			longCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			EXPECT_EQ((-2147483647 - 1), longCol.GetValue());
			f(102);
			EXPECT_EQ(2147483647, longCol.GetValue());
			f(100);
			EXPECT_TRUE(longCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, WriteShortValue)
	{
		TableId tableId = TableId::INTEGERTYPES_TMP;

		wstring colName = ToDbCase(L"tsmallint");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQLSMALLINT buffer = 0;
			SqlCPointerBuffer shortCol(colName, SQL_SMALLINT, (SQLPOINTER)&buffer, SQL_C_SSHORT, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0);

			// Prepare the id-col (required) and the col to insert
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol.SetSqlType(SQL_INTEGER);
				shortCol.SetSqlType(SQL_INTEGER);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);
			shortCol.BindParameter(2, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			shortCol.SetCb(sizeof(buffer));
			buffer = -32768;
			i();

			idBuffer = 102;
			buffer = 32767;
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			ShortColumnBuffer shortCol(colName);
			shortCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			EXPECT_EQ(-32768, shortCol.GetValue());
			f(102);
			EXPECT_EQ(32767, shortCol.GetValue());
			f(100);
			EXPECT_TRUE(shortCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, WriteCharValue)
	{
		TableId tableId = TableId::CHARTYPES_TMP;

		wstring colName = ToDbCase(L"tchar_10");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQLCHAR buffer[10 + 1];
			SqlCPointerBuffer charCol(colName, SQL_CHAR, (SQLPOINTER)buffer, SQL_C_CHAR, sizeof(buffer), ColumnFlag::CF_NULLABLE, 10, 0);

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				charCol.SetSqlType(SQL_CHAR);
				charCol.SetColumnSize(10);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			charCol.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			charCol.SetCb(SQL_NTS);
			strcpy((char*)buffer, "HelloWorld");
			i();

			if (m_pDb->GetDbms() != DatabaseProduct::MY_SQL)
			{
				// MySql fails here, with SQLSTATE HY000
				// Incorrect string value: '\xE4 \xF6 \xFC' for column 'tchar_10' at row 1
				idBuffer = 102;
				strcpy((char*)buffer, "ה צ ü");
				i();
			}

			idBuffer = 103;
			strcpy((char*)buffer, "abcdefgh  ");
			i();

			m_pDb->CommitTrans();
		}

		{
			CharColumnArrayBuffer charCol(colName, 128 + 1);
			charCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES_TMP, colName);

			f(101);
			EXPECT_EQ("HelloWorld", charCol.GetString());

			if (m_pDb->GetDbms() != DatabaseProduct::MY_SQL)
			{
				f(102);
				EXPECT_EQ("ה צ ü", boost::trim_right_copy(charCol.GetString()));
			}

			f(103);
			// It seems like MySql always trims whitespaces - even if we've set them explicitly
			if (m_pDb->GetDbms() == DatabaseProduct::MY_SQL)
			{
				EXPECT_EQ("abcdefgh", charCol.GetString());
			}
			else
			{
				EXPECT_EQ("abcdefgh  ", charCol.GetString());
			}

			f(100);
			EXPECT_TRUE(charCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, WriteWCharValue)
	{
		TableId tableId = TableId::CHARTYPES_TMP;

		wstring colName = ToDbCase(L"tchar_10");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQLWCHAR buffer[10 + 1];
			SqlCPointerBuffer charCol(colName, SQL_WCHAR, (SQLPOINTER)buffer, SQL_C_WCHAR, sizeof(buffer), ColumnFlag::CF_NULLABLE, 10, 0);

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				charCol.SetSqlType(SQL_CHAR);
				charCol.SetColumnSize(10);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			charCol.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			charCol.SetCb(SQL_NTS);
			wcscpy(buffer, L"HelloWorld");
			i();

			idBuffer = 102;
			wcscpy(buffer, L"ה צ ü");
			i();

			idBuffer = 103;
			wcscpy(buffer, L"abcdefgh  ");
			i();

			m_pDb->CommitTrans();
		}

		{
			WCharColumnArrayBuffer charCol(colName, 128 + 1);
			charCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES_TMP, colName);

			f(101);
			EXPECT_EQ(L"HelloWorld", charCol.GetWString());

			f(102);
			EXPECT_EQ(L"ה צ ü", boost::trim_right_copy(charCol.GetWString()));

			f(103);
			// It seems like MySql always trims whitespaces - even if we've set them explicitly
			if (m_pDb->GetDbms() == DatabaseProduct::MY_SQL)
			{
				EXPECT_EQ(L"abcdefgh", charCol.GetWString());
			}
			else
			{
				EXPECT_EQ(L"abcdefgh  ", charCol.GetWString());
			}

			f(100);
			EXPECT_TRUE(charCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, WriteVarcharValue)
	{
		TableId tableId = TableId::CHARTYPES_TMP;

		wstring colName = ToDbCase(L"tvarchar");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQLCHAR buffer[128 + 1];
			SqlCPointerBuffer varcharCol(colName, SQL_CHAR, (SQLPOINTER)buffer, SQL_C_CHAR, sizeof(buffer), ColumnFlag::CF_NULLABLE, 128, 0);

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				varcharCol.SetSqlType(SQL_VARCHAR);
				varcharCol.SetColumnSize(128);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			varcharCol.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			varcharCol.SetCb(SQL_NTS);
			strcpy((char*)buffer, "Hello World");
			i();

			if (m_pDb->GetDbms() != DatabaseProduct::MY_SQL)
			{
				// MySql fails here, with SQLSTATE HY000
				// Incorrect string value: '\xE4 \xF6 \xFC' for column 'tchar_10' at row 1
				idBuffer = 102;
				strcpy((char*)buffer, "ה צ ü");
				i();
			}

			idBuffer = 103;
			strcpy((char*)buffer, "   ");
			i();

			m_pDb->CommitTrans();
		}

		{
			CharColumnArrayBuffer varcharCol(colName, 128 + 1);
			varcharCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES_TMP, colName);

			f(101);
			EXPECT_EQ("Hello World", varcharCol.GetString());

			if (m_pDb->GetDbms() != DatabaseProduct::MY_SQL)
			{
				f(102);
				EXPECT_EQ("ה צ ü", varcharCol.GetString());
			}

			f(103);
			EXPECT_EQ("   ", varcharCol.GetString());

			f(100);
			EXPECT_TRUE(varcharCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, WriteWVarcharValue)
	{
		TableId tableId = TableId::CHARTYPES_TMP;

		wstring colName = ToDbCase(L"tvarchar");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQLWCHAR buffer[128 + 1];
			SqlCPointerBuffer varcharCol(colName, SQL_WCHAR, (SQLPOINTER)buffer, SQL_C_WCHAR, sizeof(buffer), ColumnFlag::CF_NULLABLE, 128, 0);

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				varcharCol.SetSqlType(SQL_VARCHAR);
				varcharCol.SetColumnSize(128);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			varcharCol.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			varcharCol.SetCb(SQL_NTS);
			wcscpy(buffer, L"Hello World");
			i();

			idBuffer = 102;
			wcscpy(buffer, L"ה צ ü");
			i();

			idBuffer = 103;
			wcscpy(buffer, L"   ");
			i();

			m_pDb->CommitTrans();
		}

		{
			WCharColumnArrayBuffer varcharCol(colName, 128 + 1);
			varcharCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES_TMP, colName);

			f(101);
			EXPECT_EQ(L"Hello World", varcharCol.GetWString());

			f(102);
			EXPECT_EQ(L"ה צ ü", varcharCol.GetWString());

			f(103);
			EXPECT_EQ(L"   ", varcharCol.GetWString());

			f(100);
			EXPECT_TRUE(varcharCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, WriteDateValue)
	{
		TableId tableId = TableId::DATETYPES_TMP;

		wstring colName = ToDbCase(L"tdate");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQL_DATE_STRUCT buffer;
			SqlCPointerBuffer date(colName, SQL_DATE, (SQLPOINTER)&buffer, SQL_C_TYPE_DATE, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0);

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				date.SetSqlType(SQL_TYPE_DATE);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			date.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			buffer.day = 26;
			buffer.month = 01;
			buffer.year = 1983;

			idBuffer = 101;
			date.SetCb(sizeof(buffer));
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			TypeDateColumnBuffer date(colName);
			date.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			const SQL_DATE_STRUCT& d = date.GetValue();
			EXPECT_EQ(26, d.day);
			EXPECT_EQ(1, d.month);
			EXPECT_EQ(1983, d.year);

			f(100);
			EXPECT_TRUE(date.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, WriteTimeValue)
	{
		TableId tableId = TableId::DATETYPES_TMP;

		wstring colName = ToDbCase(L"ttime");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQL_TIME_STRUCT buffer;
			SqlCPointerBuffer time(colName, SQL_TIME, (SQLPOINTER)&buffer, SQL_C_TYPE_TIME, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0);

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				time.SetSqlType(SQL_TYPE_TIME);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			time.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			buffer.hour = 15;
			buffer.minute = 22;
			buffer.second = 45;
			time.SetCb(sizeof(buffer));
			idBuffer = 101;
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			TypeTimeColumnBuffer time(colName);
			time.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			const SQL_TIME_STRUCT& t = time.GetValue();
			EXPECT_EQ(15, t.hour);
			EXPECT_EQ(22, t.minute);
			EXPECT_EQ(45, t.second);

			f(100);
			EXPECT_TRUE(time.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, WriteTimestampValue)
	{
		TableId tableId = TableId::DATETYPES_TMP;

		wstring colName = ToDbCase(L"ttimestamp");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// test without any fractions
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQL_TIMESTAMP_STRUCT buffer;
			SqlCPointerBuffer tsCol(colName, SQL_TIMESTAMP, (SQLPOINTER)&buffer, SQL_C_TYPE_TIMESTAMP, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0);

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				tsCol.SetSqlType(SQL_TYPE_TIMESTAMP);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			tsCol.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			buffer.day = 9;
			buffer.month = 02;
			buffer.year = 1982;
			buffer.hour = 23;
			buffer.minute = 59;
			buffer.second = 59;
			buffer.fraction = 0;
			tsCol.SetCb(sizeof(buffer));
			idBuffer = 101;
			i();

			m_pDb->CommitTrans();
		}

		{
			// and test with fractions
			SQLUINTEGER fraction = 0;
			SQLSMALLINT decimalDigits = 0;
			if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
			{
				// SQL-Server has a max precision of 3 for the fractions
				fraction = 123000000;
			}
			if (m_pDb->GetDbms() == DatabaseProduct::DB2)
			{
				// DB2 has a max precision of 6 for the fraction
				fraction = 123456000;
			}

			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQL_TIMESTAMP_STRUCT buffer;
			SqlCPointerBuffer tsCol(colName, SQL_TIMESTAMP, (SQLPOINTER)&buffer, SQL_C_TYPE_TIMESTAMP, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0);

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol.SetSqlType(SQL_INTEGER);
				tsCol.SetSqlType(SQL_TYPE_TIMESTAMP);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			tsCol.BindParameter(2, m_pStmt, queryParameterInfo);

			buffer.day = 9;
			buffer.month = 02;
			buffer.year = 1982;
			buffer.hour = 23;
			buffer.minute = 59;
			buffer.second = 59;
			buffer.fraction = fraction;
			tsCol.SetCb(sizeof(buffer));
			idBuffer = 102;
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			{
				TypeTimestampColumnBuffer tsCol(colName);
				tsCol.BindSelect(1, m_pStmt);
				FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

				f(101);
				const SQL_TIMESTAMP_STRUCT& ts = tsCol.GetValue();
				EXPECT_EQ(9, ts.day);
				EXPECT_EQ(2, ts.month);
				EXPECT_EQ(1982, ts.year);
				EXPECT_EQ(23, ts.hour);
				EXPECT_EQ(59, ts.minute);
				EXPECT_EQ(59, ts.second);
				EXPECT_EQ(0, ts.fraction);

				f(100);
				EXPECT_TRUE(tsCol.IsNull());
			}
			{
				// and test with fractions
				SQLUINTEGER fraction = 0;
				SQLSMALLINT decimalDigits = 0;
				if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
				{
					// SQL-Server has a max precision of 3 for the fractions
					fraction = 123000000;
					decimalDigits = 3;
				}
				if (m_pDb->GetDbms() == DatabaseProduct::DB2)
				{
					// DB2 has a max precision of 6 for the fraction
					fraction = 123456000;
					decimalDigits = 6;
				}

				TypeTimestampColumnBuffer tsCol(colName);
				tsCol.SetDecimalDigits(decimalDigits);
				tsCol.BindSelect(1, m_pStmt);
				FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

				f(102);
				const SQL_TIMESTAMP_STRUCT& ts = tsCol.GetValue();
				EXPECT_EQ(9, ts.day);
				EXPECT_EQ(2, ts.month);
				EXPECT_EQ(1982, ts.year);
				EXPECT_EQ(23, ts.hour);
				EXPECT_EQ(59, ts.minute);
				EXPECT_EQ(59, ts.second);
				EXPECT_EQ(fraction, ts.fraction);
			}
		}
	}


	TEST_F(SqlCPointerTest, WriteDoubleValue)
	{
		TableId tableId = TableId::FLOATTYPES_TMP;

		wstring colName = ToDbCase(L"tdouble");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQLDOUBLE buffer;
			SqlCPointerBuffer doubleCol(colName, SQL_DOUBLE, (SQLPOINTER)&buffer, SQL_C_DOUBLE, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0);

			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol.SetSqlType(SQL_INTEGER);
				doubleCol.SetSqlType(SQL_DOUBLE);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);
			doubleCol.BindParameter(2, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			doubleCol.SetCb(sizeof(SQLDOUBLE));
			buffer = 0.0;
			i();

			idBuffer = 102;
			buffer = 3.141592;
			i();

			idBuffer = 103;
			buffer = -3.141592;
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			DoubleColumnBuffer doubleCol(colName);
			doubleCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			EXPECT_EQ(0.0, doubleCol.GetValue());
			f(102);
			EXPECT_EQ(3.141592, doubleCol.GetValue());
			f(103);
			EXPECT_EQ(-3.141592, doubleCol.GetValue());
			f(100);
			EXPECT_TRUE(doubleCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, WriteRealValue)
	{
		// \todo: This is pure luck that the test works, rounding errors might occur already on inserting
		TableId tableId = TableId::FLOATTYPES_TMP;

		wstring colName = ToDbCase(L"tfloat");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQLREAL buffer;
			SqlCPointerBuffer realCol(colName, SQL_REAL, (SQLPOINTER)&buffer, SQL_C_FLOAT, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0);

			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol.SetSqlType(SQL_INTEGER);
				realCol.SetSqlType(SQL_REAL);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);
			realCol.BindParameter(2, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			realCol.SetCb(sizeof(buffer));
			buffer = 0.0;
			i();

			idBuffer = 102;
			buffer = 3.141f;
			i();

			idBuffer = 103;
			buffer = -3.141f;
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			RealColumnBuffer realCol(colName);
			realCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			EXPECT_EQ(0.0, realCol.GetValue());
			f(102);
			EXPECT_EQ((int)(3.141 * 1e3), (int)(1e3 * realCol.GetValue()));
			f(103);
			EXPECT_EQ((int)(-3.141 * 1e3), (int)(1e3 * realCol.GetValue()));
			f(100);
			EXPECT_TRUE(realCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, WriteNumeric_18_0_Value)
	{
		TableId tableId = TableId::NUMERICTYPES_TMP;

		wstring colName = ToDbCase(L"tdecimal_18_0");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQL_NUMERIC_STRUCT buffer;
			SqlCPointerBuffer num18_0_Col(colName, SQL_NUMERIC, (SQLPOINTER)&buffer, SQL_C_NUMERIC, sizeof(buffer), ColumnFlag::CF_NULLABLE, 18, 0);

			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol.SetSqlType(SQL_INTEGER);
				num18_0_Col.SetSqlType(SQL_NUMERIC);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);
			num18_0_Col.BindParameter(2, m_pStmt, m_pDb->GetDbms() != DatabaseProduct::ACCESS);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			SQLBIGINT v = 0;
			::ZeroMemory(buffer.val, sizeof(buffer.val));
			buffer.precision = 18;
			buffer.scale = 0;
			buffer.sign = 1;
			::memcpy(buffer.val, &v, sizeof(v));
			num18_0_Col.SetCb(sizeof(buffer));
			idBuffer = 101;
			i();

			buffer.precision = 18;
			buffer.scale = 0;
			buffer.sign = 1;
			v = 123456789012345678;
			::memcpy(buffer.val, &v, sizeof(v));
			idBuffer = 102;
			i();

			buffer.precision = 18;
			buffer.scale = 0;
			buffer.sign = 0;
			v = 123456789012345678;
			::memcpy(buffer.val, &v, sizeof(v));
			idBuffer = 103;
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			NumericColumnBuffer num18_0_Col(colName);
			num18_0_Col.SetColumnSize(18);
			num18_0_Col.SetDecimalDigits(0);
			num18_0_Col.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			const SQL_NUMERIC_STRUCT& num = num18_0_Col.GetValue();
			SQLBIGINT* pVal = (SQLBIGINT*)&num.val;
			EXPECT_EQ(18, num.precision);
			EXPECT_EQ(0, num.scale);
			EXPECT_EQ(1, num.sign);
			EXPECT_EQ(0, *pVal);

			f(102);
			EXPECT_EQ(18, num.precision);
			EXPECT_EQ(0, num.scale);
			EXPECT_EQ(1, num.sign);
			EXPECT_EQ(123456789012345678, *pVal);

			f(103);
			EXPECT_EQ(18, num.precision);
			EXPECT_EQ(0, num.scale);
			EXPECT_EQ(0, num.sign);
			EXPECT_EQ(123456789012345678, *pVal);

			f(100);
			EXPECT_TRUE(num18_0_Col.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, WriteNumeric_18_10_Value)
	{
		TableId tableId = TableId::NUMERICTYPES_TMP;

		wstring colName = ToDbCase(L"tdecimal_18_10");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQL_NUMERIC_STRUCT buffer;
			SqlCPointerBuffer num18_10_Col(colName, SQL_NUMERIC, (SQLPOINTER)&buffer, SQL_C_NUMERIC, sizeof(buffer), ColumnFlag::CF_NULLABLE, 18, 10);

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS
				|| m_pDb->GetDbms() == DatabaseProduct::MY_SQL);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				// MySql implements SqlDescribeParam, but returns columnSize of 255 and decimalDigits 0
				// and a type that indicates it wants varchars for numeric-things.
				// but setting everything manually works fine
				idCol.SetSqlType(SQL_INTEGER);
				num18_10_Col.SetSqlType(SQL_NUMERIC);
				num18_10_Col.SetColumnSize(18);
				num18_10_Col.SetDecimalDigits(10);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			num18_10_Col.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			SQLBIGINT v = 0;
			::ZeroMemory(buffer.val, sizeof(buffer.val));
			buffer.precision = 18;
			buffer.scale = 10;
			buffer.sign = 1;
			::memcpy(buffer.val, &v, sizeof(v));
			num18_10_Col.SetCb(sizeof(buffer));
			idBuffer = 101;
			i();

			buffer.precision = 18;
			buffer.scale = 10;
			buffer.sign = 1;
			v = 123456789012345678;
			::memcpy(buffer.val, &v, sizeof(v));
			idBuffer = 102;
			i();

			buffer.precision = 18;
			buffer.scale = 10;
			buffer.sign = 0;
			v = 123456789012345678;
			::memcpy(buffer.val, &v, sizeof(v));
			idBuffer = 103;
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			NumericColumnBuffer num18_10_Col(colName);
			num18_10_Col.SetColumnSize(18);
			num18_10_Col.SetDecimalDigits(10);
			num18_10_Col.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			const SQL_NUMERIC_STRUCT& num = num18_10_Col.GetValue();
			SQLBIGINT* pVal = (SQLBIGINT*)&num.val;
			EXPECT_EQ(18, num.precision);
			EXPECT_EQ(10, num.scale);
			EXPECT_EQ(1, num.sign);
			EXPECT_EQ(0, *pVal);

			f(102);
			EXPECT_EQ(18, num.precision);
			EXPECT_EQ(10, num.scale);
			EXPECT_EQ(1, num.sign);
			EXPECT_EQ(123456789012345678, *pVal);

			f(103);
			EXPECT_EQ(18, num.precision);
			EXPECT_EQ(10, num.scale);
			EXPECT_EQ(0, num.sign);
			EXPECT_EQ(123456789012345678, *pVal);

			f(100);
			EXPECT_TRUE(num18_10_Col.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, WriteNumeric_5_3_Value)
	{
		TableId tableId = TableId::NUMERICTYPES_TMP;

		wstring colName = ToDbCase(L"tdecimal_5_3");
		wstring idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			SQLINTEGER idBuffer;
			SqlCPointerBuffer idCol(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0);
			SQL_NUMERIC_STRUCT buffer;
			SqlCPointerBuffer num5_3_Col(colName, SQL_NUMERIC, (SQLPOINTER)&buffer, SQL_C_NUMERIC, sizeof(buffer), ColumnFlag::CF_NULLABLE, 5, 3);

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS
				|| m_pDb->GetDbms() == DatabaseProduct::MY_SQL);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				// MySql implements SqlDescribeParam, but returns columnSize of 255 and decimalDigits 0
				// and a type that indicates it wants varchars for numeric-things.
				// but setting everything manually works fine
				idCol.SetSqlType(SQL_INTEGER);
				num5_3_Col.SetSqlType(SQL_NUMERIC);
				num5_3_Col.SetColumnSize(5);
				num5_3_Col.SetDecimalDigits(3);
			}
			FInserter i(m_pDb->GetDbms(), m_pStmt, tableId, colName);
			idCol.BindParameter(1, m_pStmt, queryParameterInfo);
			num5_3_Col.BindParameter(2, m_pStmt, queryParameterInfo);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			SQLBIGINT v = 0;
			::ZeroMemory(buffer.val, sizeof(buffer.val));
			buffer.precision = 5;
			buffer.scale = 3;
			buffer.sign = 1;
			::memcpy(buffer.val, &v, sizeof(v));
			num5_3_Col.SetCb(sizeof(buffer));
			idBuffer = 101;
			i();

			buffer.precision = 5;
			buffer.scale = 3;
			buffer.sign = 1;
			v = 12345;
			::memcpy(buffer.val, &v, sizeof(v));
			idBuffer = 102;
			i();

			buffer.precision = 5;
			buffer.scale = 3;
			buffer.sign = 0;
			v = 12345;
			::memcpy(buffer.val, &v, sizeof(v));
			idBuffer = 103;
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			NumericColumnBuffer num5_3_Col(colName);
			num5_3_Col.SetColumnSize(5);
			num5_3_Col.SetDecimalDigits(3);
			num5_3_Col.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, tableId, colName);

			f(101);
			const SQL_NUMERIC_STRUCT& num = num5_3_Col.GetValue();
			SQLBIGINT* pVal = (SQLBIGINT*)&num.val;
			EXPECT_EQ(5, num.precision);
			EXPECT_EQ(3, num.scale);
			EXPECT_EQ(1, num.sign);
			EXPECT_EQ(0, *pVal);

			f(102);
			EXPECT_EQ(5, num.precision);
			EXPECT_EQ(3, num.scale);
			EXPECT_EQ(1, num.sign);
			EXPECT_EQ(12345, *pVal);

			f(103);
			EXPECT_EQ(5, num.precision);
			EXPECT_EQ(3, num.scale);
			EXPECT_EQ(0, num.sign);
			EXPECT_EQ(12345, *pVal);

			f(100);
			EXPECT_TRUE(num5_3_Col.IsNull());
		}
	}
} //namespace exodbctest
