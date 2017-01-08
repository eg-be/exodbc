/*!
* \file DatabaseInfo.cpp
* \author Elias Gerber <eg@elisium.ch>
* \date 03.04.2016
* \copyright GNU Lesser General Public License Version 3
*
* ShortIntro Sample.
*/


#pragma warning(disable: 4503)	// 'identifier' : decorated name length exceeded, name was truncated
#ifdef _WIN32
    #include <SDKDDKVer.h>
    #include <tchar.h>
#endif
#include <iostream>

#include "exodbc/exOdbc.h"
#include "exodbc/Environment.h"
#include "exodbc/Database.h"
#include "exodbc/InfoObject.h"
#include "exodbc/Table.h"
#include "exodbc/ColumnBufferVisitors.h"

#include "boost/algorithm/string.hpp"
#include "boost/format.hpp"

namespace ba = boost::algorithm;

using namespace exodbc;
using namespace std;

void PrintDbHeader(ConstDatabasePtr pDb)
{
	DatabaseInfo dbInfo = pDb->GetDbInfo();

	// print header for trac
	WRITE_STDOUT_ENDL(boost::str(boost::format(u8"== %s (%s) ==") % dbInfo.GetStringProperty(DatabaseInfo::StringProperty::DbmsName) % dbInfo.GetStringProperty(DatabaseInfo::StringProperty::DbmsVersion)));
}


void PrintDriverInfo(ConstDatabasePtr pDb)
{
	DatabaseInfo dbInfo = pDb->GetDbInfo();

	// print header for trac
	WRITE_STDOUT_ENDL(boost::str(boost::format(u8"=== Driver Info ===")));
	WRITE_STDOUT_ENDL(boost::str(boost::format(u8"* Driver Name: %s") % dbInfo.GetStringProperty(DatabaseInfo::StringProperty::DriverName)));
	WRITE_STDOUT_ENDL(boost::str(boost::format(u8"* Driver Version: %s") % dbInfo.GetStringProperty(DatabaseInfo::StringProperty::DriverVersion)));
	WRITE_STDOUT_ENDL(boost::str(boost::format(u8"* Driver ODBC Version: %s") % dbInfo.GetStringProperty(DatabaseInfo::StringProperty::DriverOdbcVersion)));
}


void PrintDbInfo(ConstDatabasePtr pDb)
{
	DatabaseInfo dbInfo = pDb->GetDbInfo();

	// And a table with all information
	WRITE_STDOUT_ENDL(u8"=== Database Info ===");
	WRITE_STDOUT_ENDL(u8"||=Property Name =||= Property Value =||");
	DatabaseInfo::StringMap stringMap = dbInfo.GetStringMap();
	for (auto it = stringMap.begin(); it != stringMap.end(); ++it)
	{
		WRITE_STDOUT_ENDL(boost::str(boost::format(u8"||%-38s ||  %s  ||") % dbInfo.GetPropertyName(it->first) % it->second));
	}
	DatabaseInfo::USmallIntMap usmallIntMap = dbInfo.GetUSmallIntMap();
	for (auto it = usmallIntMap.begin(); it != usmallIntMap.end(); ++it)
	{
		WRITE_STDOUT_ENDL(boost::str(boost::format(u8"||%-38s || %#8x (%8d)||") % dbInfo.GetPropertyName(it->first) % it->second % it->second));
	}
	DatabaseInfo::UIntMap uintMap = dbInfo.GetUIntMap();
	for (auto it = uintMap.begin(); it != uintMap.end(); ++it)
	{
		WRITE_STDOUT_ENDL(boost::str(boost::format(u8"||%-38s || %#8x (%8d)||") % dbInfo.GetPropertyName(it->first) % it->second % it->second));
	}
	DatabaseInfo::IntMap intMap = dbInfo.GetIntMap();
	for (auto it = intMap.begin(); it != intMap.end(); ++it)
	{
		WRITE_STDOUT_ENDL(boost::str(boost::format(u8"||%-38s || %#8x (%8d)||") % dbInfo.GetPropertyName(it->first) % it->second %it->second));
	}
}


