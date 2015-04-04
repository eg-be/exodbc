/*!
* \file TestDbCreator.cpp
* \author Elias Gerber <egerber@gmx.net>
* \date 04.04.2015
* \copyright wxWindows Library Licence, Version 3.1
*
* [Brief CPP-file description]
*/

#include "stdafx.h"

// Own header
#include "TestDbCreator.h"

// Same component headers
// Other headers

// Debug
#include "DebugNew.h"

// Static consts
// -------------

using namespace  std;
namespace exodbc
{
	// Construction
	// -------------
	TestDbCreator::TestDbCreator(const SOdbcInfo& odbcInfo)
		: m_odbcInfo(odbcInfo)
	{
		// prepare env
		m_env.AllocateEnvironmentHandle();
		m_env.SetOdbcVersion(OdbcVersion::V_3);
		// Create and open Database
		m_db.AllocateConnectionHandle(m_env);
		m_db.Open(odbcInfo.m_dsn, odbcInfo.m_username, odbcInfo.m_password);
	}

	// Destructor
	// -----------
	TestDbCreator::~TestDbCreator()
	{

	}

	// Implementation
	// --------------
	void TestDbCreator::CreateBlobtypes(bool dropIfExists)
	{
		exASSERT(m_db.IsOpen());

		set<wstring> createTableNames;
		if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
		{
			createTableNames = set<wstring>({ L"blobtypes_tmp", L"blobtypes" });
		}
		else
		{
			createTableNames = set<wstring>({ L"exodbc.blobtypes_tmp", L"exodbc.blobtypes" });
		}
		for (set<wstring>::const_iterator it = createTableNames.begin(); it != createTableNames.end(); ++it)
		{
			if (dropIfExists)
			{
				DropIfExists(*it);
			}
			
			wstring create;
			if (m_db.GetDbms() == DatabaseProduct::DB2)
			{
				create = boost::str(boost::wformat(L"CREATE TABLE %s ( IDBLOBTYPES INTEGER NOT NULL, TBLOB CHAR(16) FOR BIT DATA, TVARBLOB_20 VARCHAR(20) FOR BIT DATA,PRIMARY KEY ( IDBLOBTYPES ) );") % *it);
			}
			else
			{
				create = boost::str(boost::wformat(L"CREATE TABLE %s ( "
					L"idblobtypes int NOT NULL,"
					L"tblob binary (16) NULL,"
					L"tvarblob_20 varbinary (20) NULL,"
					L"PRIMARY KEY(idblobtypes))") % *it);
			}
				
			m_db.ExecSql(create);
		}

		// Insert our test-data
		wstring insertTableName = L"exodbc.blobtypes";
		if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
		{
			insertTableName = L"blobtypes";
		}

		wstring insert;
		if (m_db.GetDbms() == DatabaseProduct::DB2)
		{
			insert = boost::str(boost::wformat(L"INSERT INTO %s (IDBLOBTYPES, TBLOB, TVARBLOB_20) VALUES(1, x'00000000000000000000000000000000', NULL)") % insertTableName);
			m_db.ExecSql(insert);
			insert = boost::str(boost::wformat(L"INSERT INTO %s (IDBLOBTYPES, TBLOB, TVARBLOB_20) VALUES(2, x'ffffffffffffffffffffffffffffffff', NULL)") % insertTableName);
			m_db.ExecSql(insert);
			insert = boost::str(boost::wformat(L"INSERT INTO %s (IDBLOBTYPES, TBLOB, TVARBLOB_20) VALUES(3, x'abcdeff01234567890abcdef01234567', NULL)") % insertTableName);
			m_db.ExecSql(insert);
			insert = boost::str(boost::wformat(L"INSERT INTO %s (IDBLOBTYPES, TBLOB, TVARBLOB_20) VALUES(4, NULL, x'abcdeff01234567890abcdef01234567')") % insertTableName);
			m_db.ExecSql(insert);
			insert = boost::str(boost::wformat(L"INSERT INTO %s (IDBLOBTYPES, TBLOB, TVARBLOB_20) VALUES(5, NULL, x'abcdeff01234567890abcdef01234567ffffffff')") % insertTableName);
			m_db.ExecSql(insert);
		}
		else
		{
			insert = boost::str(boost::wformat(L"INSERT INTO %s (IDBLOBTYPES, TBLOB, TVARBLOB_20) VALUES(1, 0x00000000000000000000000000000000, NULL)") % insertTableName);
			m_db.ExecSql(insert);
			insert = boost::str(boost::wformat(L"INSERT INTO %s (IDBLOBTYPES, TBLOB, TVARBLOB_20) VALUES(2, 0xffffffffffffffffffffffffffffffff, NULL)") % insertTableName);
			m_db.ExecSql(insert);
			insert = boost::str(boost::wformat(L"INSERT INTO %s (IDBLOBTYPES, TBLOB, TVARBLOB_20) VALUES(3, 0xabcdeff01234567890abcdef01234567, NULL)") % insertTableName);
			m_db.ExecSql(insert);
			insert = boost::str(boost::wformat(L"INSERT INTO %s (IDBLOBTYPES, TBLOB, TVARBLOB_20) VALUES(4, NULL, 0xabcdeff01234567890abcdef01234567)") % insertTableName);
			m_db.ExecSql(insert);
			insert = boost::str(boost::wformat(L"INSERT INTO %s (IDBLOBTYPES, TBLOB, TVARBLOB_20) VALUES(5, NULL, 0xabcdeff01234567890abcdef01234567ffffffff)") % insertTableName);
			m_db.ExecSql(insert);
		}
		m_db.CommitTrans();
	}


