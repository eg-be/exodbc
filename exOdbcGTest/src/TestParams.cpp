/*!
 * \file TestParams.cpp
 * \author Elias Gerber <eg@elisium.ch>
 * \date 22.07.2014
 * \copyright GNU Lesser General Public License Version 3
 * 
 * [Brief CPP-file description]
 */ 

#include "stdafx.h"

// Own header
#include "TestParams.h"

// Same component headers
// Other headers

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
	::std::ostream& operator<<(::std::ostream& os, const TestParams& oi) {
		std::wstringstream wos;
		if (oi.HasConnectionString())
		{
			wos << L"Connection String: " << oi.m_connectionString;
		}
		else
		{
			wos << L"DSN: " << oi.m_dsn
				<< L"; Username: " << oi.m_username
				<< L"; Password: " << oi.m_password;
		}
		wos << L"; Names: " << (oi.m_namesCase == test::Case::LOWER ? L"lowercase" : L"uppercase");
		std::string s;

		// \todo Resolve with ticket #44 #53 - ugly conversion (but okay here, we know its only ascii)
		std::wstring ws = wos.str();
		std::stringstream ss;
		for(size_t i = 0; i < ws.length(); i++)
		{
			char c = (char) ws[i];
			ss << c;
		}
		s = ss.str();

		return os << s.c_str();
	}


	::std::wostream& operator<<(::std::wostream& wos, const TestParams& oi) {
		if (oi.HasConnectionString())
		{
			wos << L"Connection String: " << oi.m_connectionString;
		}
		else
		{
			wos << L"DSN: " << oi.m_dsn
				<< L"; Username: " << oi.m_username
				<< L"; Password: " << oi.m_password;
		}
		wos << L"; Names: " << (oi.m_namesCase == test::Case::LOWER ? L"lowercase" : L"uppercase");
		return wos;
	}


	void TestParams::Load(const boost::filesystem::wpath& settingsFile)
	{
		namespace pt = boost::property_tree;
		namespace io = boost::iostreams;
		namespace fs = boost::filesystem;
		namespace ba = boost::algorithm;

		try
		{
			pt::wptree tree;

			// Parse the XML into the property tree.
			pt::read_xml(settingsFile.string(), tree);

			for (const pt::wptree::value_type &v : tree.get_child(L"TestSettings"))
			{
				wstring elementName = v.first.data();
				pt::wptree subTree = v.second;
				if (elementName == L"Dsn")
				{
					bool disabled = subTree.get<bool>(L"Disabled");
					if (!disabled)
					{
						m_dsn = subTree.get<wstring>(L"Name");
						m_username = subTree.get<wstring>(L"User");
						m_password = subTree.get<wstring>(L"Pass");
					}
				}
				if (elementName == L"ConnectionString")
				{
					bool disabled = subTree.get<bool>(L"Disabled");
					if (!disabled)
					{
						m_connectionString = subTree.get<wstring>(L"Value");
					}
				}
				if (elementName == L"ConnectionString" || elementName == L"Dsn")
				{
					bool disabled = subTree.get<bool>(L"Disabled");
					if (!disabled)
					{
						wstring namesCase = subTree.get<wstring>(L"Case");
						if (ba::iequals(namesCase, L"u") || ba::iequals(namesCase, L"upper"))
						{
							m_namesCase = test::Case::UPPER;
						}
						else if (ba::iequals(namesCase, L"l") || ba::iequals(namesCase, L"lower"))
						{
							m_namesCase = test::Case::LOWER;
						}
						else
						{
							wstringstream ws;
							ws << L"TestSettings.Case must be either 'upper', 'u', 'lower' or 'l', but it is " << namesCase;
							IllegalArgumentException ae(ws.str());
							SET_EXCEPTION_SOURCE(ae);
							throw ae;
						}
					}
				}
			}

			if (!IsUsable())
			{
				LOG_WARNING(L"DSN and ConnectionString are both disabled");
			}

			m_createDb = tree.get<bool>(L"TestSettings.CreateDb");
			
			wstring logLevel = tree.get<wstring>(L"TestSettings.LogLevel");
			if (ba::iequals(logLevel, L"T") || ba::iequals(logLevel, L"Trace"))
			{
				m_logSeverity = boost::log::trivial::trace;
			}
			else if (ba::iequals(logLevel, L"D") || ba::iequals(logLevel, L"Debug"))
			{
				m_logSeverity = boost::log::trivial::debug;
			}
			else if (ba::iequals(logLevel, L"I") || ba::iequals(logLevel, L"Info"))
			{
				m_logSeverity = boost::log::trivial::info;
			}
			else if (ba::iequals(logLevel, L"E") || ba::iequals(logLevel, L"Error"))
			{
				m_logSeverity = boost::log::trivial::error;
			}
			else if (ba::iequals(logLevel, L"F") || ba::iequals(logLevel, L"Fatal"))
			{
				m_logSeverity = boost::log::trivial::fatal;
			}
			else
			{
				LOG_WARNING(L"No valid log level specified, falling back to Info");
				m_logSeverity = boost::log::trivial::info;
			}
		}
		catch (const std::exception& ex)
		{
			WrapperException we(ex);
			SET_EXCEPTION_SOURCE(we);
			throw we;
		}
	}


	// Interfaces
	// ----------
	
} // namespace exodbc