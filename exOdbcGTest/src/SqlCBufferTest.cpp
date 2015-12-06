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


	TEST_F(NumericColumnTest, Read_18_10_Value)
	{
		wstring colName = L"tdecimal_18_10";
		SqlNumericStructBuffer num18_10_Col(colName);
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


	TEST_F(NumericColumnTest, Read_5_3_Value)
	{
		wstring colName = L"tdecimal_5_3";
		SqlNumericStructBuffer num5_3_Col(colName);
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


	TEST_F(TypeTimeColumnTest, ReadValue)
	{
		wstring colName = L"ttime";
		SqlTypeTimeStructBuffer time(colName);
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


	TEST_F(TypeDateColumnTest, ReadValue)
	{
		wstring colName = L"tdate";
		SqlTypeDateStructBuffer date(colName);
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


	TEST_F(TypeTimestampColumnTest, ReadValue)
	{
		wstring colName = L"ttimestamp";
		{
			// Test without any fraction

			SqlTypeTimestampStructBuffer tsCol(colName);
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
			if (m_pDb->GetDbms() == DatabaseProduct::MS_SQL_SERVER)
			{
				// SQL-Server has a max precision of 3 for the fractions
				SqlTypeTimestampStructBuffer tsCol(colName);
				tsCol.SetDecimalDigits(3);
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
				EXPECT_EQ(123000000, ts.fraction);
			}
			
			if (m_pDb->GetDbms() == DatabaseProduct::DB2)
			{
				// DB2 has a max precision of 6 for the fraction
				SqlTypeTimestampStructBuffer tsCol(colName);
				tsCol.SetDecimalDigits(6);
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
				EXPECT_EQ(123456000, ts.fraction);
			}
		}

	}


	TEST_F(WCharColumnTest, ReadCharValues)
	{
		// note that when working witch chars, we add one element for the terminating \0 char.
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
		// note that when working witch chars, we add one element for the terminating \0 char.
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
			SqlBinaryArray blobCol(colName, 16);
			blobCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::BLOBTYPES, colName);

			f(1);
			EXPECT_EQ(16, blobCol.GetCb());
			EXPECT_EQ(empty, *blobCol.GetBuffer());

			f(2);
			EXPECT_EQ(16, blobCol.GetCb());
			EXPECT_EQ(ff, *blobCol.GetBuffer());

			f(3);
			EXPECT_EQ(16, blobCol.GetCb());
			EXPECT_EQ(abc, *blobCol.GetBuffer());

			f(4);
			EXPECT_TRUE(blobCol.IsNull());
		}

		{
			wstring colName = L"tvarblob_20";
			SqlBinaryArray varBlobCol(colName, 20);
			varBlobCol.BindSelect(1, m_pStmt);
			FSelectFetcher f(m_pDb->GetDbms(), m_pStmt, TableId::BLOBTYPES, colName);

			f(4);
			// This is a varblob. The buffer is sized to 20, but in this column we read only 16 bytes.
			// Cb must reflect this
			EXPECT_EQ(16, varBlobCol.GetCb());
			const shared_ptr<vector<SQLCHAR>> pBuff = varBlobCol.GetBuffer();
			EXPECT_EQ(20, pBuff->size());
			// Only compare first 16 elements
			vector<SQLCHAR> first16Elements(pBuff->begin(), pBuff->begin() + 16);
			EXPECT_EQ(abc, first16Elements);

			f(5);
			EXPECT_EQ(20, varBlobCol.GetCb());
			EXPECT_EQ(abc_ff, *varBlobCol.GetBuffer());

			f(3);
			EXPECT_TRUE(varBlobCol.IsNull());
		}

	}

} //namespace exodbc
