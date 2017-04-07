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
	wcout << boost::str(boost::wformat(L"== %s (%s) ==") % dbInfo.GetWStringProperty(DatabaseInfo::WStringProperty::DbmsName) % dbInfo.GetWStringProperty(DatabaseInfo::WStringProperty::DbmsVersion)) << endl;
}


void PrintDriverInfo(ConstDatabasePtr pDb)
{
	DatabaseInfo dbInfo = pDb->GetDbInfo();

	// print header for trac
	wcout << boost::str(boost::wformat(L"=== Driver Info ===")) << endl;
	wcout << boost::str(boost::wformat(L"* Driver Name: %s") % dbInfo.GetWStringProperty(DatabaseInfo::WStringProperty::DriverName)) << endl;
	wcout << boost::str(boost::wformat(L"* Driver Version: %s") % dbInfo.GetWStringProperty(DatabaseInfo::WStringProperty::DriverVersion)) << endl;
	wcout << boost::str(boost::wformat(L"* Driver ODBC Version: %s") % dbInfo.GetWStringProperty(DatabaseInfo::WStringProperty::DriverOdbcVersion)) << endl;
}


void PrintDbInfo(ConstDatabasePtr pDb)
{
	DatabaseInfo dbInfo = pDb->GetDbInfo();

	// And a table with all information
	wcout << L"=== Database Info ===" << endl;
	wcout << L"||=Property Name =||= Property Value =||" << endl;
	DatabaseInfo::WStringMap wstringMap = dbInfo.GetWstringMap();
	for (auto it = wstringMap.begin(); it != wstringMap.end(); ++it)
	{
		wcout << boost::str(boost::wformat(L"||%-38s ||  %s  ||") % dbInfo.GetPropertyName(it->first) % it->second) << endl;
	}
	DatabaseInfo::USmallIntMap usmallIntMap = dbInfo.GetUSmallIntMap();
	for (auto it = usmallIntMap.begin(); it != usmallIntMap.end(); ++it)
	{
		wcout << boost::str(boost::wformat(L"||%-38s || %#8x (%8d)||") % dbInfo.GetPropertyName(it->first) % it->second % it->second) << endl;
	}
	DatabaseInfo::UIntMap uintMap = dbInfo.GetUIntMap();
	for (auto it = uintMap.begin(); it != uintMap.end(); ++it)
	{
		wcout << boost::str(boost::wformat(L"||%-38s || %#8x (%8d)||") % dbInfo.GetPropertyName(it->first) % it->second % it->second) << endl;
	}
	DatabaseInfo::IntMap intMap = dbInfo.GetIntMap();
	for (auto it = intMap.begin(); it != intMap.end(); ++it)
	{
		wcout << boost::str(boost::wformat(L"||%-38s || %#8x (%8d)||") % dbInfo.GetPropertyName(it->first) % it->second %it->second) << endl;
	}
}


void printDatatypesInfo(ConstDatabasePtr pDb)
{
	wcout << L"=== Datatypes Info ===" << endl;
	SqlTypeInfosVector types = pDb->GetTypeInfos();
	bool first = true;
	for (auto it = types.begin(); it != types.end(); ++it)
	{
		SSqlTypeInfo typeInfo = *it;
		wcout << typeInfo.ToOneLineStrForTrac(first) << endl;
		first = false;
	}
}

