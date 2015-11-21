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
// Other headers
#include "SqlCBuffer.h"

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
		SqlWCharArray arr(24);
		EXPECT_TRUE(arr.IsNull());
		EXPECT_EQ(24, arr.GetNrOfElements());
		EXPECT_EQ(sizeof(SQLWCHAR) * 24, arr.GetBufferLength());
	}


	TEST_F(SqlCArrayBufferTest, SetAndGetValue)
	{
		SqlWCharArray arr(24);
		wstring s(L"Hello");
		arr.SetValue(s.c_str(), (s.length() + 1) * sizeof(SQLWCHAR), SQL_NTS);
		wstring v(arr.GetBuffer().get());
		EXPECT_EQ(s, v);
		EXPECT_EQ(SQL_NTS, arr.GetCb());
	}


	TEST_F(SqlCArrayBufferTest, CopyConstruction)
	{
		// must internally use the same buffer
		SqlWCharArray arr(24);
		wstring s(L"Hello");
		arr.SetValue(s.c_str(), (s.length() + 1) * sizeof(SQLWCHAR), SQL_NTS);
		wstring v(arr.GetBuffer().get());
		ASSERT_EQ(s, v);
		ASSERT_EQ(SQL_NTS, arr.GetCb());

		// create copy
		SqlWCharArray arr2(arr);
		wstring v2(arr2.GetBuffer().get());
		EXPECT_EQ(s, v2);

		// changing either must change the other too
		s = L"World";
		arr.SetValue(s.c_str(), (s.length() + 1) * sizeof(SQLWCHAR), SQL_NTS);
		v = arr.GetBuffer().get();
		v2 = arr2.GetBuffer().get();
		EXPECT_EQ(s, v);
		EXPECT_EQ(s, v2);

		s = L"Moon";
		arr2.SetValue(s.c_str(), (s.length() + 1) * sizeof(SQLWCHAR), SQL_NTS);
		v = arr.GetBuffer().get();
		v2 = arr2.GetBuffer().get();
		EXPECT_EQ(s, v);
		EXPECT_EQ(s, v2);
	}

} //namespace exodbc
