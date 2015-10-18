/*!
* \file IntegerColumnBufferTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 31.03.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "IntegerColumnBufferTest.h"

// Same component headers
// Other headers
#include "ColumnBufferFactory.h"
#include "IntegerColumnBuffer.h"

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
	TEST_F(IntegerColumnBufferTest, Construction)
	{
		IColumnBufferPtr pUShort = ColumnBufferFactory::Instance().CreateColumnBuffer(SQL_C_USHORT);
		IColumnBufferPtr pULong = ColumnBufferFactory::Instance().CreateColumnBuffer(SQL_C_ULONG);
		IColumnBufferPtr pUBigInt = ColumnBufferFactory::Instance().CreateColumnBuffer(SQL_C_UBIGINT);
		IColumnBufferPtr pShort = ColumnBufferFactory::Instance().CreateColumnBuffer(SQL_C_SSHORT);
		IColumnBufferPtr pLong = ColumnBufferFactory::Instance().CreateColumnBuffer(SQL_C_SLONG);
		IColumnBufferPtr pBigInt = ColumnBufferFactory::Instance().CreateColumnBuffer(SQL_C_SBIGINT);

		// should default to null
		EXPECT_TRUE(pUShort->IsNull());
		EXPECT_TRUE(pULong->IsNull());
		EXPECT_TRUE(pUBigInt->IsNull());
		EXPECT_TRUE(pShort->IsNull());
		EXPECT_TRUE(pLong->IsNull());
		EXPECT_TRUE(pBigInt->IsNull());
	}


	TEST_F(IntegerColumnBufferTest, GetSqlCType)
	{
		IColumnBufferPtr pUShort = ColumnBufferFactory::Instance().CreateColumnBuffer(SQL_C_USHORT);
		IColumnBufferPtr pULong = ColumnBufferFactory::Instance().CreateColumnBuffer(SQL_C_ULONG);
		IColumnBufferPtr pUBigInt = ColumnBufferFactory::Instance().CreateColumnBuffer(SQL_C_UBIGINT);
		IColumnBufferPtr pShort = ColumnBufferFactory::Instance().CreateColumnBuffer(SQL_C_SSHORT);
		IColumnBufferPtr pLong = ColumnBufferFactory::Instance().CreateColumnBuffer(SQL_C_SLONG);
		IColumnBufferPtr pBigInt = ColumnBufferFactory::Instance().CreateColumnBuffer(SQL_C_SBIGINT);

		EXPECT_EQ(SQL_C_USHORT, pUShort->GetSqlCType());
		EXPECT_EQ(SQL_C_ULONG, pULong->GetSqlCType());
		EXPECT_EQ(SQL_C_UBIGINT, pUBigInt->GetSqlCType());

		EXPECT_EQ(SQL_C_SSHORT, pShort->GetSqlCType());
		EXPECT_EQ(SQL_C_SLONG, pLong->GetSqlCType());
		EXPECT_EQ(SQL_C_SBIGINT, pBigInt->GetSqlCType());
	}


	TEST_F(IntegerColumnBufferTest, SetAndGetValue)
	{
		IColumnBufferPtr pUShort = ColumnBufferFactory::Instance().CreateColumnBuffer(SQL_C_USHORT);
		std::shared_ptr<IntegerColumnBuffer> pBuff = std::dynamic_pointer_cast<IntegerColumnBuffer>(pUShort);
		ASSERT_TRUE(pBuff != NULL);
		pBuff->SetValue(13);
		SQLSMALLINT si = 0;
		pBuff->GetValue(si);
		EXPECT_EQ(13, si);
		EXPECT_FALSE(pBuff->IsNull());
	}


} //namespace exodbc