void printExOdbcTables(ConstDatabasePtr pDb)
{
	TableInfosVector allTables = pDb->FindTables(L"", L"", L"", L"");
	struct Finder
	{
		Finder()
			: m_dbms(DatabaseProduct::UNKNOWN)
		{};

		Finder(DatabaseProduct dbms)
			: m_dbms(dbms)
		{};

		DatabaseProduct m_dbms;
		
		const set<wstring> exOdbcTables = {
			L"blobtypes", L"blobtypes_tmp",
			L"chartable", L"chartypes",	L"chartypes_tmp",
			L"datetypes", L"datetypes_tmp",
			L"floattypes", L"floattypes_tmp",
			L"integertypes", L"integertypes_tmp",
			L"multikey",
			L"numerictypes", L"numerictypes_tmp",
			L"selectonly",
			L"not_existing",
			L"not_supported",
			L"not_supported_tmp"
		};

		const set<wstring> excelTables = { L"testtable$" };

		bool operator()(const TableInfo& ti)
		{
			wstring tiName = ba::to_lower_copy(ti.GetPureName());

			if (m_dbms == DatabaseProduct::EXCEL)
			{
				return excelTables.find(tiName) != excelTables.end();
			}
			return exOdbcTables.find(tiName) != exOdbcTables.end();
		}
	};

	TableInfosVector exodbcTables;
	std::copy_if(allTables.begin(), allTables.end(), back_inserter(exodbcTables), Finder(pDb->GetDbms()));

	wcout << L"=== Tables ===" << endl;
	for (auto it = exodbcTables.begin(); it != exodbcTables.end(); ++it)
	{
		TableInfo ti = *it;
		wcout << boost::str(boost::wformat(L"==== %s ====") % ti.GetPureName()) << endl;
		wcout << L"===== Structure =====" << endl;
		wcout << boost::str(boost::wformat(L"* Catalog Name: %s") % (ti.HasCatalog() ? ti.GetCatalog() : L"<no catalog>")) << endl;
		wcout << boost::str(boost::wformat(L"* Schema Name: %s") % (ti.HasSchema() ? ti.GetSchema() : L"<no schema>")) << endl;
		wcout << boost::str(boost::wformat(L"* Table Name: %s") % ti.GetPureName()) << endl;
		wcout << boost::str(boost::wformat(L"* Query Name: %s") % ti.GetQueryName()) << endl;

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
			wcout << L"'''Warning: ''' Not all columns have been created successfully, will try to skip columns:" << endl;
			wcout << L"{{{" << endl;
			wcout << ex.ToString() << endl;
			wcout << L"}}}" << endl;
			try
			{
				columns = t.CreateAutoColumnBufferPtrs(true, true, queryPrimaryKeys);
			}
			catch (const Exception& ex)
			{
				wcout << L"'''Error: ''' Failed to create columns:" << endl;
				wcout << L"{{{" << endl;
				wcout << ex.ToString() << endl;
				wcout << L"}}}" << endl;
				continue;
			}
		}
		t.Open();
		for (auto itLog = logHandlers.begin(); itLog != logHandlers.end(); ++itLog)
		{
			LogManager::Get().RegisterLogHandler(*itLog);
		}
		wcout << L"||=Column Query Name =||=Sql Type =||= Column Size=||= Decimal Digits=||= Nullable=||= Primary Key=||" << endl;
		for (auto itCol = columns.begin(); itCol != columns.end(); ++itCol)
		{
			ColumnBufferPtrVariant pBuff = *itCol;
			SQLSMALLINT sqlType = boost::apply_visitor(sqlTypeVisitor, pBuff);
			wstring name = boost::apply_visitor(nameVisitor, pBuff);
			ColumnPropertiesPtr pProps = boost::apply_visitor(propsVisitor, pBuff);
			ColumnFlagsPtr pFlags = boost::apply_visitor(flagsVisitor, pBuff);
			wstring nullable = pFlags->Test(ColumnFlag::CF_NULLABLE) ? L"NULLABLE" : L"";
			wstring primary = pFlags->Test(ColumnFlag::CF_PRIMARY_KEY) ? L"PRIMARY": L"";
			wcout << boost::str(boost::wformat(L"|| %s || %s || %d || %d || %s || %s ||") % name % SqlType2s(sqlType) % pProps->GetColumnSize() % pProps->GetDecimalDigits() % nullable % primary) << endl;
		}

		// Print the table content, by opening it as wchar-table.
		t.Close();
		t.ClearColumns();
		// but skip _tmp tables
		if (ba::ends_with(ba::to_lower_copy(ti.GetPureName()), L"_tmp"))
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
			wcout << L"'''Warning: ''' Not all columns have been created successfully, will try to skip columns:" << endl;
			wcout << L"{{{" << endl;
			wcout << ex.ToString() << endl;
			wcout << L"}}}" << endl;
		}
		t.Open();

		// print header
		wcout << L"===== Content =====" << endl;
		auto itCol = columns.begin();
		bool first = true;
		do
		{
			if (first && !columns.empty())
			{
				wcout << L"||= ";
				first = false;
			}
			ColumnBufferPtrVariant pBuff = *itCol;
			wstring name = boost::apply_visitor(nameVisitor, pBuff);
			wcout << name;
			++itCol;
			if (itCol != columns.end())
			{
				wcout << L" =||= ";
			}
			else
			{
				wcout << L" =||";
			}

		} while (itCol != columns.end());
		wcout << endl;

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
					wcout << L"||= ";
					first = false;
				}
				WCharColumnBufferPtr pBuff = boost::get<WCharColumnBufferPtr>(*itCol);
				if (!pBuff->IsNull())
				{
					wcout << pBuff->GetWString();
				}
				else
				{
					wcout << L"''NULL''";
				}
				++itCol;
				if (itCol != columns.end())
				{
					wcout << L" =||= ";
				}
				else
				{
					wcout << L" =||" << endl;
				}

			} while (itCol != columns.end());
		}
	}
}

struct SConnectionInfo
{
	wstring m_dsn;
	wstring m_user;
	wstring m_pass;
	wstring m_cs;
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
		//wstring connectionString;
		//wstring dsn, user, pass;
		vector<SConnectionInfo> connectionInfos;
		for (int i = 0; i < argc; i++)
		{
			std::wstring dsnKey = L"dsn=";
			std::wstring csKey = L"cs=";
#ifdef _WIN32
			std::wstring arg(argv[i]);
#else
            std::string s1(argv[i]);

            std::wstring ws1 = L"utf-16";
            std::wstring ws2 = L"utf-32";
            const char16_t* a1 = u"utf-16";
            const char32_t* a2 = U"utf-32";
            
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            
            // the UTF-8 - UTF-32 standard conversion facet
            std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;
            
			std::u32string u32 = cvt.from_bytes(argv[i]);
// 			std::wstring ws22 = converter.from_bytes(a2);
//             std::wcout << L"Foo: " << ws2 << std::endl;
            
            std::wstring arg(utf8ToUtf16(argv[i]));
#endif
			std::wstring dsnValue;
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
				std::vector<std::wstring> tokens;
				boost::split(tokens, dsnValue, boost::is_any_of(L";"));
				if (tokens.size() != 3)
				{
					LOG_WARNING(boost::str(boost::wformat(L"Ignoring Dsn entry '%s' because it does not match the form 'dsnValue;user;pass'") % arg));
				}
				else if (tokens[0].empty())
				{
					LOG_WARNING(boost::str(boost::wformat(L"Ignoring Dsn entry '%s' because DSN is empty.") % arg));
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
			LOG_ERROR(L"No connection string given and no dsn given, exiting");
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
			wcout << endl;
			wcout << endl;
		}

	}
	catch (const Exception& ex)
	{
		std::wcerr << ex.ToString() << endl;
	}
	return 0;
}
