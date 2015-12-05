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
		FSelectFetcher(DatabaseProduct dbms, SqlStmtHandlePtr pStmt, exodbctest::TableId tableId, const std::wstring& columnQueryName)
			: m_tableId(tableId)
			, m_columnQueryName(ToDbCase(columnQueryName))
			, m_pStmt(pStmt)
			, m_dbms(dbms)
		{
		}

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

		DatabaseProduct m_dbms;
		SqlStmtHandlePtr m_pStmt;
		exodbctest::TableId m_tableId;
		wstring m_columnQueryName;
	};


	TEST_F(ShortColumnTest, ReadValue)
	{
		wstring colName = ToDbCase(L"tsmallint");
		SqlSShortBuffer shortCol(colName);
		shortCol.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::INTEGERTYPES, colName);

		f(1);
		EXPECT_EQ(-32768, shortCol.GetValue());
		f(2);
		EXPECT_EQ(32767, shortCol.GetValue());
		f(3);
		EXPECT_TRUE(shortCol.IsNull());
	}


	TEST_F(LongColumnTest, ReadValue)
	{
		wstring colName = ToDbCase(L"tint");
		SqlSLongBuffer longCol(colName);
		longCol.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::INTEGERTYPES, colName);

		f(3);
		EXPECT_EQ((-2147483647 -1), longCol.GetValue());
		f(4);
		EXPECT_EQ(2147483647, longCol.GetValue());
		f(5);
		EXPECT_TRUE(longCol.IsNull());
	}


	TEST_F(BigIntColumnTest, ReadValue)
	{
		wstring colName = ToDbCase(L"tbigint");
		SqlSBigIntBuffer biCol(colName);
		biCol.BindSelect(1, m_pStmt);
		FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::INTEGERTYPES, colName);

		f(5);
		EXPECT_EQ((-9223372036854775807 - 1), biCol.GetValue());
		f(6);
		EXPECT_EQ(9223372036854775807, biCol.GetValue());
		f(4);
		EXPECT_TRUE(biCol.IsNull());
	}


	TEST_F(DoubleColumnTest, ReadValue)
	{
		wstring colName = L"tdouble";
		SqlDoubleBuffer doubleCol(colName);
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


	TEST_F(RealColumnTest, ReadValue)
	{
		wstring colName = L"tfloat";
		SqlRealBuffer realCol(colName);
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


	TEST_F(NumericColumnTest, Read_18_0_Value)
	{
		wstring colName = L"tdecimal_18_0";
		SqlNumericStructBuffer num18_0_Col(colName);
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


	TEST_F(WCharColumnTest, ReadCharValues)
	{
		{
			wstring colName = L"tvarchar";
			SqlWCharArray varcharCol(colName, 128 + 1);
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
			SqlWCharArray charCol(colName, 128 + 1);
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


	TEST_F(CharColumnTest, ReadCharValues)
	{
		{
			wstring colName = L"tvarchar";
			SqlCharArray varcharCol(colName, 128 + 1);
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
			SqlCharArray charCol(colName, 128 + 1);
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

} //namespace exodbc