void printDatatypesInfo(ConstDatabasePtr pDb)
{
	WRITE_STDOUT_ENDL(u8"=== Datatypes Info ===");
	SqlTypeInfosVector types = pDb->GetTypeInfos();
	bool first = true;
	for (auto it = types.begin(); it != types.end(); ++it)
	{
		SSqlTypeInfo typeInfo = *it;
		WRITE_STDOUT_ENDL(typeInfo.ToOneLineStrForTrac(first));
		first = false;
	}
}

void printExOdbcTables(ConstDatabasePtr pDb)
{
	TableInfosVector allTables = pDb->FindTables(u8"", u8"", u8"", u8"");
	struct Finder
	{
		Finder()
			: m_dbms(DatabaseProduct::UNKNOWN)
		{};

		Finder(DatabaseProduct dbms)
			: m_dbms(dbms)
		{};

		DatabaseProduct m_dbms;
		
		const set<string> exOdbcTables = {
			u8"blobtypes", u8"blobtypes_tmp",
			u8"chartable", u8"chartypes",	u8"chartypes_tmp",
			u8"datetypes", u8"datetypes_tmp",
			u8"floattypes", u8"floattypes_tmp",
			u8"integertypes", u8"integertypes_tmp",
			u8"multikey",
			u8"numerictypes", u8"numerictypes_tmp",
			u8"selectonly",
			u8"not_existing",
			u8"not_supported",
			u8"not_supported_tmp"
		};

		const set<string> excelTables = { u8"testtable$" };

		bool operator()(const TableInfo& ti)
		{
			string tiName = ba::to_lower_copy(ti.GetPureName());

			if (m_dbms == DatabaseProduct::EXCEL)
			{
				return excelTables.find(tiName) != excelTables.end();
			}
			return exOdbcTables.find(tiName) != exOdbcTables.end();
		}
	};

	TableInfosVector exodbcTables;
	std::copy_if(allTables.begin(), allTables.end(), back_inserter(exodbcTables), Finder(pDb->GetDbms()));

	WRITE_STDOUT_ENDL(u8"=== Tables ===");
	for (auto it = exodbcTables.begin(); it != exodbcTables.end(); ++it)
	{
		TableInfo ti = *it;
		WRITE_STDOUT_ENDL(boost::str(boost::format(u8"==== %s ====") % ti.GetPureName()));
		WRITE_STDOUT_ENDL(u8"===== Structure =====");
		WRITE_STDOUT_ENDL(boost::str(boost::format(u8"* Catalog Name: %s") % (ti.HasCatalog() ? ti.GetCatalog() : u8"<no catalog>")));
		WRITE_STDOUT_ENDL(boost::str(boost::format(u8"* Schema Name: %s") % (ti.HasSchema() ? ti.GetSchema() : u8"<no schema>")));
		WRITE_STDOUT_ENDL(boost::str(boost::format(u8"* Table Name: %s") % ti.GetPureName()));
		WRITE_STDOUT_ENDL(boost::str(boost::format(u8"* Query Name: %s") % ti.GetQueryName()));

		// And print the columns by querying them from the table
		bool queryPrimaryKeys = !(pDb->GetDbms() == DatabaseProduct::ACCESS || pDb->GetDbms() == DatabaseProduct::EXCEL);
		QueryNameVisitor nameVisitor;
		SqlTypeVisitor sqlTypeVisitor;
		ColumnPropertiesPtrVisitor propsVisitor;
		ColumnFlagsPtrVisitor flagsVisitor;
		Table t(pDb, TableAccessFlag::AF_READ, ti);
		vector<ColumnBufferPtrVariant> columns;
		// disable logger while doing this
		vector<LogHandlerPtr> logHandlers = LogManager::Get().GetLogHandlers();
		LogManager::Get().ClearLogHandlers();
		try
		{
			columns = t.CreateAutoColumnBufferPtrs(true, true, queryPrimaryKeys);
		}
		catch (const Exception& ex)
		{
			WRITE_STDOUT_ENDL(u8"'''Warning: ''' Not all columns have been created successfully, will try to skip columns:");
			WRITE_STDOUT_ENDL(u8"{{{");
			WRITE_STDOUT_ENDL(ex.ToString());
			WRITE_STDOUT_ENDL(u8"}}}");
			try
			{
				columns = t.CreateAutoColumnBufferPtrs(true, true, queryPrimaryKeys);
			}
			catch (const Exception& ex)
			{
				WRITE_STDOUT_ENDL(u8"'''Error: ''' Failed to create columns:");
				WRITE_STDOUT_ENDL(u8"{{{");
				WRITE_STDOUT_ENDL(ex.ToString());
				WRITE_STDOUT_ENDL(u8"}}}");
				continue;
			}
		}
		t.Open();
		for (auto itLog = logHandlers.begin(); itLog != logHandlers.end(); ++itLog)
		{
			LogManager::Get().RegisterLogHandler(*itLog);
		}
		WRITE_STDOUT_ENDL(u8"||=Column Query Name =||=Sql Type =||= Column Size=||= Decimal Digits=||= Nullable=||= Primary Key=||");
		for (auto itCol = columns.begin(); itCol != columns.end(); ++itCol)
		{
			ColumnBufferPtrVariant pBuff = *itCol;
			SQLSMALLINT sqlType = boost::apply_visitor(sqlTypeVisitor, pBuff);
			string name = boost::apply_visitor(nameVisitor, pBuff);
			ColumnPropertiesPtr pProps = boost::apply_visitor(propsVisitor, pBuff);
			ColumnFlagsPtr pFlags = boost::apply_visitor(flagsVisitor, pBuff);
			string nullable = pFlags->Test(ColumnFlag::CF_NULLABLE) ? u8"NULLABLE" : u8"";
			string primary = pFlags->Test(ColumnFlag::CF_PRIMARY_KEY) ? u8"PRIMARY": u8"";
			WRITE_STDOUT_ENDL(boost::str(boost::format(u8"|| %s || %s || %d || %d || %s || %s ||") % name % SqlType2s(sqlType) % pProps->GetColumnSize() % pProps->GetDecimalDigits() % nullable % primary));
		}

		// Print the table content, by opening it as char-table.
		t.Close();
		t.ClearColumns();
		// but skip _tmp tables
		if (ba::ends_with(ba::to_lower_copy(ti.GetPureName()), u8"_tmp"))
		{
			continue;
		}

		t.SetSql2BufferTypeMap(CharSql2BufferMap::Create());
		try
		{
			columns = t.CreateAutoColumnBufferPtrs(true, true, false);
		}
		catch (const Exception& ex)
		{
			WRITE_STDOUT_ENDL(u8"'''Warning: ''' Not all columns have been created successfully, will try to skip columns:");
			WRITE_STDOUT_ENDL(u8"{{{");
			WRITE_STDOUT_ENDL(ex.ToString());
			WRITE_STDOUT_ENDL(u8"}}}");
		}
		t.Open();

		// print header
		WRITE_STDOUT_ENDL(u8"===== Content =====");
		auto itCol = columns.begin();
		bool first = true;
		do
		{
			if (first && !columns.empty())
			{
				WRITE_STDOUT(u8"||= ");
				first = false;
			}
			ColumnBufferPtrVariant pBuff = *itCol;
			string name = boost::apply_visitor(nameVisitor, pBuff);
			WRITE_STDOUT(name);
			++itCol;
			if (itCol != columns.end())
			{
				WRITE_STDOUT(u8" =||= ");
			}
			else
			{
				WRITE_STDOUT(u8" =||");
			}

		} while (itCol != columns.end());
		WRITE_STDOUT_ENDL(u8"");

		// And content
		t.Select();
		while (t.SelectNext())
		{
			auto itCol = columns.begin();
			bool first = true;
			do
			{
				if (first && !columns.empty())
				{
					WRITE_STDOUT(u8"||= ");
					first = false;
				}
				CharColumnBufferPtr pBuff = boost::get<CharColumnBufferPtr>(*itCol);
				if (!pBuff->IsNull())
				{
                    string s = pBuff->GetString();
					WRITE_STDOUT(pBuff->GetString());
				}
				else
				{
					WRITE_STDOUT(u8"''NULL''");
				}
				++itCol;
				if (itCol != columns.end())
				{
					WRITE_STDOUT(u8" =||= ");
				}
				else
				{
					WRITE_STDOUT_ENDL(u8" =||");
				}

			} while (itCol != columns.end());
		}
	}
}


