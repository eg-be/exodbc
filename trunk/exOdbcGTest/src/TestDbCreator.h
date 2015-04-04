/*!
* \file TestDbCreator.h
* \author Elias Gerber <egerber@gmx.net>
* \date 04.04.2015
* \copyright wxWindows Library Licence, Version 3.1
*
* [Brief Header-file description]
*/

#pragma once
#ifndef TESTDBCREATOR_H
#define TESTDBCREATOR_H

// Same component headers
#include "exOdbcGTest.h"
#include "TestTables.h"

// Other headers
#include "Environment.h"
#include "Database.h"
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

	class TestDbCreator
	{
	public:
		TestDbCreator(const SOdbcInfo& odbcInfo);
		~TestDbCreator();

		void CreateIntegertypes(bool dropIfExists);
		void CreateBlobtypes(bool dropIfExists);
		void CreateChartable(bool dropIfExists);
		void CreateChartypes(bool dropIfExists);

	private:

		void DropIfExists(const std::wstring& tableName);

		SOdbcInfo m_odbcInfo;
		Environment m_env;
		Database m_db;
	};

} // namespace exodbc

#endif // TESTDBCREATOR_H