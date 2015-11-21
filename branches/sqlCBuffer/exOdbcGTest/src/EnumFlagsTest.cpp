/*!
* \file EnumFlagsTest.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 21.11.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "EnumFlagsTest.h"

// Same component headers
// Other headers
#include "EnumFlags.h"

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

	TEST_F(EnumFlagsTest, InitFlags)
	{
		ColumnFlags flags;
		
		// No flag must be set
		EXPECT_FALSE(flags.Test(ColumnFlag::CF_SELECT));
		EXPECT_FALSE(flags.Test(ColumnFlag::CF_UPDATE));
		EXPECT_FALSE(flags.Test(ColumnFlag::CF_INSERT));
		EXPECT_FALSE(flags.Test(ColumnFlag::CF_NULLABLE));
		EXPECT_FALSE(flags.Test(ColumnFlag::CF_PRIMARY_KEY));
	}


	TEST_F(EnumFlagsTest, ConstructFlags)
	{
		ColumnFlags flags(ColumnFlag::CF_NULLABLE | ColumnFlag::CF_INSERT);

		// No flag must be set except constructed
		EXPECT_FALSE(flags.Test(ColumnFlag::CF_SELECT));
		EXPECT_FALSE(flags.Test(ColumnFlag::CF_UPDATE));
		EXPECT_TRUE(flags.Test(ColumnFlag::CF_INSERT));
		EXPECT_TRUE(flags.Test(ColumnFlag::CF_NULLABLE));
		EXPECT_FALSE(flags.Test(ColumnFlag::CF_PRIMARY_KEY));

	}

	TEST_F(EnumFlagsTest, SetAndClearFlag)
	{
		ColumnFlags f;

		EXPECT_FALSE(f.Test(ColumnFlag::CF_NULLABLE));
		f.Set(ColumnFlag::CF_NULLABLE);
		EXPECT_TRUE(f.Test(ColumnFlag::CF_NULLABLE));
		f.Clear(ColumnFlag::CF_NULLABLE);
		EXPECT_FALSE(f.Test(ColumnFlag::CF_NULLABLE));
	}

	// Interfaces
	// ----------

} //namespace exodbc
