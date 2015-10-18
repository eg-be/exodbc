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
#include "Visitors.h"

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


	TEST_F(SqlCBufferLengthIndicatorTest, SetCb)
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



	// Interfaces
	// ----------

} //namespace exodbc
