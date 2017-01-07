/*!
* \file ColumnBufferTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 31.03.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

// Own header
#include "ColumnBufferTest.h"

// Same component headers
#include "exOdbcGTestHelpers.h"

// Other headers
#include "exodbc/ColumnBuffer.h"
#include "exodbc/SqlStatementCloser.h"
#include "exodbc/ExecutableStatement.h"

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
		BigIntColumnBuffer buff(u8"TestBuffer", SQL_UNKNOWN_TYPE);
		EXPECT_TRUE(buff.IsNull());
		EXPECT_EQ(ColumnFlag::CF_NONE, buff.GetFlags());

		// test that we can construct with flags
		BigIntColumnBuffer buff2(u8"SomeName", SQL_INTEGER, ColumnFlag::CF_PRIMARY_KEY | ColumnFlag::CF_SELECT);
		EXPECT_TRUE(buff2.IsNull());
		EXPECT_TRUE(buff2.Test(ColumnFlag::CF_PRIMARY_KEY));
		EXPECT_TRUE(buff2.Test(ColumnFlag::CF_SELECT));
		EXPECT_EQ(ColumnFlag::CF_PRIMARY_KEY | ColumnFlag::CF_SELECT, buff2.GetFlags());
		EXPECT_EQ(u8"SomeName", buff2.GetQueryName());
	}


	TEST_F(ColumnBufferTest, SetAndGetValue)
	{
		BigIntColumnBuffer buff(u8"TestBuffer", SQL_UNKNOWN_TYPE);
		buff.SetValue(13, buff.GetBufferLength());
		EXPECT_EQ(13, buff.GetValue());
	}


	// SqlCArrayBuffer
	// -------------
	TEST_F(ColumnArrayBufferTest, Construction)
	{
		// after construction buffer will be Null
		WCharColumnBuffer arr(24, u8"ColumnName", SQL_UNKNOWN_TYPE);
		EXPECT_TRUE(arr.IsNull());
		EXPECT_EQ(24, arr.GetNrOfElements());
		EXPECT_EQ(sizeof(SQLWCHAR) * 24, arr.GetBufferLength());
		EXPECT_EQ(u8"ColumnName", arr.GetQueryName());
	}


	TEST_F(ColumnArrayBufferTest, SetAndGetValue)
	{
		WCharColumnBuffer arr(24, u8"ColumnName", SQL_UNKNOWN_TYPE);
		wstring s(L"Hello");
		arr.SetValue(std::vector<SQLWCHAR>(s.begin(), s.end()), SQL_NTS);
#ifdef _WIN32
		wstring v(arr.GetBuffer().data());
#else
		wstring v(reinterpret_cast<const wchar_t*>(arr.GetBuffer().data()));
#endif
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
		FSelectFetcher(DatabaseProduct dbms, SqlStmtHandlePtr pStmt, exodbctest::TableId tableId, const std::string& columnQueryName)
			: m_tableId(tableId)
			, m_columnQueryName(ToDbCase(columnQueryName))
			, m_pStmt(pStmt)
			, m_dbms(dbms)
		{}

		void operator()(SQLINTEGER idValue)
		{
			StatementCloser stmtCloser(m_pStmt);

			string tableName = GetTableName(m_tableId);
			tableName = PrependSchemaOrCatalogName(m_dbms, tableName);
			string idColName = GetIdColumnName(m_tableId);
			string sqlStmt = boost::str(boost::format(u8"SELECT %s FROM %s WHERE %s = %d") % m_columnQueryName % tableName % idColName % idValue);

			SQLRETURN ret = SQLExecDirect(m_pStmt->GetHandle(), (SQLAPICHARTYPE*) EXODBCSTR_TO_SQLAPICHARPTR(sqlStmt), SQL_NTS);
			THROW_IFN_SUCCEEDED(SQLExecDirect, ret, SQL_HANDLE_STMT, m_pStmt->GetHandle());
			ret = SQLFetch(m_pStmt->GetHandle());
			THROW_IFN_SUCCESS(SQLFetch, ret, SQL_HANDLE_STMT, m_pStmt->GetHandle());
		}
	
	protected:
		DatabaseProduct m_dbms;
		SqlStmtHandlePtr m_pStmt;
		exodbctest::TableId m_tableId;
		string m_columnQueryName;
	};


	struct FInserter2
	{
		FInserter2(DatabasePtr pDb, exodbctest::TableId tableId, const std::string& columnQueryName)
		{
			// prepare the insert statement
			m_stmt.Init(pDb, true);
			string tableName = GetTableName(tableId);
			tableName = PrependSchemaOrCatalogName(pDb->GetDbms(), tableName);
			string idColName = GetIdColumnName(tableId);
			string sqlstmt = boost::str(boost::format(u8"INSERT INTO %s (%s, %s) VALUES(?, ?)") % tableName % idColName % ToDbCase(columnQueryName));
			m_stmt.Prepare(sqlstmt);
		}

		void operator()()
		{
			m_stmt.ExecutePrepared();
		}

		ExecutableStatement m_stmt;
	};


	struct FInserter
	{
		FInserter(DatabaseProduct dbms, SqlStmtHandlePtr pStmt, exodbctest::TableId tableId, const std::string& columnQueryName)
			: m_tableId(tableId)
			, m_columnQueryName(ToDbCase(columnQueryName))
			, m_pStmt(pStmt)
			, m_dbms(dbms)
		{
			// prepare the insert statement
			string tableName = GetTableName(m_tableId);
			tableName = PrependSchemaOrCatalogName(m_dbms, tableName);
			string idColName = GetIdColumnName(m_tableId);
			string sqlstmt = boost::str(boost::format(u8"INSERT INTO %s (%s, %s) VALUES(?, ?)") % tableName % idColName % m_columnQueryName);
			SQLRETURN ret = SQLPrepare(pStmt->GetHandle(), EXODBCSTR_TO_SQLAPICHARPTR(sqlstmt), SQL_NTS);
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
		string m_columnQueryName;
	};


	TEST_F(ShortColumnTest, ReadValue)
	{
		string colName = ToDbCase(u8"tsmallint");
		ShortColumnBuffer shortCol(colName, SQL_UNKNOWN_TYPE);
		shortCol.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tsmallint");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBufferPtr idCol(new LongColumnBuffer(idColName, SQL_UNKNOWN_TYPE));
			ShortColumnBufferPtr shortCol(new ShortColumnBuffer(colName, SQL_UNKNOWN_TYPE));
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol->SetSqlType(SQL_INTEGER);
				shortCol->SetSqlType(SQL_INTEGER);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(shortCol, 2);

			// insert the default null value
			idCol->SetValue(100);
			i();

			// and some non-null values
			idCol->SetValue(101);
			shortCol->SetValue(-32768);
			i();

			idCol->SetValue(102);
			shortCol->SetValue(32767);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			ShortColumnBuffer shortCol(colName, SQL_UNKNOWN_TYPE);
			shortCol.BindColumn(1, m_pStmt);
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
		string colName = ToDbCase(u8"tint");
		LongColumnBuffer longCol(colName, SQL_UNKNOWN_TYPE);
		longCol.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tint");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBufferPtr idCol(new LongColumnBuffer(idColName, SQL_UNKNOWN_TYPE));
			LongColumnBufferPtr longCol(new LongColumnBuffer(colName, SQL_UNKNOWN_TYPE));
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol->SetSqlType(SQL_INTEGER);
				longCol->SetSqlType(SQL_INTEGER);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(longCol, 2);

			// insert the default null value
			idCol->SetValue(100);
			i();

			// and some non-null values
			idCol->SetValue(101);
			longCol->SetValue((-2147483647 - 1));
			i();

			idCol->SetValue(102);
			longCol->SetValue(2147483647);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			LongColumnBuffer longCol(colName, SQL_UNKNOWN_TYPE);
			longCol.BindColumn(1, m_pStmt);
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
		string colName = ToDbCase(u8"tbigint");
		BigIntColumnBuffer biCol(colName, SQL_UNKNOWN_TYPE);
		biCol.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tbigint");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBufferPtr idCol(new LongColumnBuffer(idColName, SQL_UNKNOWN_TYPE));
			BigIntColumnBufferPtr biCol(new BigIntColumnBuffer(colName, SQL_UNKNOWN_TYPE));
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol->SetSqlType(SQL_INTEGER);
				biCol->SetSqlType(SQL_BIGINT);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(biCol, 2);

			// insert the default null value
			idCol->SetValue(100);
			i();

			// and some non-null values
			idCol->SetValue(101);
			biCol->SetValue((-9223372036854775807 - 1));
			i();

			idCol->SetValue(102);
			biCol->SetValue(9223372036854775807);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			BigIntColumnBuffer biCol(colName, SQL_UNKNOWN_TYPE);
			biCol.BindColumn(1, m_pStmt);
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
		string colName = u8"tdouble";
		DoubleColumnBuffer doubleCol(colName, SQL_UNKNOWN_TYPE);
		doubleCol.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tdouble");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBufferPtr idCol(new LongColumnBuffer(idColName, SQL_UNKNOWN_TYPE));
			DoubleColumnBufferPtr doubleCol(new DoubleColumnBuffer(colName, SQL_UNKNOWN_TYPE));
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol->SetSqlType(SQL_INTEGER);
				doubleCol->SetSqlType(SQL_DOUBLE);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(doubleCol, 2);

			// insert the default null value
			idCol->SetValue(100);
			i();

			// and some non-null values
			idCol->SetValue(101);
			doubleCol->SetValue((0.0));
			i();

			idCol->SetValue(102);
			doubleCol->SetValue(3.141592);
			i();

			idCol->SetValue(103);
			doubleCol->SetValue(-3.141592);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			DoubleColumnBuffer doubleCol(colName, SQL_UNKNOWN_TYPE);
			doubleCol.BindColumn(1, m_pStmt);
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
		string colName = u8"tfloat";
		RealColumnBuffer realCol(colName, SQL_UNKNOWN_TYPE);
		realCol.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tfloat");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBufferPtr idCol(new LongColumnBuffer(idColName, SQL_UNKNOWN_TYPE));
			RealColumnBufferPtr realCol(new RealColumnBuffer(colName, SQL_UNKNOWN_TYPE));
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol->SetSqlType(SQL_INTEGER);
				realCol->SetSqlType(SQL_REAL);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(realCol, 2);

			// insert the default null value
			idCol->SetValue(100);
			i();

			// and some non-null values
			idCol->SetValue(101);
			realCol->SetValue((0.0));
			i();

			idCol->SetValue(102);
			realCol->SetValue(3.141f);
			i();

			idCol->SetValue(103);
			realCol->SetValue(-3.141f);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			RealColumnBuffer realCol(colName, SQL_UNKNOWN_TYPE);
			realCol.BindColumn(1, m_pStmt);
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
		string colName = u8"tdecimal_18_0";
		NumericColumnBuffer num18_0_Col(colName, SQL_UNKNOWN_TYPE);
		num18_0_Col.SetColumnSize(18);
		num18_0_Col.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tdecimal_18_0");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBufferPtr idCol(new LongColumnBuffer(idColName, SQL_UNKNOWN_TYPE));
			NumericColumnBufferPtr num18_0_Col(new NumericColumnBuffer(colName, SQL_UNKNOWN_TYPE));
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS
				|| m_pDb->GetDbms() == DatabaseProduct::MY_SQL)
			{
				idCol->SetSqlType(SQL_INTEGER);
				num18_0_Col->SetSqlType(SQL_NUMERIC);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(num18_0_Col, 2);

			// insert the default null value
			idCol->SetValue(100);
			i();

			// and some non-null values
			SQLBIGINT v = 0;
			SQL_NUMERIC_STRUCT num;
			::memset(num.val, 0, sizeof(num.val));
			num.precision = 18;
			num.scale = 0;
			num.sign = 1;
			::memcpy(num.val, &v, sizeof(v));

			idCol->SetValue(101);
			num18_0_Col->SetValue(num);
			i();

			num.precision = 18;
			num.scale = 0;
			num.sign = 1;
			v = 123456789012345678;
			::memcpy(num.val, &v, sizeof(v));

			idCol->SetValue(102);
			num18_0_Col->SetValue(num);
			i();

			num.precision = 18;
			num.scale = 0;
			num.sign = 0;
			v = 123456789012345678;
			::memcpy(num.val, &v, sizeof(v));

			idCol->SetValue(103);
			num18_0_Col->SetValue(num);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			NumericColumnBuffer num18_0_Col(colName, SQL_UNKNOWN_TYPE);
			num18_0_Col.SetColumnSize(18);
			num18_0_Col.SetDecimalDigits(0);
			num18_0_Col.BindColumn(1, m_pStmt);
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
		string colName = u8"tdecimal_18_10";
		NumericColumnBuffer num18_10_Col(colName, SQL_UNKNOWN_TYPE);
		num18_10_Col.SetColumnSize(18);
		num18_10_Col.SetDecimalDigits(10);
		num18_10_Col.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tdecimal_18_10");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBufferPtr idCol(new LongColumnBuffer(idColName, SQL_UNKNOWN_TYPE));
			NumericColumnBufferPtr num18_10_Col(new NumericColumnBuffer(colName, SQL_UNKNOWN_TYPE));
			bool queryParameterInfo = ! (		m_pDb->GetDbms() == DatabaseProduct::ACCESS
											||	m_pDb->GetDbms() == DatabaseProduct::MY_SQL);
			if(!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				// MySql implements SqlDescribeParam, but returns columnSize of 255 and decimalDigits 0, see #206
				// and a type that indicates it wants varchars for numeric-things.
				// but setting everything manually works fine
				idCol->SetSqlType(SQL_INTEGER);
				num18_10_Col->SetSqlType(SQL_NUMERIC);
				num18_10_Col->SetColumnSize(18);
				num18_10_Col->SetDecimalDigits(10);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(num18_10_Col, 2);

			// insert the default null value
			idCol->SetValue(100);
			i();

			// and some non-null values
			SQLBIGINT v = 0;
			SQL_NUMERIC_STRUCT num;
			::memset(num.val, 0, sizeof(num.val));
			num.precision = 18;
			num.scale = 10;
			num.sign = 1;
			::memcpy(num.val, &v, sizeof(v));

			idCol->SetValue(101);
			num18_10_Col->SetValue(num);
			i();

			num.precision = 18;
			num.scale = 10;
			num.sign = 1;
			v = 123456789012345678;
			::memcpy(num.val, &v, sizeof(v));

			idCol->SetValue(102);
			num18_10_Col->SetValue(num);
			i();

			num.precision = 18;
			num.scale = 10;
			num.sign = 0;
			v = 123456789012345678;
			::memcpy(num.val, &v, sizeof(v));

			idCol->SetValue(103);
			num18_10_Col->SetValue(num);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			NumericColumnBuffer num18_10_Col(colName, SQL_UNKNOWN_TYPE);
			num18_10_Col.SetColumnSize(18);
			num18_10_Col.SetDecimalDigits(10);
			num18_10_Col.BindColumn(1, m_pStmt);
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
		string colName = u8"tdecimal_5_3";
		NumericColumnBuffer num5_3_Col(colName, SQL_UNKNOWN_TYPE);
		num5_3_Col.SetColumnSize(5);
		num5_3_Col.SetDecimalDigits(3);
		num5_3_Col.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tdecimal_5_3");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBufferPtr idCol(new LongColumnBuffer( idColName, SQL_UNKNOWN_TYPE));
			NumericColumnBufferPtr num5_3_Col(new NumericColumnBuffer(colName, SQL_UNKNOWN_TYPE));
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS
				|| m_pDb->GetDbms() == DatabaseProduct::MY_SQL);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				// MySql implements SqlDescribeParam, but returns columnSize of 255 and decimalDigits 0, see #206
				// and a type that indicates it wants varchars for numeric-things.
				// but setting everything manually works fine
				idCol->SetSqlType(SQL_INTEGER);
				num5_3_Col->SetSqlType(SQL_NUMERIC);
				num5_3_Col->SetColumnSize(5);
				num5_3_Col->SetDecimalDigits(3);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(num5_3_Col, 2);

			// insert the default null value
			idCol->SetValue(100);
			i();

			// and some non-null values
			SQLBIGINT v = 0;
			SQL_NUMERIC_STRUCT num;
			::memset(num.val, 0, sizeof(num.val));
			num.precision = 5;
			num.scale = 3;
			num.sign = 1;
			::memcpy(num.val, &v, sizeof(v));

			idCol->SetValue(101);
			num5_3_Col->SetValue(num);
			i();

			num.precision = 5;
			num.scale = 3;
			num.sign = 1;
			v = 12345;
			::memcpy(num.val, &v, sizeof(v));

			idCol->SetValue(102);
			num5_3_Col->SetValue(num);
			i();

			num.precision = 5;
			num.scale = 3;
			num.sign = 0;
			v = 12345;
			::memcpy(num.val, &v, sizeof(v));

			idCol->SetValue(103);
			num5_3_Col->SetValue(num);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			NumericColumnBuffer num5_3_Col(colName, SQL_UNKNOWN_TYPE);
			num5_3_Col.SetColumnSize(5);
			num5_3_Col.SetDecimalDigits(3);
			num5_3_Col.BindColumn(1, m_pStmt);
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
		string colName = u8"ttime";
		TypeTimeColumnBuffer time(colName, SQL_UNKNOWN_TYPE);
		time.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"ttime");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBufferPtr idCol(new LongColumnBuffer(idColName, SQL_UNKNOWN_TYPE));
			TypeTimeColumnBufferPtr time(new TypeTimeColumnBuffer(colName, SQL_UNKNOWN_TYPE));
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				time->SetSqlType(SQL_TYPE_TIME);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(time, 2);

			// insert the default null value
			idCol->SetValue(100);
			i();

			// and some non-null values
			SQL_TIME_STRUCT t;
			t.hour = 15;
			t.minute = 22;
			t.second = 45;

			idCol->SetValue(101);
			time->SetValue(t);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			TypeTimeColumnBuffer time(colName, SQL_UNKNOWN_TYPE);
			time.BindColumn(1, m_pStmt);
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
		string colName = u8"tdate";
		TypeDateColumnBuffer date(colName, SQL_UNKNOWN_TYPE);
		date.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tdate");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBufferPtr idCol(new LongColumnBuffer(idColName, SQL_UNKNOWN_TYPE));
			TypeDateColumnBufferPtr date(new TypeDateColumnBuffer(colName, SQL_UNKNOWN_TYPE));
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				date->SetSqlType(SQL_TYPE_DATE);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(date, 2);

			// insert the default null value
			idCol->SetValue(100);
			i();

			// and some non-null values
			SQL_DATE_STRUCT d;
			d.day = 26;
			d.month = 01;
			d.year = 1983;

			idCol->SetValue(101);
			date->SetValue(d);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			TypeDateColumnBuffer date(colName, SQL_UNKNOWN_TYPE);
			date.BindColumn(1, m_pStmt);
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
		string colName = u8"ttimestamp";
		{
			// Test without any fraction
			TypeTimestampColumnBuffer tsCol(colName, SQL_UNKNOWN_TYPE);
			tsCol.BindColumn(1, m_pStmt);
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

			TypeTimestampColumnBuffer tsCol(colName, SQL_UNKNOWN_TYPE);
			tsCol.SetDecimalDigits(decimalDigits);
			tsCol.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"ttimestamp");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// test without any fractions
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBufferPtr idCol(new LongColumnBuffer(idColName, SQL_UNKNOWN_TYPE));
			TypeTimestampColumnBufferPtr tsCol(new TypeTimestampColumnBuffer(colName, SQL_UNKNOWN_TYPE));
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				tsCol->SetSqlType(SQL_TYPE_TIMESTAMP);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(tsCol, 2);

			// insert the default null value
			idCol->SetValue(100);
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

			idCol->SetValue(101);
			tsCol->SetValue(ts);
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
			LongColumnBufferPtr idCol(new LongColumnBuffer(idColName, SQL_UNKNOWN_TYPE));
			TypeTimestampColumnBufferPtr tsCol(new TypeTimestampColumnBuffer(colName, SQL_UNKNOWN_TYPE));
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				tsCol->SetSqlType(SQL_TYPE_TIMESTAMP);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(tsCol, 2);

			SQL_TIMESTAMP_STRUCT ts;
			ts.day = 9;
			ts.month = 02;
			ts.year = 1982;
			ts.hour = 23;
			ts.minute = 59;
			ts.second = 59;
			ts.fraction = fraction;

			idCol->SetValue(102);
			tsCol->SetValue(ts);
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			{
				TypeTimestampColumnBuffer tsCol(colName, SQL_UNKNOWN_TYPE);
				tsCol.BindColumn(1, m_pStmt);
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

				TypeTimestampColumnBuffer tsCol(colName, SQL_UNKNOWN_TYPE);
				tsCol.SetDecimalDigits(decimalDigits);
				tsCol.BindColumn(1, m_pStmt);
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
			string colName = u8"tvarchar";
			WCharColumnBuffer varcharCol(128 + 1, colName, SQL_UNKNOWN_TYPE);
			varcharCol.BindColumn(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES, colName);

			f(1);
			EXPECT_EQ(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", varcharCol.GetWString());
			f(3);
			EXPECT_EQ(L"äöüàéè", varcharCol.GetWString());
			f(2);
			EXPECT_TRUE(varcharCol.IsNull());
		}

		{
			string colName = u8"tchar";
			WCharColumnBuffer charCol(128 + 1, colName, SQL_UNKNOWN_TYPE);
			charCol.BindColumn(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES, colName);

			// Note: MySql and Access trim the char values, other DBs do not trim
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS || m_pDb->GetDbms() == DatabaseProduct::MY_SQL)
			{
				f(2);
				EXPECT_EQ(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", charCol.GetWString());
				f(4);
				EXPECT_EQ(L"äöüàéè", charCol.GetWString());
			}
			else
			{
				// Some Databases like DB2 do not offer a nchar type. They use 2 CHAR to store a special char like 'ä'
				// Therefore, the number of preceding whitespaces is not equal on DB2 
				f(2);
				EXPECT_EQ(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~                                 ", charCol.GetWString());
				f(4);
				EXPECT_EQ(L"äöüàéè", boost::trim_copy(charCol.GetWString()));
			}

			f(1);
			EXPECT_TRUE(charCol.IsNull());
		}
	}


	TEST_F(WCharColumnTest, WriteVarcharValue)
	{
		TableId tableId = TableId::CHARTYPES_TMP;

		string colName = ToDbCase(u8"tvarchar");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);
		
		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBufferPtr idCol(new LongColumnBuffer(idColName, SQL_UNKNOWN_TYPE));
			WCharColumnBufferPtr varcharCol(new WCharColumnBuffer(128 + 1, colName, SQL_UNKNOWN_TYPE));
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				varcharCol->SetSqlType(SQL_VARCHAR);
				varcharCol->SetColumnSize(128);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(varcharCol, 2);

			// insert the default null value
			idCol->SetValue(100);
			i();

			// and some non-null values
			idCol->SetValue(101);
			varcharCol->SetWString(L"Hello World");
			i();

			idCol->SetValue(102);
			varcharCol->SetWString(L"ä ö ?");
			i();

			idCol->SetValue(103);
			varcharCol->SetWString(L"   ");
			i();

			m_pDb->CommitTrans();
		}

		{
			WCharColumnBuffer varcharCol(128 + 1, colName, SQL_UNKNOWN_TYPE);
			varcharCol.BindColumn(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES_TMP, colName);

			f(101);
			EXPECT_EQ(L"Hello World", varcharCol.GetWString());

			f(102);
			EXPECT_EQ(L"ä ö ?", varcharCol.GetWString());

			f(103);
			EXPECT_EQ(L"   ", varcharCol.GetWString());

			f(100);
			EXPECT_TRUE(varcharCol.IsNull());
		}
	}


	TEST_F(WCharColumnTest, WriteCharValue)
	{
		TableId tableId = TableId::CHARTYPES_TMP;

		string colName = ToDbCase(u8"tchar_10");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBufferPtr idCol(new LongColumnBuffer(idColName, SQL_UNKNOWN_TYPE));
			WCharColumnBufferPtr charCol(new WCharColumnBuffer(10 + 1, colName, SQL_UNKNOWN_TYPE));
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				charCol->SetSqlType(SQL_CHAR);
				charCol->SetColumnSize(10);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(charCol, 2);

			// insert the default null value
			idCol->SetValue(100);
			i();

			// and some non-null values
			idCol->SetValue(101);
			charCol->SetWString(L"HelloWorld");
			i();

			idCol->SetValue(102);
			charCol->SetWString(L"ä ö ?");
			i();

			idCol->SetValue(103);
			charCol->SetWString(L"abcdefgh  ");
			i();

			m_pDb->CommitTrans();
		}

		{
			WCharColumnBuffer charCol(128 + 1, colName, SQL_UNKNOWN_TYPE);
			charCol.BindColumn(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES_TMP, colName);

			f(101);
			EXPECT_EQ(L"HelloWorld", charCol.GetWString());

			f(102);
			EXPECT_EQ(L"ä ö ?", boost::trim_right_copy(charCol.GetWString()));

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
		// \note: We do not use u8 in the compares here: At least on windows against sql server
		// we do not get utf8 strings, but some other encoding.
		{
			string colName = u8"tvarchar";
			CharColumnBuffer varcharCol(128 + 1, colName, SQL_UNKNOWN_TYPE);
			varcharCol.BindColumn(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES, colName);

			f(1);
			EXPECT_EQ(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", varcharCol.GetString());
			f(3);
			EXPECT_EQ("äöüàéè", varcharCol.GetString());
			f(2);
			EXPECT_TRUE(varcharCol.IsNull());
		}

		{
			string colName = u8"tchar";
			CharColumnBuffer charCol(128 + 1, colName, SQL_UNKNOWN_TYPE);
			charCol.BindColumn(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES, colName);

			// Note: MySql and Access trim the char values, other DBs do not trim
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS || m_pDb->GetDbms() == DatabaseProduct::MY_SQL)
			{
				f(2);
				EXPECT_EQ(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", charCol.GetString());
				f(4);
				EXPECT_EQ("äöüàéè", charCol.GetString());
			}
			else
			{
				// Some Databases like DB2 do not offer a nchar type. They use 2 CHAR to store a special char like 'ä'
				// Therefore, the number of preceding whitespaces is not equal on DB2 
				f(2);
				EXPECT_EQ(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~                                 ", charCol.GetString());
				f(4);
				EXPECT_EQ("äöüàéè", boost::trim_copy(charCol.GetString()));
			}

			f(1);
			EXPECT_TRUE(charCol.IsNull());
		}
	}


	TEST_F(CharColumnTest, WriteVarcharValue)
	{
		TableId tableId = TableId::CHARTYPES_TMP;

		string colName = ToDbCase(u8"tvarchar");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBufferPtr idCol(new LongColumnBuffer(idColName, SQL_UNKNOWN_TYPE));
			CharColumnBufferPtr varcharCol(new CharColumnBuffer(128 + 1, colName, SQL_UNKNOWN_TYPE));
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				varcharCol->SetSqlType(SQL_VARCHAR);
				varcharCol->SetColumnSize(128);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(varcharCol, 2);

			// insert the default null value
			idCol->SetValue(100);
			i();

			// and some non-null values
			idCol->SetValue(101);
			varcharCol->SetString("Hello World");
			i();

			if (m_pDb->GetDbms() != DatabaseProduct::MY_SQL)
			{
				// MySql fails here, with SQLSTATE HY000
				// Incorrect string value: '\xE4 \xF6 \xFC' for column 'tchar_10' at row 1
				idCol->SetValue(102);
				varcharCol->SetString("ä ö ?");
				i();
			}

			idCol->SetValue(103);
			varcharCol->SetString("   ");
			i();

			m_pDb->CommitTrans();
		}

		{
			CharColumnBuffer varcharCol(128 + 1, colName, SQL_UNKNOWN_TYPE);
			varcharCol.BindColumn(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES_TMP, colName);

			f(101);
			EXPECT_EQ("Hello World", varcharCol.GetString());

			if (m_pDb->GetDbms() != DatabaseProduct::MY_SQL)
			{
				f(102);
				EXPECT_EQ("ä ö ?", varcharCol.GetString());
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

		string colName = ToDbCase(u8"tchar_10");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBufferPtr idCol(new LongColumnBuffer(idColName, SQL_UNKNOWN_TYPE));
			CharColumnBufferPtr charCol(new CharColumnBuffer(10 + 1, colName, SQL_UNKNOWN_TYPE));
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				charCol->SetSqlType(SQL_CHAR);
				charCol->SetColumnSize(10);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(charCol, 2);

			// insert the default null value
			idCol->SetValue(100);
			i();

			// and some non-null values
			idCol->SetValue(101);
			charCol->SetString("HelloWorld");
			i();

			if (m_pDb->GetDbms() != DatabaseProduct::MY_SQL)
			{
				// MySql fails here, with SQLSTATE HY000
				// Incorrect string value: '\xE4 \xF6 \xFC' for column 'tchar_10' at row 1
				idCol->SetValue(102);
				charCol->SetString("ä ö ?");
				i();
			}

			idCol->SetValue(103);
			charCol->SetString("abcdefgh  ");
			i();

			m_pDb->CommitTrans();
		}

		{
			CharColumnBuffer charCol(128 + 1, colName, SQL_UNKNOWN_TYPE);
			charCol.BindColumn(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES_TMP, colName);

			f(101);
			EXPECT_EQ("HelloWorld", charCol.GetString());

			if (m_pDb->GetDbms() != DatabaseProduct::MY_SQL)
			{
				f(102);
				EXPECT_EQ("ä ö ?", boost::trim_right_copy(charCol.GetString()));
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
			string colName = u8"tblob";
			BinaryColumnBuffer blobCol(16, colName, SQL_UNKNOWN_TYPE);
			blobCol.BindColumn(1, m_pStmt);
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
			string colName = u8"tvarblob_20";
			BinaryColumnBuffer varBlobCol(20, colName, SQL_UNKNOWN_TYPE);
			varBlobCol.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tblob");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBufferPtr idCol(new LongColumnBuffer(idColName, SQL_UNKNOWN_TYPE));
			BinaryColumnBufferPtr blobCol(new BinaryColumnBuffer(16, colName, SQL_UNKNOWN_TYPE));
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				blobCol->SetSqlType(SQL_BINARY);
				blobCol->SetColumnSize(16);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(blobCol, 2);

			// insert the default null value
			idCol->SetValue(100);
			i();

			// and some non-null values
			idCol->SetValue(101);
			blobCol->SetValue(empty);
			i();

			idCol->SetValue(102);
			blobCol->SetValue(ff);
			i();

			idCol->SetValue(103);
			blobCol->SetValue(abc);
			i();

			m_pDb->CommitTrans();
		}

		{
			// read back written values
			string colName = u8"tblob";
			BinaryColumnBuffer blobCol(16, colName, SQL_UNKNOWN_TYPE);
			blobCol.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tvarblob_20");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			LongColumnBufferPtr idCol(new LongColumnBuffer(idColName, SQL_UNKNOWN_TYPE));
			BinaryColumnBufferPtr varblobCol(new BinaryColumnBuffer(20, colName, SQL_UNKNOWN_TYPE));
			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				varblobCol->SetSqlType(SQL_BINARY);
				varblobCol->SetColumnSize(20);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(varblobCol, 2);

			// insert the default null value
			idCol->SetValue(100);
			i();

			idCol->SetValue(101);
			varblobCol->SetValue(empty);
			i();

			idCol->SetValue(102);
			varblobCol->SetValue(ff);
			i();

			idCol->SetValue(103);
			varblobCol->SetValue(abc);
			i();

			idCol->SetValue(104);
			varblobCol->SetValue(abc_ff);
			i();

			m_pDb->CommitTrans();
		}

		{
			// read back written values
			string colName = u8"tvarblob_20";
			BinaryColumnBuffer varblobCol(20, colName, SQL_UNKNOWN_TYPE);
			varblobCol.BindColumn(1, m_pStmt);
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
		string colName = ToDbCase(u8"tsmallint");
		SQLSMALLINT buffer = 0;
		SqlCPointerBuffer shortCol(colName, SQL_INTEGER, &buffer, SQL_C_SSHORT, sizeof(buffer), ColumnFlag::CF_NONE, 0, 0);
		shortCol.BindColumn(1, m_pStmt);
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
		string colName = ToDbCase(u8"tint");
		SQLINTEGER buffer = 0;
		SqlCPointerBuffer longCol(colName, SQL_INTEGER, &buffer, SQL_C_SLONG, sizeof(buffer), ColumnFlag::CF_NONE, 0, 0);

		longCol.BindColumn(1, m_pStmt);
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
		string colName = ToDbCase(u8"tbigint");
		SQLBIGINT buffer = 0;
		SqlCPointerBuffer biCol(colName, SQL_INTEGER, &buffer, SQL_C_SBIGINT, sizeof(buffer), ColumnFlag::CF_NONE, 0, 0);

		biCol.BindColumn(1, m_pStmt);
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
			string colName = u8"tblob";
			vector<SQLCHAR> buffer(16);
			SqlCPointerBuffer blobCol(colName, SQL_BINARY, &buffer[0], SQL_C_BINARY, 16 * sizeof(SQLCHAR), ColumnFlag::CF_NONE, 0, 0);
			blobCol.BindColumn(1, m_pStmt);
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
			string colName = u8"tvarblob_20";
			vector<SQLCHAR> buffer(20);
			SqlCPointerBuffer varBlobCol(colName, SQL_BINARY, &buffer[0], SQL_C_BINARY, 20 * sizeof(SQLCHAR), ColumnFlag::CF_NONE, 0, 0);
			varBlobCol.BindColumn(1, m_pStmt);
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
			string colName = u8"tvarchar";
			std::vector<SQLCHAR> buffer(128 + 1);
			SqlCPointerBuffer varcharCol(colName, SQL_VARCHAR, &buffer[0], SQL_C_CHAR, (128 + 1) * sizeof(SQLCHAR), ColumnFlag::CF_NONE, 128, 0);
			varcharCol.BindColumn(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES, colName);

			string s;

			f(1);
			s = (char*)buffer.data();
			EXPECT_EQ(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", s);
			f(3);
			s = (char*)buffer.data();
			EXPECT_EQ("äöüàéè", s);
			f(2);
			EXPECT_TRUE(varcharCol.IsNull());
		}

		{
			string colName = u8"tchar";
			std::vector<SQLCHAR> buffer(128 + 1);
			SqlCPointerBuffer charCol(colName, SQL_CHAR, &buffer[0], SQL_C_CHAR, (128 + 1) * sizeof(SQLCHAR), ColumnFlag::CF_NONE, 128, 0);
			charCol.BindColumn(1, m_pStmt);
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
				EXPECT_EQ("äöüàéè", s);
			}
			else
			{
				// Some Databases like DB2 do not offer a nchar type. They use 2 CHAR to store a special char like 'ä'
				// Therefore, the number of preceding whitespaces is not equal on DB2 
				f(2);
				s = (char*)buffer.data();
				EXPECT_EQ(" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~                                 ", s);
				f(4);
				s = (char*)buffer.data();
				EXPECT_EQ("äöüàéè", boost::trim_copy(s));
			}

			f(1);
			EXPECT_TRUE(charCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, ReadWCharValues)
	{
		// note that when working witch chars, we add one element for the terminating \0 char.
		{
			string colName = u8"tvarchar";
			std::vector<SQLWCHAR> buffer(128 + 1);
			SqlCPointerBuffer varcharCol(colName, SQL_VARCHAR, &buffer[0], SQL_C_WCHAR, (128 + 1) * sizeof(SQLWCHAR), ColumnFlag::CF_NONE, 128, 0);
			varcharCol.BindColumn(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES, colName);

			wstring ws;
			f(1);
#ifdef _WIN32
			ws = wstring(buffer.data());
#else
			ws = wstring(reinterpret_cast<const wchar_t*>(buffer.data()));
#endif
			EXPECT_EQ(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", ws);
			f(3);
#ifdef _WIN32
			ws = wstring(buffer.data());
#else
			ws = wstring(reinterpret_cast<const wchar_t*>(buffer.data()));
#endif
			EXPECT_EQ(L"äöüàéè", ws);
			f(2);
			EXPECT_TRUE(varcharCol.IsNull());
		}

		{
			string colName = u8"tchar";
			std::vector<SQLWCHAR> buffer(128 + 1);
			SqlCPointerBuffer charCol(colName, SQL_CHAR, &buffer[0], SQL_C_WCHAR, (128 + 1) * sizeof(SQLWCHAR), ColumnFlag::CF_NONE, 128, 0);
			charCol.BindColumn(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES, colName);

			// Note: MySql and Access trim the char values, other DBs do not trim
			wstring ws;
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS || m_pDb->GetDbms() == DatabaseProduct::MY_SQL)
			{
				f(2);
#ifdef _WIN32
				ws = buffer.data();
#else
				ws = reinterpret_cast<const wchar_t*>(buffer.data());
#endif
				EXPECT_EQ(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~", ws);
				f(4);
				ws = buffer.data();
				EXPECT_EQ(L"äöüàéè", ws);
			}
			else
			{
				// Some Databases like DB2 do not offer a nchar type. They use 2 CHAR to store a special char like 'ä'
				// Therefore, the number of preceding whitespaces is not equal on DB2 
				f(2);
				ws = buffer.data();
				EXPECT_EQ(L" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~                                 ", ws);
				f(4);
				ws = buffer.data();
				EXPECT_EQ(L"äöüàéè", boost::trim_copy(ws));
			}

			f(1);
			EXPECT_TRUE(charCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, ReadDateValues)
	{
		string colName = u8"tdate";

		SQL_DATE_STRUCT buffer;
		SqlCPointerBuffer date(colName, SQL_DATE, &buffer, SQL_C_TYPE_DATE, sizeof(buffer), ColumnFlag::CF_NONE, 0, 0);
		date.BindColumn(1, m_pStmt);
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
		string colName = u8"ttime";
		SQL_TIME_STRUCT buffer;
		SqlCPointerBuffer time(colName, SQL_TIME, &buffer, SQL_C_TYPE_TIME, sizeof(buffer), ColumnFlag::CF_NONE, 0, 0);
		time.BindColumn(1, m_pStmt);
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
		string colName = u8"ttimestamp";
		{
			// Test without any fraction
			SQL_TIMESTAMP_STRUCT buffer;
			SqlCPointerBuffer tsCol(colName, SQL_TIMESTAMP, &buffer, SQL_C_TYPE_TIMESTAMP, sizeof(buffer), ColumnFlag::CF_NONE, 0, 0);
			tsCol.BindColumn(1, m_pStmt);
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
			tsCol.BindColumn(1, m_pStmt);
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
		string colName = u8"tdouble";
		SQLDOUBLE buffer;
		SqlCPointerBuffer doubleCol(colName, SQL_DOUBLE, &buffer, SQL_C_DOUBLE, sizeof(buffer), ColumnFlag::CF_NONE, 0, 0);
		doubleCol.BindColumn(1, m_pStmt);
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
		string colName = u8"tfloat";
		SQLREAL buffer;
		SqlCPointerBuffer realCol(colName, SQL_REAL, &buffer, SQL_C_FLOAT, sizeof(buffer), ColumnFlag::CF_NONE, 0, 0);
		realCol.BindColumn(1, m_pStmt);
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
		string colName = u8"tdecimal_18_0";
		SQL_NUMERIC_STRUCT buffer;
		SqlCPointerBuffer num18_0_Col(colName, SQL_NUMERIC, &buffer, SQL_C_NUMERIC, sizeof(buffer), ColumnFlag::CF_NONE, 18, 0);
		num18_0_Col.BindColumn(1, m_pStmt);
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
		string colName = u8"tdecimal_18_10";
		SQL_NUMERIC_STRUCT buffer;
		SqlCPointerBuffer num18_10_Col(colName, SQL_NUMERIC, &buffer, SQL_C_NUMERIC, sizeof(buffer), ColumnFlag::CF_NONE, 18, 10);
		num18_10_Col.SetColumnSize(18);
		num18_10_Col.SetDecimalDigits(10);
		num18_10_Col.BindColumn(1, m_pStmt);
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
		string colName = u8"tdecimal_5_3";
		SQL_NUMERIC_STRUCT buffer;
		SqlCPointerBuffer num5_3_Col(colName, SQL_NUMERIC, &buffer, SQL_C_NUMERIC, sizeof(buffer), ColumnFlag::CF_NONE, 5, 3);
		num5_3_Col.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tblob");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQLCHAR buffer[16];
			SqlCPointerBufferPtr blobCol(new SqlCPointerBuffer(colName, SQL_BINARY, (SQLPOINTER)buffer, SQL_C_BINARY, sizeof(buffer), ColumnFlag::CF_NULLABLE, 16, 0));

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				blobCol->SetSqlType(SQL_BINARY);
				blobCol->SetColumnSize(16);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(blobCol, 2);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			// Note that we also need to set Cb
			idBuffer = 101;
			blobCol->SetCb(16);
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
			string colName = u8"tblob";
			vector<SQLCHAR> buffer(16);
			SqlCPointerBuffer blobCol(colName, SQL_BINARY, &buffer[0], SQL_C_BINARY, 16 * sizeof(SQLCHAR), ColumnFlag::CF_NONE, 16, 0);
			blobCol.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tvarblob_20");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQLCHAR buffer[20];
			SqlCPointerBufferPtr varblobCol(new SqlCPointerBuffer(colName, SQL_BINARY, (SQLPOINTER)buffer, SQL_C_BINARY, sizeof(buffer), ColumnFlag::CF_NULLABLE, 20, 0));

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				varblobCol->SetSqlType(SQL_BINARY);
				varblobCol->SetColumnSize(20);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(varblobCol, 2);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			// Note that we also need to set Cb
			idBuffer = 101;
			varblobCol->SetCb(empty.size());
			memcpy((void*)buffer, (void*)&empty[0], empty.size());
			i();

			idBuffer = 102;
			varblobCol->SetCb(ff.size());
			memcpy((void*)buffer, (void*)&ff[0], ff.size());
			i();

			idBuffer = 103;
			varblobCol->SetCb(abc.size());
			memcpy((void*)buffer, (void*)&abc[0], abc.size());
			i();

			idBuffer = 104;
			varblobCol->SetCb(abc_ff.size());
			memcpy((void*)buffer, (void*)&abc_ff[0], abc_ff.size());
			i();

			m_pDb->CommitTrans();
		}

		{
			// read back written values
			string colName = u8"tvarblob_20";
			BinaryColumnBuffer varblobCol(20, colName, SQL_UNKNOWN_TYPE);
			varblobCol.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tbigint");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQLBIGINT buffer = 0;
			SqlCPointerBufferPtr biCol(new SqlCPointerBuffer(colName, SQL_BIGINT, (SQLPOINTER)&buffer, SQL_C_SBIGINT, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0));

			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol->SetSqlType(SQL_INTEGER);
				biCol->SetSqlType(SQL_BIGINT);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(biCol, 2);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			biCol->SetCb(sizeof(buffer));
			buffer = (-9223372036854775807 - 1);
			i();

			idBuffer = 102;
			buffer = 9223372036854775807;
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			BigIntColumnBuffer biCol(colName, SQL_UNKNOWN_TYPE);
			biCol.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tint");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQLINTEGER buffer = 0;
			SqlCPointerBufferPtr longCol(new SqlCPointerBuffer(colName, SQL_INTEGER, (SQLPOINTER)&buffer, SQL_C_SLONG, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0));

			// Prepare the id-col (required) and the col to insert
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol->SetSqlType(SQL_INTEGER);
				longCol->SetSqlType(SQL_INTEGER);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(longCol, 2);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			longCol->SetCb(sizeof(buffer));
			buffer = (-2147483647 - 1);
			i();

			idBuffer = 102;
			buffer = 2147483647;
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			LongColumnBuffer longCol(colName, SQL_UNKNOWN_TYPE);
			longCol.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tsmallint");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQLSMALLINT buffer = 0;
			SqlCPointerBufferPtr shortCol(new SqlCPointerBuffer(colName, SQL_SMALLINT, (SQLPOINTER)&buffer, SQL_C_SSHORT, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0));

			// Prepare the id-col (required) and the col to insert
			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol->SetSqlType(SQL_INTEGER);
				shortCol->SetSqlType(SQL_INTEGER);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(shortCol, 2);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			shortCol->SetCb(sizeof(buffer));
			buffer = -32768;
			i();

			idBuffer = 102;
			buffer = 32767;
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			ShortColumnBuffer shortCol(colName, SQL_UNKNOWN_TYPE);
			shortCol.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tchar_10");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQLCHAR buffer[10 + 1];
			SqlCPointerBufferPtr charCol(new SqlCPointerBuffer(colName, SQL_CHAR, (SQLPOINTER)buffer, SQL_C_CHAR, sizeof(buffer), ColumnFlag::CF_NULLABLE, 10, 0));

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				charCol->SetSqlType(SQL_CHAR);
				charCol->SetColumnSize(10);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(charCol, 2);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			charCol->SetCb(SQL_NTS);
			strcpy((char*)buffer, "HelloWorld");
			i();

			if (m_pDb->GetDbms() != DatabaseProduct::MY_SQL)
			{
				// MySql fails here, with SQLSTATE HY000
				// Incorrect string value: '\xE4 \xF6 \xFC' for column 'tchar_10' at row 1
				idBuffer = 102;
				strcpy((char*)buffer, "ä ö ?");
				i();
			}

			idBuffer = 103;
			strcpy((char*)buffer, "abcdefgh  ");
			i();

			m_pDb->CommitTrans();
		}

		{
			CharColumnBuffer charCol(128 + 1, colName, SQL_UNKNOWN_TYPE);
			charCol.BindColumn(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES_TMP, colName);

			f(101);
			EXPECT_EQ("HelloWorld", charCol.GetString());

			if (m_pDb->GetDbms() != DatabaseProduct::MY_SQL)
			{
				f(102);
				EXPECT_EQ("ä ö ?", boost::trim_right_copy(charCol.GetString()));
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

		string colName = ToDbCase(u8"tchar_10");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQLWCHAR buffer[10 + 1];
			SqlCPointerBufferPtr charCol(new SqlCPointerBuffer(colName, SQL_WCHAR, (SQLPOINTER)buffer, SQL_C_WCHAR, sizeof(buffer), ColumnFlag::CF_NULLABLE, 10, 0));

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				charCol->SetSqlType(SQL_CHAR);
				charCol->SetColumnSize(10);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(charCol, 2);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			charCol->SetCb(SQL_NTS);
			wcscpy(buffer, L"HelloWorld");
			i();

			idBuffer = 102;
			wcscpy(buffer, L"ä ö ?");
			i();

			idBuffer = 103;
			wcscpy(buffer, L"abcdefgh  ");
			i();

			m_pDb->CommitTrans();
		}

		{
			WCharColumnBuffer charCol(128 + 1, colName, SQL_UNKNOWN_TYPE);
			charCol.BindColumn(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES_TMP, colName);

			f(101);
			EXPECT_EQ(L"HelloWorld", charCol.GetWString());

			f(102);
			EXPECT_EQ(L"ä ö ?", boost::trim_right_copy(charCol.GetWString()));

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

		string colName = ToDbCase(u8"tvarchar");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQLCHAR buffer[128 + 1];
			SqlCPointerBufferPtr varcharCol(new SqlCPointerBuffer(colName, SQL_CHAR, (SQLPOINTER)buffer, SQL_C_CHAR, sizeof(buffer), ColumnFlag::CF_NULLABLE, 128, 0));

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				varcharCol->SetSqlType(SQL_VARCHAR);
				varcharCol->SetColumnSize(128);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(varcharCol, 2);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			varcharCol->SetCb(SQL_NTS);
			strcpy((char*)buffer, "Hello World");
			i();

			if (m_pDb->GetDbms() != DatabaseProduct::MY_SQL)
			{
				// MySql fails here, with SQLSTATE HY000
				// Incorrect string value: '\xE4 \xF6 \xFC' for column 'tchar_10' at row 1
				idBuffer = 102;
				strcpy((char*)buffer, "ä ö ?");
				i();
			}

			idBuffer = 103;
			strcpy((char*)buffer, "   ");
			i();

			m_pDb->CommitTrans();
		}

		{
			CharColumnBuffer varcharCol(128 + 1, colName, SQL_UNKNOWN_TYPE);
			varcharCol.BindColumn(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES_TMP, colName);

			f(101);
			EXPECT_EQ("Hello World", varcharCol.GetString());

			if (m_pDb->GetDbms() != DatabaseProduct::MY_SQL)
			{
				f(102);
				EXPECT_EQ("ä ö ?", varcharCol.GetString());
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

		string colName = ToDbCase(u8"tvarchar");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQLWCHAR buffer[128 + 1];
			SqlCPointerBufferPtr varcharCol(new SqlCPointerBuffer(colName, SQL_WCHAR, (SQLPOINTER)buffer, SQL_C_WCHAR, sizeof(buffer), ColumnFlag::CF_NULLABLE, 128, 0));

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				varcharCol->SetSqlType(SQL_VARCHAR);
				varcharCol->SetColumnSize(128);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(varcharCol, 2);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			varcharCol->SetCb(SQL_NTS);
			wcscpy(buffer, L"Hello World");
			i();

			idBuffer = 102;
			wcscpy(buffer, L"ä ö ?");
			i();

			idBuffer = 103;
			wcscpy(buffer, L"   ");
			i();

			m_pDb->CommitTrans();
		}

		{
			WCharColumnBuffer varcharCol(128 + 1, colName, SQL_UNKNOWN_TYPE);
			varcharCol.BindColumn(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::CHARTYPES_TMP, colName);

			f(101);
			EXPECT_EQ(L"Hello World", varcharCol.GetWString());

			f(102);
			EXPECT_EQ(L"ä ö ?", varcharCol.GetWString());

			f(103);
			EXPECT_EQ(L"   ", varcharCol.GetWString());

			f(100);
			EXPECT_TRUE(varcharCol.IsNull());
		}
	}


	TEST_F(SqlCPointerTest, WriteDateValue)
	{
		TableId tableId = TableId::DATETYPES_TMP;

		string colName = ToDbCase(u8"tdate");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQL_DATE_STRUCT buffer;
			SqlCPointerBufferPtr date(new SqlCPointerBuffer(colName, SQL_DATE, (SQLPOINTER)&buffer, SQL_C_TYPE_DATE, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0));

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				date->SetSqlType(SQL_TYPE_DATE);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(date, 2);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			buffer.day = 26;
			buffer.month = 01;
			buffer.year = 1983;

			idBuffer = 101;
			date->SetCb(sizeof(buffer));
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			TypeDateColumnBuffer date(colName, SQL_UNKNOWN_TYPE);
			date.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"ttime");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQL_TIME_STRUCT buffer;
			SqlCPointerBufferPtr time(new SqlCPointerBuffer(colName, SQL_TIME, (SQLPOINTER)&buffer, SQL_C_TYPE_TIME, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0));

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				time->SetSqlType(SQL_TYPE_TIME);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(time, 2);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			buffer.hour = 15;
			buffer.minute = 22;
			buffer.second = 45;
			time->SetCb(sizeof(buffer));
			idBuffer = 101;
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			TypeTimeColumnBuffer time(colName, SQL_UNKNOWN_TYPE);
			time.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"ttimestamp");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// test without any fractions
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQL_TIMESTAMP_STRUCT buffer;
			SqlCPointerBufferPtr tsCol(new SqlCPointerBuffer(colName, SQL_TIMESTAMP, (SQLPOINTER)&buffer, SQL_C_TYPE_TIMESTAMP, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0));

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				tsCol->SetSqlType(SQL_TYPE_TIMESTAMP);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(tsCol, 2);

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
			tsCol->SetCb(sizeof(buffer));
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
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQL_TIMESTAMP_STRUCT buffer;
			SqlCPointerBufferPtr tsCol(new SqlCPointerBuffer(colName, SQL_TIMESTAMP, (SQLPOINTER)&buffer, SQL_C_TYPE_TIMESTAMP, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0));

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				idCol->SetSqlType(SQL_INTEGER);
				tsCol->SetSqlType(SQL_TYPE_TIMESTAMP);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(tsCol, 2);

			buffer.day = 9;
			buffer.month = 02;
			buffer.year = 1982;
			buffer.hour = 23;
			buffer.minute = 59;
			buffer.second = 59;
			buffer.fraction = fraction;
			tsCol->SetCb(sizeof(buffer));
			idBuffer = 102;
			i();

			m_pDb->CommitTrans();
		}

		{
			// Read back just inserted values
			{
				TypeTimestampColumnBuffer tsCol(colName, SQL_UNKNOWN_TYPE);
				tsCol.BindColumn(1, m_pStmt);
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

				TypeTimestampColumnBuffer tsCol(colName, SQL_UNKNOWN_TYPE);
				tsCol.SetDecimalDigits(decimalDigits);
				tsCol.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tdouble");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQLDOUBLE buffer;
			SqlCPointerBufferPtr doubleCol(new SqlCPointerBuffer(colName, SQL_DOUBLE, (SQLPOINTER)&buffer, SQL_C_DOUBLE, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0));

			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol->SetSqlType(SQL_INTEGER);
				doubleCol->SetSqlType(SQL_DOUBLE);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(doubleCol, 2);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			doubleCol->SetCb(sizeof(SQLDOUBLE));
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
			DoubleColumnBuffer doubleCol(colName, SQL_UNKNOWN_TYPE);
			doubleCol.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tfloat");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQLREAL buffer;
			SqlCPointerBufferPtr realCol(new SqlCPointerBuffer(colName, SQL_REAL, (SQLPOINTER)&buffer, SQL_C_FLOAT, sizeof(buffer), ColumnFlag::CF_NULLABLE, 0, 0));

			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol->SetSqlType(SQL_INTEGER);
				realCol->SetSqlType(SQL_REAL);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(realCol, 2);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			idBuffer = 101;
			realCol->SetCb(sizeof(buffer));
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
			RealColumnBuffer realCol(colName, SQL_UNKNOWN_TYPE);
			realCol.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tdecimal_18_0");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQL_NUMERIC_STRUCT buffer;
			SqlCPointerBufferPtr num18_0_Col(new SqlCPointerBuffer(colName, SQL_NUMERIC, (SQLPOINTER)&buffer, SQL_C_NUMERIC, sizeof(buffer), ColumnFlag::CF_NULLABLE, 18, 0));

			if (m_pDb->GetDbms() == DatabaseProduct::ACCESS)
			{
				idCol->SetSqlType(SQL_INTEGER);
				num18_0_Col->SetSqlType(SQL_NUMERIC);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(num18_0_Col, 2);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			SQLBIGINT v = 0;
			::memset(buffer.val, 0, sizeof(buffer.val));
			buffer.precision = 18;
			buffer.scale = 0;
			buffer.sign = 1;
			::memcpy(buffer.val, &v, sizeof(v));
			num18_0_Col->SetCb(sizeof(buffer));
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
			NumericColumnBuffer num18_0_Col(colName, SQL_UNKNOWN_TYPE);
			num18_0_Col.SetColumnSize(18);
			num18_0_Col.SetDecimalDigits(0);
			num18_0_Col.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tdecimal_18_10");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			// Prepare the id-col (required) and the col to insert
			SQLINTEGER idBuffer;
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQL_NUMERIC_STRUCT buffer;
			SqlCPointerBufferPtr num18_10_Col(new SqlCPointerBuffer(colName, SQL_NUMERIC, (SQLPOINTER)&buffer, SQL_C_NUMERIC, sizeof(buffer), ColumnFlag::CF_NULLABLE, 18, 10));

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS
				|| m_pDb->GetDbms() == DatabaseProduct::MY_SQL);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				// MySql implements SqlDescribeParam, but returns columnSize of 255 and decimalDigits 0, see #206
				// and a type that indicates it wants varchars for numeric-things.
				// but setting everything manually works fine
				idCol->SetSqlType(SQL_INTEGER);
				num18_10_Col->SetSqlType(SQL_NUMERIC);
				num18_10_Col->SetColumnSize(18);
				num18_10_Col->SetDecimalDigits(10);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(num18_10_Col, 2);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			SQLBIGINT v = 0;
			::memset(buffer.val, 0, sizeof(buffer.val));
			buffer.precision = 18;
			buffer.scale = 10;
			buffer.sign = 1;
			::memcpy(buffer.val, &v, sizeof(v));
			num18_10_Col->SetCb(sizeof(buffer));
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
			NumericColumnBuffer num18_10_Col(colName, SQL_UNKNOWN_TYPE);
			num18_10_Col.SetColumnSize(18);
			num18_10_Col.SetDecimalDigits(10);
			num18_10_Col.BindColumn(1, m_pStmt);
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

		string colName = ToDbCase(u8"tdecimal_5_3");
		string idColName = GetIdColumnName(tableId);
		ClearTmpTable(tableId);

		{
			// must close the statement before using it for reading. Else at least MySql fails.
			StatementCloser closer(m_pStmt);

			SQLINTEGER idBuffer;
			SqlCPointerBufferPtr idCol(new SqlCPointerBuffer(colName, SQL_INTEGER, &idBuffer, SQL_C_SLONG, sizeof(idBuffer), ColumnFlag::CF_NONE, 0, 0));
			SQL_NUMERIC_STRUCT buffer;
			SqlCPointerBufferPtr num5_3_Col(new SqlCPointerBuffer(colName, SQL_NUMERIC, (SQLPOINTER)&buffer, SQL_C_NUMERIC, sizeof(buffer), ColumnFlag::CF_NULLABLE, 5, 3));

			bool queryParameterInfo = !(m_pDb->GetDbms() == DatabaseProduct::ACCESS
				|| m_pDb->GetDbms() == DatabaseProduct::MY_SQL);
			if (!queryParameterInfo)
			{
				// Access does not implement SqlDescribeParam
				// MySql implements SqlDescribeParam, but returns columnSize of 255 and decimalDigits 0, see #206
				// and a type that indicates it wants varchars for numeric-things.
				// but setting everything manually works fine
				idCol->SetSqlType(SQL_INTEGER);
				num5_3_Col->SetSqlType(SQL_NUMERIC);
				num5_3_Col->SetColumnSize(5);
				num5_3_Col->SetDecimalDigits(3);
			}
			FInserter2 i(m_pDb, tableId, colName);
			i.m_stmt.BindParameter(idCol, 1);
			i.m_stmt.BindParameter(num5_3_Col, 2);

			// insert the default null value
			idBuffer = 100;
			i();

			// and some non-null values
			SQLBIGINT v = 0;
			::memset(buffer.val, 0, sizeof(buffer.val));
			buffer.precision = 5;
			buffer.scale = 3;
			buffer.sign = 1;
			::memcpy(buffer.val, &v, sizeof(v));
			num5_3_Col->SetCb(sizeof(buffer));
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
			NumericColumnBuffer num5_3_Col(colName, SQL_UNKNOWN_TYPE);
			num5_3_Col.SetColumnSize(5);
			num5_3_Col.SetDecimalDigits(3);
			num5_3_Col.BindColumn(1, m_pStmt);
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
