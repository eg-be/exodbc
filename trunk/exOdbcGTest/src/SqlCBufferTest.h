/*!
* \file SqlCBufferTest.h
* \author Elias Gerber <eg@elisium.ch>
* \date 18.10.2015
* \copyright GNU Lesser General Public License Version 3
*
* [Brief Header-file description]
*/

#pragma once

// Same component headers
#include "exOdbcGTest.h"

// Other headers
#include "gtest/gtest.h"
#include "Database.h"

// System headers

// Forward declarations
// --------------------

// Structs
// -------

// Classes
// -------
namespace exodbc
{
	class SqlCBufferLengthIndicatorTest : public ::testing::Test
	{
	};


	class SqlCBufferTest : public ::testing::Test
	{
	};


	class SqlCArrayBufferTest : public ::testing::Test
	{
	};


	class ColumnTestBase : public ::testing::Test
	{
		protected:
			virtual void SetUp();

			DatabasePtr m_pDb;
			SqlStmtHandlePtr m_pStmt;
	};


	class ShortColumnTest : public ColumnTestBase
	{};

	class LongColumnTest : public ColumnTestBase
	{};

	class BigIntColumnTest : public ColumnTestBase
	{};

	class DoubleColumnTest : public ColumnTestBase
	{};

	class RealColumnTest : public ColumnTestBase
	{};

	class NumericColumnTest : public ColumnTestBase
	{};

	class TypeTimeColumnTest : public ColumnTestBase
	{};

	class TypeDateColumnTest : public ColumnTestBase
	{};

	class TypeTimestampColumnTest : public ColumnTestBase
	{};

	class WCharColumnTest : public ColumnTestBase
	{};

	class CharColumnTest : public ColumnTestBase
	{};

	class BinaryColumnTest : public ColumnTestBase
	{};

	class SqlCPointerTest : public ColumnTestBase
	{};

} // namespace exodbc
