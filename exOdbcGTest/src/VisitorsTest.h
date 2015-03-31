/*!
* \file VisitorsTest.h
* \author Elias Gerber <egerber@gmx.net>
* \date 31.03.2015
* \copyright wxWindows Library Licence, Version 3.1
*
* [Brief Header-file description]
*/

#pragma once
#ifndef EXCELTEST_H
#define EXCELTEST_H

// Same component headers
#include "exOdbcGTest.h"

// Other headers
#include "gtest/gtest.h"

// System headers

// Forward declarations
// --------------------

// Structs
// -------

// Classes
// -------
namespace exodbc
{

	class BigIntVisitorTest : public ::testing::Test {

	protected:
		virtual void SetUp();

		virtual void TearDown();

		SQLBIGINT m_bINull = 0;
		SQLBIGINT m_bIMax = 9223372036854775807;
		SQLBIGINT m_bIMin = (-9223372036854775807 -1);
		
		SQLINTEGER m_iNull = 0;
		SQLINTEGER m_iMax = 2147483647;
		SQLINTEGER m_iMin = (-2147483647 -1);

		SQLSMALLINT m_sINull = 0;
		SQLSMALLINT m_sIMax = 32767;
		SQLSMALLINT m_sIMin = (-32767 -1);

		BufferPtrVariant m_pbINull;
		BufferPtrVariant m_pbIMax;
		BufferPtrVariant m_pbIMin;

		BufferPtrVariant m_piNull;
		BufferPtrVariant m_piMax;
		BufferPtrVariant m_piMin;

		BufferPtrVariant m_psINull;
		BufferPtrVariant m_psIMax;
		BufferPtrVariant m_psIMin;


	};
} // namespace exodbc

#endif // EXCELTEST_H