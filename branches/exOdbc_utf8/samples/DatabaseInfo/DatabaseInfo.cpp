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
	cout << boost::str(boost::format(u8"== %s (%s) ==") % dbInfo.GetStringProperty(DatabaseInfo::StringProperty::DbmsName) % dbInfo.GetStringProperty(DatabaseInfo::StringProperty::DbmsVersion)) << endl;
}


void PrintDriverInfo(ConstDatabasePtr pDb)
{
	DatabaseInfo dbInfo = pDb->GetDbInfo();

	// print header for trac
	cout << boost::str(boost::format(u8"=== Driver Info ===")) << endl;
	cout << boost::str(boost::format(u8"* Driver Name: %s") % dbInfo.GetStringProperty(DatabaseInfo::StringProperty::DriverName)) << endl;
	cout << boost::str(boost::format(u8"* Driver Version: %s") % dbInfo.GetStringProperty(DatabaseInfo::StringProperty::DriverVersion)) << endl;
	cout << boost::str(boost::format(u8"* Driver ODBC Version: %s") % dbInfo.GetStringProperty(DatabaseInfo::StringProperty::DriverOdbcVersion)) << endl;
}


void PrintDbInfo(ConstDatabasePtr pDb)
{
	DatabaseInfo dbInfo = pDb->GetDbInfo();

	// And a table with all information
	cout << u8"=== Database Info ===" << endl;
	cout << u8"||=Property Name =||= Property Value =||" << endl;
	DatabaseInfo::StringMap stringMap = dbInfo.GetStringMap();
	for (auto it = stringMap.begin(); it != stringMap.end(); ++it)
	{
		cout << boost::str(boost::format(u8"||%-38s ||  %s  ||") % dbInfo.GetPropertyName(it->first) % it->second) << endl;
	}
	DatabaseInfo::USmallIntMap usmallIntMap = dbInfo.GetUSmallIntMap();
	for (auto it = usmallIntMap.begin(); it != usmallIntMap.end(); ++it)
	{
		cout << boost::str(boost::format(u8"||%-38s || %#8x (%8d)||") % dbInfo.GetPropertyName(it->first) % it->second % it->second) << endl;
	}
	DatabaseInfo::UIntMap uintMap = dbInfo.GetUIntMap();
	for (auto it = uintMap.begin(); it != uintMap.end(); ++it)
	{
		cout << boost::str(boost::format(u8"||%-38s || %#8x (%8d)||") % dbInfo.GetPropertyName(it->first) % it->second % it->second) << endl;
	}
	DatabaseInfo::IntMap intMap = dbInfo.GetIntMap();
	for (auto it = intMap.begin(); it != intMap.end(); ++it)
	{
		cout << boost::str(boost::format(u8"||%-38s || %#8x (%8d)||") % dbInfo.GetPropertyName(it->first) % it->second %it->second) << endl;
	}
}