	void TestDbCreator::CreateIntegertypes(bool dropIfExists)
	{
		exASSERT(m_db.IsOpen());

		set<wstring> createTableNames;
		if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
		{
			createTableNames = set<wstring>({ L"integertypes_tmp", L"integertypes" });
		}
		else
		{
			createTableNames = set<wstring>({ L"exodbc.integertypes_tmp", L"exodbc.integertypes" });
		}
		for (set<wstring>::const_iterator it = createTableNames.begin(); it != createTableNames.end(); ++it)
		{
			if (dropIfExists)
			{
				DropIfExists(*it);
			}

			wstring create = boost::str(boost::wformat(L"CREATE TABLE %s("
				L"idintegertypes int NOT NULL,"
				L"tsmallint smallint NULL,"
				L"tint int NULL,"
				L"tbigint bigint NULL,"
				L"PRIMARY KEY(idintegertypes))") % *it);

			m_db.ExecSql(create);
		}

		// Insert our test-data
		wstring insertTableName = L"exodbc.integertypes";
		if (m_db.GetDbms() == DatabaseProduct::MY_SQL)
		{
			insertTableName = L"integertypes";
		}
		wstring insert;
		insert = boost::str(boost::wformat(L"INSERT INTO %s(idintegertypes, tsmallint, tint, tbigint) VALUES(1, -32768, NULL, NULL)") % insertTableName);
		m_db.ExecSql(insert);
		insert = boost::str(boost::wformat(L"INSERT INTO %s(idintegertypes, tsmallint, tint, tbigint) VALUES(2, 32767, NULL, NULL);") % insertTableName);
		m_db.ExecSql(insert);
		insert = boost::str(boost::wformat(L"INSERT INTO %s(idintegertypes, tsmallint, tint, tbigint) VALUES(3, NULL, -2147483648, NULL);") % insertTableName);
		m_db.ExecSql(insert);
		insert = boost::str(boost::wformat(L"INSERT INTO %s(idintegertypes, tsmallint, tint, tbigint) VALUES(4, NULL, 2147483647, NULL);") % insertTableName);
		m_db.ExecSql(insert);
		insert = boost::str(boost::wformat(L"INSERT INTO %s(idintegertypes, tsmallint, tint, tbigint) VALUES(5, NULL, NULL, -9223372036854775808);") % insertTableName);
		m_db.ExecSql(insert);
		insert = boost::str(boost::wformat(L"INSERT INTO %s(idintegertypes, tsmallint, tint, tbigint) VALUES(6, NULL, NULL, 9223372036854775807);") % insertTableName);
		m_db.ExecSql(insert);
		insert = boost::str(boost::wformat(L"INSERT INTO %s(idintegertypes, tsmallint, tint, tbigint) VALUES(7, -13, 26, 10502)") % insertTableName);
		m_db.ExecSql(insert);

		m_db.CommitTrans();
	}


	void TestDbCreator::DropIfExists(const std::wstring& tableName)
	{
		try
		{
			wstring drop = boost::str(boost::wformat(L"DROP TABLE %s") % tableName);
			m_db.ExecSql(drop);
		}
		catch (const Exception& ex)
		{
			LOG_WARNING(boost::str(boost::wformat(L"Failed to drop Table %s: %s") % tableName % ex.ToString()));
		}
	}

	// Interfaces
	// ----------

} //namespace exodbc
