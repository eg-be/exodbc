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
		EXPECT_FALSE(flags.Test(ColumnFlag::SELECT));
		EXPECT_FALSE(flags.Test(ColumnFlag::UPDATE));
		EXPECT_FALSE(flags.Test(ColumnFlag::INSERT));
		EXPECT_FALSE(flags.Test(ColumnFlag::NULLABLE));
		EXPECT_FALSE(flags.Test(ColumnFlag::PRIMARY_KEY));
	}


	TEST_F(EnumFlagsTest, ConstructFlags)
	{
		ColumnFlags flags(ColumnFlag::NULLABLE | ColumnFlag::INSERT);

		// No flag must be set except constructed
		EXPECT_FALSE(flags.Test(ColumnFlag::SELECT));
		EXPECT_FALSE(flags.Test(ColumnFlag::UPDATE));
		EXPECT_TRUE(flags.Test(ColumnFlag::INSERT));
		EXPECT_TRUE(flags.Test(ColumnFlag::NULLABLE));
		EXPECT_FALSE(flags.Test(ColumnFlag::PRIMARY_KEY));

	}

	TEST_F(EnumFlagsTest, SetAndClearFlag)
	{
		ColumnFlags f;

		EXPECT_FALSE(f.Test(ColumnFlag::NULLABLE));
		f.Set(ColumnFlag::NULLABLE);
		EXPECT_TRUE(f.Test(ColumnFlag::NULLABLE));
		f.Clear(ColumnFlag::NULLABLE);
		EXPECT_FALSE(f.Test(ColumnFlag::NULLABLE));
	}

	// Interfaces
	// ----------

} //namespace exodbc