void printUsage()
{
	WRITE_STDOUT_ENDL(u8"Usage: DatabaseInfo [-U user] [-P pass] [-DSN dsn] [-CS connectionString] [-PrintTestTables]");
	WRITE_STDOUT_ENDL(u8"       -DSN or -CS must be given. -U and -P is only used in combination with -DSN");
	WRITE_STDOUT_ENDL(u8"       -PrintTestTable: If passed, exodbctest tables are printed, else db and datatype infos");
}


struct SConnectionInfo
{
	string m_dsn;
	string m_user;
	string m_pass;
	string m_cs;
};

#ifdef _WIN32
int _tmain(int argc, _TCHAR* argv[])
#else
int main(int argc, char* argv[])
#endif
{
	try
	{
		// Parse arguments
		if (argc < 3)
		{
			printUsage();
			return - 1;
		}
		const std::string userKey = u8"-U";
		const std::string passKey = u8"-P";
		const std::string dsnKey = u8"-DSN";
		const std::string csKey = u8"-CS";
		const std::string testTablesKey = u8"-PrintTestTables";
		bool printTestTables = false;
		std::string userValue;
		std::string passValue;
		std::string dsnValue;
		std::string csValue;
		vector<SConnectionInfo> connectionInfos;
		for (int i = 0; i < argc; i++)
		{
			std::string argNext;
#ifdef _WIN32
			std::string arg( utf16ToUtf8(argv[i]));
			if(i + 1 < argc)
				argNext = utf16ToUtf8(argv[i + 1]);
#else
            std::string arg(argv[i]);
			if (i + 1 < argc)
				argNext = argv[i + 1];
#endif
			if (ba::equals(arg, u8"--help"))
			{
				printUsage();
				return 0;
			}
			if (ba::equals(arg, userKey) && i + 1 < argc)
			{
				userValue = argNext;
			}
			if (ba::equals(arg, passKey) && i + 1 < argc)
			{
				passValue = argNext;
			}
			if (ba::equals(arg, dsnKey) && i + 1 < argc)
			{
				dsnValue = argNext;
			}
			if (ba::equals(arg, csKey) && i + 1 < argc)
			{
				csValue = argNext;
			}
			if (ba::equals(arg, testTablesKey))
			{
				printTestTables = true;
			}
			if (i + 1 >= argc && ba::istarts_with(arg, u8"-") && !ba::equals(arg, testTablesKey))
			{
				std::stringstream ss;
				ss << u8"Ignoring argument '" << arg << u8"' because no value follows after argument. See --help for usage.";
				LOG_WARNING(ss.str());
			}
		}

		if (!dsnValue.empty())
		{
			SConnectionInfo conInfo;
			conInfo.m_dsn = dsnValue;
			if (!userValue.empty())
				conInfo.m_user = userValue;
			if (!passValue.empty())
				conInfo.m_pass = passValue;
			connectionInfos.push_back(conInfo);
		}
		if (!csValue.empty())
		{
			SConnectionInfo conInfo;
			conInfo.m_cs = csValue;
			connectionInfos.push_back(conInfo);
		}

		if (connectionInfos.empty())
		{
			LOG_ERROR(u8"No connection string (-CS) and no dsn (-DSN) passed, exiting. See --help for usage.");
			return -1;
		}

		// Create an environment with ODBC Version 3.0
		LogManager::Get().SetGlobalLogLevel(LogLevel::Error);
		EnvironmentPtr pEnv = Environment::Create(OdbcVersion::V_3);

		// And connect to a database using the environment,
		// read info for every connection
		for (auto it = connectionInfos.begin(); it != connectionInfos.end(); ++it)
		{
			SConnectionInfo conInf = *it;
			DatabasePtr pDb = Database::Create(pEnv);
			if (!conInf.m_cs.empty())
			{
				pDb->Open(conInf.m_cs);
			}
			else
			{
				pDb->Open(conInf.m_dsn, conInf.m_user, conInf.m_pass);
			}

			// Print db-header and driver-info (always):
			PrintDbHeader(pDb);
			PrintDriverInfo(pDb);

			// print db and types, or test-tables:
			if (printTestTables)
			{
				printExOdbcTables(pDb);
			}
			else
			{
				PrintDbInfo(pDb);
				printDatatypesInfo(pDb);
			}

			// add some blank lines
			WRITE_STDOUT_ENDL("");
			WRITE_STDOUT_ENDL("");
		}

	}
	catch (const Exception& ex)
	{
		WRITE_STDERR_ENDL(ex.ToString());
	}
	return 0;
}