void printDatatypesInfo(ConstDatabasePtr pDb)
{
	cout << u8"=== Datatypes Info ===" << endl;
	SqlTypeInfosVector types = pDb->GetTypeInfos();
	bool first = true;
	for (auto it = types.begin(); it != types.end(); ++it)
	{
		SSqlTypeInfo typeInfo = *it;
		cout << typeInfo.ToOneLineStrForTrac(first) << endl;
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

	cout << u8"=== Tables ===" << endl;
	for (auto it = exodbcTables.begin(); it != exodbcTables.end(); ++it)
	{
		TableInfo ti = *it;
		cout << boost::str(boost::format(u8"==== %s ====") % ti.GetPureName()) << endl;
		cout << u8"===== Structure =====" << endl;
		cout << boost::str(boost::format(u8"* Catalog Name: %s") % (ti.HasCatalog() ? ti.GetCatalog() : u8"<no catalog>")) << endl;
		cout << boost::str(boost::format(u8"* Schema Name: %s") % (ti.HasSchema() ? ti.GetSchema() : u8"<no schema>")) << endl;
		cout << boost::str(boost::format(u8"* Table Name: %s") % ti.GetPureName()) << endl;
		cout << boost::str(boost::format(u8"* Query Name: %s") % ti.GetQueryName()) << endl;

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
			cout << u8"'''Warning: ''' Not all columns have been created successfully, will try to skip columns:" << endl;
			cout << u8"{{{" << endl;
			cout << ex.ToString() << endl;
			cout << u8"}}}" << endl;
			try
			{
				columns = t.CreateAutoColumnBufferPtrs(true, true, queryPrimaryKeys);
			}
			catch (const Exception& ex)
			{
				cout << u8"'''Error: ''' Failed to create columns:" << endl;
				cout << u8"{{{" << endl;
				cout << ex.ToString() << endl;
				cout << u8"}}}" << endl;
				continue;
			}
		}
		t.Open();
		for (auto itLog = logHandlers.begin(); itLog != logHandlers.end(); ++itLog)
		{
			LogManager::Get().RegisterLogHandler(*itLog);
		}
		cout << u8"||=Column Query Name =||=Sql Type =||= Column Size=||= Decimal Digits=||= Nullable=||= Primary Key=||" << endl;
		for (auto itCol = columns.begin(); itCol != columns.end(); ++itCol)
		{
			ColumnBufferPtrVariant pBuff = *itCol;
			SQLSMALLINT sqlType = boost::apply_visitor(sqlTypeVisitor, pBuff);
			string name = boost::apply_visitor(nameVisitor, pBuff);
			ColumnPropertiesPtr pProps = boost::apply_visitor(propsVisitor, pBuff);
			ColumnFlagsPtr pFlags = boost::apply_visitor(flagsVisitor, pBuff);
			string nullable = pFlags->Test(ColumnFlag::CF_NULLABLE) ? u8"NULLABLE" : u8"";
			string primary = pFlags->Test(ColumnFlag::CF_PRIMARY_KEY) ? u8"PRIMARY": u8"";
			cout << boost::str(boost::format(u8"|| %s || %s || %d || %d || %s || %s ||") % name % SqlType2s(sqlType) % pProps->GetColumnSize() % pProps->GetDecimalDigits() % nullable % primary) << endl;
		}

		// Print the table content, by opening it as wchar-table.
		t.Close();
		t.ClearColumns();
		// but skip _tmp tables
		if (ba::ends_with(ba::to_lower_copy(ti.GetPureName()), u8"_tmp"))
		{
			continue;
		}

		t.SetSql2BufferTypeMap(WCharSql2BufferMap::Create());
		try
		{
			columns = t.CreateAutoColumnBufferPtrs(true, true, false);
		}
		catch (const Exception& ex)
		{
			cout << u8"'''Warning: ''' Not all columns have been created successfully, will try to skip columns:" << endl;
			cout << u8"{{{" << endl;
			cout << ex.ToString() << endl;
			cout << u8"}}}" << endl;
		}
		t.Open();

		// print header
		cout << u8"===== Content =====" << endl;
		auto itCol = columns.begin();
		bool first = true;
		do
		{
			if (first && !columns.empty())
			{
				cout << u8"||= ";
				first = false;
			}
			ColumnBufferPtrVariant pBuff = *itCol;
			string name = boost::apply_visitor(nameVisitor, pBuff);
			cout << name;
			++itCol;
			if (itCol != columns.end())
			{
				cout << u8" =||= ";
			}
			else
			{
				cout << u8" =||";
			}

		} while (itCol != columns.end());
		cout << endl;

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
					cout << u8"||= ";
					first = false;
				}
				WCharColumnBufferPtr pBuff = boost::get<WCharColumnBufferPtr>(*itCol);
				if (!pBuff->IsNull())
				{
					cout << pBuff->GetString();
				}
				else
				{
					cout << u8"''NULL''";
				}
				++itCol;
				if (itCol != columns.end())
				{
					cout << u8" =||= ";
				}
				else
				{
					cout << u8" =||" << endl;
				}

			} while (itCol != columns.end());
		}
	}
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
		//string connectionString;
		//string dsn, user, pass;
		vector<SConnectionInfo> connectionInfos;
		for (int i = 0; i < argc; i++)
		{
			std::string dsnKey = u8"dsn=";
			std::string csKey = u8"cs=";
#ifdef _WIN32
			std::string arg( utf16ToUtf8(argv[i]));
#else
            std::string s1(argv[i]);

            std::string ws1 = u8"utf-16";
            std::string ws2 = u8"utf-32";
            const char16_t* a1 = u"utf-16";
            const char32_t* a2 = U"utf-32";
            
            std::string_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            
            // the UTF-8 - UTF-32 standard conversion facet
            std::string_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;
            
			std::u32string u32 = cvt.from_bytes(argv[i]);
// 			std::string ws22 = converter.from_bytes(a2);
//             std::cout << u8"Foo: " << ws2 << std::endl;
            
            std::string arg(utf8ToUtf16(argv[i]));
#endif
			std::string dsnValue;
			if (ba::starts_with(arg, dsnKey) && arg.length() > dsnKey.length())
			{
				dsnValue = arg.substr(dsnKey.length());
			}
			else if (ba::starts_with(arg, csKey) && arg.length() > csKey.length())
			{
				SConnectionInfo conInfo;
				conInfo.m_cs = arg.substr(csKey.length());
				connectionInfos.push_back(conInfo);
			}
			if (dsnValue.length() > 0)
			{
				std::vector<std::string> tokens;
				boost::split(tokens, dsnValue, boost::is_any_of(u8";"));
				if (tokens.size() != 3)
				{
					LOG_WARNING(boost::str(boost::format(u8"Ignoring Dsn entry '%s' because it does not match the form 'dsnValue;user;pass'") % arg));
				}
				else if (tokens[0].empty())
				{
					LOG_WARNING(boost::str(boost::format(u8"Ignoring Dsn entry '%s' because DSN is empty.") % arg));
				}
				else
				{
					SConnectionInfo conInfo;
					conInfo.m_dsn = tokens[0];
					conInfo.m_user = tokens[1];
					conInfo.m_pass = tokens[2];
					connectionInfos.push_back(conInfo);
				}
			}
		}

		if (connectionInfos.empty())
		{
			LOG_ERROR(u8"No connection string given and no dsn given, exiting");
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

			// Print some info
			// or maybe print tables
			bool printTables = true;
			if (!printTables)
			{
				PrintDbHeader(pDb);
				PrintDriverInfo(pDb);
				PrintDbInfo(pDb);
				printDatatypesInfo(pDb);
			}
			else
			{
				PrintDbHeader(pDb);
				PrintDriverInfo(pDb);
				printExOdbcTables(pDb);
			}

			// add some blank lines
			cout << endl;
			cout << endl;
		}

	}
	catch (const Exception& ex)
	{
		std::cerr << ex.ToString() << endl;
	}
	return 0;
}

